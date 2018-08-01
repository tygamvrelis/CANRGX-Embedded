/**
 * @file App_MPU9250.c
 * @author Tyler
 * @brief Application code utilizing the MPU9250
 *
 * @defgroup MPU9250_App MPU9250 Application Code
 * @{
 */




/********************************** Includes *********************************/
#include "App/App_MPU9250.h"




/***************************** Extern declarations ***************************/
extern osSemaphoreId semMPU9250Handle;
extern osThreadId ControlTaskHandle;




/******************************* Public variables ****************************/
MPU9250_t myMPU9250; /**< Global MPU9250 object */




/***************************** Public Functions ******************************/
/**
 * @defgroup MPU9250_App_Public_Functions MPU9250 Application Public Functions
 * @ingroup MPU9250_App
 * @{
 */

/**
 * @brief   Updates the acceleration data container with fresh data
 * @details Each axis of accelerometer data is digitally filtered
 * @param   accelData Pointer to the acceleration data container
 * @param   myMPU9250 Pointer to the data structure which stores the data read
 *          from the MPU9250 sensor
 */
void updateAccelReading(
        accelerometerData_t* accelData,
        MPU9250_t* myMPU9250
)
{
    // Acceleration - Read ax, ay, az
    uint32_t accelStatus = accelReadDMA(myMPU9250, semMPU9250Handle);
    if(accelStatus == 1){
        filterAccelMPU9250(myMPU9250);
        computeTotalAcceleration(myMPU9250);
    }
    else{
        // The accelerometer was not able to be read from properly,
        // handle this here
        generateClocks(&hi2c3, 1, 1);
        myMPU9250->A = NAN;
    }
    accelData->ax = myMPU9250->ax;
    accelData->ay = myMPU9250->ay;
    accelData->az = myMPU9250->az;
}

/**
 * @brief Updates the magnetic flux density data container with fresh data
 * @param magData Pointer to the magnetic flux density data container
 * @param myMPU9250 Pointer to the data structure which stores the data read
 *        from the MPU9250 sensor
 */
void updateMagReading(
        magnetometerData_t* magData,
        MPU9250_t* myMPU9250
)
{
    // Magnetometer - Read hx, hy, hz
    uint32_t magStatus = magFluxReadDMA(myMPU9250, semMPU9250Handle);
    if(magStatus != 1){
        // The magnetometer was not able to be read from properly,
        // handle this here
        generateClocks(&hi2c1, 1, 1);
    }
    magData->hx = myMPU9250->hx;
    magData->hy = myMPU9250->hy;
    magData->hz = myMPU9250->hz;
}

/**
 * @brief Based on the latest accelerometer readings, this function determines
 *        which state we are in: reduced gravity, or none. If the state is
 *        different than the last time this thread ran its cycle, an event is
 *        generated which notifies the control thread (to start/stop
 *        experiments).
 * @param myMPU9250 Pointer to the data structure which stores the data read
 *        from the MPU9250 sensor
 */
void MPU9250EventHandler(MPU9250_t* myMPU9250){
    /* Initial state is sensing no event, and no command to transmit */
    static enum flightEvents_e sensedEvent = NONE;

    // Use the acceleration magnitude to update state
//    if(myMPU9250->az < 0.981 && sensedEvent == NONE){ // can use this line for testing
    if(myMPU9250->A < 0.981 && sensedEvent == NONE){
        sensedEvent = REDUCEDGRAVITY;

        // Notify task to start experiment
        xTaskNotify(ControlTaskHandle,
                    NOTIFY_FROM_MPU(sensedEvent),
                    eSetBits
        );
    }
//    else if(myMPU9250->az > 3.13 && sensedEvent == REDUCEDGRAVITY){ // can use this line for testing
    else if(myMPU9250->A > 3.13 && sensedEvent == REDUCEDGRAVITY){
        sensedEvent = NONE;

        // Notify task to stop experiment
        xTaskNotify(ControlTaskHandle,
                    NOTIFY_FROM_MPU(sensedEvent),
                    eSetBits
        );
    }
}

/**
 * @}
 */
/* end - MPU9250_App_Public_Functions */

/**
 * @}
 */
/* end - MPU9250_App */

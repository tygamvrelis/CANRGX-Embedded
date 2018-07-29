/*
 * App_MPU9250.c
 *
 *  Created on: Jul 28, 2018
 *      Author: Tyler
 */




/********************************** Includes *********************************/
#include "App/App_MPU9250.h"




/***************************** Extern declarations ***************************/
extern osSemaphoreId semMPU9250Handle;
extern osThreadId ControlTaskHandle;




/***************************** Public Functions ******************************/
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

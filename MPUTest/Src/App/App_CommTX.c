/**
 * @file App_CommTX.c
 * @author Tyler
 * @brief Functions and data related to the transmission side of PC
 *        communication
 *
 * @defgroup Communication Communication
 * @brief PC-MCU communication
 *
 * @defgroup CommTX TX
 * @brief Handles sending packets to the PC for logging
 * @ingroup Communication
 * @{
 */

/********************************** Includes *********************************/
#include "App/App_CommTX.h"




/***************************** Extern declarations ***************************/
extern osSemaphoreId semTxHandle;
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim10;




/***************************** Private Variables *****************************/
/**
 * @brief Buffers sensor data to send to PC. The contents of this buffer are
 *        sent to the PC every 3 ms.
 */
static uint8_t buffer[50] = {0};

// Addresses in buffer for each datum
static uint8_t* tickStart = &buffer[2];  /**< OS tick timestamp              */
static uint8_t* accelX = &buffer[6];     /**< X-axis acceleration            */
static uint8_t* accelY = &buffer[10];    /**< Y-axis acceleration            */
static uint8_t* accelZ = &buffer[14];    /**< Z-axis acceleration            */
static uint8_t* magX = &buffer[18];      /**< X-axis magnetic flux density   */
static uint8_t* magY = &buffer[22];      /**< Y-axis magnetic flux density   */
static uint8_t* magZ = &buffer[26];      /**< Z-axis magnetic flux density   */
static uint8_t* mag1Power = &buffer[30]; /**< Mag 1 control signal amplitude */
static uint8_t* mag2Power = &buffer[32]; /**< Mag 2 control signal amplitude */
static uint8_t* tec1Power = &buffer[34]; /**< TEC 1 control signal amplitude */
static uint8_t* tec2Power = &buffer[36]; /**< TEC 2 control signal amplitude */
static uint8_t* temp1a = &buffer[38];    /**< Temp sensor 1A                 */
static uint8_t* temp1b = &buffer[40];    /**< Temp sensor 1B                 */
static uint8_t* temp2a = &buffer[42];    /**< Temp sensor 2A                 */
static uint8_t* temp2b = &buffer[44];    /**< Temp sensor 2B                 */
static uint8_t* temp3a = &buffer[46];    /**< Temp sensor 3A                 */
static uint8_t* temp3b = &buffer[48];    /**< Temp sensor 3B                 */

/**
 * @brief   Used to track which sensors have fresh data in the buffer
 * @details When a bit is 1, it indicates fresh data has been received from its
 *          corresponding sensor. All the bits are cleared once a transmission
 *          is initiated. The bit-to-sensor mapping is as follows:
 *  - Bit 0: accelerometer data
 *  - Bit 1: magnetometer data
 *  - Bit 2: control signal data
 *  - Bit 3: temperature sensor data
 */
static uint8_t taskFlags = 0x00;

/**
 * @brief Pointer in which received accelerometer data container addresses are
 *        stored
 */
static accelerometerData_t* accelerometerDataPtr = NULL;

/**
 * @brief Pointer in which received magnetometer data container addresses are
 *        stored
 */
static magnetometerData_t* magnetometerDataPtr = NULL;

/**
 * @brief Pointer in which received control signal data container addresses are
 *        stored
 */
static controlData_t* controlDataPtr = NULL;

/**
 * @brief Pointer in which received temperature data container addresses are
 *        stored
 */
static temperatureData_t* temperatureDataPtr = NULL;




/***************************** Public Functions ******************************/
/**
 * @defgroup CommTXPublicFunctions Public functions
 * @brief @brief Functions used externally
 * @ingroup CommTX
 * @{
 */

/**
 * @brief Initializes the buffer contents by writing in the start sequence
 */
void commTXInit(void){
    // Dummy bits to indicate packet start
    buffer[0] = 0xFF;
    buffer[1] = 0xFF;
}

/**
 * @brief When a sensor thread sends data to the queue xTXDataQueue, this
 *        function is executed to interpret the data based on the sender and
 *        pack it into the appropriate place in the buffer
 * @param receivedData Pointer to the TXData_t structure received from the
 *        queue
 */
void commTXEventHandler(TXData_t* receivedData){
    switch(receivedData->type){
        case accelerometer_t:
            accelerometerDataPtr = (accelerometerData_t*)receivedData->data;

            if(accelerometerDataPtr == NULL){break;}

            // Update task flags to indicate accelerometer data has been
            // received
            taskFlags = taskFlags | 0b00000001;

            // Copy data to buffer
            memcpy(accelX, &(accelerometerDataPtr -> ax), sizeof(float));
            memcpy(accelY, &(accelerometerDataPtr -> ay), sizeof(float));
            memcpy(accelZ, &(accelerometerDataPtr -> az), sizeof(float));

            break;
        case magnetometer_t:
            magnetometerDataPtr = (magnetometerData_t*)receivedData->data;

            if(magnetometerDataPtr == NULL){break;}

            // Update task flags to indicate magnetometer data has been
            // received
            taskFlags = taskFlags | 0b00000010;

            memcpy(magX, &(magnetometerDataPtr -> hx), sizeof(float));
            memcpy(magY, &(magnetometerDataPtr -> hy), sizeof(float));
            memcpy(magZ, &(magnetometerDataPtr -> hz), sizeof(float));

            break;
        case control_t:
            controlDataPtr = (controlData_t*)receivedData->data;

            if(controlDataPtr == NULL){break;}

            // Update task flags to indicate control task has been received from
            taskFlags = taskFlags | 0b00000100;

            // Copy data to buffer
            int16_t mag1data = controlDataPtr -> mag1Power;
            int16_t mag2data = controlDataPtr -> mag2Power;
            uint16_t tec1data = controlDataPtr -> tec1Power;
            uint16_t tec2data = controlDataPtr -> tec2Power;
            memcpy(mag1Power, &mag1data, sizeof(int16_t));
            memcpy(mag2Power, &mag2data, sizeof(int16_t));
            memcpy(tec1Power, &tec1data, sizeof(uint16_t));
            memcpy(tec2Power, &tec2data, sizeof(uint16_t));

            break;
        case temperature_t:
            temperatureDataPtr = (temperatureData_t*)receivedData->data;

            if(temperatureDataPtr == NULL){break;}

            // Update task flags to indicate temperature data has been
            // received
            taskFlags = taskFlags | 0b00001000;

            // Copy data to buffer
            uint16_t temp1adata = temperatureDataPtr -> temp1a;
            uint16_t temp1bdata = temperatureDataPtr -> temp1b;
            uint16_t temp2adata = temperatureDataPtr -> temp2a;
            uint16_t temp2bdata = temperatureDataPtr -> temp2b;
            uint16_t temp3adata = temperatureDataPtr -> temp3a;
            uint16_t temp3bdata = temperatureDataPtr -> temp3b;
            memcpy(temp1a, &temp1adata, sizeof(uint16_t));
            memcpy(temp1b, &temp1bdata, sizeof(uint16_t));
            memcpy(temp2a, &temp2adata, sizeof(uint16_t));
            memcpy(temp2b, &temp2bdata, sizeof(uint16_t));
            memcpy(temp3a, &temp3adata, sizeof(uint16_t));
            memcpy(temp3b, &temp3bdata, sizeof(uint16_t));

            break;
        default:
            break;
    }
}

/**
 * @brief   Initiates an asynchronous packet transmission after 3 ms have
 *          passed since the last time
 * @details When this function is called, it first waits until 3 ms have passed
 *          since the last time a packet transmission was initiated. Then, it
 *          clears the task flags and gets the current OS tick. The OS tick is
 *          loaded into the packet and stored in the cycleStartTick variable,
 *          hence indicating that the next cycle has started. These steps
 *          ensure deterministic 3 ms timing. An 87 microsecond wait occurs (to
 *          (guarantee an interpacket delay of about 2 bytes), then finally the
 *          asynchronous transfer is initiated. The TX thread is then blocked
 *          until the callback occurs or 3 ms have elapsed (timeout).
 * @param   lastWakeTime Pointer to the OS tick at which vTaskDelayUntil was
 *          last called
 * @param   cycleStartTick Pointer to the OS tick at which the current
 *          transmission cycle began
 */
void commTXSendPacket(TickType_t* lastWakeTime, TickType_t* cycleStartTick){
    // Send packets every 3 milliseconds. At a symbol rate of 230400, it takes 2.17 ms
    // to send 50 bytes. To ease the data analysis process, this has been rounded up to
    // 3 ms so that it will be easier to "line up" data with the FreeRTOS tick.
    vTaskDelayUntil(lastWakeTime, pdMS_TO_TICKS(3));

    // Clear task flags
    taskFlags = 0x00;

    // Get the tick at which we are starting the next cycle, for timeout
    // purposes
    *cycleStartTick = xTaskGetTickCount();

    // Pack the tick at which we are sending this packet
    TickType_t curTick = xTaskGetTickCount();
    memcpy(tickStart, &curTick, sizeof(TickType_t));

    // Block for 87 microseconds, the time for 2 bytes to be transmitted
    // via UART. This idle time guarantees interpacket delay, thus
    // reducing the probability of a framing error. Time out after 1 ms.
    HAL_TIM_Base_Start_IT(&htim10);
    xTaskNotifyWait(UINT32_MAX, UINT32_MAX, NULL, 1);

    // Transmit
    HAL_UART_Transmit_DMA(&huart2, buffer, sizeof(buffer));

    // Wait until transmit is done or timeout
    xSemaphoreTake(semTxHandle, pdMS_TO_TICKS(3));
}

/**
 * @return true if control data, accelerometer data, and magnetometer data
 *         have been added to the packet, otherwise false
 */
bool isControlAndMPUDataCollected(){
    return (taskFlags & 0b00000111) == 0b00000111;
}

/**
 * @param  cycleStartTick The OS tick at which the current transmission cycle
 *         began
 * @return true if 3 ms or greater have elapsed since the current transmission
 *         cycle began
 */
bool hasTimeoutElapsed(TickType_t cycleStartTick){
    return xTaskGetTickCount() - cycleStartTick >= pdMS_TO_TICKS(3);
}

/**
 * @}
 */
/* end - CommTXPublicFunctions */

/**
 * @}
 */
/* end - CommTX */

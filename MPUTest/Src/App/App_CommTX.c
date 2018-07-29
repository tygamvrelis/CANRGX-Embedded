/*
 * App_CommTX.c
 *
 *  Created on: Jul 29, 2018
 *      Author: Tyler
 */

/********************************** Includes *********************************/
#include "App/App_CommTX.h"




/***************************** Extern declarations ***************************/
extern osSemaphoreId semTxHandle;
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim10;




/***************************** Private Variables *****************************/
// Buffers sensor data to send to PC
static uint8_t buffer[50] = {0};

// Addresses in buffer for each datum
static uint8_t* tickStart = &buffer[2];
static uint8_t* accelX = &buffer[6];
static uint8_t* accelY = &buffer[10];
static uint8_t* accelZ = &buffer[14];
static uint8_t* magX = &buffer[18];
static uint8_t* magY = &buffer[22];
static uint8_t* magZ = &buffer[26];
static uint8_t* mag1Power = &buffer[30];
static uint8_t* mag2Power = &buffer[32];
static uint8_t* tec1Power = &buffer[34];
static uint8_t* tec2Power = &buffer[36];
static uint8_t* temp1a = &buffer[38];
static uint8_t* temp1b = &buffer[40];
static uint8_t* temp2a = &buffer[42];
static uint8_t* temp2b = &buffer[44];
static uint8_t* temp3a = &buffer[46];
static uint8_t* temp3b = &buffer[48];

uint8_t taskFlags = 0x00; // Used to track which tasks have fresh data

static accelerometerData_t* accelerometerDataPtr = NULL;
static magnetometerData_t* magnetometerDataPtr = NULL;
static controlData_t* controlDataPtr = NULL;
static temperatureData_t* temperatureDataPtr = NULL;




/***************************** Public Functions ******************************/
void commTXInit(void){
    // Dummy bits to indicate packet start
    buffer[0] = 0xFF;
    buffer[1] = 0xFF;
}

void commTXEventHandler(TXData_t* receivedData){
    switch(receivedData->type){
        case accelerometer_t:
            accelerometerDataPtr = (accelerometerData_t*)receivedData->data;

            if(accelerometerDataPtr == NULL){break;}

            // Update task flags to indicate MPU task has been received from
            taskFlags = taskFlags | 0b00000001;

            // Copy data to buffer
            memcpy(accelX, &(accelerometerDataPtr -> ax), sizeof(float));
            memcpy(accelY, &(accelerometerDataPtr -> ay), sizeof(float));
            memcpy(accelZ, &(accelerometerDataPtr -> az), sizeof(float));

            break;
        case magnetometer_t:
            magnetometerDataPtr = (magnetometerData_t*)receivedData->data;

            if(magnetometerDataPtr == NULL){break;}

            // Update task flags to indicate MPU task has been received from
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

            // Update task flags to indicate temperature task has been received from
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

    // Wait until transmit is done
    xSemaphoreTake(semTxHandle, portMAX_DELAY);
}

bool isControlAndMPUDataCollected(){
    return (taskFlags & 0b00000111) == 0b00000111;
}

bool hasTimeoutElapsed(TickType_t cycleStartTick){
    return xTaskGetTickCount() - cycleStartTick >= pdMS_TO_TICKS(3);
}


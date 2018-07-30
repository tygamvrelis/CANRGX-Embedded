/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "App/App_Control.h"
#include "App/App_CommTX.h"
#include "App/App_MPU9250.h"
#include "App/App_CommRX.h"
#include "App/App_Temperature.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
uint32_t defaultTaskBuffer[ 1024 ];
osStaticThreadDef_t defaultTaskControlBlock;
osThreadId ControlTaskHandle;
uint32_t ControlTaskBuffer[ 512 ];
osStaticThreadDef_t ControlTaskControlBlock;
osThreadId TxTaskHandle;
uint32_t DataLogTaskBuffer[ 512 ];
osStaticThreadDef_t DataLogTaskControlBlock;
osThreadId MPU9250TaskHandle;
uint32_t MPU9250TaskBuffer[ 1024 ];
osStaticThreadDef_t MPU9250TaskControlBlock;
osThreadId RxTaskHandle;
uint32_t rxTaskBuffer[ 512 ];
osStaticThreadDef_t rxTaskControlBlock;
osThreadId TempTaskHandle;
uint32_t TempTaskBuffer[ 512 ];
osStaticThreadDef_t TempTaskControlBlock;
osMessageQId xTXDataQueueHandle;
uint8_t xTXDataQueueBuffer[ 4 * sizeof( TXData_t ) ];
osStaticMessageQDef_t xTXDataQueueControlBlock;
osTimerId tmrLEDBlinkHandle;
osStaticTimerDef_t tmrLEDBlinkControlBlock;
osSemaphoreId semMPU9250Handle;
osStaticSemaphoreDef_t semMPU9250ControlBlock;
osSemaphoreId semTxHandle;
osStaticSemaphoreDef_t semTxControlBlock;
osSemaphoreId semRxHandle;
osStaticSemaphoreDef_t semRxControlBlock;

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartControlTask(void const * argument);
void StartTxTask(void const * argument);
void StartMPU9250Task(void const * argument);
void StartRxTask(void const * argument);
void StartTempTask(void const * argument);
void tmrLEDCallback(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}

/**
 * @brief Toggles the green LED, LD2
 * @note  Useful for debugging
 */
inline void LED(){
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
}
/* USER CODE END 1 */

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];
  
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )  
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}                   
/* USER CODE END GET_TIMER_TASK_MEMORY */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of semMPU9250 */
  osSemaphoreStaticDef(semMPU9250, &semMPU9250ControlBlock);
  semMPU9250Handle = osSemaphoreCreate(osSemaphore(semMPU9250), 1);

  /* definition and creation of semTx */
  osSemaphoreStaticDef(semTx, &semTxControlBlock);
  semTxHandle = osSemaphoreCreate(osSemaphore(semTx), 1);

  /* definition and creation of semRx */
  osSemaphoreStaticDef(semRx, &semRxControlBlock);
  semRxHandle = osSemaphoreCreate(osSemaphore(semRx), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* definition and creation of tmrLEDBlink */
  osTimerStaticDef(tmrLEDBlink, tmrLEDCallback, &tmrLEDBlinkControlBlock);
  tmrLEDBlinkHandle = osTimerCreate(osTimer(tmrLEDBlink), osTimerPeriodic, NULL);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  osTimerStart(tmrLEDBlinkHandle, 1000); // Start timer to blink status LED
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadStaticDef(defaultTask, StartDefaultTask, osPriorityIdle, 0, 1024, defaultTaskBuffer, &defaultTaskControlBlock);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of ControlTask */
  osThreadStaticDef(ControlTask, StartControlTask, osPriorityNormal, 0, 512, ControlTaskBuffer, &ControlTaskControlBlock);
  ControlTaskHandle = osThreadCreate(osThread(ControlTask), NULL);

  /* definition and creation of TxTask */
  osThreadStaticDef(TxTask, StartTxTask, osPriorityHigh, 0, 512, DataLogTaskBuffer, &DataLogTaskControlBlock);
  TxTaskHandle = osThreadCreate(osThread(TxTask), NULL);

  /* definition and creation of MPU9250Task */
  osThreadStaticDef(MPU9250Task, StartMPU9250Task, osPriorityNormal, 0, 1024, MPU9250TaskBuffer, &MPU9250TaskControlBlock);
  MPU9250TaskHandle = osThreadCreate(osThread(MPU9250Task), NULL);

  /* definition and creation of RxTask */
  osThreadStaticDef(RxTask, StartRxTask, osPriorityRealtime, 0, 512, rxTaskBuffer, &rxTaskControlBlock);
  RxTaskHandle = osThreadCreate(osThread(RxTask), NULL);

  /* definition and creation of TempTask */
  osThreadStaticDef(TempTask, StartTempTask, osPriorityNormal, 0, 512, TempTaskBuffer, &TempTaskControlBlock);
  TempTaskHandle = osThreadCreate(osThread(TempTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of xTXDataQueue */
  osMessageQStaticDef(xTXDataQueue, 4, TXData_t, xTXDataQueueBuffer, &xTXDataQueueControlBlock);
  xTXDataQueueHandle = osMessageCreate(osMessageQ(xTXDataQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
    for(;;){

    }
  /* USER CODE END StartDefaultTask */
}

/* StartControlTask function */
void StartControlTask(void const * argument)
{
  /* USER CODE BEGIN StartControlTask */
    TXData_t txDataControl;
    controlData_t controlData = { 0 };

    txDataControl.type = control_t;
    txDataControl.data = &controlData;

    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    uint32_t notification;

    controlInit();

    for(;;){
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CONTROL_CYCLE_MS)); // Service this task every CONTROL_CYCLE_MS milliseconds

        // Check for flight events
        if(xTaskNotifyWait(0, UINT32_MAX, &notification,
                pdMS_TO_TICKS(1)) == pdTRUE){
            controlEventHandler(notification);
        }

        // Update PWM duty cycle for magnets
        updateControlSignals();

        // Tell transmit task that new data is ready
        updateControlData(&controlData);
        xQueueSend(xTXDataQueueHandle, &txDataControl, 1);
    }
  /* USER CODE END StartControlTask */
}

/* StartTxTask function */
void StartTxTask(void const * argument)
{
  /* USER CODE BEGIN StartTxTask */
    // For intertask communication
    TXData_t receivedData;

    // For packet timing management
    BaseType_t status;
    TickType_t xLastWakeTime, cycleStartTick;
    xLastWakeTime = xTaskGetTickCount();
    cycleStartTick = xTaskGetTickCount();

    for(;;){
        // Trigger this thread every 1 ms. This way, we are guaranteed to meet
        // the 3 ms deadline
        vTaskDelay(pdMS_TO_TICKS(1));

        // Wait for something to transmit
        do{
            status = xQueueReceive(xTXDataQueueHandle, &receivedData, 0);

            if(status == pdTRUE){
                commTXEventHandler(&receivedData);
            }
        }while (status == pdTRUE);

        // This runs when:
        //    1. the control & MPU9250 threads have responded
        //    2. the timeout has elapsed
        if(isControlAndMPUDataCollected() || hasTimeoutElapsed(cycleStartTick)){
            commTXSendPacket(&xLastWakeTime, &cycleStartTick);
        }
    }
  /* USER CODE END StartTxTask */
}

/* StartMPU9250Task function */
void StartMPU9250Task(void const * argument)
{
  /* USER CODE BEGIN StartMPU9250Task */
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    TXData_t txDataAccel, txDataMag;
    magnetometerData_t magnetometerData;
    accelerometerData_t accelerometerData;

    txDataAccel.type = accelerometer_t;
    txDataAccel.data = &accelerometerData;

    txDataMag.type = magnetometer_t;
    txDataMag.data = &magnetometerData;

    initAllMPU9250Filters();

    for(;;){
        // Service this task every MPU9250_CYCLE_MS milliseconds
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MPU9250_CYCLE_MS));

        // Read from sensor
        updateAccelReading(&accelerometerData, &myMPU9250);
        updateMagReading(&magnetometerData, &myMPU9250);

        // Tell transmit task that new data is ready
        xQueueSend(xTXDataQueueHandle, &txDataAccel, 1);
        xQueueSend(xTXDataQueueHandle, &txDataMag, 1);

        // Send updates to control task if there is an event
        MPU9250EventHandler(&myMPU9250);
    }
  /* USER CODE END StartMPU9250Task */
}

/* StartRxTask function */
void StartRxTask(void const * argument)
{
  /* USER CODE BEGIN StartRxTask */
    for(;;){
        commRXInitReception();

        if(xSemaphoreTake(semRxHandle, pdMS_TO_TICKS(30)) == pdTRUE){
            commRXEventHandler();
        }
        else{
            commRXCancelReception();
        }
    }
  /* USER CODE END StartRxTask */
}

/* StartTempTask function */
void StartTempTask(void const * argument)
{
  /* USER CODE BEGIN StartTempTask */
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    TXData_t txDataTemperature;
    temperatureData_t temperatureData = { 0 };

    txDataTemperature.type = temperature_t;
    txDataTemperature.data = &temperatureData;

    Temp_Scan_Start();

    for(;;){
        // Service this task every TEMP_CYCLE_MS milliseconds
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TEMP_CYCLE_MS));

        updateTemperatureData(&temperatureData);

        xQueueSend(xTXDataQueueHandle, &txDataTemperature, 1);
    }
  /* USER CODE END StartTempTask */
}

/* tmrLEDCallback function */
void tmrLEDCallback(void const * argument)
{
  /* USER CODE BEGIN tmrLEDCallback */
    LED();
  /* USER CODE END tmrLEDCallback */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

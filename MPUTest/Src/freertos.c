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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "MPU9250.h"
#include "pid_contrl.h"


/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
uint32_t defaultTaskBuffer[ 1024 ];
osStaticThreadDef_t defaultTaskControlBlock;
osThreadId ControlTaskHandle;
uint32_t ControlTaskBuffer[ 256 ];
osStaticThreadDef_t ControlTaskControlBlock;
osThreadId TxTaskHandle;
uint32_t DataLogTaskBuffer[ 512 ];
osStaticThreadDef_t DataLogTaskControlBlock;
osThreadId MPU9250TaskHandle;
uint32_t MPU9250TaskBuffer[ 256 ];
osStaticThreadDef_t MPU9250TaskControlBlock;
osThreadId RxTaskHandle;
uint32_t rxTaskBuffer[ 512 ];
osStaticThreadDef_t rxTaskControlBlock;
osThreadId TempTaskHandle;
uint32_t TempTaskBuffer[ 256 ];
osStaticThreadDef_t TempTaskControlBlock;
osMessageQId xMPUToTXQueueHandle;
uint8_t xMPUToTXQueueBuffer[ 1 * sizeof( uint32_t ) ];
osStaticMessageQDef_t xMPUToTXQueueControlBlock;
osMessageQId xControlToTXQueueHandle;
uint8_t xControlToTXQueueBuffer[ 1 * sizeof( uint32_t ) ];
osStaticMessageQDef_t xControlToTXQueueControlBlock;
osMessageQId xControlCommandQueueHandle;
uint8_t xControlCommandQueueBuffer[ 2 * sizeof( uint32_t ) ];
osStaticMessageQDef_t xControlCommandQueueControlBlock;
osTimerId tmrLEDBlinkHandle;
osStaticTimerDef_t tmrLEDBlinkControlBlock;
osSemaphoreId semMPU9250Handle;
osStaticSemaphoreDef_t semMPU9250ControlBlock;
osSemaphoreId semTxHandle;
osStaticSemaphoreDef_t semTxControlBlock;
osSemaphoreId semRxHandle;
osStaticSemaphoreDef_t semRxControlBlock;

/* USER CODE BEGIN Variables */
//#define ADC_DATA_N 12
//volatile uint16_t uhADC_results[ADC_DATA_N];

QueueSetHandle_t xTxQueueSet;

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

// Make sure that queue sets are enabled (no way to do this via cube)
#if (configUSE_QUEUE_SETS == 0)
	#error *** HEY! Tyler here. Make configUSE_QUEUE_SETS in FreeRTOS.h equal to 1 ***
#endif

// LED blink for debugging (green LED, LD2)
#define LED() HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5)

// All flight events besides none are transitions
enum flightEvents_e{
	NONE,
	REDUCEDGRAVITY,
	PULLUP,
	PULLOUT
};

enum controllerStates_e{
	IDLE,
	EXPERIMENT
};
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
  osThreadStaticDef(ControlTask, StartControlTask, osPriorityNormal, 0, 256, ControlTaskBuffer, &ControlTaskControlBlock);
  ControlTaskHandle = osThreadCreate(osThread(ControlTask), NULL);

  /* definition and creation of TxTask */
  osThreadStaticDef(TxTask, StartTxTask, osPriorityHigh, 0, 512, DataLogTaskBuffer, &DataLogTaskControlBlock);
  TxTaskHandle = osThreadCreate(osThread(TxTask), NULL);

  /* definition and creation of MPU9250Task */
  osThreadStaticDef(MPU9250Task, StartMPU9250Task, osPriorityNormal, 0, 256, MPU9250TaskBuffer, &MPU9250TaskControlBlock);
  MPU9250TaskHandle = osThreadCreate(osThread(MPU9250Task), NULL);

  /* definition and creation of RxTask */
  osThreadStaticDef(RxTask, StartRxTask, osPriorityRealtime, 0, 512, rxTaskBuffer, &rxTaskControlBlock);
  RxTaskHandle = osThreadCreate(osThread(RxTask), NULL);

  /* definition and creation of TempTask */
  osThreadStaticDef(TempTask, StartTempTask, osPriorityNormal, 0, 256, TempTaskBuffer, &TempTaskControlBlock);
  TempTaskHandle = osThreadCreate(osThread(TempTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of xMPUToTXQueue */
  osMessageQStaticDef(xMPUToTXQueue, 1, uint32_t, xMPUToTXQueueBuffer, &xMPUToTXQueueControlBlock);
  xMPUToTXQueueHandle = osMessageCreate(osMessageQ(xMPUToTXQueue), NULL);

  /* definition and creation of xControlToTXQueue */
  osMessageQStaticDef(xControlToTXQueue, 1, uint32_t, xControlToTXQueueBuffer, &xControlToTXQueueControlBlock);
  xControlToTXQueueHandle = osMessageCreate(osMessageQ(xControlToTXQueue), NULL);

  /* definition and creation of xControlCommandQueue */
  osMessageQStaticDef(xControlCommandQueue, 2, uint32_t, xControlCommandQueueBuffer, &xControlCommandQueueControlBlock);
  xControlCommandQueueHandle = osMessageCreate(osMessageQ(xControlCommandQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

  /* Create the queue set(s) */
  /* definition and creation of xTxQueueSet */
  xTxQueueSet = xQueueCreateSet( 1 + 2 ); // Argument is the sum of the queue sizes for all event queues
  xQueueAddToSet(xMPUToTXQueueHandle, xTxQueueSet);
  xQueueAddToSet(xControlToTXQueueHandle, xTxQueueSet);
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {

  }
  /* USER CODE END StartDefaultTask */
}

/* StartControlTask function */
void StartControlTask(void const * argument)
{
  /* USER CODE BEGIN StartControlTask */
	float dutyCycleTEC1 = 0;
	float dutyCycleTEC2 = 0;
	float dutyCycleMag1 = 0;
	float dutyCycleMag2 = 0;

	TickType_t curTick;

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	uint32_t commandDataBuffer[2] = {0};

	/* Startup procedure */
	enum flightEvents_e receivedEvent = NONE;
	enum controllerStates_e controllerState = IDLE;

	TEC_stop();

	/* Infinite loop */
	for(;;)
	{
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CONTROL_CYCLE_MS)); // Service this task every CONTROL_CYCLE_MS milliseconds

		/********** Check for flight events from command queue **********/
		if(uxQueueMessagesWaiting(xControlCommandQueueHandle) != 0){
			xQueueReceive(xControlCommandQueueHandle, &receivedEvent, 0);

			// One-time state update for the event
			switch(receivedEvent){
				case REDUCEDGRAVITY:
					controllerState = EXPERIMENT;

					// TODO: one-time setting of duty cycle for TECs
					dutyCycleTEC1 = 0.85;
					dutyCycleTEC2 = 0.85;
					TEC_set_valuef(dutyCycleTEC1, dutyCycleTEC2);

					// Make status LED blink at 10 Hz
					osTimerStop(tmrLEDBlinkHandle);
					osTimerStart(tmrLEDBlinkHandle, 50);
					break;
				case NONE:
					controllerState = IDLE;

					dutyCycleTEC1 = 0;
					dutyCycleTEC2 = 0;
					TEC_set_valuef(dutyCycleTEC1, dutyCycleTEC2);

					// Make status LED blink at 2 Hz
					osTimerStop(tmrLEDBlinkHandle);
					osTimerStart(tmrLEDBlinkHandle, 1000);
					break;
				default:
					break; // Should never reach here
			}

		}

		switch(controllerState){
			case IDLE:
				break;
			case EXPERIMENT:
				/* Update PWM duty cycle for magnets */
				curTick = xTaskGetTickCount();

				dutyCycleMag1 = (1.0 + sinf(0.02 * curTick)) / 2.0;
				dutyCycleMag2 = (1.0 + cosf(0.02 * curTick)) / 2.0;
				TEC_set_valuef(dutyCycleTEC1, dutyCycleTEC2);
				break;
			default:
				break; // Should never reach here
		}

		/********** Tell transmit task that new data is ready **********/
		commandDataBuffer[0] = ((uint16_t)(dutyCycleMag1 * 100) << 16) | ((uint16_t)(dutyCycleMag2 * 100));
		commandDataBuffer[1] = ((uint16_t)(dutyCycleTEC1 * 100) << 16) | ((uint16_t)(dutyCycleTEC2 * 100));
		xQueueSend(xControlToTXQueueHandle, commandDataBuffer, 1);
	}
  /* USER CODE END StartControlTask */
}

/* StartTxTask function */
void StartTxTask(void const * argument)
{
  /* USER CODE BEGIN StartTxTask */
  /********** For inter-task communication **********/
  QueueSetMemberHandle_t xActivatedMember; // Used to see which queue sent event data
  uint32_t taskRxEventBuff[2]; // Buffer to receive data from event queues
  uint8_t taskFlags = 0x00; // Used to track which tasks have fresh data
// TODO: uint32_t tick = 0; 		// Used as a timeout


  /********** Local vars used for packet **********/
  uint8_t buffer[50] = {0};

  // Dummy bits to indicate packet start
  buffer[0] = 0xFF;
  buffer[1] = 0xFF;

  // Addresses in buffer for each datum
  uint8_t* tickStart = &buffer[2];
  uint8_t* accelX = &buffer[6];
  uint8_t* accelY = &buffer[10];
  uint8_t* accelZ = &buffer[14];
  uint8_t* magX = &buffer[18];
  uint8_t* magY = &buffer[22];
  uint8_t* magZ = &buffer[26];
  uint8_t* mag1Power = &buffer[30];
  uint8_t* mag2Power = &buffer[32];
  uint8_t* tec1Power = &buffer[34];
  uint8_t* tec2Power = &buffer[36];
//  uint8_t* thermocouple1 = &buffer[38];
//  uint8_t* thermocouple2 = &buffer[40];
//  uint8_t* thermocouple3 = &buffer[42];
//  uint8_t* thermocouple4 = &buffer[44];
//  uint8_t* thermocouple5 = &buffer[46];
//  uint8_t* thermocouple6 = &buffer[48];


  /* Infinite loop */
  for(;;)
  {
	  /********** Wait for something to transmit **********/
	  xActivatedMember = xQueueSelectFromSet(xTxQueueSet, portMAX_DELAY);

	  if(xActivatedMember != NULL){
		  if(xActivatedMember == xMPUToTXQueueHandle){
			  xQueueReceive(xActivatedMember, &taskRxEventBuff, 0);

			  /* Update task flags to indicate MPU task has been received from */
			  taskFlags = taskFlags | 0b00000001;

			  /* Copy data to buffer */
			  memcpy(accelX, &myMPU9250.ax, sizeof(float));
			  memcpy(accelY, &myMPU9250.ay, sizeof(float));
			  memcpy(accelZ, &myMPU9250.az, sizeof(float));
			  memcpy(magX, &myMPU9250.hx, sizeof(float));
			  memcpy(magY, &myMPU9250.hy, sizeof(float));
			  memcpy(magZ, &myMPU9250.hz, sizeof(float));
		  }
		  else if(xActivatedMember == xControlToTXQueueHandle){
			  xQueueReceive(xActivatedMember, &taskRxEventBuff, 0);

			  /* Update task flags to indicate TEC task has been received from */
			  taskFlags = taskFlags | 0b00000010;

			  /* Copy data to buffer */
			  uint8_t mag1data = (taskRxEventBuff[0] >> 16) & 0xFFFF;
			  uint8_t mag2data = taskRxEventBuff[0] & 0xFFFF;
			  uint16_t tec1data = (taskRxEventBuff[1] >> 16) & 0xFFFF;
			  uint16_t tec2data = taskRxEventBuff[1] & 0xFFFF;
			  memcpy(mag1Power, &mag1data, sizeof(uint16_t));
			  memcpy(mag2Power, &mag2data, sizeof(uint16_t));
			  memcpy(tec1Power, &tec1data, sizeof(uint16_t));
			  memcpy(tec2Power, &tec2data, sizeof(uint16_t));
		  }
	  }


	  /********** This runs when all data acquisition tasks have responded or timed out **********/
	  if(taskFlags == 0b00000011){
		  /* Obligatory packing */
		  TickType_t curTick = xTaskGetTickCount();
		  memcpy(tickStart, &curTick, sizeof(TickType_t));

		  /* Transmit */
		  HAL_UART_Transmit_DMA(&huart2, buffer, sizeof(buffer));
		  xSemaphoreTake(semTxHandle, portMAX_DELAY);

		  /* Clear activation flags */
		  taskFlags = 0x00;
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

  /* Initial state is sensing no event, and no command to transmit */
  enum flightEvents_e sensedEvent = NONE;

  /* Infinite loop */
  for(;;)
  {
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MPU9250_CYCLE_MS)); // Service this task every MPU9250_CYCLE_MS milliseconds

    /* Acceleration */
    accelReadDMA(&myMPU9250, semMPU9250Handle); // Read ax, ay, az
    myMPU9250.A = sqrt(myMPU9250.az * myMPU9250.az + myMPU9250.ay * myMPU9250.ay + myMPU9250.ax * myMPU9250.ax);

	/* Magnetometer */
	magFluxReadDMA(&myMPU9250, semMPU9250Handle); // Read hx, hy, hz

	/********** Tell transmit task that new data is read **********/
	uint32_t dummyToSend = 1;
	xQueueSend(xMPUToTXQueueHandle, &dummyToSend, 1);

	/********** Use the acceleration magnitude to update state **********/
	// TODO: This myMPU9250.A reading should be filtered, with perhaps a
	// moving average over the last 10 samples, since the sensor tends to
	// be prone to some noise
//	if(myMPU9250.az < 0.981){ // can use this line for testing communication with the other tasks
	if(myMPU9250.A < 0.981){
		if(sensedEvent != REDUCEDGRAVITY){
			sensedEvent = REDUCEDGRAVITY;
			xQueueSend(xControlCommandQueueHandle, (uint32_t*)&sensedEvent, 1); // Notify task to start experiment
		}
	}
	else{
		if(sensedEvent != NONE){
			sensedEvent = NONE;
			xQueueSend(xControlCommandQueueHandle, (uint32_t*)&sensedEvent, 1); // Notify task to stop experiment
		}
	}
	// TODO: Make it more difficult to come out of the experiment than going above 0.981...
  }
  /* USER CODE END StartMPU9250Task */
}

/* StartRxTask function */
void StartRxTask(void const * argument)
{
  /* USER CODE BEGIN StartRxTask */
  uint8_t buffer[1];
  enum flightEvents_e manualOverride = REDUCEDGRAVITY;

  /* Infinite loop */
  for(;;)
  {
	 HAL_UART_Receive_IT(&huart2, buffer, 1);
	 if(xSemaphoreTake(semRxHandle, portMAX_DELAY) == pdTRUE){
		 // TODO: Parse input and do something based on it
		 if(buffer[0] == ' '){
			 // Manual override for starting experiment
			 // TODO: figure out how to make sure this message makes it no matter
			 // what (had some issues with xQueueOverwrite); maybe use a specific
			 // queue for manual override or something...
			 xQueueSend(xControlCommandQueueHandle, (uint32_t*)&manualOverride, 1);

			 // Spacebar alternates between starting and stopping experiment
			 if(manualOverride == REDUCEDGRAVITY){
				 manualOverride = NONE;
			 }
			 else{
				 manualOverride = REDUCEDGRAVITY;
			 }
		 }
		 else if(buffer[0] == 'F'){
			 // Frame shift error handling
		 }
//		 LED();
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

  /* Infinite loop */
  for(;;)
  {
	  // TODO: vTaskDelayUntil may not be needed in this thread
	  vTaskDelayUntil(&xLastWakeTime, TEMP_CYCLE_MS); // Service this task every TEMP_CYCLE_MS milliseconds

	  // TODO: Process data acquired from temperature sensors here.
	  // The DMA wakeup occurs roughly every 133 microseconds for total buffer size of 100
	  // and wakeups every half-full cycle
	  //
	  // (1/((45*10^6)/8)) * 15 * 50 * 1000 = 0.133 ms per buffer half full
	  osDelay(1);
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

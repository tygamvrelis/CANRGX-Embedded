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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "MPU9250.h"


/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
uint32_t defaultTaskBuffer[ 1024 ];
osStaticThreadDef_t defaultTaskControlBlock;
osThreadId TECContrlTaskHandle;
uint32_t TECContrlTaskBuffer[ 256 ];
osStaticThreadDef_t TECContrlTaskControlBlock;
osThreadId TxTaskHandle;
uint32_t DataLogTaskBuffer[ 512 ];
osStaticThreadDef_t DataLogTaskControlBlock;
osThreadId MPU9250TaskHandle;
uint32_t MPU9250TaskBuffer[ 256 ];
osStaticThreadDef_t MPU9250TaskControlBlock;
osThreadId RxTaskHandle;
uint32_t rxTaskBuffer[ 512 ];
osStaticThreadDef_t rxTaskControlBlock;
osMessageQId xMPUEventQueueHandle;
uint8_t xMPUEventQueueBuffer[ 1 * sizeof( uint32_t ) ];
osStaticMessageQDef_t xMPUEventQueueControlBlock;
osMessageQId xTECEventQueueHandle;
uint8_t xTECEventQueueBuffer[ 1 * sizeof( uint32_t ) ];
osStaticMessageQDef_t xTECEventQueueControlBlock;
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
void StartTECContrlTask(void const * argument);
void StartTxTask(void const * argument);
void StartMPU9250Task(void const * argument);
void StartRxTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

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

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadStaticDef(defaultTask, StartDefaultTask, osPriorityIdle, 0, 1024, defaultTaskBuffer, &defaultTaskControlBlock);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of TECContrlTask */
  osThreadStaticDef(TECContrlTask, StartTECContrlTask, osPriorityHigh, 0, 256, TECContrlTaskBuffer, &TECContrlTaskControlBlock);
  TECContrlTaskHandle = osThreadCreate(osThread(TECContrlTask), NULL);

  /* definition and creation of TxTask */
  osThreadStaticDef(TxTask, StartTxTask, osPriorityRealtime, 0, 512, DataLogTaskBuffer, &DataLogTaskControlBlock);
  TxTaskHandle = osThreadCreate(osThread(TxTask), NULL);

  /* definition and creation of MPU9250Task */
  osThreadStaticDef(MPU9250Task, StartMPU9250Task, osPriorityHigh, 0, 256, MPU9250TaskBuffer, &MPU9250TaskControlBlock);
  MPU9250TaskHandle = osThreadCreate(osThread(MPU9250Task), NULL);

  /* definition and creation of RxTask */
  osThreadStaticDef(RxTask, StartRxTask, osPriorityHigh, 0, 512, rxTaskBuffer, &rxTaskControlBlock);
  RxTaskHandle = osThreadCreate(osThread(RxTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of xMPUEventQueue */
  osMessageQStaticDef(xMPUEventQueue, 1, uint32_t, xMPUEventQueueBuffer, &xMPUEventQueueControlBlock);
  xMPUEventQueueHandle = osMessageCreate(osMessageQ(xMPUEventQueue), NULL);

  /* definition and creation of xTECEventQueue */
  osMessageQStaticDef(xTECEventQueue, 1, uint32_t, xTECEventQueueBuffer, &xTECEventQueueControlBlock);
  xTECEventQueueHandle = osMessageCreate(osMessageQ(xTECEventQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

  /* Create the queue set(s) */
  /* definition and creation of xTxQueueSet */
  xTxQueueSet = xQueueCreateSet( 1 + 1 ); // Argument is the sum of the queue sizes for all event queues
  xQueueAddToSet(xMPUEventQueueHandle, xTxQueueSet);
  xQueueAddToSet(xTECEventQueueHandle, xTxQueueSet);
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

/* StartTECContrlTask function */
void StartTECContrlTask(void const * argument)
{
  /* USER CODE BEGIN StartTECContrlTask */
  /* Infinite loop */
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	static float ratio_tmp = 0;
	for(;;)
	{
		vTaskDelayUntil(&xLastWakeTime, TEC_CYCLE_MS); // Service this task every TEC_CYCLE_MS milliseconds


		/********** Update PWM duty cycle **********/
		ratio_tmp = (1.0 + sinf(0.02 * xTaskGetTickCount())) / 2.0;
		TEC_set_valuef(ratio_tmp, ratio_tmp);


		/********** Tell transmit task that new data is ready **********/
//		xQueueSend(xTECEventQueueHandle, &ratio_tmp, 0);
		xQueueOverwrite(xTECEventQueueHandle, &ratio_tmp);
	}
  /* USER CODE END StartTECContrlTask */
}

/* StartTxTask function */
void StartTxTask(void const * argument)
{
  /* USER CODE BEGIN StartTxTask */
  /********** For inter-task communication **********/
  QueueSetMemberHandle_t xActivatedMember; // Used to see which queue sent event data
  uint32_t taskRxEventBuff; // Buffer to receive data from event queues
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
  uint8_t* thermocouple1 = &buffer[38];
  uint8_t* thermocouple2 = &buffer[40];
  uint8_t* thermocouple3 = &buffer[42];
  uint8_t* thermocouple4 = &buffer[44];
  uint8_t* thermocouple5 = &buffer[46];
  uint8_t* thermocouple6 = &buffer[48];


  /* Infinite loop */
  for(;;)
  {
	  /********** Wait for something to transmit **********/
	  xActivatedMember = xQueueSelectFromSet(xTxQueueSet, portMAX_DELAY);

	  if(xActivatedMember == xMPUEventQueueHandle){
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
	  else if(xActivatedMember == xTECEventQueueHandle){
		  xQueueReceive(xActivatedMember, &taskRxEventBuff, 0);

		  /* Update task flags to indicate TEC task has been received from */
		  taskFlags = taskFlags | 0b00000010;

		  /* Copy data to buffer */
		  uint16_t tec1data = taskRxEventBuff & 0xFF;
		  uint16_t tec2data = (taskRxEventBuff >> 8) & 0xFF;
		  memcpy(tec1Power, &tec1data, sizeof(uint16_t)); // TODO: idk if this is what we wanna send back for TEC 1
		  memcpy(tec1Power, &tec2data, sizeof(uint16_t)); // TODO: idk if this is what we wanna send back for TEC 2
	  }


	  /********** This runs when all tasks have fresh data **********/
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

  /* Infinite loop */
  for(;;)
  {
    vTaskDelayUntil(&xLastWakeTime, MPU9250_CYCLE_MS); // Service this task every MPU9250_CYCLE_MS milliseconds


    /********** Acquire data **********/
    // Note: The following pattern is used below.
    //    1. Acquire data
    //    2. Shift and OR bytes together to reconstruct 16-bit data, then scale it from its measured range to its physical range
    //    3. Check sign bit and if set, the number is supposed to be negative, so change NOT all bits and add 1 (2's complement format)

    /* Acceleration */
    accelReadDMA(&myMPU9250, semMPU9250Handle); // Read ax, ay, az
	myMPU9250.A = sqrt(myMPU9250.az * myMPU9250.az + myMPU9250.ay * myMPU9250.ay + myMPU9250.ax * myMPU9250.ax);


	/* Gyroscope -- not used presently */
//	gyroReadDMA(&myMPU9250, semMPU9250Handle); // Read vx, vy, vz


	/* Magnetometer */
	magFluxReadDMA(&myMPU9250, semMPU9250Handle); // Read hx, hy, hz


	/********** Tell transmit task that new data is read **********/
	uint32_t dummyToSend = 1;
	xQueueOverwrite(xMPUEventQueueHandle, &dummyToSend);


	/********** Use the acceleration magnitude to update state **********/
	//TODO
	if(myMPU9250.A < 0.981){
		myMPU9250.theEvent = REDUCEDGRAVITY;
	}
	else{
		myMPU9250.theEvent = NONE;
	}


	// TODO: Notify task to start experiment here
  }
  /* USER CODE END StartMPU9250Task */
}

/* StartRxTask function */
void StartRxTask(void const * argument)
{
  /* USER CODE BEGIN StartRxTask */
  uint8_t buffer[1];

  /* Infinite loop */
  for(;;)
  {
	 HAL_UART_Receive_IT(&huart2, buffer, 1);
	 if(xSemaphoreTake(semRxHandle, portMAX_DELAY) == pdTRUE){
		 // TODO: Parse input and do something based on it
		 LED();
	 }
	 else{
		 // Should never reach here
	 }
  }
  /* USER CODE END StartRxTask */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

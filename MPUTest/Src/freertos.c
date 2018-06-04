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
#include "pid_contrl.h"
#include "../Drivers/MPU9250/MPU9250.h"
#include "userTypes.h"


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
uint8_t xControlToTXQueueBuffer[ 1 * sizeof( controlData_t ) ];
osStaticMessageQDef_t xControlToTXQueueControlBlock;
osMessageQId xControlCommandQueueHandle;
uint8_t xControlCommandQueueBuffer[ 1 * sizeof( uint32_t ) ];
osStaticMessageQDef_t xControlCommandQueueControlBlock;
osMessageQId xTemperatureToTXQueueHandle;
uint8_t xTemperatureToTXQueueBuffer[ 1 * sizeof( temperatureData_t ) ];
osStaticMessageQDef_t xTemperatureToTXQueueControlBlock;
osTimerId tmrLEDBlinkHandle;
osStaticTimerDef_t tmrLEDBlinkControlBlock;
osSemaphoreId semMPU9250Handle;
osStaticSemaphoreDef_t semMPU9250ControlBlock;
osSemaphoreId semTxHandle;
osStaticSemaphoreDef_t semTxControlBlock;
osSemaphoreId semRxHandle;
osStaticSemaphoreDef_t semRxControlBlock;
osSemaphoreId semTemperatureHandle;
osStaticSemaphoreDef_t semTemperatureControlBlock;

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

  /* definition and creation of semTemperature */
  osSemaphoreStaticDef(semTemperature, &semTemperatureControlBlock);
  semTemperatureHandle = osSemaphoreCreate(osSemaphore(semTemperature), 1);

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
  osMessageQStaticDef(xControlToTXQueue, 1, controlData_t, xControlToTXQueueBuffer, &xControlToTXQueueControlBlock);
  xControlToTXQueueHandle = osMessageCreate(osMessageQ(xControlToTXQueue), NULL);

  /* definition and creation of xControlCommandQueue */
  osMessageQStaticDef(xControlCommandQueue, 1, uint32_t, xControlCommandQueueBuffer, &xControlCommandQueueControlBlock);
  xControlCommandQueueHandle = osMessageCreate(osMessageQ(xControlCommandQueue), NULL);

  /* definition and creation of xTemperatureToTXQueue */
  osMessageQStaticDef(xTemperatureToTXQueue, 1, temperatureData_t, xTemperatureToTXQueueBuffer, &xTemperatureToTXQueueControlBlock);
  xTemperatureToTXQueueHandle = osMessageCreate(osMessageQ(xTemperatureToTXQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

  /* Create the queue set(s) */
  /* definition and creation of xTxQueueSet */
  xTxQueueSet = xQueueCreateSet( 1 + 1 + 1 ); // Argument is the sum of the queue sizes for all event queues
  xQueueAddToSet(xMPUToTXQueueHandle, xTxQueueSet);
  xQueueAddToSet(xControlToTXQueueHandle, xTxQueueSet);
  xQueueAddToSet(xTemperatureToTXQueueHandle, xTxQueueSet);
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
	const float TEC_ON_DUTY_CYCLE = 0.85;

	float TEC1DutyCycle = 0;
	float TEC2DutyCycle = 0;
	MagnetInfo_t magnet1Info = {MAGNET1, COAST, 0.0};
	MagnetInfo_t magnet2Info = {MAGNET2, COAST, 0.0};

	TickType_t curTick;

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	controlData_t controlData = {0};

	/* Startup procedure */
	enum flightEvents_e receivedEvent = NONE;
	enum controllerStates_e controllerState = IDLE;

	TEC_stop();

	/* Infinite loop */
	for(;;)
	{
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CONTROL_CYCLE_MS)); // Service this task every CONTROL_CYCLE_MS milliseconds

		/********** Check for flight events from command queue **********/
		while(uxQueueMessagesWaiting(xControlCommandQueueHandle) != 0){
			xQueueReceive(xControlCommandQueueHandle, &receivedEvent, 0);

			// One-time state update for the event
			switch(receivedEvent){
				case REDUCEDGRAVITY:
					controllerState = EXPERIMENT;

					TEC1DutyCycle = TEC_ON_DUTY_CYCLE;
					TEC2DutyCycle = TEC_ON_DUTY_CYCLE;
					TEC_set_valuef(TEC1DutyCycle, TEC2DutyCycle);

					// POSITIVECURRENT and NEGATIVECURRENT make the magnetic field
					// be generated in opposite directions
					magnet1Info.magnetState = POSITIVECURRENT;
					magnet2Info.magnetState = POSITIVECURRENT;
//					magnet1Info.magnetState = NEGATIVECURRENT;
//					magnet2Info.magnetState = NEGATIVECURRENT;

					// Make status LED blink at 10 Hz
					osTimerStop(tmrLEDBlinkHandle);
					osTimerStart(tmrLEDBlinkHandle, 50);
					break;
				case NONE:
					controllerState = IDLE;

					TEC1DutyCycle = 0;
					TEC2DutyCycle = 0;
					TEC_stop();

					magnet1Info.magnetState = COAST;
					magnet2Info.magnetState = COAST;
					magnet1Info.dutyCycle = 0;
					magnet2Info.dutyCycle = 0;
					setMagnet(&magnet1Info);
					setMagnet(&magnet2Info);


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
				// Update PWM duty cycle for magnets
				curTick = xTaskGetTickCount();

				magnet1Info.dutyCycle = (1.0 + sinf(0.02 * curTick)) / 2.0;
				magnet2Info.dutyCycle = (1.0 + cosf(0.02 * curTick)) / 2.0;
				setMagnet(&magnet1Info);
				setMagnet(&magnet2Info);
				break;
			default:
				break; // Should never reach here
		}

		/********** Tell transmit task that new data is ready **********/
		controlData.mag1Power = (uint16_t)(magnet1Info.dutyCycle * 100);
		controlData.mag2Power = (uint16_t)(magnet2Info.dutyCycle * 100);
		controlData.tec1Power = (uint16_t)(TEC1DutyCycle * 100);
		controlData.tec2Power = (uint16_t)(TEC2DutyCycle * 100);
		xQueueSend(xControlToTXQueueHandle, &controlData, 1);
	}
  /* USER CODE END StartControlTask */
}

/* StartTxTask function */
void StartTxTask(void const * argument)
{
  /* USER CODE BEGIN StartTxTask */
  /********** For intertask communication **********/
  QueueSetMemberHandle_t xActivatedMember; // Used to see which queue sent event data
  uint8_t taskFlags = 0x00; // Used to track which tasks have fresh data

  uint32_t eventBuffuint32_t;
  controlData_t controlDataBuff;
  temperatureData_t temperatureDataBuff;

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

	  if(xActivatedMember != NULL){
		  if(xActivatedMember == xMPUToTXQueueHandle){
			  xQueueReceive(xActivatedMember, &eventBuffuint32_t, 0);

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
			  xQueueReceive(xActivatedMember, &controlDataBuff, 0);

			  /* Update task flags to indicate control task has been received from */
			  taskFlags = taskFlags | 0b00000010;

			  /* Copy data to buffer */
			  uint16_t mag1data = controlDataBuff.mag1Power;
			  uint16_t mag2data = controlDataBuff.mag2Power;
			  uint16_t tec1data = controlDataBuff.tec1Power;
			  uint16_t tec2data = controlDataBuff.tec2Power;
			  memcpy(mag1Power, &mag1data, sizeof(uint16_t));
			  memcpy(mag2Power, &mag2data, sizeof(uint16_t));
			  memcpy(tec1Power, &tec1data, sizeof(uint16_t));
			  memcpy(tec2Power, &tec2data, sizeof(uint16_t));
		  }
		  else if(xActivatedMember == xTemperatureToTXQueueHandle){
			  xQueueReceive(xActivatedMember, &temperatureDataBuff, 0);

			  /* Update task flags to indicate temperature task has been received from */
			  taskFlags = taskFlags | 0b00000100;

			  /* Copy data to buffer */
			  uint16_t thermocouple1data = temperatureDataBuff.thermocouple1;
			  uint16_t thermocouple2data = temperatureDataBuff.thermocouple2;
			  uint16_t thermocouple3data = temperatureDataBuff.thermocouple3;
			  uint16_t thermocouple4data = temperatureDataBuff.thermocouple4;
			  uint16_t thermocouple5data = temperatureDataBuff.thermocouple5;
			  uint16_t thermocouple6data = temperatureDataBuff.thermocouple6;
			  memcpy(thermocouple1, &thermocouple1data, sizeof(uint16_t));
			  memcpy(thermocouple2, &thermocouple2data, sizeof(uint16_t));
			  memcpy(thermocouple3, &thermocouple3data, sizeof(uint16_t));
			  memcpy(thermocouple4, &thermocouple4data, sizeof(uint16_t));
			  memcpy(thermocouple5, &thermocouple5data, sizeof(uint16_t));
			  memcpy(thermocouple6, &thermocouple6data, sizeof(uint16_t));
		  }
	  }


	  /********** This runs when the first 2 data acquisition tasks have responded or timed out **********/
	  if((taskFlags & 0b00000011) == 0b00000011){
		  /* Obligatory packing */
		  TickType_t curTick = xTaskGetTickCount();
		  memcpy(tickStart, &curTick, sizeof(TickType_t));

		  /* Block for 87 microseconds, the time for 2 bytes to be transmitted via
		   * UART. This idle time guarantees interpacket delay, thus reducing
		   * the probability of a framing error. Time out after 1 ms. */
		  HAL_TIM_Base_Start_IT(&htim10);
		  xTaskNotifyWait(UINT32_MAX, UINT32_MAX, NULL, 1);

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
  int8_t accelStatus = 1;
  int8_t magStatus = 1;

  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  /* Initial state is sensing no event, and no command to transmit */
  enum flightEvents_e sensedEvent = NONE;

  /* Infinite loop */
  for(;;)
  {
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MPU9250_CYCLE_MS)); // Service this task every MPU9250_CYCLE_MS milliseconds

    /* Acceleration */
    accelStatus = accelReadDMA(&myMPU9250, semMPU9250Handle); // Read ax, ay, az
    if(accelStatus == 1){
    	myMPU9250.A = sqrt(myMPU9250.az * myMPU9250.az + myMPU9250.ay * myMPU9250.ay + myMPU9250.ax * myMPU9250.ax);
    }
    else{
    	/* The accelerometer was not able to be read from properly, handle this here. */
    	myMPU9250.A = NAN;
    	accelStatus = runtimeResetIMU(semMPU9250Handle);
    	if(accelStatus != 1){
    		/* If we still couldn't successfully communicate with this device,
			 * either it is not working properly, or there is
			 * an I2C issue. */
    	}
    }

	/* Magnetometer */
	magStatus = magFluxReadDMA(&myMPU9250, semMPU9250Handle); // Read hx, hy, hz
	if(magStatus != 1){
		/* The magnetometer was not able to be read from properly, handle this here. */
		magStatus = runtimeResetMagnetometer(semMPU9250Handle);
    	if(magStatus != 1){
    		/* If we still couldn't successfully communicate with the magnetometer,
    		 * either the magnetometer module is not working properly, or there is
    		 * an I2C issue. */
    	}
	}

	/********** Tell transmit task that new data is ready **********/
	uint32_t dummyToSend = 1;
	xQueueSend(xMPUToTXQueueHandle, &dummyToSend, 1);

	/********** Use the acceleration magnitude to update state **********/
	if(myMPU9250.az < 0.981 && sensedEvent == NONE){ // can use this line for testing communication with the other tasks
//	if(myMPU9250.A < 0.981 && sensedEvent == NONE){
		sensedEvent = REDUCEDGRAVITY;
		xQueueSend(xControlCommandQueueHandle, (uint32_t*)&sensedEvent, 1); // Notify task to start experiment
	}
	else if(myMPU9250.az > 3.13 && sensedEvent == REDUCEDGRAVITY){ // can use this line for testing communication with the other tasks
//	else if(myMPU9250.A > 3.13 && sensedEvent == REDUCEDGRAVITY){
		sensedEvent = NONE;
		xQueueSend(xControlCommandQueueHandle, (uint32_t*)&sensedEvent, 1); // Notify task to stop experiment
	}
	else{
		/* Will reach here when no transition between events is detected; i.e., when the same state
		 * (either NONE or REDUCEDGRAVITY) is detected in consecutive cycles */
	}
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
	 HAL_UART_Receive_IT(&huart2, buffer, sizeof(buffer));
	 if(xSemaphoreTake(semRxHandle, portMAX_DELAY) == pdTRUE){
		 // TODO: Parse input and do something based on it
		 if(buffer[0] == ' '){
			 // Manual override for starting experiment
			 // TODO: figure out how to make sure this message makes it no matter
			 // what (had some issues with xQueueOverwrite); maybe use a specific
			 // queue for manual override or something...
			 // TODO: How to select experiment number?
			 // Potential idea: pressing 1, 2, 3, or 4 will enqueue the respective
			 // experiment numbers into a size 1 "manual override queue"
			 xQueueSend(xControlCommandQueueHandle, (uint32_t*)&manualOverride, 1); // I don't know if timeout = 1 is good here

			 // Spacebar alternates between starting and stopping experiment
			 if(manualOverride == REDUCEDGRAVITY){
				 manualOverride = NONE;
			 }
			 else{
				 manualOverride = REDUCEDGRAVITY;
			 }
		 }
//		 LED(); // Debugging for RX
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

  temperatureData_t temperatureData = {0};

  /* Infinite loop */
  for(;;)
  {
	  // TODO: vTaskDelayUntil may not be needed in this thread
	  vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TEMP_CYCLE_MS)); // Service this task every TEMP_CYCLE_MS milliseconds

	  // TODO: Process data acquired from temperature sensors here.
	  // The DMA wakeup occurs roughly every 133 microseconds for total buffer size of 100
	  // and wakeups every half-full cycle
	  // (1/((45*10^6)/8)) * 15 * 50 * 1000 = 0.133 ms per buffer half full

	  Temp_Scan_Start();
	  xSemaphoreTake(semTemperatureHandle, TEMP_CYCLE_MS - 2);
	  Temp_Scan_Stop();

	  temperatureData.thermocouple1 = ADC_processed[TEMP1];
	  temperatureData.thermocouple2 = ADC_processed[TEMP2];
	  temperatureData.thermocouple3 = ADC_processed[TEMP3];
	  temperatureData.thermocouple4 = ADC_processed[TEMP4];
	  temperatureData.thermocouple5 = ADC_processed[TEMP5];
	  temperatureData.thermocouple6 = ADC_processed[TEMP6];

    xQueueSend(xTemperatureToTXQueueHandle, &temperatureData, 1);
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

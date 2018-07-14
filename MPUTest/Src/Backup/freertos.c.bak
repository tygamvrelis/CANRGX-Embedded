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
uint32_t ControlTaskBuffer[ 512 ];
osStaticThreadDef_t ControlTaskControlBlock;
osThreadId TxTaskHandle;
uint32_t DataLogTaskBuffer[ 512 ];
osStaticThreadDef_t DataLogTaskControlBlock;
osThreadId MPU9250TaskHandle;
uint32_t MPU9250TaskBuffer[ 512 ];
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
	EXPERIMENT1,
	EXPERIMENT2,
	EXPERIMENT3,
	EXPERIMENT4
};

#define MANUAL_OVERRIDE_START_BITMASK 0x80000000
#define MANUAL_OVERRIDE_STOP_BITMASK 0x08000000
#define MPU_BITMASK 0x00800000
#define NOTIFY_FROM_MANUAL_OVERRIDE_START(x) (MANUAL_OVERRIDE_START_BITMASK | (x))
#define NOTIFY_FROM_MPU(x) (MPU_BITMASK | (x))
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
  osThreadStaticDef(ControlTask, StartControlTask, osPriorityNormal, 0, 512, ControlTaskBuffer, &ControlTaskControlBlock);
  ControlTaskHandle = osThreadCreate(osThread(ControlTask), NULL);

  /* definition and creation of TxTask */
  osThreadStaticDef(TxTask, StartTxTask, osPriorityHigh, 0, 512, DataLogTaskBuffer, &DataLogTaskControlBlock);
  TxTaskHandle = osThreadCreate(osThread(TxTask), NULL);

  /* definition and creation of MPU9250Task */
  osThreadStaticDef(MPU9250Task, StartMPU9250Task, osPriorityNormal, 0, 512, MPU9250TaskBuffer, &MPU9250TaskControlBlock);
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
	MagnetInfo_t magnet1Info = {MAGNET1, COAST, ACTIVE_HIGH, 0.0};
	MagnetInfo_t magnet2Info = {MAGNET2, COAST, ACTIVE_HIGH, 0.0};
	setMagnet(&magnet1Info);
	setMagnet(&magnet2Info);

	TXData_t txDataControl;
	controlData_t controlData = {0};

	txDataControl.type = control_t;
	txDataControl.data = &controlData;

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	TickType_t curTick;
	uint32_t notification;

	enum controllerStates_e nextControllerState = EXPERIMENT1;

	/* Startup procedure */
	enum flightEvents_e receivedEvent = NONE;
	enum controllerStates_e controllerState = IDLE;

	TEC_stop();

	/* Infinite loop */
	for(;;)
	{
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CONTROL_CYCLE_MS)); // Service this task every CONTROL_CYCLE_MS milliseconds

		/********** Check for flight events **********/
		if(xTaskNotifyWait(0, UINT32_MAX, &notification, pdMS_TO_TICKS(1)) == pdTRUE){

			// One-time state update for the event. The cases towards the end have the highest
			// priority if they occur, which is why they are if statements and not if-else
			if((notification & MPU_BITMASK) == MPU_BITMASK){
				// This is the case for when the MPU9250 senses an event. In this case, the
				// task notification holds the value of the enumerated type flightEvents_e
				// sent by the MPU9250 thread.
				receivedEvent = notification & (~MPU_BITMASK);
			}
			if((notification & MANUAL_OVERRIDE_START_BITMASK) == MANUAL_OVERRIDE_START_BITMASK){
				// This is the case for when a manual override START sequence is received. In
				// this case, the task notification holds the number of the experiment to run.
				receivedEvent = REDUCEDGRAVITY;
				nextControllerState = notification & (~MANUAL_OVERRIDE_START_BITMASK);
			}
			if((notification & MANUAL_OVERRIDE_STOP_BITMASK) == MANUAL_OVERRIDE_STOP_BITMASK){
				// This is the case for when a manual override STOP sequence is received
				receivedEvent = NONE;
			}

			switch(receivedEvent){
				case REDUCEDGRAVITY:
					controllerState = nextControllerState;

					TEC1DutyCycle = TEC_ON_DUTY_CYCLE;
					TEC2DutyCycle = TEC_ON_DUTY_CYCLE;
					TEC_set_valuef(TEC1DutyCycle, TEC2DutyCycle);

					magnet1Info.magnetState = PWM;
					magnet2Info.magnetState = PWM;

					// Make status LED blink at 10 Hz
					osTimerStop(tmrLEDBlinkHandle);
					osTimerStart(tmrLEDBlinkHandle, 50);
					break;
				case NONE:
					controllerState = IDLE;

					// The next controller state will increment until the last state,
					// at which point it will wrap around
					switch(nextControllerState){
						case IDLE:
						case EXPERIMENT1:
						case EXPERIMENT2:
						case EXPERIMENT3:
							nextControllerState++;
							break;
						case EXPERIMENT4:
							nextControllerState = EXPERIMENT1;
							break;
					}

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

		// Update PWM duty cycle for magnets
		switch(controllerState){
			case IDLE:
				break;
			case EXPERIMENT1:
				curTick = xTaskGetTickCount();
				magnet1Info.dutyCycle = sinf(0.002 * curTick);
				magnet2Info.dutyCycle = sinf(0.002 * curTick);
				setMagnet(&magnet1Info);
				setMagnet(&magnet2Info);
				break;
			case EXPERIMENT2:
				curTick = xTaskGetTickCount();
				magnet1Info.dutyCycle = (1.0 + sinf(0.002 * curTick)) / 2.0;
				magnet2Info.dutyCycle = (1.0 + sinf(0.002 * curTick)) / 2.0;
				setMagnet(&magnet1Info);
				setMagnet(&magnet2Info);
				break;
			case EXPERIMENT3:
				curTick = xTaskGetTickCount();
				magnet1Info.dutyCycle = sinf(0.002 * curTick);
				magnet2Info.dutyCycle = cosf(0.002 * curTick);
				setMagnet(&magnet1Info);
				setMagnet(&magnet2Info);
				break;
			case EXPERIMENT4:
				curTick = xTaskGetTickCount();
				magnet1Info.dutyCycle = (1.0 + sinf(0.002 * curTick)) / 2.0;
				magnet2Info.dutyCycle = (1.0 + cosf(0.002 * curTick)) / 2.0;
				setMagnet(&magnet1Info);
				setMagnet(&magnet2Info);
				break;
			default:
				break; // Should never reach here
		}

		/********** Tell transmit task that new data is ready **********/
		// Here, we multiply the duty cycle by 100 * 100 so that we capture the
		// integer part and the fractional part (to 2 decimal places)
		controlData.mag1Power = (int16_t)(magnet1Info.dutyCycle * 10000);
		controlData.mag2Power = (int16_t)(magnet2Info.dutyCycle * 10000);
		controlData.tec1Power = (uint16_t)(TEC1DutyCycle * 10000);
		controlData.tec2Power = (uint16_t)(TEC2DutyCycle * 10000);
		xQueueSend(xTXDataQueueHandle, &txDataControl, 1);
	}
  /* USER CODE END StartControlTask */
}

/* StartTxTask function */
void StartTxTask(void const * argument)
{
  /* USER CODE BEGIN StartTxTask */
  /********** For intertask communication **********/
  uint8_t taskFlags = 0x00; // Used to track which tasks have fresh data

  TXData_t receivedData;
  accelerometerData_t* accelerometerDataPtr = NULL;
  magnetometerData_t* magnetometerDataPtr = NULL;
  controlData_t* controlDataPtr = NULL;
  temperatureData_t* temperatureDataPtr = NULL;

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

  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  /* Infinite loop */
  for(;;)
  {
	  /********** Wait for something to transmit **********/
	  while(xQueueReceive(xTXDataQueueHandle, &receivedData, portMAX_DELAY) != pdTRUE);

	  switch(receivedData.type){
		  case accelerometer_t:
			  accelerometerDataPtr = (accelerometerData_t*)receivedData.data;

			  if(accelerometerDataPtr == NULL){break;}

			  /* Update task flags to indicate MPU task has been received from */
			  taskFlags = taskFlags | 0b00000001;

			  /* Copy data to buffer */
			  memcpy(accelX, &(accelerometerDataPtr -> ax), sizeof(float));
			  memcpy(accelY, &(accelerometerDataPtr -> ay), sizeof(float));
			  memcpy(accelZ, &(accelerometerDataPtr -> az), sizeof(float));

			  break;
		  case magnetometer_t:
			  magnetometerDataPtr = (magnetometerData_t*)receivedData.data;

			  if(magnetometerDataPtr == NULL){break;}

			  /* Update task flags to indicate MPU task has been received from */
			  taskFlags = taskFlags | 0b00000010;

			  memcpy(magX, &(magnetometerDataPtr -> hx), sizeof(float));
			  memcpy(magY, &(magnetometerDataPtr -> hy), sizeof(float));
			  memcpy(magZ, &(magnetometerDataPtr -> hz), sizeof(float));

			  break;
		  case control_t:
			  controlDataPtr = (controlData_t*)receivedData.data;

			  if(controlDataPtr == NULL){break;}

			  /* Update task flags to indicate control task has been received from */
			  taskFlags = taskFlags | 0b00000100;

			  /* Copy data to buffer */
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
			  temperatureDataPtr = (temperatureData_t*)receivedData.data;

			  if(temperatureDataPtr == NULL){break;}

			  /* Update task flags to indicate temperature task has been received from */
			  taskFlags = taskFlags | 0b00001000;

			  /* Copy data to buffer */
			  uint16_t thermocouple1data = temperatureDataPtr -> thermocouple1;
			  uint16_t thermocouple2data = temperatureDataPtr -> thermocouple2;
			  uint16_t thermocouple3data = temperatureDataPtr -> thermocouple3;
			  uint16_t thermocouple4data = temperatureDataPtr -> thermocouple4;
			  uint16_t thermocouple5data = temperatureDataPtr -> thermocouple5;
			  uint16_t thermocouple6data = temperatureDataPtr -> thermocouple6;
			  memcpy(thermocouple1, &thermocouple1data, sizeof(uint16_t));
			  memcpy(thermocouple2, &thermocouple2data, sizeof(uint16_t));
			  memcpy(thermocouple3, &thermocouple3data, sizeof(uint16_t));
			  memcpy(thermocouple4, &thermocouple4data, sizeof(uint16_t));
			  memcpy(thermocouple5, &thermocouple5data, sizeof(uint16_t));
			  memcpy(thermocouple6, &thermocouple6data, sizeof(uint16_t));

			  break;
		  default:
			  break;
	  }


	  /********** This runs when the first 2 data acquisition tasks have responded or timed out **********/
	  if((taskFlags & 0b00000111) == 0b00000111){
		  // Send packets every 3 milliseconds. At a symbol rate of 230400, it takes 2.17 ms
		  // to send 50 bytes. To ease the data analysis process, this has been rounded up to
		  // 3 ms so that it will be easier to "line up" data with the FreeRTOS tick.
		  vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3));

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
  int8_t accelStatus;
  int8_t magStatus;

  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  TXData_t txDataAccel, txDataMag;
  magnetometerData_t magnetometerData;
  accelerometerData_t accelerometerData;

  txDataAccel.type = accelerometer_t;
  txDataAccel.data = &accelerometerData;

  txDataMag.type = magnetometer_t;
  txDataMag.data = &magnetometerData;

  /* Initial state is sensing no event, and no command to transmit */
  enum flightEvents_e sensedEvent = NONE;

  MPUFilterType azFilter, ayFilter, axFilter;
  MPUFilter_init(&axFilter);
  MPUFilter_init(&ayFilter);
  MPUFilter_init(&azFilter);

  /* Infinite loop */
  for(;;)
  {
	// TODO: if MPU sensor fails a few consecutive cycles, we can decrease
	// its sampling rate by 100 times until it works again so that we don't
	// slow down the system
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MPU9250_CYCLE_MS)); // Service this task every MPU9250_CYCLE_MS milliseconds

    /* Acceleration */
    accelStatus = accelReadDMA(&myMPU9250, semMPU9250Handle); // Read ax, ay, az
    if(accelStatus == 1){
    	/***** Filter the signals along each axis *****/
    	MPUFilter_writeInput(&axFilter, myMPU9250.ax);
    	myMPU9250.ax = MPUFilter_readOutput(&axFilter);

    	MPUFilter_writeInput(&ayFilter, myMPU9250.ay);
    	myMPU9250.ay = MPUFilter_readOutput(&ayFilter);

    	MPUFilter_writeInput(&azFilter, myMPU9250.az);
    	myMPU9250.az = MPUFilter_readOutput(&azFilter);

    	myMPU9250.A = sqrt(myMPU9250.az * myMPU9250.az + myMPU9250.ay * myMPU9250.ay + myMPU9250.ax * myMPU9250.ax);
    	accelerometerData.ax = myMPU9250.ax;
    	accelerometerData.ay = myMPU9250.ay;
    	accelerometerData.az = myMPU9250.az;
    }
    else{
    	/* The accelerometer was not able to be read from properly, handle this here. */
    	generateClocks(&hi2c3, 1, 1);
    	myMPU9250.A = NAN;
    }

	/* Magnetometer */
	magStatus = magFluxReadDMA(&myMPU9250, semMPU9250Handle); // Read hx, hy, hz
	if(magStatus == 1){
		magnetometerData.hx = myMPU9250.hx;
		magnetometerData.hy = myMPU9250.hy;
		magnetometerData.hz = myMPU9250.hz;
	}
	else{
		/* The magnetometer was not able to be read from properly, handle this here. */
		generateClocks(&hi2c1, 1, 1);
	}

	/********** Tell transmit task that new data is ready **********/
	xQueueSend(xTXDataQueueHandle, &txDataAccel, 1);
	xQueueSend(xTXDataQueueHandle, &txDataMag, 1);

	/********** Use the acceleration magnitude to update state **********/
	if(myMPU9250.az < 0.981 && sensedEvent == NONE){ // can use this line for testing
//	if(myMPU9250.A < 0.981 && sensedEvent == NONE){
		sensedEvent = REDUCEDGRAVITY;

		// Notify task to start experiment
		xTaskNotify(ControlTaskHandle, NOTIFY_FROM_MPU(sensedEvent), eSetBits);
	}
	else if(myMPU9250.az > 3.13 && sensedEvent == REDUCEDGRAVITY){ // can use this line for testing
//	else if(myMPU9250.A > 3.13 && sensedEvent == REDUCEDGRAVITY){
		sensedEvent = NONE;

		// Notify task to stop experiment
		xTaskNotify(ControlTaskHandle, NOTIFY_FROM_MPU(sensedEvent), eSetBits);
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
  const char MANUAL_OVERRIDE_START_CHAR = 'S';
  const char MANUAL_OVERRIDE_STOP_SEQ[] = {'X', 'X'};
  const char RESET_SEQ[] = {'R', 'S'};

  uint8_t buffer[3]; // buffer[0] == control character, buffer[1] == accompanying data, buffer[2] == '\n'

  /* Infinite loop */
  for(;;)
  {
	 HAL_UART_Receive_IT(&huart2, buffer, sizeof(buffer));
	 if(xSemaphoreTake(semRxHandle, portMAX_DELAY) == pdTRUE){
		 if(buffer[0] == MANUAL_OVERRIDE_START_CHAR){
			 // Manual override for starting experiment
			 xTaskNotify(ControlTaskHandle, NOTIFY_FROM_MANUAL_OVERRIDE_START(buffer[1] - '0'), eSetBits);
		 }
		 else if(buffer[0] == MANUAL_OVERRIDE_STOP_SEQ[0] &&
				 buffer[1] == MANUAL_OVERRIDE_STOP_SEQ[1])
		 {
			 // Manual override for stopping experiment
			 xTaskNotify(ControlTaskHandle, MANUAL_OVERRIDE_STOP_BITMASK, eSetBits);
		 }
		 else if(buffer[0] == RESET_SEQ[0] &&
				 buffer[1] == RESET_SEQ[1])
		 {
			 // TODO: shut down safely, i.e., let all I/O transactions
			 // finish so that we can start up properly again without
			 // getting hung up
			 // Do nothing while a UART transaction is ongoing
			 while(hi2c1.State != HAL_I2C_STATE_RESET)
			 {

			 }
			 while(huart2.gState != HAL_UART_STATE_READY)
			 {

			 }
			 // Full system reset
			 NVIC_SystemReset();
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

  TXData_t txDataTemperature;
  temperatureData_t temperatureData = {0};

  txDataTemperature.type = temperature_t;
  txDataTemperature.data = &temperatureData;

  /* Infinite loop */
  for(;;)
  {
	  vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TEMP_CYCLE_MS)); // Service this task every TEMP_CYCLE_MS milliseconds

	  Temp_Scan_Start();
	  xSemaphoreTake(semTemperatureHandle, TEMP_CYCLE_MS - 2);
	  Temp_Scan_Stop();

	  temperatureData.thermocouple1 = ADC_processed[TEMP1];
	  temperatureData.thermocouple2 = ADC_processed[TEMP2];
	  temperatureData.thermocouple3 = ADC_processed[TEMP3];
	  temperatureData.thermocouple4 = ADC_processed[TEMP4];
	  temperatureData.thermocouple5 = ADC_processed[TEMP5];
	  temperatureData.thermocouple6 = ADC_processed[TEMP6];

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

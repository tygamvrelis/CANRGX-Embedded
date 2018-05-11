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
#include "fmpi2c.h"
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
osMessageQId txQueueHandle;
uint8_t txQueueBuffer[ 64 * sizeof( uint32_t ) ];
osStaticMessageQDef_t txQueueControlBlock;
osSemaphoreId semMPU9250Handle;
osStaticSemaphoreDef_t semMPU9250ControlBlock;
osSemaphoreId semTxHandle;
osStaticSemaphoreDef_t semTxControlBlock;
osSemaphoreId semRxHandle;
osStaticSemaphoreDef_t semRxControlBlock;

/* USER CODE BEGIN Variables */
//#define ADC_DATA_N 12
//volatile uint16_t uhADC_results[ADC_DATA_N];

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartTECContrlTask(void const * argument);
void StartTxTask(void const * argument);
void StartRxTask(void const * argument);
void StartMPU9250Task(void const * argument);

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

// Stuff for data logging
typedef enum{
	TECTask,
	MPUTask
}taskID_t;

typedef struct{
	uint8_t size; // Total number of useful bytes used in this message
	taskID_t taskID; // Indicates which task is sending the message
	union data{
		double arrDouble[10];
		uint8_t arrUint[40];
		int arrInt[10];
	};
}Message_t;

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
  osThreadStaticDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1024, defaultTaskBuffer, &defaultTaskControlBlock);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of TECContrlTask */
  osThreadStaticDef(TECContrlTask, StartTECContrlTask, osPriorityAboveNormal, 0, 256, TECContrlTaskBuffer, &TECContrlTaskControlBlock);
  TECContrlTaskHandle = osThreadCreate(osThread(TECContrlTask), NULL);

  /* definition and creation of TxTask */
  osThreadStaticDef(TxTask, StartTxTask, osPriorityHigh, 0, 512, DataLogTaskBuffer, &DataLogTaskControlBlock);
  TxTaskHandle = osThreadCreate(osThread(TxTask), NULL);

  /* definition and creation of MPU9250Task */
  osThreadStaticDef(MPU9250Task, StartMPU9250Task, osPriorityRealtime, 0, 256, MPU9250TaskBuffer, &MPU9250TaskControlBlock);
  MPU9250TaskHandle = osThreadCreate(osThread(MPU9250Task), NULL);

  /* definition and creation of RxTask */
  osThreadStaticDef(RxTask, StartRxTask, osPriorityHigh, 0, 512, rxTaskBuffer, &rxTaskControlBlock);
  RxTaskHandle = osThreadCreate(osThread(RxTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of txQueue */
  osMessageQStaticDef(txQueue, 64, uint32_t, txQueueBuffer, &txQueueControlBlock);
  txQueueHandle = osMessageCreate(osMessageQ(txQueue), NULL);

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

/* StartTECContrlTask function */
void StartTECContrlTask(void const * argument)
{
  /* USER CODE BEGIN StartTECContrlTask */
  /* Infinite loop */

	// TODO: Delete this while loop preventing the task from running
	while(1){osDelay(10);}

	static float ratio_tmp=0;
	for(;;)
	{
		ratio_tmp=(1.0+sinf(0.02*xTaskGetTickCount()))/2.0;
		TEC_set_valuef(ratio_tmp,ratio_tmp);
	  	osDelay(2);
	}
  /* USER CODE END StartTECContrlTask */
}

/* StartTxTask function */
void StartTxTask(void const * argument)
{
  /* USER CODE BEGIN StartTxTask */
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  // Local vars that will be packed into tx packets
  uint32_t tick = 0;
  uint32_t id = 0;
  uint32_t buffer[100];

  /* Infinite loop */
  for(;;)
  {
	  // TODO: Basically, the way this can work is as follows. First,
	  // there have to be ring buffers within each task for the data
	  // that is to be transmitted. Then, the pointers to the various
	  // elements in the ring buffers are enqueued. The transmit task
	  // can use the pointers to access each element and compose packets
	  //
	  // How to synchronize data between tasks? One way is that each task
	  // could get the current tick count, and we could group them based
	  // on that...


	  /********** Wait for something to transmit **********/
	  if(xQueueReceive(txQueueHandle, buffer, portMAX_DELAY)){

		  /********** Obligatory packing **********/
		  tick = xTaskGetTickCount(); // Current tick
		  id = id++; // Message ID

		  /********** Transmit **********/
		  HAL_UART_Transmit_DMA(&huart2, buffer, buffer[0]);
		  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // Toggle green LED
		  xSemaphoreTake(semTxHandle, portMAX_DELAY);
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

  uint8_t mpu_buff[7]; // Temporary buffer to hold data from sensor
  uint16_t temp;

  /* Infinite loop */
  for(;;)
  {
    vTaskDelayUntil(&xLastWakeTime, MPU9250_CYCLE_MS); // Service this task every MPU9250_CYCLE_MS milliseconds




    /********** Acquire data **********/
    // Note: The following pattern is used below.
    //    1. Acquire data
    //    2. Shift and OR bytes together to reconstruct 16-bit data, then scale it from its measured range to its physical range
    //    3. Check sign bit and if set, the number is supposed to be negative, so change NOT all bits and add 1 (2's complement format)


    /********** Acceleration **********/
    // Read az
//    HAL_I2C_Mem_Read(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_ACCEL_Z_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 2, 100);
    HAL_I2C_Mem_Read_DMA(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_ACCEL_Z_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 2);
    xSemaphoreTake(semMPU9250Handle, portMAX_DELAY);
    temp = (mpu_buff[0] << 8 | mpu_buff[1]); // Shift bytes into appropriate positions
    temp = (mpu_buff[0] & 0x80) == 0x80 ? ~temp + 1 : temp; // Check sign bit, perform two's complement transformation if necessary
    float myVar = (temp * MPU9250_ACCEL_FULL_SCALE  / (32767.0)); // Scale to physical units
    myMPU9250.az = myVar;

    // Read ay
    HAL_I2C_Mem_Read_DMA(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_ACCEL_Y_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 2);
	xSemaphoreTake(semMPU9250Handle, portMAX_DELAY);
	temp = (mpu_buff[0] << 8 | mpu_buff[1]);
	temp = (mpu_buff[0] & 0x80) == 0x80 ? ~temp + 1 : temp;
	myVar = (temp * MPU9250_ACCEL_FULL_SCALE  / (32767.0));
	myMPU9250.ay = myVar;

	// Read ax
	HAL_I2C_Mem_Read_DMA(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_ACCEL_X_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 2);
	xSemaphoreTake(semMPU9250Handle, portMAX_DELAY);
	temp = (mpu_buff[0] << 8 | mpu_buff[1]);
	temp = (mpu_buff[0] & 0x80) == 0x80 ? ~temp + 1 : temp;
	myVar = (temp * MPU9250_ACCEL_FULL_SCALE  / (32767.0));
	myMPU9250.ax = myVar;


	/********** Gyroscope **********/
    // Read vz
	HAL_I2C_Mem_Read_DMA(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_GYRO_Z_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 2);
	xSemaphoreTake(semMPU9250Handle, portMAX_DELAY);
	temp = (mpu_buff[0] << 8 | mpu_buff[1]);
	temp = (mpu_buff[0] & 0x80) == 0x80 ? ~temp + 1 : temp;
	myMPU9250.vz = (temp / (32767.0) * MPU9250_ACCEL_FULL_SCALE);

    // Read vy
//	HAL_I2C_Mem_Read(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_GYRO_Y_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 2, 100);
    HAL_I2C_Mem_Read_DMA(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_GYRO_Y_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 2);
    xSemaphoreTake(semMPU9250Handle, portMAX_DELAY);
	temp = (mpu_buff[0] << 8 | mpu_buff[1]);
	temp = (mpu_buff[0] & 0x80) == 0x80 ? ~temp + 1 : temp;
	myMPU9250.vy = (temp / (32767.0) * MPU9250_ACCEL_FULL_SCALE);

    // Read vx
	HAL_I2C_Mem_Read_DMA(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_GYRO_X_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 2);
	xSemaphoreTake(semMPU9250Handle, portMAX_DELAY);
	temp = (mpu_buff[0] << 8 | mpu_buff[1]);
	temp = (mpu_buff[0] & 0x80) == 0x80 ? ~temp + 1 : temp;
	myMPU9250.vx = (temp / (32767.0) * MPU9250_ACCEL_FULL_SCALE);


	/********** Magnetometer **********/
	// Read magnetic field. Note that the high and low bytes switch places for the magnetic field readings
	// due to the way the registers are mapped. Note that 7 bytes are read because the magnetometer requires
	// the ST2 register to be read in addition to other data
	magnetometerReadIT(MPU9250_MAG_X_ADDR_L, 7, mpu_buff, semMPU9250Handle);
//	magnetometerRead(MPU9250_MAG_X_ADDR_L, 7, mpu_buff);

	temp = (mpu_buff[1] << 8 | mpu_buff[0]);
	temp = (mpu_buff[1] & 0x80) == 0x80 ? ~temp + 1 : temp;
	myMPU9250.hx = (temp / (32760.0) * MPU9250_MAG_FULL_SCALE);

	temp = (mpu_buff[3] << 8 | mpu_buff[2]);
	temp = (mpu_buff[3] & 0x80) == 0x80 ? ~temp + 1 : temp;
	myMPU9250.hy = (temp / (32760.0) * MPU9250_MAG_FULL_SCALE);

	temp =  (mpu_buff[5] << 8 | mpu_buff[4]);
	temp = (mpu_buff[5] & 0x80) == 0x80 ? ~temp + 1 : temp;
	myMPU9250.hz = (temp / (32760.0) * MPU9250_MAG_FULL_SCALE);




	/********** Use the data to update state **********/
	//TODO
	float accelMag = sqrt(myMPU9250.az * myMPU9250.az + myMPU9250.ay * myMPU9250.ay + myMPU9250.ax * myMPU9250.ax);
	myMPU9250.A = accelMag;
	if(accelMag < 0.981){
		myMPU9250.theEvent = REDUCEDGRAVITY;
	}
	else{
		myMPU9250.theEvent = NONE;
	}
//	if(myMPU9250.vy > 2.5 && myMPU9250.az < -14.715){
//		// Transition from straight and level to pull-up
//		myMPU9250.theEvent = PULLUP;
//	}
//	else if(myMPU9250.vy > 2.5 && myMPU9250.az < -14.715){
//		// Transition from pull-up to reduced gravity
//		myMPU9250.theEvent = REDUCEDGRAVITY;
//	}
//	else if(myMPU9250.vy > 2.5 && myMPU9250.az < -14.715){
//		// Transition from reduced gravity to pull-out
//		myMPU9250.theEvent = PULLOUT;
//	}
//	else{
//		// No state change
//		myMPU9250.theEvent = NONE;
//	}


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
	 if(xSemaphoreTake(semRxHandle, portMAX_DELAY)){
		 // TODO: Parse input and do something based on it
		 // TODO: Remove DMA channel for UART in cube
		 HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // Toggle green LED
	 }
  }
  /* USER CODE END StartRxTask */
}

/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

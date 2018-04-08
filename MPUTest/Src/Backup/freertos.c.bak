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
osThreadId DataLogTaskHandle;
uint32_t DataLogTaskBuffer[ 512 ];
osStaticThreadDef_t DataLogTaskControlBlock;
osThreadId MPU9250TaskHandle;
uint32_t MPU9250TaskBuffer[ 256 ];
osStaticThreadDef_t MPU9250TaskControlBlock;

/* USER CODE BEGIN Variables */
//#define ADC_DATA_N 12
//volatile uint16_t uhADC_results[ADC_DATA_N];
static char print_buff[300];
uint16_t str_size;
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartTECContrlTask(void const * argument);
void StartDataLogTask(void const * argument);
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

  /* definition and creation of DataLogTask */
  osThreadStaticDef(DataLogTask, StartDataLogTask, osPriorityNormal, 0, 512, DataLogTaskBuffer, &DataLogTaskControlBlock);
  DataLogTaskHandle = osThreadCreate(osThread(DataLogTask), NULL);

  /* definition and creation of MPU9250Task */
  osThreadStaticDef(MPU9250Task, StartMPU9250Task, osPriorityRealtime, 0, 256, MPU9250TaskBuffer, &MPU9250TaskControlBlock);
  MPU9250TaskHandle = osThreadCreate(osThread(MPU9250Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  osDelay(5);
  for(;;)
  {
    osDelay(100);
  }
  /* USER CODE END StartDefaultTask */
}

/* StartTECContrlTask function */
void StartTECContrlTask(void const * argument)
{
  /* USER CODE BEGIN StartTECContrlTask */
  /* Infinite loop */
	static float ratio_tmp=0;
	for(;;)
	{
		ratio_tmp=(1.0+sinf(0.02*xTaskGetTickCount()))/2.0;
		TEC_set_valuef(ratio_tmp,ratio_tmp);
	  	osDelay(2);
	}
  /* USER CODE END StartTECContrlTask */
}

/* StartDataLogTask function */
void StartDataLogTask(void const * argument)
{
  /* USER CODE BEGIN StartDataLogTask */

  /* Infinite loop */
  for(;;)
  {
	  //str_size=sprintf(print_buff, "t %5d : %d %d \r\n",xTaskGetTickCount(),uhADC_results[0],uhADC_results[1]);
	  str_size=sprintf(print_buff, "%5d, %d, %4.2f, %d, %4.2f \r\n",xTaskGetTickCount(),uhADC_results[0],ADC1_Filtered(0),uhADC_results[1],ADC1_Filtered(1));
	  //uint16_t str_size=strlen(print_buff);
	  //HAL_UART_Transmit(&huart2,print_buff,strlen(print_buff),1000);

	  //TODO: Down a semaphore here, and up it in the DMA callback
	  while(HAL_UART_Transmit_DMA(&huart2,print_buff, str_size)!= HAL_OK){
		  osDelay(5);
	  }
	  osDelay(10);
	  //vTaskList(print_buff);
	  //HAL_UART_Transmit_DMA(&huart2,print_buff, 150);
	  //osDelay(30);
  }
  /* USER CODE END StartDataLogTask */
}

/* StartMPU9250Task function */
void StartMPU9250Task(void const * argument)
{
  /* USER CODE BEGIN StartMPU9250Task */
  MPU9250Init(myMPU9250); // Initialize MPU9250

  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  uint8_t mpu_buff[6]; // Temporary buffer to hold data from sensor

  /* Infinite loop */
  for(;;)
  {
    vTaskDelayUntil(&xLastWakeTime, MPU9250_CYCLE_MS); // Service this task every MPU9250_CYCLE_MS milliseconds

    // Read az
    HAL_FMPI2C_Mem_Read_DMA(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_ACCEL_Z_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 2);
    myMPU9250 -> az = (mpu_buff[0] << 8 | mpu_buff[1]) / (65535) * MPU9250_ACCEL_FULL_SCALE;

    // Read vy
	HAL_FMPI2C_Mem_Read_DMA(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_GYRO_Y_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 2);
	myMPU9250 -> vy = (mpu_buff[0] << 8 | mpu_buff[1]) / (65535) * MPU9250_GYRO_FULL_SCALE;

	// Read magnetic field. Note that the high and low bytes switch places for the magnetic field readings
	// due to the way the registers are mapped
	HAL_FMPI2C_Mem_Read_DMA(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_GYRO_Y_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 6);
	myMPU9250 -> hx = (mpu_buff[1] << 8 | mpu_buff[0]) / (65535) * MPU9250_MAG_FULL_SCALE;
	myMPU9250 -> hy = (mpu_buff[3] << 8 | mpu_buff[2]) / (65535) * MPU9250_MAG_FULL_SCALE;
	myMPU9250 -> hz = (mpu_buff[5] << 8 | mpu_buff[4]) / (65535) * MPU9250_MAG_FULL_SCALE;
  }
  /* USER CODE END StartMPU9250Task */
}

/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

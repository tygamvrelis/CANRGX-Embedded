
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "../Drivers/MPU9250/MPU9250.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  MX_I2C3_Init();
  MX_RTC_Init();
  MX_TIM3_Init();
  MX_TIM12_Init();
  MX_USART2_UART_Init();
  MX_TIM10_Init();
  MX_SPI2_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  /********** Start-up procedure prior to starting the scheduler **********/
  MANUAL_MX_RTC_Init(); // Fix HAL bug where RTC is not initialized in the generated code

  HAL_Delay(100); // We're in no rush here...

  uint8_t recvBuff = 0;

  // Tell PC we are powered on
  char msg[] = "Microcontroller init sequence begin\n";
  do{
	  HAL_UART_Transmit(&huart2, (uint8_t*)msg, sizeof(msg), 10);
	  HAL_UART_Receive(&huart2, &recvBuff, 1, 1000);
  }while(recvBuff != 'A');
  recvBuff = 0; // Clear ACK

  /*************************************************************************
   *                      MPU9250 Initialization                           *
   *************************************************************************/
  // Attempt to initialize the MPU9250, up to 5 attempts
  int mpuInitStatus;
  for(int i = 0; i < 5; i++){
      mpuInitStatus = MPU9250Init(&myMPU9250);
      if(mpuInitStatus == 1){
    	  break;
      }
      else if(mpuInitStatus > -8){
    	  /* This is the case if there is a problem with the IMU module.
    	   * Try hard-resetting the IMU module. */
    	  uint8_t dataToWrite = 0x80;
		  HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100);
      }
      else if(mpuInitStatus <= -8){
    	  /* This is the case if there is a problem with the magnetometer.
    	   * Try software-resetting the magnetometer. */
    	  uint8_t dataToWrite = 1;
    	  HAL_I2C_Mem_Write(&hi2c3, MPU9250_MAG_ADDR, CNTL2, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100);
      }
      if(i == 2){
    	  /* When the microcontroller program starts up, it is not guaranteed that
		   * it is from a power cycle. Instead, the program may be starting up from
		   * a reset. A reset, however, only affects the microcontroller, and does not
		   * affect any peripherals connected to it. Thus, it is possible for the I2C
		   * bus to get locked when the slave is asserting an ACK and then the master
		   * clock drops out. The solution is to send some clock pulses to transition the
		   * state of the slave that's freezing the bus. */
    	  generateClocks(&hi2c3, 10, 0); // Generate 10 clock periods for accelerometer
    	  generateClocks(&hi2c1, 10, 0); // Generate 10 clock periods for magnetometer
      }
      if(i == 3){
    	  /* There is a silicon bug in some ST MCUs where a filter in the I2C module
    	   * randomly asserts the busy flag. The function below follows certain procedures
    	   * presented in an errata document to alleviate the issue. */
    	  generateClocks(&hi2c3, 1, 1); // Send 1 stop bit
    	  generateClocks(&hi2c1, 1, 1); // Send 1 stop bit
      }
      HAL_Delay(10);
  }

  // Transmit MPU init status
  if(mpuInitStatus == 1){
	  char msg[] = "MPU9250 init success\n";
	  do{
		  HAL_UART_Transmit(&huart2, (uint8_t*)msg, sizeof(msg), 10);
		  HAL_UART_Receive(&huart2, &recvBuff, 1, 1000);
	  }while(recvBuff != 'A');
	  recvBuff = 0; // Clear ACK
  }
  else{
	  char msg[] = "MPU9250 init fail\n";
	  do{
		  HAL_UART_Transmit(&huart2, (uint8_t*)msg, sizeof(msg), 10);
		  HAL_UART_Receive(&huart2, &recvBuff, 1, 1000);
	  }while(recvBuff != 'A');
	  recvBuff = 0; // Clear ACK
  }


  /*************************************************************************
   *                        RTC Initialization                             *
   *************************************************************************/
  // Time is of the form HH.MM.SS.mmmuuu, sent as ASCII (16 bytes total)
  uint8_t buff[16] = {0};
  uint8_t* ptrHours = &buff[0]; // 2 elements
  uint8_t* ptrMinutes = &buff[3]; // 2 elements
  uint8_t* ptrSeconds = &buff[6]; // 2 elements
  uint8_t* ptrSubseconds = &buff[9]; // 6 elements; milliseconds and microseconds
  buff[15] = '\n'; // This character is added so that serial.readline() can be used on the PC side

  // Receive time
  HAL_UART_Receive(&huart2, (uint8_t*)buff, 15, 100);

  // This is the received subseconds (millis and micros) in float form.
  // We may lose some precision by doing it this way, but this step will
  // still yield much higher calibration accuracy than if we skip this.
  float recvSubseconds = {
		  aToUint((char*)ptrSubseconds) * 0.1 +
		  aToUint((char*)(ptrSubseconds + 1)) * 0.01 +
		  aToUint((char*)(ptrSubseconds + 2)) * 0.001 +
		  aToUint((char*)(ptrSubseconds + 3)) * 0.0001 +
		  aToUint((char*)(ptrSubseconds + 4)) * 0.00001 +
		  aToUint((char*)(ptrSubseconds + 5)) * 0.000001
  	  };

  // TODO: Compensate for the comm link delay
  float linkDelay = 0.000502; // Estimate for number of seconds to send time info

  // Set RTC registers for hours, minutes, seconds (BCD)
  RTC_TimeTypeDef theTime;
  RTC_DateTypeDef theDate;
  theTime.Hours = aToBCD((char*)ptrHours);
  theTime.Minutes = aToBCD((char*)ptrMinutes);
  theTime.Seconds = aToBCD((char*)ptrSeconds);
  theTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  theTime.StoreOperation = RTC_STOREOPERATION_RESET;
  HAL_RTC_SetTime(&hrtc, &theTime, RTC_FORMAT_BCD);

  // Read the subseconds register and use the RTC_SHIFTR register for
  // subsecond synchronization. Note that BOTH the time and date must
  // ALWAYS be read, and in that order, to ensure proper behaviour of
  // the RTC.
  HAL_RTC_GetTime(&hrtc, &theTime, RTC_FORMAT_BCD);
  HAL_RTC_GetDate(&hrtc, &theDate, RTC_FORMAT_BCD);
  float readSubseconds = 1.0 - ((float)theTime.SubSeconds / (float)theTime.SecondFraction);

  if(readSubseconds > recvSubseconds){
	  // We need to delay the RTC
	  float millisToDelay = readSubseconds - recvSubseconds;
	  uint16_t S = theTime.SecondFraction;
	  uint16_t ticksToDelay = S - (uint16_t)(millisToDelay * S);
	  HAL_RTCEx_SetSynchroShift(&hrtc, RTC_SHIFTADD1S_RESET, ticksToDelay);
  }
  else{
	  // We need to advance the RTC. I suppose this case is also entered if,
	  // through some miracle, readSubseconds is exactly equal to recvSubseconds.
	  float millisToAdvance = recvSubseconds - readSubseconds;
	  uint16_t S = theTime.SecondFraction;
	  uint16_t ticksToAdvance = S - (uint16_t)(millisToAdvance * S);
	  HAL_RTCEx_SetSynchroShift(&hrtc, RTC_SHIFTADD1S_SET, ticksToAdvance);
  }

  HAL_Delay(1500); // Wait 1500 ms, then send time. This way the PC will have logged data
  	  	  	  	   // demonstrating its accuracy over a reasonable time period. Also,
  	  	  	  	   // 1500 ms is necessary because after setting the RTC shift register,
                   // it takes about a second or so for it to work again properly.

  // Load transmit buffer with fresh time
  do{
	  HAL_RTC_GetTime(&hrtc, &theTime, RTC_FORMAT_BCD);
	  HAL_RTC_GetDate(&hrtc, &theDate, RTC_FORMAT_BCD);

	  *ptrHours = BCDToA(theTime.Hours) >> 8;
	  *(ptrHours + 1) = BCDToA(theTime.Hours) & 0xFF;
	  *ptrMinutes = BCDToA(theTime.Minutes) >> 8;
	  *(ptrMinutes + 1) = BCDToA(theTime.Minutes) & 0xFF;
	  *ptrSeconds = BCDToA(theTime.Seconds) >> 8;
	  *(ptrSeconds + 1) = BCDToA(theTime.Seconds) & 0xFF;
	  readSubseconds = 1.0 - ((float)theTime.SubSeconds / (float)theTime.SecondFraction);

	  // For subseconds, we use sprintf to convert from floating-point to ASCII
	  // We need to save this in an intermediate buffer so that we can ignore the
	  // "0." at the beginning of the float; this will make the time format consistent.
	  char subSecondsBuff[8] = {0};
	  sprintf((char*)subSecondsBuff, "%.6f", readSubseconds);
	  memcpy(ptrSubseconds, &subSecondsBuff[2], 6);

	  HAL_UART_Transmit(&huart2, (uint8_t*)buff, 16, 100);
	  HAL_UART_Receive(&huart2, &recvBuff, 1, 10);
  }while(recvBuff != 'A');
  recvBuff = 0; // Clear ACK

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Activate the Over-Drive mode 
    */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

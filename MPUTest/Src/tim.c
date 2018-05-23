/**
  ******************************************************************************
  * File Name          : TIM.c
  * Description        : This file provides code for the configuration
  *                      of the TIM instances.
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
#include "tim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim12;

/* TIM3 init function */
void MX_TIM3_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = MAGNET_PWM_PERIOD-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_OC_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_ACTIVE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim3);

}
/* TIM12 init function */
void MX_TIM12_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC;

  htim12.Instance = TIM12;
  htim12.Init.Prescaler = 0;
  htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim12.Init.Period = TEC_PWM_PERIOD-1;
  htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim12) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim12);

}

void HAL_TIM_OC_MspInit(TIM_HandleTypeDef* tim_ocHandle)
{

  if(tim_ocHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspInit 0 */

  /* USER CODE END TIM3_MspInit 0 */
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();
  /* USER CODE BEGIN TIM3_MspInit 1 */

  /* USER CODE END TIM3_MspInit 1 */
  }
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* tim_pwmHandle)
{

  if(tim_pwmHandle->Instance==TIM12)
  {
  /* USER CODE BEGIN TIM12_MspInit 0 */

  /* USER CODE END TIM12_MspInit 0 */
    /* TIM12 clock enable */
    __HAL_RCC_TIM12_CLK_ENABLE();
  /* USER CODE BEGIN TIM12_MspInit 1 */

  /* USER CODE END TIM12_MspInit 1 */
  }
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(timHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspPostInit 0 */

  /* USER CODE END TIM3_MspPostInit 0 */
    /**TIM3 GPIO Configuration    
    PA6     ------> TIM3_CH1
    PA7     ------> TIM3_CH2
    PB0     ------> TIM3_CH3
    PB1     ------> TIM3_CH4 
    */
    GPIO_InitStruct.Pin = Magnet_1A_Pin|Magnet_1B_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = Magnet_2A_Pin|Magnet_2B_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM3_MspPostInit 1 */

  /* USER CODE END TIM3_MspPostInit 1 */
  }
  else if(timHandle->Instance==TIM12)
  {
  /* USER CODE BEGIN TIM12_MspPostInit 0 */

  /* USER CODE END TIM12_MspPostInit 0 */
  
    /**TIM12 GPIO Configuration    
    PB14     ------> TIM12_CH1
    PB15     ------> TIM12_CH2 
    */
    GPIO_InitStruct.Pin = TEC_Top_Pin|TEC_Bot_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM12_MspPostInit 1 */

  /* USER CODE END TIM12_MspPostInit 1 */
  }

}

void HAL_TIM_OC_MspDeInit(TIM_HandleTypeDef* tim_ocHandle)
{

  if(tim_ocHandle->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspDeInit 0 */

  /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();
  /* USER CODE BEGIN TIM3_MspDeInit 1 */

  /* USER CODE END TIM3_MspDeInit 1 */
  }
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* tim_pwmHandle)
{

  if(tim_pwmHandle->Instance==TIM12)
  {
  /* USER CODE BEGIN TIM12_MspDeInit 0 */

  /* USER CODE END TIM12_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM12_CLK_DISABLE();
  /* USER CODE BEGIN TIM12_MspDeInit 1 */

  /* USER CODE END TIM12_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
int8_t setMagnet(MagnetInfo_t* magnetInfo){
	/* Sets the state of the specified magnet to coast, break, or PWM in the selected
	 * direction.
	 *
	 * Arguments: magnetInfo, pointer to a struct that contains the configuration
	 * 			  info for the magnet
	 *
	 * Returns: 1 if successful, otherwise a negative error code
	 */

	// Non-PWM modes of operation
	if(magnetInfo -> magnetState == COAST || magnetInfo -> magnetState == BRAKE){
		if(magnetInfo -> magnet == MAGNET1){
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);

// TODO: The if-else statements below were supposed to allow you to enter COAST
// or BRAKE state, but they don't seem to have any effect when these pins are used
// as timer channel outputs. The default with the PWM_Stop lines above is equivalent
// to COAST, but we should still figure out how to use these pins as GPIO
//			if(magnetInfo -> magnetState == COAST){
//				// low, low
//				HAL_GPIO_WritePin(GPIOA, Magnet_1A_Pin, GPIO_PIN_RESET);
//				HAL_GPIO_WritePin(GPIOA, Magnet_1B_Pin, GPIO_PIN_RESET);
//			}
//			else{
//				// high, high
//				HAL_GPIO_WritePin(GPIOA, Magnet_1A_Pin, GPIO_PIN_SET);
//				HAL_GPIO_WritePin(GPIOA, Magnet_1B_Pin, GPIO_PIN_SET);
//			}
		}
		else{ // if(magnetInfo -> magnet == MAGNET2)
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);

//			if(magnetInfo -> magnetState == COAST){
//				// low, low
//				HAL_GPIO_WritePin(GPIOB, Magnet_2A_Pin, GPIO_PIN_RESET);
//				HAL_GPIO_WritePin(GPIOB, Magnet_2B_Pin, GPIO_PIN_RESET);
//			}
//			else{
//				// high, high
//				HAL_GPIO_WritePin(GPIOB, Magnet_2A_Pin, GPIO_PIN_SET);
//				HAL_GPIO_WritePin(GPIOB, Magnet_2B_Pin, GPIO_PIN_SET);
//			}
		}
		return 1;
	}


	// PWM modes
	if((magnetInfo -> dutyCycle) < 0 || (magnetInfo -> dutyCycle) > 1){
		return -1;
	}

	TIM_OC_InitTypeDef sConfigOC;
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = (magnetInfo -> dutyCycle) * MAGNET_PWM_PERIOD;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

	if(magnetInfo -> magnet == MAGNET1){
		if(magnetInfo -> magnetState == POSITIVECURRENT){
			// Magnet 1A is GPIO here and Magnet 1B is PWM
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1); // Magnet 1A
			HAL_GPIO_WritePin(GPIOA, Magnet_1A_Pin, GPIO_PIN_RESET);

		    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK){
		      /* The call to HAL_TIME_PWM_ConfigChannel failed, so one or more arguments passed
		       * to it must be invalid. Handle the error here. */
		    	return -2;
		    }
			HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
		}
		else{ // if(magnetInfo -> magnetState == NEGATIVECURRENT)
			// Magnet 1B is GPIO here and Magnet 1A is PWM
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2); // Magnet 1B
			HAL_GPIO_WritePin(GPIOA, Magnet_1B_Pin, GPIO_PIN_RESET);

		    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
		      /* The call to HAL_TIME_PWM_ConfigChannel failed, so one or more arguments passed
		       * to it must be invalid. Handle the error here. */
		    	return -3;
		    }
			HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
		}
	}
	else{ // if(magnetInfo -> magnet == MAGNET2)
		if(magnetInfo -> magnetState == POSITIVECURRENT){
			// Magnet 2A is GPIO here and Magnet 2B is PWM
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3); // Magnet 2A
			HAL_GPIO_WritePin(GPIOB, Magnet_2A_Pin, GPIO_PIN_RESET);

		    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK){
		      /* The call to HAL_TIME_PWM_ConfigChannel failed, so one or more arguments passed
		       * to it must be invalid. Handle the error here. */
		    	return -4;
		    }
			HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
		}
		else{ // if(magnetInfo -> magnetState == NEGATIVECURRENT)
			// Magnet 2B is GPIO here and Magnet 2A is PWM
			HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4); // Magnet 2B
			HAL_GPIO_WritePin(GPIOB, Magnet_2B_Pin, GPIO_PIN_RESET);

		    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK){
		      /* The call to HAL_TIME_PWM_ConfigChannel failed, so one or more arguments passed
		       * to it must be invalid. Handle the error here. */
		    	return -5;
		    }
			HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
		}
	}

	return 1;
}

int8_t TEC_set_valuef(float TEC_Top_duty_cycle, float TEC_Bot_duty_cycle){
	/* Sets the PWM duty cycle used to drive the top and bottom TECs used for
	 * heating the parafluid.
	 *
	 * Arguments: the duty cycle for the top and bottom tec, respectively,
	 * 			  indicating what fraction of a period each one should be
	 * 			  on for (arguments in range [0, 1] are valid)
	 *
	 * Returns: 1 if successful, otherwise a negative error code
	 */

	// Check argument validity
	if(TEC_Top_duty_cycle < 0 || TEC_Top_duty_cycle > 1){
		return -1;
	}

	if(TEC_Bot_duty_cycle < 0 || TEC_Bot_duty_cycle > 1){
		return -2;
	}

    TIM_OC_InitTypeDef sConfigOC;

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = TEC_Top_duty_cycle * TEC_PWM_PERIOD;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
    	/* The call to HAL_TIME_PWM_ConfigChannel failed, so one or more arguments passed
    	 * to it must be invalid. Handle the error here. */
    	return -3;
    }
	HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);

	sConfigOC.Pulse = TEC_Bot_duty_cycle * TEC_PWM_PERIOD;
	if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_2) != HAL_OK){
		/* The call to HAL_TIME_PWM_ConfigChannel failed, so one or more arguments passed
		 * to it must be invalid. Handle the error here. */
		return -4;
	}
	HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);

	return 1;
}

void TEC_stop(void){
	/* Turns off the timer channels used for TEC PWM.
	 *
	 * Arguments: none
	 *
	 * Returns: none
	 */

	HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);
}
/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
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
#include "usart.h"

#include "gpio.h"
#include "dma.h"

/* USER CODE BEGIN 0 */
#include "cmsis_os.h"
/* USER CODE END 0 */

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 230400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Stream6;
    hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart2_tx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
extern osSemaphoreId semTxHandle;
extern osSemaphoreId semRxHandle;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    if (huart->Instance == USART2){
    	// Check USART instance
    	xSemaphoreGiveFromISR(semTxHandle, pdTRUE);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if (huart->Instance == USART2){
    	// Check USART instance
    	xSemaphoreGiveFromISR(semRxHandle, pdTRUE);
    }
}

//#define IO_RX_BUFFER_SIZE 256
//#define IO_TX_QUEUE_SIZE 20
//
//uint8_t IO_rx_buff[IO_RX_BUFFER_SIZE];
//uint16_t IO_rx_buff_head;
//uint16_t IO_rx_buff_tail;
//uint16_t IO_rx_buff_fill;
//
//uint8_t *IO_tx_queue_message_content[IO_TX_QUEUE_SIZE];
//uint16_t IO_tx_queue_message_length[IO_TX_QUEUE_SIZE];
//uint16_t IO_tx_buff_head;
//uint16_t IO_tx_buff_tail;
//uint16_t IO_tx_buff_fill;
//uint8_t IO_rx_receive_placeholder;
////SemaphoreHandle_t xUSART_Rx_Available;
//
//
//void USART2_IO_init(void) {
//  IO_rx_buff_head = 0;
//  IO_rx_buff_tail = 0;
//  IO_rx_buff_fill = 0;
//  IO_tx_buff_head = 0;
//  IO_tx_buff_tail = 0;
//  IO_tx_buff_fill = 0;
//
//  //Start asking for data
//  //HAL_UART_Receive_DMA(&huart2, &IO_rx_receive_placeholder, 1);
//
//  //xUSART_Rx_Available=xSemaphoreCreateCounting(IO_RX_BUFFER_SIZE,0);
//}
//
//
///*void USART2_IO_IRQHandler(UART_HandleTypeDef *huart) {
//
//  IO_rx_buff[IO_rx_buff_tail] = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
//  IO_rx_buff_tail++;
//  if (IO_rx_buff_tail == IO_RX_BUFFER_SIZE) {
//    IO_rx_buff_tail = 0;
//  }
//  IO_rx_buff_fill++;
//  //xSemaphoreGiveFromISR();
//  if (IO_rx_buff_fill > IO_RX_BUFFER_SIZE) {
//    //Buffer Overflow
//    _Error_Handler(__FILE__, __LINE__);
//  }
//}*/
//
//void USART2_IO_RxCpltCallback(UART_HandleTypeDef *huart) {
//  IO_rx_buff[IO_rx_buff_tail] = IO_rx_receive_placeholder;
//  IO_rx_buff_tail++;
//  if (IO_rx_buff_tail == IO_RX_BUFFER_SIZE) {
//    IO_rx_buff_tail = 0;
//  }
//  IO_rx_buff_fill++;
//  //xSemaphoreGiveFromISR();
//  if (IO_rx_buff_fill > IO_RX_BUFFER_SIZE) {
//    //Buffer Overflow
//    _Error_Handler(__FILE__, __LINE__);
//  }
//  HAL_UART_Receive_DMA(&huart2, &IO_rx_receive_placeholder, 1);
//}
//
//
//void USART2_IO_TxCpltCallback(UART_HandleTypeDef *huart) {
//  IO_tx_buff_fill--;
//  IO_tx_buff_head++;
//  if (IO_tx_buff_head == IO_TX_QUEUE_SIZE) {
//    IO_tx_buff_head = 0;
//  }
//  if (IO_tx_buff_fill > 0) {
//    //There is still things to transmit. Need to send.
//    if (HAL_UART_Transmit_DMA(huart,
//                              IO_tx_queue_message_content[IO_tx_buff_head],
//                              IO_tx_queue_message_length[IO_tx_buff_head]) != HAL_OK) {
//      //Unexpected condition: The TX bus should not be busy immediately after TxCpltCallback.
//      _Error_Handler(__FILE__, __LINE__);
//    }
//  }
//}
//
//
//
//void USART2_IO_Transmit(uint8_t *pData, uint16_t size) {
//
//  IO_tx_queue_message_content[IO_tx_buff_tail] = pData;
//  IO_tx_queue_message_length[IO_tx_buff_tail] = size;
//  IO_tx_buff_tail++;
//  IO_tx_buff_fill++;
//  if (IO_tx_buff_tail == IO_TX_QUEUE_SIZE) {
//    IO_tx_buff_tail = 0;
//  }
//  if (IO_tx_buff_fill == 1) {
//    //It was 0 before we added 1, so the queue should be empty before. So we need to initiate a transmit.
//    if (HAL_UART_Transmit_DMA(&huart2, pData, size) != HAL_OK) {
//      //Unexpected Behavior. Should not be busy if it is empty.
//      _Error_Handler(__FILE__, __LINE__);
//    }
//  }
//}
//
//HAL_StatusTypeDef USART2_IO_Receive(uint8_t *pData, uint16_t size) {
//  if (size > IO_rx_buff_fill)
//  {
//    return HAL_TIMEOUT;
//  } else {
//    for (int i = 0; i < size; ++i) {
//      pData[i] = IO_rx_buff[IO_rx_buff_head];
//      IO_rx_buff_fill--;
//      IO_rx_buff_head++;
//      if (IO_rx_buff_head == IO_RX_BUFFER_SIZE) {
//        IO_rx_buff_head = 0;
//      }
//    }
//    return HAL_OK;
//  }
//}
//
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef * huart) {
//  if (huart->Instance == USART2) {
//    USART2_IO_TxCpltCallback(huart);
//  }
//
//}
//
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart) {
//  if (huart->Instance == USART2) {
//    USART2_IO_RxCpltCallback(huart);
//  }
//
//}

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

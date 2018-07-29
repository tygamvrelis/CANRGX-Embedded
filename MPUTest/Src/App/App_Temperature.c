/*
 * App_Temperature.c
 *
 *  Created on: Jul 29, 2018
 *      Author: Tyler
 */

/********************************** Includes *********************************/
#include "App/App_Temperature.h"




/*********************************** Types ***********************************/
typedef enum bufferState{
    STATE_HALF_FULL,
    STATE_FULL
}bufferState_e;




/********************************** Macros **********************************/
#define ADC_DATA_N 64 // 64 array elements per channel




/***************************** Private Variables *****************************/
volatile uint32_t ADC_buff[3][ADC_DATA_N];




/***************************** Public Variables ******************************/
uint16_t ADC_processed[6];




/***************************** Private Functions *****************************/
/**
 * @brief Given an ADC_HandleTypeDef pointer, this function returns the
 *        associated raw data buffer and the processed data destination
 *        by reference.
 * @param hadc Pointer to the data structure containing all ADC configuration
 *        data
 * @param adc_buff Is assigned the address of the raw data buffer for hadc
 * @param adc_processed Is assigned the address of the output destination for
 *        the processed data corresponding to hadc
 */
static inline void setUpADCProcessing(
    ADC_HandleTypeDef* hadc,
    volatile uint32_t** adc_buff,
    uint16_t** adc_processed
)
{
    if(hadc == &hadc1){
        *adc_buff = ADC_buff[0];
        *adc_processed = &ADC_processed[0];
    }
    else if(hadc == &hadc2){
        *adc_buff = ADC_buff[1];
        *adc_processed = &ADC_processed[2];
    }
    else{
        *adc_buff = ADC_buff[2];
        *adc_processed = &ADC_processed[4];
    }
}

/**
 * @brief Processes the raw ADC data by block averaging the samples from a
 *        specified half of the raw data buffer
 * @param hadc Pointer to the data structure containing all ADC configuration
 *        data
 * @param buffState Indicates whether the processing is to occur on the first
 *        or second half of the samples in the raw data buffer
 */
static inline void processADC(ADC_HandleTypeDef* hadc, bufferState_e buffState){
    uint32_t  accumulate[2] = {0};
    volatile uint32_t* adc_raw_data_ptr = NULL; // Beginning of buffer
    uint16_t* adc_output_ptr = NULL;

    setUpADCProcessing(hadc, &adc_raw_data_ptr, &adc_output_ptr);

    if(buffState == STATE_FULL){
        adc_raw_data_ptr += ADC_DATA_N / 2; // Middle of buffer
    }

    uint32_t raw_data;
    for(uint8_t i = 0; i < ADC_DATA_N / 2; i++){
        raw_data = *(adc_raw_data_ptr);
        *accumulate += raw_data & 0xFFFF;
        *(accumulate + 1) += (raw_data >> 16) & 0xFFFF ;
        adc_raw_data_ptr++;
    }

    *adc_output_ptr = (uint16_t)(*accumulate >> 5);
    *(adc_output_ptr + 1) = (uint16_t)(*(accumulate + 1) >> 5);
}

/**
 * @brief Buffer half full callback. Here we process the first half of the
 *        buffered data
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    processADC(hadc, STATE_HALF_FULL);
}

/**
 * @brief Buffer full callback. Here we just process the second half of the
 *        buffered data
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    processADC(hadc, STATE_FULL);
}




/***************************** Public Functions ******************************/
/**
 * @brief  Initializes ADC1, ADC2, and ADC3 to use DMA to transfer sample data
 *         to buffers.
 * @return 1 if successful, otherwise a negative error code
 */
int Temp_Scan_Start(void){
    if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_buff[0], 2 * ADC_DATA_N) != HAL_OK){
        return -1;
    }
    if(HAL_ADC_Start_DMA(&hadc2, (uint32_t*)ADC_buff[1], 2 * ADC_DATA_N) != HAL_OK){
        HAL_ADC_Stop_DMA(&hadc1);
        return -2;
    }
    if(HAL_ADC_Start_DMA(&hadc3, (uint32_t*)ADC_buff[2], 2 * ADC_DATA_N) != HAL_OK){
        HAL_ADC_Stop_DMA(&hadc1);
        HAL_ADC_Stop_DMA(&hadc2);
        return -3;
    }

    return 1;
}

/**
 * @brief  Stops ADC peripherals used for sensing temperature.
 * @return 1 if successful, otherwise a negative error code
 */
int Temp_Scan_Stop(void){
    if(HAL_ADC_Stop_DMA(&hadc1) != HAL_OK){
        return -1;
    }
    if(HAL_ADC_Stop_DMA(&hadc2) != HAL_OK){
        return -2;
    }
    if(HAL_ADC_Stop_DMA(&hadc3) != HAL_OK){
        return -3;
    }

    return 1;
}

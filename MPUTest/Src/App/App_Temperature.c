/**
 * @file App_Temperature.c
 * @author Tyler
 * @brief Functions and data structures related to acquiring temperature sensor
 *        data, and consequently interfacing with the ADCs
 *
 * @defgroup Temperature Temperature
 * @brief Sensing temperature using ADCs
 * @{
 */

/********************************** Includes *********************************/
#include "App/App_Temperature.h"




/*********************************** Types ***********************************/
/** @brief Describes the state of the data buffer */
typedef enum bufferState{
    STATE_HALF_FULL, /**< The first half of the buffer contains the freshest
                      *   data                                               */
    STATE_FULL       /**< The last half of the buffer contains the freshest
                      *   data                                               */
}bufferState_e;




/********************************** Macros **********************************/
/** @brief There are 64 data points buffered per channel */
#define ADC_DATA_N 64




/***************************** Private Variables *****************************/
/**
 * @brief   The data buffer into which ADC samples are asynchronously
 *          transferred via DMA
 * @details Since the ADC samples are 12 bits, each element of this array holds
 *          2 samples. For example, ADC1 reads from channels 0 and 1 in
 *          sequence, so for n an even integer and m an odd integer,
 *          `ADC_buff[0][n]` would contain channel 0 data and `ADC_buff[0][m]`
 *          would contain channel 1 data
 */
volatile uint32_t ADC_buff[3][ADC_DATA_N];




/***************************** Public Variables ******************************/
/** Holds the filtered (block-averaged) results for each ADC channel */
uint16_t ADC_processed[6];




/***************************** Private Functions *****************************/
/**
 * @defgroup TemperaturePrivateFunctions Private functions
 * @brief Functions used internally
 * @ingroup Temperature
 * @{
 */

/**
 * @brief Given an ADC_HandleTypeDef pointer, this function returns the
 *        associated raw data buffer and the processed data destination
 *        by reference.
 * @param hadc pointer to a ADC_HandleTypeDef structure that contains the
 *        configuration information for the specified ADC
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
 * @param hadc pointer to a ADC_HandleTypeDef structure that contains the
 *        configuration information for the specified ADC
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
 * @}
 */
/* end - TemperaturePrivateFunctions */




/***************************** Public Functions ******************************/
/**
 * @defgroup TemperaturePublicFunctions Public functions
 * @brief Functions used externally
 * @ingroup Temperature
 * @{
 */

/**
 * @brief  Initializes ADC1, ADC2, and ADC3 to use DMA to transfer sample data
 *         to buffers
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
 * @brief  Stops ADC peripherals used for sensing temperature
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

/**
 * @}
 */
/* end - TemperaturePublicFunctions */




/********************************* Callbacks *********************************/
/**
 * @defgroup TemperatureCallbacks Callbacks
 * @brief Event handlers for when the sample buffers are sufficiently fresh
 * @ingroup Temperature
 * @{
 */

/**
 * @brief Buffer half full callback. Here we process the first half of the
 *        buffered data
 * @param hadc pointer to a ADC_HandleTypeDef structure that contains the
 *        configuration information for the specified ADC
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    processADC(hadc, STATE_HALF_FULL);
}

/**
 * @brief Buffer full callback. Here we just process the second half of the
 *        buffered data
 * @param hadc pointer to a ADC_HandleTypeDef structure that contains the
 *        configuration information for the specified ADC
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    processADC(hadc, STATE_FULL);
}

/**
 * @}
 */
/* end - TemperatureCallbacks */

/**
 * @}
 */
/* end - Temperature */

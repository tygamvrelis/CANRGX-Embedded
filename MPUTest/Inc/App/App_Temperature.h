/**
 * @file App_Temperature.h
 * @author Tyler
 *
 * @defgroup TemperatureHeader Temperature Header
 * @ingroup Temperature
 * @{
 */

#ifndef APP_TEMPERATURE_H_
#define APP_TEMPERATURE_H_




/********************************** Includes *********************************/
#include "adc.h"
#include "userTypes.h"




/*********************************** Enums ***********************************/
/** The temperature sensors in our system */
enum tempSensors{
    TEMP1A, /**< Temperature sensor 1A */
    TEMP1B, /**< Temperature sensor 1B */
    TEMP2A, /**< Temperature sensor 2A */
    TEMP2B, /**< Temperature sensor 2B */
    TEMP3A, /**< Temperature sensor 3A */
    TEMP3B  /**< Temperature sensor 3B */
};




/****************************** Public Variables *****************************/
extern uint16_t ADC_processed[6]; // Processed results




/********************************* Functions *********************************/
int Temp_Scan_Start(void);
int Temp_Scan_Stop(void);

/**
 * @brief Updates the temperature data container with the most recent filtered
 *        data. A block average over 32 samples is used for filtering
 */
inline void updateTemperatureData(temperatureData_t* temperatureData){
    temperatureData->temp1a = ADC_processed[TEMP1A];
    temperatureData->temp1b = ADC_processed[TEMP1B];
    temperatureData->temp2a = ADC_processed[TEMP2A];
    temperatureData->temp2b = ADC_processed[TEMP2B];
    temperatureData->temp3a = ADC_processed[TEMP3A];
    temperatureData->temp3b = ADC_processed[TEMP3B];
}

/**
 * @}
 */
/* end - TemperatureHeader */

#endif /* APP_TEMPERATURE_H_ */

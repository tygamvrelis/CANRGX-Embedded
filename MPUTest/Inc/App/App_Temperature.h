/*
 * App_Temperature.h
 *
 *  Created on: Jul 29, 2018
 *      Author: Tyler
 */

#ifndef APP_TEMPERATURE_H_
#define APP_TEMPERATURE_H_

/********************************** Includes *********************************/
#include "adc.h"
#include "userTypes.h"




/*********************************** Enums ***********************************/
enum tempSensors{
    TEMP1A,
    TEMP1B,
    TEMP2A,
    TEMP2B,
    TEMP3A,
    TEMP3B
};




/****************************** Public Variables *****************************/
extern uint16_t ADC_processed[6]; // Processed results




/********************************* Functions *********************************/
int Temp_Scan_Start(void);
int Temp_Scan_Stop(void);

inline void updateTemperatureData(temperatureData_t* temperatureData){
    temperatureData->temp1a = ADC_processed[TEMP1A];
    temperatureData->temp1b = ADC_processed[TEMP1B];
    temperatureData->temp2a = ADC_processed[TEMP2A];
    temperatureData->temp2b = ADC_processed[TEMP2B];
    temperatureData->temp3a = ADC_processed[TEMP3A];
    temperatureData->temp3b = ADC_processed[TEMP3B];
}

#endif /* APP_TEMPERATURE_H_ */

/*
 * App_Control.h
 *
 *  Created on: Jul 28, 2018
 *      Author: Tyler
 */

#ifndef APP_CONTROL_H_
#define APP_CONTROL_H_




/********************************** Includes *********************************/
#include <stdbool.h>
#include "cmsis_os.h"
#include "main.h"
#include "userTypes.h"
#include "tim.h"




/********************************* Functions *********************************/
void controlInit(void);
void controlEventHandler(uint32_t notification);
void updateControlSignals(void);
void updateControlData(controlData_t* controlData);

#endif /* APP_CONTROL_H_ */

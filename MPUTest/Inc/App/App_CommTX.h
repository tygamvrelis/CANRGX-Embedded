/*
 * App_CommTX.h
 *
 *  Created on: Jul 29, 2018
 *      Author: Tyler
 */

#ifndef APP_COMMTX_H_
#define APP_COMMTX_H_

/********************************** Includes *********************************/
#include <stdbool.h>
#include <string.h>
#include "cmsis_os.h"
#include "userTypes.h"




/********************************* Functions *********************************/
void commTXInit(void);
void commTXEventHandler(TXData_t* receivedData);
void commTXSendPacket(TickType_t* lastWakeTime, TickType_t* cycleStartTick);
bool isControlAndMPUDataCollected();
bool hasTimeoutElapsed(TickType_t cycleStartTick);

#endif /* APP_COMMTX_H_ */

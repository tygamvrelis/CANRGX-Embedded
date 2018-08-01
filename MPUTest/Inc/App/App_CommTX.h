/**
 * @file App_CommTX.h
 * @author Tyler
 *
 * @defgroup CommTXHeader Header
 * @ingroup CommTX
 * @{
 */

#ifndef APP_COMMTX_H_
#define APP_COMMTX_H_




/********************************** Includes *********************************/
#include <stdbool.h>
#include <string.h>
#include "cmsis_os.h"
#include "usart.h"
#include "userTypes.h"




/********************************* Functions *********************************/
void commTXInit(void);
void commTXEventHandler(TXData_t* receivedData);
void commTXSendPacket(TickType_t* lastWakeTime, TickType_t* cycleStartTick);
bool isControlAndMPUDataCollected();
bool hasTimeoutElapsed(TickType_t cycleStartTick);

/**
 * @}
 */
/* end - CommTXHeader */

#endif /* APP_COMMTX_H_ */

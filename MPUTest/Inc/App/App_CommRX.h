/**
 * @file App_CommRX.h
 * @author Tyler
 *
 * @defgroup CommRXHeader Communication: RX Header
 * @ingroup CommRX
 * @{
 */

#ifndef APP_COMMRX_H_
#define APP_COMMRX_H_




/********************************** Includes *********************************/
#include "cmsis_os.h"
#include "usart.h"
#include "userTypes.h"
#include "App/App_Control.h"




/********************************* Functions *********************************/
void commRXInitReception(void);
void commRXEventHandler(void);
void commRXCancelReception(void);

/*
 * @}
 */
/* end - CommRXHeader */

#endif /* APP_COMMRX_H_ */

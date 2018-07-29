/*
 * App_CommRX.h
 *
 *  Created on: Jul 29, 2018
 *      Author: Tyler
 */

#ifndef APP_COMMRX_H_
#define APP_COMMRX_H_

/********************************** Includes *********************************/
#include "cmsis_os.h"
#include "userTypes.h"
#include "App/App_Control.h"




/********************************* Functions *********************************/
void commRXInitReception(void);
void commRXEventHandler(void);

#endif /* APP_COMMRX_H_ */

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




/*********************************** Enums ***********************************/
enum magnets_e{
    MAGNET1,
    MAGNET2,
};

enum magnetStates_e{
    COAST, /**< Logical outputs: A=high, B=high */
    BRAKE, /**< Logical outputs: A=low, B=low */
    PWM /**< Drive current through coils */
};

enum driveMode_e{
    ACTIVE_LOW,
    ACTIVE_HIGH
};




/*********************************** Types ***********************************/
typedef struct{
    enum magnets_e magnet; /**< Magnet 1 or magnet 2 */
    enum magnetStates_e magnetState; /**< What the magnet is suppose to do */
    enum driveMode_e driveMode; /**< Channel polarity (i.e. whether 100% duty cycle is ON or OFF) */
    float dutyCycle; /**< Only needed if magnetState is PWM */
}MagnetInfo_t;

typedef enum{
    CURRENT_NONE,
    POSITIVE,
    NEGATIVE
}current_e;




/********************************* Functions *********************************/
void controlInit(void);
void controlEventHandler(uint32_t notification);
void updateControlSignals(void);
void updateControlData(controlData_t* controlData);
void controlSetSignalsToIdleState();

#endif /* APP_CONTROL_H_ */

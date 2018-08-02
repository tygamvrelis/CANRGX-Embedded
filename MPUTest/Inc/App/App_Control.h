/**
 * @file App_Control.h
 * @author Tyler
 *
 * @defgroup ControlHeader Header
 * @ingroup Control
 * @{
 */

#ifndef APP_CONTROL_H_
#define APP_CONTROL_H_




/********************************** Includes *********************************/
#include <stdbool.h>
#include "cmsis_os.h"
#include "main.h"
#include "tim.h"
#include "userTypes.h"




/*********************************** Enums ***********************************/
/** @brief The magnets in our system */
enum magnets_e{
    MAGNET1, /**< First magnet  */
    MAGNET2, /**< Second magnet */
};

/** @brief The states we can put the driver circuits for the magnets in */
enum magnetStates_e{
    COAST, /**< Logical outputs: A=high, B=high */
    BRAKE, /**< Logical outputs: A=low, B=low   */
    PWM    /**< Drive current through coils     */
};

/**
 * @brief The PWM libraries accept a polarity parameter which determines
 *        whether a duty cycle of 100% is interpreted as a high voltage or a
 *        low voltage on the pin
 */
enum driveMode_e{
    ACTIVE_LOW, /**< Duty cycle of 100% is low voltage  */
    ACTIVE_HIGH /**< Duty cycle of 100% is high voltage */
};

/** @brief The states the camera synchronization LED can be in */
enum cameraState_e{
    ON = GPIO_PIN_SET,   /**< LED is on  */
    OFF = GPIO_PIN_RESET /**< LED is off */
};




/*********************************** Types ***********************************/
/** @brief Container for magnet state information */
typedef struct{
    enum magnets_e magnet;           /**< Magnet 1 or magnet 2               */
    enum magnetStates_e magnetState; /**< What the magnet is suppose to do   */
    enum driveMode_e driveMode;      /**< Channel polarity (i.e. whether 100%
                                      *   duty cycle is ON or OFF)           */
    float dutyCycle;                 /**< Only needed if magnetState is PWM  */
}MagnetInfo_t;




/********************************* Functions *********************************/
void controlInit(void);
void controlEventHandler(uint32_t notification);
void updateControlSignals(void);
void updateControlData(controlData_t* controlData);
void controlSetSignalsToIdleState(void);

inline void setCameraLEDState(enum cameraState_e state){
    HAL_GPIO_WritePin(Camera_LED_GPIO_Port, Camera_LED_Pin, state);
}

/**
 * @}
 */
/* end - ControlHeader */

#endif /* APP_CONTROL_H_ */

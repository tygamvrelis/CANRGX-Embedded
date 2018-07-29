/*
 * App_Control.c
 *
 *  Created on: Jul 28, 2018
 *      Author: Tyler
 */




/********************************** Includes *********************************/
#include "App/App_Control.h"




/***************************** Extern declarations ***************************/
extern osTimerId tmrLEDBlinkHandle;




/********************************* Constants *********************************/
static const float TEC_ON_DUTY_CYCLE = 0.85;




/***************************** Private Variables *****************************/
static float TEC1DutyCycle = 0;
static float TEC2DutyCycle = 0;

static MagnetInfo_t magnet1Info;
static MagnetInfo_t magnet2Info;

static enum flightEvents_e currentEvent = NONE;
static enum controllerStates_e controllerState = IDLE;
static enum controllerStates_e nextControllerState = EXPERIMENT1;

static TickType_t curTick;




/***************************** Private Functions *****************************/
static inline void MAGNET_MAKE_GPIO(GPIO_TypeDef* gpio, uint16_t magnet_pin){
    /* Force PWM pin function to GPIO
     *
     * Arguments: port, pin
     *
     * Returns: none
     */

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = magnet_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

static inline void MAGNET_MAKE_PWM(GPIO_TypeDef* gpio, uint16_t magnet_pin){
    /* Force PWM pin function to PWM
     *
     * Arguments: port, pin
     *
     * Returns: none
     */

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = magnet_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

static int8_t setMagnet(MagnetInfo_t* magnetInfo){
    /* Sets the state of the specified magnet to coast, break, or PWM in the direction
     * selected by the duty cycle.
     *
     * Arguments: magnetInfo, pointer to a struct that contains the configuration
     *            info for the magnet
     *
     * Returns: 1 if successful, otherwise a negative error code
     */

    static current_e current;

    /***** Non-PWM mode of operation *****/
    if(magnetInfo -> magnetState == COAST){
        current = CURRENT_NONE;
        if(magnetInfo -> magnet == MAGNET1){
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
            MAGNET_MAKE_GPIO(Magnet_1A_GPIO_Port, Magnet_1A_Pin);
            MAGNET_MAKE_GPIO(Magnet_1B_GPIO_Port, Magnet_1B_Pin);
            HAL_GPIO_WritePin(GPIOA, Magnet_1A_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOA, Magnet_1B_Pin, GPIO_PIN_SET);
        }
        else{ // if(magnetInfo -> magnet == MAGNET2)
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
            MAGNET_MAKE_GPIO(Magnet_2A_GPIO_Port, Magnet_2A_Pin);
            MAGNET_MAKE_GPIO(Magnet_2B_GPIO_Port, Magnet_2B_Pin);
            HAL_GPIO_WritePin(GPIOB, Magnet_2A_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB, Magnet_2B_Pin, GPIO_PIN_SET);
        }
        return 1;
    }
    else if(magnetInfo -> magnetState == BRAKE){
        current = CURRENT_NONE;
        if(magnetInfo -> magnet == MAGNET1){
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
            MAGNET_MAKE_GPIO(Magnet_1A_GPIO_Port, Magnet_1A_Pin);
            MAGNET_MAKE_GPIO(Magnet_1B_GPIO_Port, Magnet_1B_Pin);
            HAL_GPIO_WritePin(GPIOA, Magnet_1A_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, Magnet_1B_Pin, GPIO_PIN_RESET);
        }
        else{ // if(magnetInfo -> magnet == MAGNET2)
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
            MAGNET_MAKE_GPIO(Magnet_2A_GPIO_Port, Magnet_2A_Pin);
            MAGNET_MAKE_GPIO(Magnet_2B_GPIO_Port, Magnet_2B_Pin);
            HAL_GPIO_WritePin(GPIOB, Magnet_2A_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB, Magnet_2B_Pin, GPIO_PIN_RESET);
        }
        return 1;
    }


    /***** PWM modes *****/
    if((magnetInfo -> dutyCycle) < -1 || (magnetInfo -> dutyCycle) > 1){
        return -1;
    }

    current = (magnetInfo -> dutyCycle < 0) ? NEGATIVE : POSITIVE; // Compute polarity

    TIM_OC_InitTypeDef sConfigOC;
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = (fabs(magnetInfo -> dutyCycle)) * MAGNET_PWM_PERIOD;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if(magnetInfo -> magnet == MAGNET1){
        if(current == POSITIVE){
            // Magnet 1A is GPIO here and Magnet 1B is PWM
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1); // Magnet 1A

            if(magnetInfo -> driveMode == ACTIVE_LOW){
                sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
                MAGNET_MAKE_PWM(Magnet_1B_GPIO_Port, Magnet_1B_Pin);
                MAGNET_MAKE_GPIO(Magnet_1A_GPIO_Port, Magnet_1A_Pin);
                HAL_GPIO_WritePin(Magnet_1A_GPIO_Port, Magnet_1A_Pin, GPIO_PIN_SET);
            }
            else{
                sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
                MAGNET_MAKE_PWM(Magnet_1B_GPIO_Port, Magnet_1B_Pin);
                MAGNET_MAKE_GPIO(Magnet_1A_GPIO_Port, Magnet_1A_Pin);
                HAL_GPIO_WritePin(Magnet_1A_GPIO_Port, Magnet_1A_Pin, GPIO_PIN_RESET);
            }

            HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
        }
        else{ // if(current == NEGATIVE)
            // Magnet 1B is GPIO here and Magnet 1A is PWM
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2); // Magnet 1B

            if(magnetInfo -> driveMode == ACTIVE_LOW){
                sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
                MAGNET_MAKE_PWM(Magnet_1A_GPIO_Port, Magnet_1A_Pin);
                MAGNET_MAKE_GPIO(Magnet_1B_GPIO_Port, Magnet_1B_Pin);
                HAL_GPIO_WritePin(Magnet_1B_GPIO_Port, Magnet_1B_Pin, GPIO_PIN_SET);
            }
            else{
                sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
                MAGNET_MAKE_PWM(Magnet_1A_GPIO_Port, Magnet_1A_Pin);
                MAGNET_MAKE_GPIO(Magnet_1B_GPIO_Port, Magnet_1B_Pin);
                HAL_GPIO_WritePin(Magnet_1B_GPIO_Port, Magnet_1B_Pin, GPIO_PIN_RESET);
            }

            HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
        }
    }
    else{ // if(magnetInfo -> magnet == MAGNET2)
        if(current == POSITIVE){
            // Magnet 2A is GPIO here and Magnet 2B is PWM
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3); // Magnet 2A

            if(magnetInfo -> driveMode == ACTIVE_LOW){
                sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
                MAGNET_MAKE_PWM(Magnet_2B_GPIO_Port, Magnet_2B_Pin);
                MAGNET_MAKE_GPIO(Magnet_2A_GPIO_Port, Magnet_2A_Pin);
                HAL_GPIO_WritePin(Magnet_2A_GPIO_Port, Magnet_2A_Pin, GPIO_PIN_SET);
            }
            else{
                sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
                MAGNET_MAKE_PWM(Magnet_2B_GPIO_Port, Magnet_2B_Pin);
                MAGNET_MAKE_GPIO(Magnet_2A_GPIO_Port, Magnet_2A_Pin);
                HAL_GPIO_WritePin(Magnet_2A_GPIO_Port, Magnet_2A_Pin, GPIO_PIN_RESET);
            }

            HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4);
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
        }
        else{ // if(current == NEGATIVE)
            // Magnet 2B is GPIO here and Magnet 2A is PWM
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4); // Magnet 2B

            if(magnetInfo -> driveMode == ACTIVE_LOW){
                sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
                MAGNET_MAKE_PWM(Magnet_2A_GPIO_Port, Magnet_2A_Pin);
                MAGNET_MAKE_GPIO(Magnet_2B_GPIO_Port, Magnet_2B_Pin);
                HAL_GPIO_WritePin(Magnet_2B_GPIO_Port, Magnet_2B_Pin, GPIO_PIN_SET);
            }
            else{
                sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
                MAGNET_MAKE_PWM(Magnet_2A_GPIO_Port, Magnet_2A_Pin);
                MAGNET_MAKE_GPIO(Magnet_2B_GPIO_Port, Magnet_2B_Pin);
                HAL_GPIO_WritePin(Magnet_2B_GPIO_Port, Magnet_2B_Pin, GPIO_PIN_RESET);
            }

            HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3);
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
        }
    }

    return 1;
}

static int8_t TEC_set_valuef(float TEC_Top_duty_cycle, float TEC_Bot_duty_cycle){
    /* Sets the PWM duty cycle used to drive the top and bottom TECs used for
     * heating the parafluid.
     *
     * Arguments: the duty cycle for the top and bottom tec, respectively,
     *            indicating what fraction of a period each one should be
     *            on for (arguments in range [0, 1] are valid)
     *
     * Returns: 1 if successful, otherwise a negative error code
     */

    // Check argument validity
    if(TEC_Top_duty_cycle < 0 || TEC_Top_duty_cycle > 1){
        return -1;
    }

    if(TEC_Bot_duty_cycle < 0 || TEC_Bot_duty_cycle > 1){
        return -2;
    }

    TIM_OC_InitTypeDef sConfigOC;

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = TEC_Top_duty_cycle * TEC_PWM_PERIOD;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_1) != HAL_OK){
        /* The call to HAL_TIME_PWM_ConfigChannel failed, so one or more arguments passed
         * to it must be invalid. Handle the error here. */
        return -3;
    }
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);

    sConfigOC.Pulse = TEC_Bot_duty_cycle * TEC_PWM_PERIOD;
    if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_2) != HAL_OK){
        /* The call to HAL_TIME_PWM_ConfigChannel failed, so one or more arguments passed
         * to it must be invalid. Handle the error here. */
        return -4;
    }
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);

    return 1;
}

static void TEC_stop(void){
    /* Turns off the timer channels used for TEC PWM.
     *
     * Arguments: none
     *
     * Returns: none
     */

    HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);
}

static void processReceivedEvent(uint32_t receivedEvent){
    switch(receivedEvent){
        case REDUCEDGRAVITY:
            controllerState = nextControllerState;

            TEC1DutyCycle = TEC_ON_DUTY_CYCLE;
            TEC2DutyCycle = TEC_ON_DUTY_CYCLE;
            TEC_set_valuef(TEC1DutyCycle, TEC2DutyCycle);

            magnet1Info.magnetState = PWM;
            magnet2Info.magnetState = PWM;

            // Make status LED blink at 10 Hz
            osTimerStop(tmrLEDBlinkHandle);
            osTimerStart(tmrLEDBlinkHandle, 50);
            break;
        case NONE:
            controllerState = IDLE;

            // The next controller state will increment until the last state,
            // at which point it will wrap around
            switch(nextControllerState){
                case IDLE:
                case EXPERIMENT1:
                case EXPERIMENT2:
                case EXPERIMENT3:
                    nextControllerState++;
                    break;
                case EXPERIMENT4:
                    nextControllerState = EXPERIMENT1;
                    break;
            }

            controlSetSignalsToIdleState();

            // Make status LED blink at 2 Hz
            osTimerStop(tmrLEDBlinkHandle);
            osTimerStart(tmrLEDBlinkHandle, 1000);
            break;
        default:
            break; // Should never reach here
    }
}




/***************************** Public Functions ******************************/
void controlInit(void){
    magnet1Info.magnet = MAGNET1;
    magnet1Info.magnetState = BRAKE;
    magnet1Info.driveMode = ACTIVE_HIGH;
    magnet1Info.dutyCycle = 0.0;

    magnet2Info.magnet = MAGNET2;
    magnet2Info.magnetState = BRAKE;
    magnet2Info.driveMode = ACTIVE_HIGH;
    magnet2Info.dutyCycle = 0.0;

    TEC_stop();
    setMagnet(&magnet1Info);
    setMagnet(&magnet2Info);

    currentEvent = NONE;
    controllerState = IDLE;
    nextControllerState = EXPERIMENT1;
}

void controlEventHandler(uint32_t notification){
    enum flightEvents_e receivedEvent = NONE;
    bool manualOverrideStart = false;

    // One-time state update for the event. The cases towards the end have the highest
    // priority if they occur, which is why they are if statements and not if-else
    if((notification & MPU_BITMASK) == MPU_BITMASK){
        // This is the case for when the MPU9250 senses an event. In this case, the
        // task notification holds the value of the enumerated type flightEvents_e
        // sent by the MPU9250 thread.
        receivedEvent = notification & (~MPU_BITMASK);
    }
    if((notification & MANUAL_OVERRIDE_START_BITMASK) == MANUAL_OVERRIDE_START_BITMASK){
        // This is the case for when a manual override START sequence is received. In
        // this case, the task notification holds the number of the experiment to run.
        receivedEvent = REDUCEDGRAVITY;
        nextControllerState = notification & (~MANUAL_OVERRIDE_START_BITMASK);
        manualOverrideStart = true;
    }
    if((notification & MANUAL_OVERRIDE_STOP_BITMASK) == MANUAL_OVERRIDE_STOP_BITMASK){
        // This is the case for when a manual override STOP sequence is received
        receivedEvent = NONE;
    }

    // Only handle the received event if it is different than the
    // current event. That is, if the current event is NONE, then if
    // we receive another "NONE" event, we should do nothing. The only
    // exception is with manual override start, since it is possible
    // that it may be desired to change the experiment number without
    // pressing stop in between.
    if(receivedEvent != currentEvent || manualOverrideStart)
    {
        currentEvent = receivedEvent;
        manualOverrideStart = false;

        processReceivedEvent(receivedEvent);
    }
}

void updateControlSignals(void){
    // Update PWM duty cycle for magnets
    switch(controllerState){
        case IDLE:
            break;
        case EXPERIMENT1:
            curTick = xTaskGetTickCount();
            magnet1Info.dutyCycle = sinf(0.002 * curTick);
            magnet2Info.dutyCycle = sinf(0.002 * curTick);
            setMagnet(&magnet1Info);
            setMagnet(&magnet2Info);
            break;
        case EXPERIMENT2:
            curTick = xTaskGetTickCount();
            magnet1Info.dutyCycle = (1.0 + sinf(0.002 * curTick)) / 2.0;
            magnet2Info.dutyCycle = (1.0 + sinf(0.002 * curTick)) / 2.0;
            setMagnet(&magnet1Info);
            setMagnet(&magnet2Info);
            break;
        case EXPERIMENT3:
            curTick = xTaskGetTickCount();
            magnet1Info.dutyCycle = sinf(0.002 * curTick);
            magnet2Info.dutyCycle = cosf(0.002 * curTick);
            setMagnet(&magnet1Info);
            setMagnet(&magnet2Info);
            break;
        case EXPERIMENT4:
            curTick = xTaskGetTickCount();
            magnet1Info.dutyCycle = (1.0 + sinf(0.002 * curTick)) / 2.0;
            magnet2Info.dutyCycle = (1.0 + cosf(0.002 * curTick)) / 2.0;
            setMagnet(&magnet1Info);
            setMagnet(&magnet2Info);
            break;
        default:
            break; // Should never reach here
    }
}

void updateControlData(controlData_t* controlData){
    // Here, we multiply the duty cycle by 100 * 100 so that we capture the
    // integer part and the fractional part (to 2 decimal places)
    controlData->mag1Power = (int16_t)(magnet1Info.dutyCycle * 10000);
    controlData->mag2Power = (int16_t)(magnet2Info.dutyCycle * 10000);
    controlData->tec1Power = (uint16_t)(TEC1DutyCycle * 10000);
    controlData->tec2Power = (uint16_t)(TEC2DutyCycle * 10000);
}


void controlSetSignalsToIdleState(){
    TEC1DutyCycle = 0;
    TEC2DutyCycle = 0;
    TEC_stop();

    magnet1Info.magnetState = BRAKE;
    magnet2Info.magnetState = BRAKE;
    magnet1Info.dutyCycle = 0;
    magnet2Info.dutyCycle = 0;
    setMagnet(&magnet1Info);
    setMagnet(&magnet2Info);
}

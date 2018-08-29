/**
 * @file App_Control.c
 * @author Tyler
 * @brief All the functions related to generating control signals
 *
 * @defgroup Control Control
 * @brief Generating control signals for the electromagnets, TECs, and LEDs
 *        for status indication and camera synchronization
 * @{
 */




/********************************** Includes *********************************/
#include "App/App_Control.h"
#include "App/App_Math_Helpers.h"




/***************************** Extern declarations ***************************/
extern osTimerId tmrLEDBlinkHandle; /**< OS timer handle for status LED */
extern osTimerId tmrCameraLEDHandle; /**< OS timer handle for camera LED */




/********************************* Constants *********************************/
/** When the TECs are on, they are given PWM signals with this duty cycle */
static const float TEC_ON_DUTY_CYCLE = 1.00;




/*********************************** Types ***********************************/
/** Indicates the polarity of current through the magnets */
typedef enum{
    CURRENT_NONE, /**< No current (idle) */
    POSITIVE,     /**< Positive polarity */
    NEGATIVE      /**< Negative polarity */
}current_e;




/***************************** Private Variables *****************************/
static float TEC1DutyCycle = 0; /**< State variable -- TEC 1 PWM duty cycle */
static float TEC2DutyCycle = 0; /**< State variable -- TEC 2 PWM duty cycle */

static MagnetInfo_t magnet1Info; /**< State variable -- Magnet 1 container */
static MagnetInfo_t magnet2Info; /**< State variable -- Magnet 1 container */

/** State variable -- The current event driving execution */
static enum flightEvents_e currentEvent = NONE;

/** State variable -- The experiment currently being executed */
static enum controllerStates_e controllerState = IDLE;

/**
 * State variable -- The experiment to be executed on the next reduced gravity
 * cycle
 */
static enum controllerStates_e nextControllerState = EXPERIMENT1;

static TickType_t curTick; /**< State variable -- Current system time */




/***************************** Private Functions *****************************/
/**
 * @defgroup ControlPrivateFunctions Private Functions
 * @brief Functions used internally
 * @ingroup Control
 * @{
 */

/**
 * @brief Force PWM pin function to GPIO
 * @param gpio The port for the pin whose function is to be made GPIO
 * @param magnet_pin The pin on whose function is to be made GPIO
 */
static inline void MAGNET_MAKE_GPIO(GPIO_TypeDef* gpio, uint16_t magnet_pin){
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = magnet_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

/**
 * @brief Force PWM pin function to PWM
 * @param gpio The port for the pin whose function is to be made PWM
 * @param magnet_pin The pin on whose function is to be made PWM
 */
static inline void MAGNET_MAKE_PWM(GPIO_TypeDef* gpio, uint16_t magnet_pin){
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = magnet_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

/**
 * @brief  Sets the state of the specified magnet to coast, break, or PWM in
 *         the direction selected by the duty cycle
 * @param  magnetInfo Pointer to a struct that contains the configuration info
 *         for the magnet
 * @return 1 if successful, otherwise a negative error code
 */
static int8_t setMagnet(MagnetInfo_t* magnetInfo){
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

/**
 * @brief   Sets the PWM duty cycle used to drive the top and bottom TECs used
 *          for heating the parafluid
 * @details Arguments in range [0, 1] are valid, as they are used to indicate
 *          what fraction of a period each TEC should be on
 * @param   TEC_Top_duty_cycle The duty cycle for the top TEC
 * @param   TEC_Bot_duty_cycle The duty cycle for the bottom TEC
 * @return  1 if successful, otherwise a negative error code
 */
static int8_t TEC_set_valuef(float TEC_Top_duty_cycle, float TEC_Bot_duty_cycle){
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

/**
 * @brief  Stops the TECs
 * @note   Sets the PWM duty cycle to 0 for both TECs
 * @return none
 */
static void TEC_stop(void){
    TEC_set_valuef(0.0, 0.0);
}

/**
 * @brief   Given an event that is different that the current event, this
 *          function handles the event by updating the controller state
 *          accordingly
 * @details When the state is updated to REDUCEDGRAVITY, PWM is initiated for
 *          the magnets and TECs, and the LED connected to LD2 is blinked at 10
 *          Hz. When the state is updated to NONE, the control signals are set
 *          to 0 duty cycle, and the LED connected to LD2 is blinked at 0.5 Hz
 * @param   receivedEvent The value which encodes the event
 */
static void processReceivedEvent(enum flightEvents_e receivedEvent){
    switch(receivedEvent){
        case REDUCEDGRAVITY:
            controllerState = nextControllerState;

            // One-time update for the new state
            switch(controllerState){
                case IDLE:
                case EXPERIMENT1:
                    // Experiment 1 is a baseline
                    break;
                case EXPERIMENT0:
                case EXPERIMENT2:
                case EXPERIMENT3:
                case EXPERIMENT4:
                case EXPERIMENT5:
                case EXPERIMENT6:
                case EXPERIMENT7:
                case EXPERIMENT8:
                case EXPERIMENT9:
                case EXPERIMENT10:
                    // These experiments involve magnetism and heating
                    TEC1DutyCycle = TEC_ON_DUTY_CYCLE;
                    TEC2DutyCycle = TEC_ON_DUTY_CYCLE;
                    TEC_set_valuef(TEC1DutyCycle, TEC2DutyCycle);

                    magnet1Info.magnetState = PWM;
                    magnet2Info.magnetState = PWM;
                    break;
                default:
                    break;
            }

            // Make status LED blink at 10 Hz
            osTimerStop(tmrLEDBlinkHandle);
            osTimerStart(tmrLEDBlinkHandle, 50);

            // Turn on camera synchronization LED for about 3 frames
            setCameraLEDState(ON);
            osTimerStop(tmrCameraLEDHandle);
            osTimerStart(tmrCameraLEDHandle, 100);
            break;
        case NONE:
            controllerState = IDLE;

            // The next controller state will increment until the last state,
            // at which point it will wrap around
            switch(nextControllerState){
                case IDLE:
                    nextControllerState = EXPERIMENT0;
                    break;
                case EXPERIMENT0:
                case EXPERIMENT1:
                case EXPERIMENT2:
                case EXPERIMENT3:
                case EXPERIMENT4:
                case EXPERIMENT5:
                case EXPERIMENT6:
                case EXPERIMENT7:
                case EXPERIMENT8:
                case EXPERIMENT9:
                    nextControllerState++;
                    break;
                case EXPERIMENT10:
                    nextControllerState = EXPERIMENT0;
                    break;
                default:
                    break;
            }

            controlSetSignalsToIdleState();

            // Make status LED blink at 0.5 Hz
            osTimerStop(tmrLEDBlinkHandle);
            osTimerStart(tmrLEDBlinkHandle, 1000);

            // Turn off the camera synchronization LED (it is probably already
            // off due to the timer callback, but we do it anyway just to make
            // sure)
            setCameraLEDState(OFF);
            osTimerStop(tmrCameraLEDHandle);
            break;
        default:
            break; // Should never reach here
    }
}

/**
 * @}
 */
/* end - ControlPrivateFunctions */




/***************************** Public Functions ******************************/
/**
 * @defgroup ControlPublicFunctions Public Functions
 * @brief Functions used externally
 * @ingroup Control
 * @{
 */

/**
 * @brief  Initializes the control state to IDLE, and asserts all control
 *         signals accordingly
 * @return none
 */
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
    nextControllerState = EXPERIMENT0;
}

/**
 * @brief   Given a task notification value, this function extracts the command
 *          or event information from it, then conditionally passes the
 *          extracted event to processReceivedEvent for handling
 * @details The call to processReceivedEvent occurs if the received event is
 *          different than the current event, or the received event is
 *          manual override start
 * @param   notification The notification value passed to the control task
 */
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

        // Notification value (indicating experiment number)
        nextControllerState = (notification & (~MANUAL_OVERRIDE_START_BITMASK));
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

/**
 * @brief Updates the values of the time-varying control signals, using
 *        different functions of time depending on the experiment
 * @note  The control signals for the magnets are the only ones that are time-
 *        varying
 */
void updateControlSignals(void){
    uint32_t period_ms;
    float val;

    bool updateMagnets = true;
    curTick = xTaskGetTickCount();

    switch(controllerState){
        case IDLE:
            updateMagnets = false;
            break;
        case EXPERIMENT0:
            magnet1Info.dutyCycle = 1.0;
            magnet2Info.dutyCycle = 0.0;
            break;
        case EXPERIMENT1:
            // Baseline run: no magnetic field, no TECs
            updateMagnets = false;
            break;
        case EXPERIMENT2:
            magnet1Info.dutyCycle = 1.0;
            magnet2Info.dutyCycle = -0.1;
            break;
        case EXPERIMENT3:
            magnet1Info.dutyCycle = 1.0;
            magnet2Info.dutyCycle = -0.3;
            break;
        case EXPERIMENT4:
            magnet1Info.dutyCycle = 1.0;
            magnet2Info.dutyCycle = -0.5;
            break;
        case EXPERIMENT5:
            // AC trapezoid anti-Helmholtz (in-phase)
            period_ms = 2000;
            val = acTrapezoid(curTick, (uint32_t)period_ms, 90.0, 1.0);

            magnet1Info.dutyCycle = val;
            magnet2Info.dutyCycle = val;
            break;
        case EXPERIMENT6:
            magnet1Info.dutyCycle = 1.0;
            magnet2Info.dutyCycle = -0.1;
            break;
        case EXPERIMENT7:
            magnet1Info.dutyCycle = 1.0;
            magnet2Info.dutyCycle = -0.3;
            break;
        case EXPERIMENT8:
            magnet1Info.dutyCycle = 1.0;
            magnet2Info.dutyCycle = -0.5;
            break;
        case EXPERIMENT9:
            // AC trapezoid Helmholtz (180 degrees out of phase)
            period_ms = 2000;
            val = acTrapezoid(curTick, (uint32_t)period_ms, 90.0, 1.0);

            magnet1Info.dutyCycle = val;
            magnet2Info.dutyCycle = -1.0 * val;
            break;
        case EXPERIMENT10:
            magnet1Info.dutyCycle = 1.0;
            magnet2Info.dutyCycle = 1.0;
            break;
        default:
            break; // Should never reach here
    }

    if(updateMagnets){
        setMagnet(&magnet1Info);
        setMagnet(&magnet2Info);
    }
}

/**
 * @brief  Updates the control data container with the most recent signal values
 * @note   All fields are multiplied by 10,000 so that we capture the integer
 *         part, and the fractional part to 2 decimal places
 * @param  controlData Pointer to the control data container
 */
void updateControlData(controlData_t* controlData){
    controlData->mag1Power = (int16_t)(magnet1Info.dutyCycle * 10000);
    controlData->mag2Power = (int16_t)(magnet2Info.dutyCycle * 10000);
    controlData->tec1Power = (uint16_t)(TEC1DutyCycle * 10000);
    controlData->tec2Power = (uint16_t)(TEC2DutyCycle * 10000);
    controlData->state     = controllerState;
}

/**
 * @brief  Sets the control signals to their idle state. This turns off the
 *         PWM for the TECs (0 duty cycle) and brakes the magnets
 * @return none
 */
void controlSetSignalsToIdleState(void){
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

/**
 * @}
 */
/* end - ControlPublicFunctions */

/**
 * @}
 */
/* end - Control */

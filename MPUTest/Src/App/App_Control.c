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
static inline void HeatingWireOn(){
    HAL_GPIO_WritePin(Heating_Wire_GPIO_Port, Heating_Wire_Pin, GPIO_PIN_SET);
}

static inline void HeatingWireOff(){
    HAL_GPIO_WritePin(Heating_Wire_GPIO_Port, Heating_Wire_Pin, GPIO_PIN_RESET);
}

static void processReceivedEvent(uint32_t receivedEvent){
    switch(receivedEvent){
        case REDUCEDGRAVITY:
            controllerState = nextControllerState;

            TEC1DutyCycle = TEC_ON_DUTY_CYCLE;
            TEC2DutyCycle = TEC_ON_DUTY_CYCLE;
            TEC_set_valuef(TEC1DutyCycle, TEC2DutyCycle);

            HeatingWireOn();

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

            TEC1DutyCycle = 0;
            TEC2DutyCycle = 0;
            TEC_stop();

            HeatingWireOff();

            magnet1Info.magnetState = BRAKE;
            magnet2Info.magnetState = BRAKE;
            magnet1Info.dutyCycle = 0;
            magnet2Info.dutyCycle = 0;
            setMagnet(&magnet1Info);
            setMagnet(&magnet2Info);

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
    HeatingWireOff();
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

void updateControlState(void){
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

/*
 * App_CommRX.c
 *
 *  Created on: Jul 29, 2018
 *      Author: Tyler
 */

/********************************** Includes *********************************/
#include "App/App_CommRX.h"




/***************************** Extern declarations ***************************/
extern osThreadId ControlTaskHandle;
extern UART_HandleTypeDef huart2;




/********************************* Constants *********************************/
static const char MANUAL_OVERRIDE_START_CHAR = 'S';
static const char MANUAL_OVERRIDE_STOP_SEQ[] = { 'X', 'X' };
static const char RESET_SEQ[] = { 'R', 'S' };




/***************************** Private Variables *****************************/
/**
 * Buffer for received commands. All such commands are a fixed size of 3
 * characters.
 *
 * buffer[0] == control character,
 * buffer[1] == accompanying data or additional control character,
 * buffer[2] == '\n'
 * */
static uint8_t buffer[3];




/***************************** Private Functions *****************************/
static inline uint32_t NOTIFY_FROM_MANUAL_OVERRIDE_START(uint32_t x){
    return MANUAL_OVERRIDE_START_BITMASK | x;
}




/***************************** Public Functions ******************************/
void commRXInitReception(void){
    HAL_UART_Receive_IT(&huart2, buffer, sizeof(buffer));
}

void commRXEventHandler(void){
    if(buffer[0] == MANUAL_OVERRIDE_START_CHAR){
        // Manual override for starting experiment
        xTaskNotify(ControlTaskHandle,
                    NOTIFY_FROM_MANUAL_OVERRIDE_START(buffer[1] - '0'),
                    eSetBits
        );
    }
    else if(buffer[0] == MANUAL_OVERRIDE_STOP_SEQ[0] &&
            buffer[1] == MANUAL_OVERRIDE_STOP_SEQ[1])
    {
        // Manual override for stopping experiment
        xTaskNotify(ControlTaskHandle,
                    MANUAL_OVERRIDE_STOP_BITMASK,
                    eSetBits
        );
    }
    else if(buffer[0] == RESET_SEQ[0] && buffer[1] == RESET_SEQ[1]){
        // Enter critical section; disable interrupts
        taskENTER_CRITICAL();

        // Brake the magnets and turn off the TECs
        controlSetSignalsToIdleState();

        // Wait 1 second for the magnetic field energy to start
        // dissipating safely. This time is also sufficient to allow
        // existing I2C transactions with the MPU9250 to finish.
        HAL_Delay(1000);

        // Full system reset
        NVIC_SystemReset();
    }
}


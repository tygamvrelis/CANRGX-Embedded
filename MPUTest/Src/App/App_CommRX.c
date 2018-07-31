/**
 * @file App_CommRX.c
 * @author Tyler
 * @brief Functions and data related to the reception side of PC communication
 *
 * @defgroup CommRX
 * @{
 */

/********************************** Includes *********************************/
#include "App/App_CommRX.h"




/***************************** Extern declarations ***************************/
extern osThreadId ControlTaskHandle;
extern UART_HandleTypeDef huart2;




/********************************* Constants *********************************/
/**
 * @brief The manual override start command contains an 'S' followed by the
 *        number of the experiment to be started
 */
static const char MANUAL_OVERRIDE_START_CHAR = 'S';

/**
 * @brief  The manual override stop command contains this sequence as its first
 *         2 characters
 */
static const char MANUAL_OVERRIDE_STOP_SEQ[] = { 'X', 'X' };

/**
 * @brief The reset command contains this sequence as its first two characters
 */
static const char RESET_SEQ[] = { 'R', 'S' };




/***************************** Private Variables *****************************/
/**
 * @brief   Buffer for received commands. All such commands are a fixed size of
 *          3 characters.
 * @details
 *  -# `buffer[0]` == control character,
 *  -# `buffer[1]` == accompanying data or additional control character,
 *  -# `buffer[2]` == '\\n'
 */
static uint8_t buffer[3];




/***************************** Private Functions *****************************/
/**
 * @defgroup CommRXPrivateFunctions Communication: RX Private Functions
 * @ingroup CommRX
 * @{
 */

/**
 * @brief Helper method to pass an experiment number to the control task while
 *        simultaneously passing in the manual override start code
 */
static inline uint32_t NOTIFY_FROM_MANUAL_OVERRIDE_START(uint32_t x){
    return MANUAL_OVERRIDE_START_BITMASK | x;
}

/**
 * @}
 */
/* end - CommRXPrivateFunctions */




/***************************** Public Functions ******************************/
/**
 * @defgroup CommRXPublicFunctions Communication: RX Public Functions
 * @ingroup CommRX
 * @{
 */

/** Initiates an interrupt-based reception of `sizeof(buffer)` bytes */
void commRXInitReception(void){
    HAL_UART_Receive_IT(&huart2, buffer, sizeof(buffer));
}

/**
 * @brief   Upon receiving a full buffer of bytes, this function will be
 *          invoked from the PC RX thread to parse the data in the buffer and
 *          take the appropriate action depending on the message
 * @details There are 4 cases:
 *   -# Manual override start: notify the control task and pass in the
 *      experiment start number with the manual override start bitmask
 *   -# Manual override stop: notify the control task with the manual override
 *      stop bitmask
 *   -# Manual hard reset: enter a critical section, set control signals to
 *      idle, wait 1 s for the magnetic field to drain its energy, then perform
 *      a full system reset
 *   -# Invalid case; ignore it
 */
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

/** @brief Aborts the current interrupt-based reception */
void commRXCancelReception(void){
    HAL_UART_AbortReceive_IT(&huart2);
}

/**
 * @}
 */
/* end - CommRXPublicFunctions */

/**
 * @}
 */
/* end - CommRX */

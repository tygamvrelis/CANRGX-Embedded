/**
 * @file App_MPU9250.h
 * @author Tyler
 *
 * @defgroup MPU9250_App_Header MPU9250 Application Code Header
 * @ingroup MPU9250_App
 */

#ifndef APP_MPU9250_H_
#define APP_MPU9250_H_




/********************************** Includes *********************************/
#include <math.h> // For NAN
#include "../../Drivers/MPU9250/MPU9250.h"
#include "userTypes.h"




/******************************* Public variables ****************************/
extern MPU9250_t myMPU9250;




/********************************* Functions *********************************/
/**
 * @brief  Applies the MPU9250 notification bitmask to a word of data
 * @param  x The word of data to which the MPU9250 bitmask should be applied
 * @return The logical OR of MPU_BITMASK and the word x
 */
inline uint32_t NOTIFY_FROM_MPU(uint32_t x){
    return MPU_BITMASK | x;
}

void updateAccelReading(
        accelerometerData_t* accelData,
        MPU9250_t* myMPU9250
);

void updateMagReading(
        magnetometerData_t* accelData,
        MPU9250_t* myMPU9250
);

void MPU9250EventHandler(MPU9250_t* myMPU9250);

#endif /* APP_MPU9250_H_ */

/*
 * App_MPU9250.h
 *
 *  Created on: Jul 28, 2018
 *      Author: Tyler
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

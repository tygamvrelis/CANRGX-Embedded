/*
 * MPU9250.h
 *
 *  Created on: April 8, 2018
 *      Author: Tyler
 */

#ifndef MPU9250_H_
#define MPU9250_H_




/********************************** Includes *********************************/
#include <math.h>
#include "cmsis_os.h"
#include "i2c.h"
#include "MPUFilter.h"
#include "MPU9250_t.h"




/********************************* Functions *********************************/
// These 3 should only be used before the scheduler is started
int MPU9250Init(MPU9250_t* myMPU);
void resetIMUBlocking(void);
void resetMagnetometerBlocking(void);

// These 3 are the only ones that should really be used during runtime once the
// scheduler has started.
int accelReadDMA(MPU9250_t* myMPU, osSemaphoreId sem);
int gyroReadDMA(MPU9250_t* myMPU, osSemaphoreId sem);
int magFluxReadDMA(MPU9250_t* myMPU, osSemaphoreId sem);

inline void computeTotalAcceleration(MPU9250_t* myMPU9250){
    myMPU9250->A = sqrt(myMPU9250->az * myMPU9250->az +
                        myMPU9250->ay * myMPU9250->ay +
                        myMPU9250->ax * myMPU9250->ax
    );
}

void generateClocks(I2C_HandleTypeDef* hi2c, uint8_t numClocks, uint8_t sendStopBits); // Use with caution

#endif /* MPU9250_H_ */

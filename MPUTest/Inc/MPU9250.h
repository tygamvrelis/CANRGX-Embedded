/*
 * MPU9250.h
 *
 *  Created on: April 8, 2018
 *      Author: Tyler
 */

/************************* Prevent recursive inclusion ************************/
#ifndef MPU9250_H_
#define MPU9250_H_




/********************************** Includes **********************************/
#include "stm32f4xx_hal.h"
#include <math.h>




/*********************************** Macros ***********************************/
// MPU9250_ACCEL_AND_GYRO_ADDR is the slave address that should be used when trying
// to get the acceleration and gyroscope data. If the magnetometer data is to be
// read, the slave address needs to be MPU9250_MAG_ADDR
#define MPU9250_ACCEL_AND_GYRO_ADDR 0x68 // pg. 32 of datasheet (might also be 0x69 depending on the AD0 pin on the PCB)
#define MPU9250_MAG_ADDR 0x0C // pg. 24 of datasheet

// Register addresses for accelerometer data (pg. 8 register map)
#define MPU9250_ACCEL_X_ADDR_H 0x3B // AXH
#define MPU9250_ACCEL_X_ADDR_L 0x3C // AXL
#define MPU9250_ACCEL_Y_ADDR_H 0x3D // AYH
#define MPU9250_ACCEL_Y_ADDR_L 0x3E // AYL
#define MPU9250_ACCEL_Z_ADDR_H 0x3F // AZH
#define MPU9250_ACCEL_Z_ADDR_L 0x40 // AZL

// Register addresses for gyroscope data (pg. 8 register map)
#define MPU9250_GYRO_X_ADDR_H 0x43 // VXH
#define MPU9250_GYRO_X_ADDR_L 0x44 // VXL
#define MPU9250_GYRO_Y_ADDR_H 0x45 // VYH
#define MPU9250_GYRO_Y_ADDR_L 0x46 // VYL
#define MPU9250_GYRO_Z_ADDR_H 0x47 // VZH
#define MPU9250_GYRO_Z_ADDR_L 0x48 // VZL

// Register addresses for magnetometer data (pg. 47 register map)
#define MPU9250_MAG_X_ADDR_L 0x03 // HXL
#define MPU9250_MAG_X_ADDR_H 0x04 // HXH
#define MPU9250_MAG_Y_ADDR_L 0x05 // HYL
#define MPU9250_MAG_Y_ADDR_H 0x06 // HYH
#define MPU9250_MAG_Z_ADDR_L 0x07 // HZL
#define MPU9250_MAG_Z_ADDR_H 0x08 // HZH

// Scales for readings
#define MPU9250_ACCEL_FULL_SCALE 2 * g // twice the gravitationa acceleration due to Earth
#define MPU9250_GYRO_FULL_SCALE 250 // degree/s
#define MPU9250_MAG_FULL_SCALE 4800 // microTeslas




/*********************************** Types ************************************/
typedef struct{
	float az; // Acceleration along z-axis
	float vy; // Pitch rate about y-axis
	float hx; // Magnetic field along x
	float hy; // Magnetic field along y
	float hz; // Magnetic field along z
}MPU9250_t;





/*********************************** Globals *********************************/
extern MPU9250_t* myMPU9250;
extern const float g;




/********************************* Functions *********************************/
int MPU9250Init(MPU9250_t* myMPU);

#endif /* MPU9250_H_ */
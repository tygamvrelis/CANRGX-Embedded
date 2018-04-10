/*
 * MPU9250.c
 *
 *  Created on: April 8, 2018
 *      Author: Tyler
 */

/********************************** Includes *********************************/
#include "MPU9250.h"



/*********************************** Globals *********************************/
MPU9250_t myMPU9250; // Global MPU9250 object
const float g = 9.807; // Acceleration due to gravity on Earth




/********************************* Functions *********************************/
int MPU9250Init(MPU9250_t* myMPU){
	/* Initializes the sensor object passed in.
	 *
	 * Arguments: pointer to MPU6050_t
	 *
	 * Returns: 1 if successful, returns a negative error code otherwise
	 */

	myMPU -> az = -1.0;
	myMPU -> vy = -1.0;
	myMPU -> hx = -1.0;
	myMPU -> hy = -1.0;
	myMPU -> hz = -1.0;




	/********** Configure accelerometer and gyroscope **********/
	// Use the best available clock source
	uint8_t dataToWrite = 0x01;
	uint8_t stat = HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 10000);
	if(stat != HAL_OK){
		return -1;
	}

	// Enable I2C master interface module
	dataToWrite = 0x20;
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, USER_CTRL, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -2;
	}


	// Set I2C module to use 400 kHz speed (pg. 19 of register map)
	dataToWrite = 0x0D;
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_MST_CTRL, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -3;
	}

	// Force accelerometer and gyroscope to ON
	dataToWrite = 0x00;
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, PWR_MGMT_2, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -4;
	}

	return 1;
}

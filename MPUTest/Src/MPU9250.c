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
int magnetometerWrite(uint8_t addr, uint8_t data);

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




	/********** Check that MPU9250 is connected **********/
	uint8_t buff[1];
	// Check for bus communication essentially. If any function should fail and issue an early return, it would most likely
	// be this one.
	if(HAL_FMPI2C_Mem_Read(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, WHO_AM_I, I2C_MEMADD_SIZE_8BIT, buff, 1, 100) != HAL_OK){
		return -1;
	}

	// Check that the WHO_AM_I register is 0x71
	if(buff[0] != 0x71){
		return -2;
	}



	/********** Configure accelerometer and gyroscope **********/
	// Use the best available clock source
	uint8_t dataToWrite = 0x01;
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -3;
	}

	// Enable I2C master interface module
	dataToWrite = 0x20;
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, USER_CTRL, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -4;
	}


	// Set I2C module to use 400 kHz speed (pg. 19 of register map)
	dataToWrite = 0x0D;
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_MST_CTRL, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -5;
	}

	// Force accelerometer and gyroscope to ON
	dataToWrite = 0x00;
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, PWR_MGMT_2, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -6;
	}

	// Enable I2C bypass
	dataToWrite = 0x02;
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, INT_PIN_CFG, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -7;
	}

	// Note: Changing the filter bandwidth didn't have a noticeable effect as far as I could tell
//	// Set accelerometer bandwidth 218 Hz
//	dataToWrite = 0x01;
//	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, ACCEL_CONFIG_2, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
//		return -8;
//	}
//
//	// Set gyroscope bandwidth to 3600 Hz
//	dataToWrite = 0x01;
//	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, CONFIG, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
//		return -9;
//	}




	/********** Configure magnetometer **********/
	// Check that correct device ID is read
	magnetometerRead(WIA, 1, buff);
	magnetometerRead(WIA, 1, buff); // For some read, reading didn't always work the very first time
	if(buff[0] != 0x48){
		return -10;
	}

	// Reset
	if(magnetometerWrite(CNTL2, 0x01) != 1){
		return -11;
	}

	HAL_Delay(10); // Resetting might take some time

	// Enable continuous measurement
	if(magnetometerWrite(CNTL1, 0x16) != 1){
		return -12;
	}

	/* Return success */
	return 1;
}

int magnetometerRead(uint8_t addr, uint8_t numBytes, uint8_t* buff){
	/* Reads from the magnetometer and stores the results in a buffer.
	 *
	 * Arguments: addr, the magnetometer register address to start reading from
	 * 			  numBytes, the number of sequential bytes to read
	 * 			  buff, the buffer to store the read data
	 *
	 * Returns: 1 if successful, otherwise a negative error code
	 */

	uint8_t dataToWrite = MPU9250_MAG_ADDR | 0x80; // slave addr | read
	HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_ADDR, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100);

	dataToWrite = addr;
	HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_REG, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100);

	dataToWrite = 0x80 | numBytes; // Enable | transfer numBytes bytes
	HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_CTRL, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100);
	HAL_FMPI2C_Mem_Read(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, EXT_SENS_DATA_00, I2C_MEMADD_SIZE_8BIT, buff, numBytes, 100);
}

int magnetometerWrite(uint8_t addr, uint8_t data){
	/* Write data to magnetometer.
	 *
	 * Arguments: addr, the magnetometer register address to write to
	 * 			  data, the data to write to the aforementioned address
	 *
	 * Returns: 1 if successful, otherwise a negative error code
	 */

	uint8_t dataToWrite = MPU9250_MAG_ADDR;
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_ADDR, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100) != HAL_OK){
		return -1;
	}

	dataToWrite = addr;
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_REG, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100) != HAL_OK){
		return -2;
	}

	dataToWrite = data; // Continuous measurement mode with 16-bit output
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_DO, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100) != HAL_OK){
		return -3;
	}

	dataToWrite = 0x80 | 1; // Enable | transfer 1 byte
	if(HAL_FMPI2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_CTRL, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100) != HAL_OK){
		return -4;
	}

	return 1;
}

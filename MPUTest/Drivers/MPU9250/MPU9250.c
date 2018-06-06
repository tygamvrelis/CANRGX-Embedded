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
const MPU9250_t* myMPU9250Ptr; // Pointer to global object
const float g = 9.807; // Acceleration due to gravity on Earth




/********************************** Private *********************************/
// Any data transfer will wait up to 1 ms on a semaphore before timing out
static const TickType_t MAX_SEM_WAIT = 1;
static int magnetometerWrite_IT(uint8_t addr, uint8_t data, osSemaphoreId sem);




/********************************* Functions *********************************/
int MPU9250Init(MPU9250_t* myMPU){
	/* Initializes the sensor object passed in.
	 *
	 * Arguments: pointer to MPU6050_t
	 *
	 * Returns: 1 if successful, returns a negative error code otherwise
	 */

	myMPU -> az = -1.0;
	myMPU -> ay = -1.0;
	myMPU -> ax = -1.0;
	myMPU -> A = -1.0;
	myMPU -> vz = -1.0;
	myMPU -> vy = -1.0;
	myMPU -> vx = -1.0;
	myMPU -> hx = -1.0;
	myMPU -> hy = -1.0;
	myMPU -> hz = -1.0;




	/********** Check that MPU9250 is connected **********/
	uint8_t buff[1];
	// Check for bus communication essentially. If any function should fail and issue an early return, it would most likely
	// be this one.
	if(HAL_I2C_Mem_Read(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, WHO_AM_I, I2C_MEMADD_SIZE_8BIT, buff, 1, 100) != HAL_OK){
		return -1;
	}

	// Check that the WHO_AM_I register is 0x71
	if(buff[0] != 0x71){
		return -2;
	}



	/********** Configure accelerometer and gyroscope **********/
	// Use the best available clock source
	uint8_t dataToWrite = 0x01;
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -3;
	}

	// Enable I2C master interface module
	dataToWrite = 0x20;
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, USER_CTRL, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -4;
	}


	// Set I2C module to use 400 kHz speed (pg. 19 of register map)
	dataToWrite = 0x0D;
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_MST_CTRL, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -5;
	}

	// Force accelerometer and gyroscope to ON
	dataToWrite = 0x00;
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, PWR_MGMT_2, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -6;
	}

	// Enable I2C bypass
	dataToWrite = 0x02;
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, INT_PIN_CFG, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
		return -7;
	}

	// Note: Changing the filter bandwidth didn't have a noticeable effect as far as I could tell
//	// Set accelerometer bandwidth 218 Hz
//	dataToWrite = 0x01;
//	if(HAL_I2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, ACCEL_CONFIG_2, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
//		return -8;
//	}
//
//	// Set gyroscope bandwidth to 3600 Hz
//	dataToWrite = 0x01;
//	if(HAL_I2C_Mem_Write(&hfmpi2c1, MPU9250_ACCEL_AND_GYRO_ADDR, CONFIG, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite), 100) != HAL_OK){
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

// TODO
int runtimeResetIMU(osSemaphoreId sem){
	/* Hard reset of the accelerometers & gyroscope module. Safe to call
	 * with FreeRTOS running.
	 *
	 * Arguments:
	 * 	  myMPU, the handle for the MPU instance to be reset
	 * 	  sem, handle for the semaphore to take while transferring data
	 *
	 * Returns:
	 *    1 if successful, -1 otherwise
	 */

	// Hard reset
	uint8_t dataToWrite = 0x80;
	if(HAL_I2C_Mem_Write_IT(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, PWR_MGMT_1, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite)) != HAL_OK){
		return -1;
	}
	if(xSemaphoreTake(sem, MAX_SEM_WAIT) != pdTRUE){
		return -2;
	}

	// Enable I2C master interface module, use the best available clock source, force accelerometer and gyroscope to ON
	uint8_t dataArr[3] = {0x20, 0x01, 0x00};
	if(HAL_I2C_Mem_Write_IT(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, USER_CTRL, I2C_MEMADD_SIZE_8BIT, dataArr, sizeof(dataArr)) != HAL_OK){
		return -3;
	}
	if(xSemaphoreTake(sem, MAX_SEM_WAIT) != pdTRUE){
		return -4;
	}

	// Set I2C module to use 400 kHz speed (pg. 19 of register map)
	dataToWrite = 0x0D;
	if(HAL_I2C_Mem_Write_IT(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_MST_CTRL, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite)) != HAL_OK){
		return -5;
	}
	if(xSemaphoreTake(sem, MAX_SEM_WAIT) != pdTRUE){
		return -6;
	}

	// Enable I2C bypass
	dataToWrite = 0x02;
	if(HAL_I2C_Mem_Write_IT(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, INT_PIN_CFG, I2C_MEMADD_SIZE_8BIT, &dataToWrite, sizeof(dataToWrite)) != HAL_OK){
		return -7;
	}
	if(xSemaphoreTake(sem, MAX_SEM_WAIT) != pdTRUE){
		return -8;
	}

	return 1;
}

// TODO
int runtimeResetMagnetometer(osSemaphoreId sem){
	/* Soft reset of the magnetometer.
	 *
	 * Note: a "success" code only
	 * guarantees that the data was written to the MPU9250's slave
	 * registers without errors. This function is incapable of returning
	 * an error code if the magnetometer does not respond properly to
	 * these values.
	 *
	 * Arguments:
	 * 	  myMPU, the handle for the MPU instance to be reset
	 * 	  sem, handle for the semaphore to take while transferring data
	 *
	 * Returns:
	 *    1 if successful, -1 otherwise
	 */

	int status;

	// Reset
	status = magnetometerWrite_IT(CNTL2, 0x01, sem);
	if(status != 1){
		return status;
	}

	// Enable continuous measurement
	status = magnetometerWrite_IT(CNTL1, 0x16, sem);
	if(status != 1){
		return 2 * status;
	}

	return 1;
}

int accelReadDMA(MPU9250_t* myMPU, osSemaphoreId sem){
	/* Read from the az, ay, ax register addresses and stores the results in the
	 * myMPU object passed in.
	 *
	 * Arguments:
	 * 	  myMPU, the object to store the acceleration values in
	 * 	  sem, handle for the semaphore to take while transferring data
	 *
	 * Returns:
	 *    1 if successful, -1 otherwise
	 */

    uint8_t mpu_buff[6]; // Temporary buffer to hold data from sensor
    int16_t temp;

    HAL_I2C_Mem_Read_DMA(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_ACCEL_X_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 6);
    if(xSemaphoreTake(sem, MAX_SEM_WAIT) != pdTRUE){
    	myMPU9250.ax = NAN;
    	myMPU9250.ay = NAN;
    	myMPU9250.az = NAN;
		return -1;
	}

    /* Process data; scale to physical units */
    temp = (mpu_buff[0] << 8 | mpu_buff[1]); 					   // Shift bytes into appropriate positions
    myMPU9250.ax = (temp * MPU9250_ACCEL_FULL_SCALE  / (32767.0)); // Scale to physical units

	temp = (mpu_buff[2] << 8 | mpu_buff[3]);
	myMPU9250.ay = (temp * MPU9250_ACCEL_FULL_SCALE  / (32767.0));

	temp = (mpu_buff[4] << 8 | mpu_buff[5]);
	myMPU9250.az = (temp * MPU9250_ACCEL_FULL_SCALE  / (32767.0));

	return 1;
}

int gyroReadDMA(MPU9250_t* myMPU, osSemaphoreId sem){
	/*
	 * Read from the az, ay, ax register addresses and stores the results in the
	 * myMPU object passed in.
	 *
	 * Arguments:
	 * 	  myMPU, the object to store the acceleration values in
	 * 	  sem, handle for the semaphore to take while transferring data
	 *
	 * Returns:
	 *    1 if successful, -1 otherwise
	 */

    uint8_t mpu_buff[6]; // Temporary buffer to hold data from sensor
    int16_t temp;

	HAL_I2C_Mem_Read_DMA(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, MPU9250_GYRO_X_ADDR_H, I2C_MEMADD_SIZE_8BIT, mpu_buff, 6);
	if(xSemaphoreTake(sem, MAX_SEM_WAIT) != pdTRUE){
		myMPU9250.vx = NAN;
		myMPU9250.vy = NAN;
		myMPU9250.vz = NAN;
		return -1;
	}

	/* Process data; scale to physical units */
	temp = (mpu_buff[0] << 8 | mpu_buff[1]);
	myMPU9250.vx = (temp / (32767.0) * MPU9250_GYRO_FULL_SCALE);

	temp = (mpu_buff[2] << 8 | mpu_buff[3]);
	myMPU9250.vy = (temp / (32767.0) * MPU9250_GYRO_FULL_SCALE);

	temp = (mpu_buff[4] << 8 | mpu_buff[5]);
	myMPU9250.vz = (temp / (32767.0) * MPU9250_GYRO_FULL_SCALE);

	return 1;
}

int magFluxReadDMA(MPU9250_t* myMPU, osSemaphoreId sem){
	/* Reads from the magnetometer and stores the results in a buffer.
	 *
	 * Note that the high and low bytes switch places for the magnetic field readings
	 * due to the way the registers are mapped. Note that 7 bytes are read because the
	 * magnetometer requires the ST2 register to be read in addition to other data
	 *
	 * Arguments:
	 *     myMPU, the object to store the acceleration values in
	 *     sem, handle for the semaphore to take while transmitting
	 *
	 * Returns:
	 *     1 if successful, otherwise a negative error code
	 */

    uint8_t mpu_buff[7]; // Temporary buffer to hold data from sensor
	int16_t temp;

	uint8_t dataToWrite[3];
	dataToWrite[0] = MPU9250_MAG_ADDR | 0x80; // slave addr | read
	dataToWrite[1] = MPU9250_MAG_X_ADDR_L; // Address within magnetometer to read from
	dataToWrite[2] = 0x80 | 7; // Enable | transfer numBytes bytes

	if(HAL_I2C_Mem_Write_IT(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_ADDR, I2C_MEMADD_SIZE_8BIT, dataToWrite, sizeof(dataToWrite)) != HAL_OK){
		return -1;
	}
	if(xSemaphoreTake(sem, MAX_SEM_WAIT) != pdTRUE){
		myMPU9250.hx = NAN;
		myMPU9250.hy = NAN;
		myMPU9250.hz = NAN;
		return -2;
	}

	if(HAL_I2C_Mem_Read_DMA(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, EXT_SENS_DATA_00, I2C_MEMADD_SIZE_8BIT, mpu_buff, 7) != HAL_OK){
		return -3;
	}
	if(xSemaphoreTake(sem, MAX_SEM_WAIT) != pdTRUE){
		myMPU9250.hx = NAN;
		myMPU9250.hy = NAN;
		myMPU9250.hz = NAN;
		return -4;
	}

	/* Process data; scale to physical units */
	temp = (mpu_buff[1] << 8 | mpu_buff[0]);
	myMPU9250.hx = (temp / (32760.0) * MPU9250_MAG_FULL_SCALE);

	temp = (mpu_buff[3] << 8 | mpu_buff[2]);
	myMPU9250.hy = (temp / (32760.0) * MPU9250_MAG_FULL_SCALE);

	temp =  (mpu_buff[5] << 8 | mpu_buff[4]);
	myMPU9250.hz = (temp / (32760.0) * MPU9250_MAG_FULL_SCALE);

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
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_ADDR, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100) != HAL_OK){
		return -1;
	}

	dataToWrite = addr;
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_REG, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100) != HAL_OK){
		return -2;
	}

	dataToWrite = 0x80 | numBytes; // Enable | transfer numBytes bytes
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_CTRL, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100) != HAL_OK){
		return -3;
	}

	if(HAL_I2C_Mem_Read(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, EXT_SENS_DATA_00, I2C_MEMADD_SIZE_8BIT, buff, numBytes, 100) != HAL_OK){
		return -4;
	}

	return 1;
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
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_ADDR, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100) != HAL_OK){
		return -1;
	}

	dataToWrite = addr;
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_REG, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100) != HAL_OK){
		return -2;
	}

	dataToWrite = data;
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_DO, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100) != HAL_OK){
		return -3;
	}

	dataToWrite = 0x80 | 1; // Enable | transfer 1 byte
	if(HAL_I2C_Mem_Write(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_CTRL, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1, 100) != HAL_OK){
		return -4;
	}

	return 1;
}

static int magnetometerWrite_IT(uint8_t addr, uint8_t data, osSemaphoreId sem){
	/* Write data to magnetometer.
	 *
	 * Arguments: addr, the magnetometer register address to write to
	 * 			  data, the data to write to the aforementioned address
	 * 			  sem, handle for the semaphore to take while transferring data
	 *
	 * Returns: 1 if successful, otherwise a negative error code
	 */

	uint8_t dataToWrite = data;
	if(HAL_I2C_Mem_Write_IT(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_DO, I2C_MEMADD_SIZE_8BIT, &dataToWrite, 1) != HAL_OK){
		return -1;
	}
	if(xSemaphoreTake(sem, MAX_SEM_WAIT) != pdTRUE){
		return -2;
	}

	uint8_t arrData[3] = {MPU9250_MAG_ADDR, addr, 0x80 | 1};
	if(HAL_I2C_Mem_Write_IT(&hi2c3, MPU9250_ACCEL_AND_GYRO_ADDR, I2C_SLV0_ADDR, I2C_MEMADD_SIZE_8BIT, arrData, sizeof(arrData)) != HAL_OK){
		return -3;
	}
	if(xSemaphoreTake(sem, MAX_SEM_WAIT) != pdTRUE){
		return -4;
	}

	return 1;
}




// Note: The following 2 functions are used as a workaround for an issue where the BUSY flag of the
// I2C module is erroneously asserted in the hardware (a silicon bug, essentially). This workaround has
// not been thoroughly tested.
//
// Overall, use these functions with EXTREME caution.
static uint8_t wait_for_gpio_state_timeout(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state, uint8_t timeout){
	/* Helper function for I2C_ClearBusyFlagErratum.
	 *
	 * Arguments: none
	 *
	 * Returns: none
	 */

    uint32_t Tickstart = HAL_GetTick();
    uint8_t ret = 0;
    /* Wait until flag is set */
    while((state != HAL_GPIO_ReadPin(port, pin)) && (1 == ret)){
        /* Check for the timeout */
		if ((timeout == 0U) || (HAL_GetTick() - Tickstart >= timeout)){
			ret = 0;
		}
        asm("nop");
    }
    return ret;
}

void generateClocks(uint8_t numClocks, uint8_t sendStopBits){
	/* This function big-bangs the I2C master clock
	 *
	 * https://electronics.stackexchange.com/questions/267972/i2c-busy-flag-strange-behaviour/281046#281046
	 * https://community.st.com/thread/35884-cant-reset-i2c-in-stm32f407-to-release-i2c-lines
	 * https://electronics.stackexchange.com/questions/272427/stm32-busy-flag-is-set-after-i2c-initialization
	 * http://www.st.com/content/ccc/resource/technical/document/errata_sheet/f5/50/c9/46/56/db/4a/f6/CD00197763.pdf/files/CD00197763.pdf/jcr:content/translations/en.CD00197763.pdf
	 *
	 *
	 * Arguments: numClocks, the number of times to cycle the I2C master clock
	 * 			  sendStopBits, 1 if stop bits are to be sent on SDA
	 *
	 * Returns: none
	 */

	static struct I2C_Module{
		I2C_HandleTypeDef*   instance;
		uint16_t            sdaPin;
		GPIO_TypeDef*       sdaPort;
		uint16_t            sclPin;
		GPIO_TypeDef*       sclPort;
	}i2cmodule = {&hi2c3, MPU_SDA_Pin, MPU_SDA_GPIO_Port, MPU_SCL_Pin, MPU_SCL_GPIO_Port};
	static struct I2C_Module* i2c = &i2cmodule;
	static uint8_t timeout = 1;

	GPIO_InitTypeDef GPIO_InitStructure;

	I2C_HandleTypeDef* handler = NULL;

	handler = i2c->instance;

	// 1. Clear PE bit.
	CLEAR_BIT(handler->Instance->CR1, I2C_CR1_PE);

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	GPIO_InitStructure.Pin = i2c->sclPin;
	HAL_GPIO_Init(i2c->sclPort, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = i2c->sdaPin;
	HAL_GPIO_Init(i2c->sdaPort, &GPIO_InitStructure);

	for(uint8_t i = 0; i < numClocks; i++){
		// 3. Check SCL and SDA High level in GPIOx_IDR.
		if(sendStopBits){HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET);}
		HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET);

		wait_for_gpio_state_timeout(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET, timeout);
		if(sendStopBits){wait_for_gpio_state_timeout(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET, timeout);}

		// 4. Configure the SDA I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
		if(sendStopBits){
			HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_RESET);
			wait_for_gpio_state_timeout(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_RESET, timeout); // 5. Check SDA Low level in GPIOx_IDR
		}

		// 6. Configure the SCL I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
		HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_RESET);
		wait_for_gpio_state_timeout(i2c->sclPort, i2c->sclPin, GPIO_PIN_RESET, timeout); // 7. Check SCL Low level in GPIOx_IDR.

		// 8. Configure the SCL I/O as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
		HAL_GPIO_WritePin(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET);
		wait_for_gpio_state_timeout(i2c->sclPort, i2c->sclPin, GPIO_PIN_SET, timeout); // 9. Check SCL High level in GPIOx_IDR.

		// 10. Configure the SDA I/O as General Purpose Output Open-Drain , High level (Write 1 to GPIOx_ODR).
		if(sendStopBits){
			HAL_GPIO_WritePin(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET);
			wait_for_gpio_state_timeout(i2c->sdaPort, i2c->sdaPin, GPIO_PIN_SET, timeout); // 11. Check SDA High level in GPIOx_IDR.
		}
	}

	// 12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Alternate = GPIO_AF4_I2C3;

	GPIO_InitStructure.Pin = i2c->sclPin;
	HAL_GPIO_Init(i2c->sclPort, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = i2c->sdaPin;
	HAL_GPIO_Init(i2c->sdaPort, &GPIO_InitStructure);

	// 13. Set SWRST bit in I2Cx_CR1 register.
	SET_BIT(handler->Instance->CR1, I2C_CR1_SWRST);
	asm("nop");

	/* 14. Clear SWRST bit in I2Cx_CR1 register. */
	CLEAR_BIT(handler->Instance->CR1, I2C_CR1_SWRST);
	asm("nop");

	/* 15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register */
	SET_BIT(handler->Instance->CR1, I2C_CR1_PE);
	asm("nop");

	HAL_I2C_Init(handler);
}

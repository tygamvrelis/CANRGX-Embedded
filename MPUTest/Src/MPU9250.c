/*
 * MPU9250.c
 *
 *  Created on: April 8, 2018
 *      Author: Tyler
 */

/********************************** Includes *********************************/
#include "MPU9250.h"



/*********************************** Globals *********************************/
MPU9250_t* myMPU9250; // Global MPU9250 object
const float g = 9.807; // Acceleration due to gravity on Earth




/********************************* Functions *********************************/
int MPU9250Init(MPU9250_t* myMPU){
	/* Initializes the sensor object passed in.
	 *
	 * Arguments: pointer to MPU6050_t
	 *
	 * Returns: 1 if successful
	 */

	myMPU -> az = FP_INFINITE;
	myMPU -> vy = FP_INFINITE;
	myMPU -> hx = FP_INFINITE;
	myMPU -> hy = FP_INFINITE;
	myMPU -> hz = FP_INFINITE;

	return 1;
}

/*
 * MPU9250_t.h
 *
 *  Created on: Jul 28, 2018
 *      Author: Tyler
 */

#ifndef MPU9250_MPU9250_T_H_
#define MPU9250_MPU9250_T_H_

/*********************************** Types ************************************/
// Stores data from the sensor in a global struct
typedef struct{
    float az; // Acceleration along z-axis
    float ay; // Acceleration along y-axis
    float ax; // Acceleration along x-axis
    float A;  // ||a||
    float vz; // Yaw rate (about z-axis)
    float vy; // Pitch rate (about y-axis)
    float vx; // Roll rate (about x-axis)
    float hx; // Magnetic field along x
    float hy; // Magnetic field along y
    float hz; // Magnetic field along z
}MPU9250_t;

#endif /* MPU9250_MPU9250_T_H_ */

/**
 * @file userTypes.h
 * @author Tyler
 * @brief Defines types, enums, and macros used throughout various program
 *        modules
 *
 * @defgroup UserTypes User Types
 * @brief Types, enums, and some macros that need to be shared across several
 *        modules
 * @{
 */

#ifndef __USER_TYPES_H__
#define __USER_TYPES_H__




/********************************** Macros ***********************************/
/** @brief Notification bitmask indicating a manual override start event */
#define MANUAL_OVERRIDE_START_BITMASK 0x80000000

/** @brief Notification bitmask indicating a manual override stop event */
#define MANUAL_OVERRIDE_STOP_BITMASK 0x08000000

/** @brief Notification bitmask indicating a MPU9250 event */
#define MPU_BITMASK 0x00800000




/*********************************** Enums ***********************************/
/** @brief Enumerates the types of events the accelerometer can detect */
enum flightEvents_e{
    NONE,             /**< Greater than 3.13 m/s^2 total acceleration  */
    REDUCEDGRAVITY,   /**< Less than 0.981 m/s^2 total acceleration    */
};

/**
 * @brief Enumerates the states the controller can be in. Each corresponds to a
 *        specific function with which the duty cycle for the magnets will be
 *        varied
 */
enum controllerStates_e{
    IDLE,         /**< No experiment; idle signals                           */
    EXPERIMENT0,  /**< Baseline run: no magnetic field, no TECs              */
    EXPERIMENT1,  /**< Same as EXPERIMENT0                                   */
    EXPERIMENT2,  /**< Magnet1: +1, Magnet2: +1, TECs on (DC anti-Helmholtz) */
    EXPERIMENT3,  /**< Magnet1: +1, Magnet2: -1, TECs on (DC Helmholtz)      */
    EXPERIMENT4,  /**< Magnet1: +1, Magnet2: 0, TECs on                      */
    EXPERIMENT5,  /**< AC trapezoid anti-Helmholtz (in-phase)                */
    EXPERIMENT6,  /**< AC trapezoid Helmholtz (180 degrees out of phase)     */
    EXPERIMENT7,  /**< Magnet1: sin, Magnet2: cos                            */
    EXPERIMENT8,  /**< Same as EXPERIMENT2                                   */
    EXPERIMENT9,  /**< Same as EXPERIMENT3                                   */
    EXPERIMENT10, /**< Same as EXPERIMENT0                                   */
};

/**
 * @brief Enumerates the different types of containers that TXData_t->data
 * might point to
 */
typedef enum{
    accelerometer_t, /**< Accelerometer data         */
    magnetometer_t,  /**< Magnetic flux density data */
    control_t,       /**< PWM signal data            */
    temperature_t    /**< Temperature sensor data    */
}TXDataTypes_e;




/*********************************** Types ***********************************/
/**
 * @brief This is the generic structure for the TX queue. Based on the value of
 *        type, the data pointer can be typecasted into one of the 4 structs
 *        following this definition
 */
typedef struct{
	TXDataTypes_e type; /**< Indicates what the pointer points to */
	void* data;         /**< Pointer to data container            */
}TXData_t;

/** @brief Container for accelerometer data  */
typedef struct{
	float ax; /**< Acceleration along x-axis */
	float ay; /**< Acceleration along y-axis */
	float az; /**< Acceleration along z-axis */
}accelerometerData_t;

/** @brief Container for magnetometer data */
typedef struct{
	float hx; /**< Magnetic flux density along x-axis */
	float hy; /**< Magnetic flux density along y-axis */
	float hz; /**< Magnetic flux density along z-axis */
}magnetometerData_t;

/**
 * @brief Container for control signal data
 * @note  Each member is equal to the value of the original duty cycle (between
 *        -1 and 1) multiplied by 10,000 and then truncated to an integer. This
 *        allows precision to 2 decimal places without using floats, as the
 *        precision of floats was not necessary for these particular quantities
 */
typedef struct{
    int16_t mag1Power;  /**< Duty cycle for magnet 1 PWM */
    int16_t mag2Power;  /**< Duty cycle for magnet 2 PWM */
    uint16_t tec1Power; /**< Duty cycle for TEC 1 PWM    */
    uint16_t tec2Power; /**< Duty cycle for TEC 2 PWM    */
}controlData_t;

/**
 * @brief Container for temperature sensor data. Since the ADC is 12 bits, the
 *        max value one of these members can take on is 4095
 */
typedef struct{
    uint16_t temp1a; /**< Filtered ADC data for temperature sensor 1A */
    uint16_t temp1b; /**< Filtered ADC data for temperature sensor 1B */
    uint16_t temp2a; /**< Filtered ADC data for temperature sensor 2A */
    uint16_t temp2b; /**< Filtered ADC data for temperature sensor 2B */
    uint16_t temp3a; /**< Filtered ADC data for temperature sensor 3A */
    uint16_t temp3b; /**< Filtered ADC data for temperature sensor 3B */
}temperatureData_t;

/**
 * @}
 */
/* end - UserTypes */

#endif /* __USER_TYPES_H__ */

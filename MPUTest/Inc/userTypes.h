
#ifndef __USER_TYPES_H__
#define __USER_TYPES_H__

// Events and states
enum flightEvents_e{
    NONE,
    REDUCEDGRAVITY,
};

enum controllerStates_e{
    IDLE,
    EXPERIMENT1,
    EXPERIMENT2,
    EXPERIMENT3,
    EXPERIMENT4
};




// Task notification-related things
#define MANUAL_OVERRIDE_START_BITMASK 0x80000000
#define MANUAL_OVERRIDE_STOP_BITMASK 0x08000000
#define MPU_BITMASK 0x00800000




typedef enum{
	accelerometer_t,
	magnetometer_t,
	control_t,
	temperature_t
}TXDataTypes_e;

// This is the generic structure for the TX queue. Based on the value of type,
// the data pointer can be typecast into one of the 4 structs following this
// definition
typedef struct{
	TXDataTypes_e type;
	void* data;
}TXData_t;

typedef struct{
	float ax;
	float ay;
	float az;
}accelerometerData_t;

typedef struct{
	float hx;
	float hy;
	float hz;
}magnetometerData_t;

typedef struct{
    int16_t mag1Power;
    int16_t mag2Power;
    uint16_t tec1Power;
    uint16_t tec2Power;
}controlData_t;

typedef struct{
    uint16_t temp1a;
    uint16_t temp1b;
    uint16_t temp2a;
    uint16_t temp2b;
    uint16_t temp3a;
    uint16_t temp3b;
}temperatureData_t;

#endif /* __USER_TYPES_H__ */

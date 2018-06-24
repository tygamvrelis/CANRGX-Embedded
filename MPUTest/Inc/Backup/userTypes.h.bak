
#ifndef __USER_TYPES_H__
#define __USER_TYPES_H__

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
    uint16_t thermocouple1;
    uint16_t thermocouple2;
    uint16_t thermocouple3;
    uint16_t thermocouple4;
    uint16_t thermocouple5;
    uint16_t thermocouple6;
}temperatureData_t;

#endif /* __USER_TYPES_H__ */


#ifndef __USER_TYPES_H__
#define __USER_TYPES_H__

typedef struct{
    uint16_t mag1Power;
    uint16_t mag2Power;
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

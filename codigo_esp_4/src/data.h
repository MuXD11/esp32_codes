// data.h
#ifndef DATA_H
#define DATA_H

#include <Arduino.h>

/*
Estructura de datos a manejar
*/
typedef struct
{
    byte TX_ID;
    byte RX_ID;
    int seq;
    float latitude;
    float longitude;
    float altitude;
    float hdop;
    float temperature;
    float pressure;
    float humidity;
    float baro_altitude;
    float ext_temperature_ours;
} Data;

typedef struct
{
    byte Type_of_message;
    byte TC_Action_ID;
    int TC_Payload;

} Data_TC_1;

extern Data data_random;    // ← declaración externa. Está definida en main.cpp
extern Data_TC_1 data_tc_1; // ← declaración externa. Está definida en main.cpp

#endif
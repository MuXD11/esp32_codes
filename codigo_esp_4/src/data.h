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

extern Data data_random; // ← declaración externa. Está definida en main.cpp

#endif
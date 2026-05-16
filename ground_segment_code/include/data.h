#ifndef DATA_H
#define DATA_H

#include <Arduino.h>

/*
Telemetry data structure received from the flight segment over LoRa.
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
    float ext_temperature;
} data;

/*
Telecommand structure sent from server to ground station, then forwarded
to the flight segment over LoRa.
*/
typedef struct
{
    byte Type_of_message;
    byte TC_Action_ID;
    int TC_Payload;
} Data_TC_1_t;

/*
Global instances. Defined in data.cpp.
*/
extern data Datos;
extern Data_TC_1_t Datos_TC_1;

#endif
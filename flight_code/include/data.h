#ifndef DATA_H
#define DATA_H

#include <Arduino.h>

/*
Data structures
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
    int TC_ID;
} Data_TC_1;

/*
Function declarations
*/

/**
 * @brief Samples sensor data into the global telemetry struct.
 *
 * Behavior depends on the operation mode flags in configuration.h:
 *  - CURRENT_MODE_DEBUG: simulated values, except for GPS lat/lon/HDOP
 *  - CURRENT_MODE_DEBUG_SENSOR_ON: real sensor reads from BME280 and GPS
 */
void gather_sensor_data(void);

/**
 * @brief Initializes the I2C bus and the BME280 sensor.
 *
 * Must be called once from setup() before gather_sensor_data() if the
 * CURRENT_MODE_DEBUG_SENSOR_ON branch will run.
 *
 * @return true on success, false if the BME280 did not respond
 */
bool init_sensors(void);

/*
Global variables
*/
extern Data data_random;
extern Data_TC_1 data_tc_1;

#endif
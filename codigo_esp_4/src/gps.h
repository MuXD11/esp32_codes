#ifndef GPS_H
#define GPS_H

/*
Includes
*/
#include <TinyGPS++.h>

/*
Declaración de variables
*/
extern TinyGPSPlus _gps;            // Definida en gps.cpp
extern HardwareSerial _serial_gps;  // Definida en gps.cpp
extern byte setAirborne1g[44];      // Definida en main.cpp
extern byte cfgNav5Poll[8];         // Definida en main.cpp
extern byte setAirbornedefault[44]; // Definida en main.cpp

/*
Declaración de funciones
*/

/**
 * @brief Esta función envía una realzia una lectura de los bytes envíados por el puerto serie al GPS
 *
 * .
 *
 * @param void
 * @return void
 */
void gps_read_loop(void);

/**
 * @brief Esta función calcula el checksum para una trama
 *
 *
 *
 * @param message Array con el mensaje a envíar, considerando los dos bytes finales vacíos para el checksum
 * @param length longitud del array
 * @return void
 */
void AIBChecksum(char *message, size_t length);

/**
 * @brief Esta función envía una trama al GPS
 *
 * Empleando el protocolo UBX, se envía un mensaje cualquiera al GPS
 *
 * @param msg Array con el mensaje a envíar, considerando los dos bytes finales vacíos para el checksum
 * @param length longitud del array
 * @return void
 */

void sendUBXmsg(byte *msg, uint16_t length);

/**
 * @brief Esta función lee un mensaje CFG-NAV5
 *
 * Se llama como respuesta a un envío de mensaje de tipo poll
 *
 * @param void
 * @return void
 */

void leerRespuestaNAV5(byte *p_dynstate_ext_ctrl_var);

/**
 * @brief Esta función procesa los datos de altitud y actúa en consecuencia
 *
 * Esta función lee la altitud  y envía un TC para cambiar el modo dinámico en función del estado
 *
 * @param dyn_state Parámetro que contiene el modo dinámico actual
 * @return void
 */

void alt_proc(byte dyn_state);

/*
Definición de parámetros GPS
*/

#define GPS_SERIAL_NUM 1
#define GPS_BAUDRATE 9600
#define USE_GPS 1
#define GPS_RX_PIN 34
#define GPS_TX_PIN 12

#endif
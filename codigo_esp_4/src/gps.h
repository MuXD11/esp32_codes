#ifndef GPS_H
#define GPS_H

/*
Includes
*/
#include <TinyGPS++.h>

/*
Declaración de variables
*/
extern TinyGPSPlus _gps; // (existe en otro archivo)
extern HardwareSerial _serial_gps;

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

/*
Definición de parámetros GPS
*/

#define GPS_SERIAL_NUM 1
#define GPS_BAUDRATE 9600
#define USE_GPS 1
#define GPS_RX_PIN 34
#define GPS_TX_PIN 12

#endif
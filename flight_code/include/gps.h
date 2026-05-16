#ifndef GPS_H
#define GPS_H

/*
Includes
*/
#include <TinyGPS++.h>

/*
Variable declarations
*/
extern TinyGPSPlus _gps;            // Defined in gps.cpp
extern HardwareSerial _serial_gps;  // Defined in gps.cpp
extern byte setAirborne1g[44];      // Defined in main.cpp
extern byte cfgNav5Poll[8];         // Defined in main.cpp
extern byte setAirbornedefault[44]; // Defined in main.cpp

/*
Function declarations
*/

/**
 * @brief This function reads the bytes sent by the GPS over the serial port
 *
 * .
 *
 * @param void
 * @return void
 */
void gps_read_loop(void);

/**
 * @brief This function computes the checksum for a frame
 *
 *
 *
 * @param message Array with the message to send, with the last two bytes left empty for the checksum
 * @param length Length of the array
 * @return void
 */
void AIBChecksum(char *message, size_t length);

/**
 * @brief This function sends a frame to the GPS
 *
 * Using the UBX protocol, sends an arbitrary message to the GPS
 *
 * @param msg Array with the message to send, with the last two bytes left empty for the checksum
 * @param length Length of the array
 * @return void
 */

void sendUBXmsg(byte *msg, uint16_t length);

/**
 * @brief This function reads a CFG-NAV5 message
 *
 * Called in response to sending a poll-type message
 *
 * @param p_dynstate_ext_ctrl_var External variable passed by pointer that stores the current dynamic model
 * @return void
 */

void leerRespuestaNAV5(byte *p_dynstate_ext_ctrl_var);

/**
 * @brief This function processes altitude data and acts accordingly
 *
 * Reads the altitude and sends a TC to change the dynamic model based on the current state
 *
 * @param dyn_state Parameter containing the current dynamic model
 * @return void
 */

void alt_proc(byte dyn_state);

/*
GPS parameter definitions
*/

#define GPS_SERIAL_NUM 1
#define GPS_BAUDRATE 9600
#define USE_GPS 1
#define GPS_RX_PIN 34
#define GPS_TX_PIN 12

#endif
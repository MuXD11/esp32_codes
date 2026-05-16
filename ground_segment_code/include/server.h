#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include "data.h"

/**
 * @brief Connects to the WiFi network configured in server.cpp.
 *
 * Blocks until connected (matches the original behavior).
 */
void server_wifi_connect(void);

/**
 * @brief Returns true if WiFi is currently connected.
 */
bool server_wifi_is_connected(void);

/**
 * @brief Attempts to reconnect WiFi (non-blocking).
 */
void server_wifi_reconnect(void);

/**
 * @brief Posts the given telemetry struct to the server as JSON.
 *
 * @param d Telemetry to send
 * @return HTTP response code (positive on success), or negative error code
 */
int server_post_telemetry(const data &d);

/**
 * @brief Polls the server for a pending telecommand.
 *
 * If a command is available, populates Datos_TC_1 and forwards it over
 * LoRa to the flight segment.
 */
void server_fetch_command(void);

#endif
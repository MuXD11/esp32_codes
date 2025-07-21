#ifndef TELECOMMANDS_H
#define TELECOMMANDS_H

/*
Includes
*/
#include <stdint.h>
#include "gps.h"

/*
Declaración de funciones
*/

/**
 * @brief Despacha un telecomando en función de su código.
 *
 * Ejecuta la función correspondiente al código de telecomando recibido.
 * (ej. 1-9 = sistema, 10-19 = LoRa reconfig, 20-29 = GPS, 30-39 = EPS)
 *
 * @param
 */
void dispatch_telecommand(void);

/**
 * @brief Telecomando 1: Reinicio del sistema
 */
void handle_reset_system(void);

/**
 * @brief Telecomando 2: Cambio de periodo de bucle
 */
void handle_loop_freq(void);

/**
 * @brief Telecomando 10: Reconfigura los parámetros LoRa
 *
 * @param sf Spreading Factor (ej. 7–12)
 * @param bw Bandwidth en Hz (ej. 125E3)
 * @param cr Coding Rate (4/5 a 4/8 → usar 5 a 8)
 */
void handle_reconfigure_lora(int sf, long bw, int cr);

/**
 * @brief Telecomando 11: Cambia la potencia de transmisión
 *
 * @param
 */
void handle_change_trans_power();

/*
Parámetros por defecto
*/
#define DEFAULT_SF 9
#define DEFAULT_BW 250000
#define DEFAULT_CR 6
#define DEFAULT_DYN_MODEL 6

#endif

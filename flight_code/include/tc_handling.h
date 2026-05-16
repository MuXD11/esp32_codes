#ifndef TELECOMMANDS_H
#define TELECOMMANDS_H

/*
Includes
*/
#include <stdint.h>
#include "gps.h"
#include "configuration.h"
#include "data.h"

/*
Function declaration
*/

/**
 * @brief Exectures the corresponding function depending on the received command.
 *
 *
 * @param tc Reference to a constant struct that contains the received command
 * @ret False if any command was executed correctly, True otherwise
 */
void dispatch_telecommand(const Data_TC_1 &tc);

/**
 * @brief Telecommand 1: System reset
 * @ret False if command was executed correctly, True otherwise
 */
bool handle_reset_system(void);

/**
 * @brief Telecommand 2: Loop period change
 *
 * @param TC_Payload Payload containing the new loop period in ms
 * @ret False if command was executed correctly, True otherwise
 */
bool handle_loop_freq(int TC_Payload);

/**
 * @brief Telecommand 10: LoRa reconfig for Spread Factor
 *
 * @param TC_Payload Payload containing the param for the config
 * @ret False if command was executed correctly, True otherwise
 */
bool handle_reconfigure_lora_sf(int TC_Payload);

/**
 * @brief Telecommand 11: LoRa reconfig for Bandwidth
 *
 * @param TC_Payload Payload containing the param for the config
 * @ret False if command was executed correctly, True otherwise
 */
bool handle_reconfigure_lora_bw(int TC_Payload);

/**
 * @brief Telecommand 12: LoRa reconfig for Code Rate
 *
 * @param TC_Payload Payload containing the param for the config
 * @ret False if command was executed correctly, True otherwise
 */
bool handle_reconfigure_lora_cr(int TC_Payload);

/**
 * @brief Telecommand 13: LoRa Transmission power change
 *
 * @param TC_Payload Payload containing the param for the config
 * @ret False if command was executed correctly, True otherwise
 */
bool handle_change_trans_power(int TC_Payload);

#endif
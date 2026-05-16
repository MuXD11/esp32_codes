#ifndef LORA_RX_H
#define LORA_RX_H

#include <Arduino.h>
#include <SPI.h>

/*
SPI bus used by the LoRa radio. Defined in lora_rx.cpp.
*/
extern SPIClass spiLora;

/**
 * @brief Initializes the LoRa radio.
 *
 * Configures the SPI bus, pins, performs the reset sequence, verifies
 * the chip version, applies the spreading factor / bandwidth / coding
 * rate from configuration.h, and brings the radio up at LORA_FREQ.
 *
 * @return true on success, false if version check or LoRa.begin() failed
 */
bool lora_init(void);

/**
 * @brief Polls the LoRa radio for a received packet.
 *
 * If a packet is available, copies it into the provided buffer.
 *
 * @param buffer       Destination buffer (typically address of Datos)
 * @param buffer_size  Size of the destination buffer in bytes
 * @return Number of bytes received, or 0 if no packet was available
 */
int lora_poll_packet(void *buffer, size_t buffer_size);

/**
 * @brief Sends a buffer of bytes as a single LoRa packet.
 *
 * @param data    Pointer to the data to send
 * @param length  Number of bytes to send
 */
void lora_send_packet(const void *data, size_t length);

/**
 * @brief Reads a single LoRa register over SPI.
 *
 * Exposed mainly for diagnostics.
 *
 * @param addr Register address (7-bit)
 * @return Register value
 */
byte lora_read_register(byte addr);

#endif
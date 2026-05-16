#ifndef CONFIGURATION_H
#define CONFIGURATION_H

/*
Includes
*/
#include <stdint.h>
#include <Arduino.h>

/*
Global configurable parameters
*/
// --- I2C pins for sensor bus ---
// Standard ESP32 default pins. Change here if your board routes I2C differently.
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// BME280 I2C address. Most breakout boards use 0x76;
// some (Adafruit) use 0x77. Check your specific module.
#define BME280_I2C_ADDR 0x76

// --- Operation mode ---

// Debug mode: sensor sampling functions return random/simulated values
// instead of reading real hardware. Use during bench testing without sensors.
#define CURRENT_MODE_DEBUG 1

// Debug mode with real sensors enabled: real hardware sampling but with
// extra logging and test fragments active. Mutually exclusive with CURRENT_MODE_DEBUG.
#define CURRENT_MODE_DEBUG_SENSOR_ON 0

// Flash management flag: if set, erase all stored files on boot.
// Leave at 0 for normal operation to preserve logs across resets.
#define FLASH_ERASE_FILES 0

// --- Default LoRa configuration ---

#define DEFAULT_LORA_FQ 868E6 // Carrier frequency in Hz (EU 868 MHz band)
#define DEFAULT_LORA_SF 10    // Spreading Factor (valid range: 6-12)
#define DEFAULT_LORA_BW 125E3 // Signal bandwidth in Hz
#define DEFAULT_LORA_CR 5     // Coding rate denominator: 5=4/5, 6=4/6, 7=4/7, 8=4/8
#define DEFAULT_LORA_POWER 17 // TX power in dBm (PA_BOOST range: 2-20)
#define LORA_CHIP_VER 0x12    // Expected SX127x REG_VERSION value, used for boot self-check

// --- Default GPS dynamic model ---

#define DEFAULT_DYN_MODEL 6 // u-blox dynamic platform model: 6 = Airborne <1g>

// --- Debug / logging ---

// Verbosity level: 0 = silent, 1 = standard logs, 2 = detailed logs.
// The DEBUG_PRINT/DEBUG_PRINTLN macros below honor this level.
#define VERBOSE_MODE 1

#if VERBOSE_MODE >= 1
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

// --- Test injection flags ---

// When set, forces high altitude values into telemetry on counter 10-20
// to exercise the GPS dynamic-mode auto-switch logic without a real flight.
#define ENABLE_ALTITUDE_TEST_INJECTION 1

// When set, injects an artificial TC at specific counter values to exercise
// the dispatch path without needing real LoRa RX from ground.
#define ENABLE_TC_TEST_INJECTION 0

#endif

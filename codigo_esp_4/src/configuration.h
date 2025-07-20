#ifndef CONFIGURATION_H
#define CONFIGURATION_H

/*
Includes
*/
#include <stdint.h>

/*
Parámetros globales configurables
*/

// --- Temporización ---
#define DEFAULT_LOOP_PERIOD_MS 10000 // Periodo del loop principal en ms
#define MIN_LOOP_PERIOD_MS 1000      // Límite inferior para transmisión rápida
#define MAX_LOOP_PERIOD_MS 60000     // Límite superior

// --- Configuración de LoRa por defecto ---
#define DEFAULT_LORA_FQ 868E6
#define DEFAULT_LORA_SF 10
#define DEFAULT_LORA_BW 125E3
#define DEFAULT_LORA_CR 5 // 4/5, 4/6, 4/7, 4/8
#define LORA_CHIP_VER 0x12

// --- Control de sensores ---
#define ENABLE_BME280 1
#define ENABLE_EXTERNAL_TEMP 1
#define ENABLE_OLED 0 // 0 para desactivarla si se desea

// --- Profundidad de debug ---
#define VERBOSE_MODE 1

// --- Modo dinámico GPS por defecto ---
#define DEFAULT_DYN_MODEL 6 // Airborne <1g>

/*
Variables configurables en ejecución
(Si se desea que puedan cambiar con TC)
*/
extern uint32_t loop_period_ms; // Modificable por telecomando

#endif

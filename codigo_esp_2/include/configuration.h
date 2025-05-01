#include <Arduino.h>
#include <lmic.h>

// LoRa pinout: Versión SEMTECH SX1276 o SX1278
#define SCK 5
#define MISO 19
#define MOSI 27
#define NSS 18 // CSS = NS
#define RST 23
#define DIO0 26

// LoRa
#define RADIO_CS_PIN 18
#define RADIO_RST_PIN 23
// #define RADIO_RST_PIN 14
#define RADIO_DIO0_PIN 26
// #define LORA_FREQ 915E6
#define LORA_FREQ 868E6

// LoRa radio config params
#define LORA_SPREAD_fACTOR 7
#define LORA_CODING_RATE 5
#define LORA_BANDWIDTH 125E3 // 125KHz
#define LORA_SYNCWORD 0x00   // UNUSED

// OLED SCREEN CONFIG
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#ifndef CLORA_H
#define CLORA_H

/*
Includes
*/
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

/*
Declaración de variables
*/
extern SPIClass spiLoRa; // (existe en otro archivo)

/*
Declaración de funciones
*/

/**
 * @brief Esta función realiza una lectura del registro que contiene el número de versión del chip lora
 *
 * .
 *
 * @param address dirección del registro
 * @return valor en byte del registro de configuración
 */
byte readLoRaRegister(byte address);

/*
Definición de parámetros LoRa
*/
// --- Definición de pines SPI y control ---
#define SCK 5
#define MISO 19
#define MOSI 27
#define NSS 18
#define RST 23
#define DIO0 26

#endif
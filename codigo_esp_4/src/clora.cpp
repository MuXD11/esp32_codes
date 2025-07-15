#include "clora.h"

/*
Definición de variables
*/
SPIClass spiLoRa(VSPI); // Usa HSPI si ya estás usando VSPI para otra cosa

/*
Definición de funciones
*/

byte readLoRaRegister(byte address)
{
    spiLoRa.beginTransaction(SPISettings(1E6, MSBFIRST, SPI_MODE0));
    digitalWrite(NSS, LOW);
    delayMicroseconds(1);
    spiLoRa.transfer(address & 0x7F);    // Bit 7 = 0 (lectura)
    byte value = spiLoRa.transfer(0x00); // Leer valor
    digitalWrite(NSS, HIGH);
    spiLoRa.endTransaction();
    return value;
}
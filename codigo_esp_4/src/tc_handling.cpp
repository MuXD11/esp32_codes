/*
Includes
*/
#include "tc_handling.h"
#include <Arduino.h>
#include <LoRa.h>
#include <string.h>
#include "clora.h"
#include "gps.h"

/*
Implementaciones de funciones
*/

// Función principal de TCs
void dispatch_telecommand(uint8_t code)
{
    switch (code)
    {
    case 1:
        handle_reset_system();
        break;

    case 10:
        handle_reconfigure_lora(DEFAULT_SF, DEFAULT_BW, DEFAULT_CR);
        break;

    case 20:
        handle_change_dynamic_model(DEFAULT_DYN_MODEL);
        break;

    default:
        Serial.print("⚠️ Código de TC no reconocido: ");
        Serial.println(code);
        break;
    }
}

// Telecomando 1: Reinicio del sistema
void handle_reset_system(void)
{
    Serial.println(" TC 2.1 recibido: Reiniciando sistema...");
    delay(1000);
    ESP.restart();
}

// Telecomando 10: Reconfiguración de parámetros LoRa
void handle_reconfigure_lora(int sf, long bw, int cr)
{
    Serial.println(" TC 2.2 recibido: Reconfigurando LoRa...");
    LoRa.setSpreadingFactor(sf);
    LoRa.setSignalBandwidth(bw);
    LoRa.setCodingRate4(cr);

    Serial.print("Nuevo SF: ");
    Serial.print(sf);
    Serial.print(" | BW: ");
    Serial.print(bw);
    Serial.print(" | CR: 4/");
    Serial.println(cr);
}

// Telecomando 20: Cambio de modelo dinámico GPS
void handle_change_dynamic_model(uint8_t model)
{
    Serial.println(" TC 2.3 recibido: Cambiando modo dinámico GPS...");

    /*
    byte msg[44];
    memcpy(msg, setAirbornedefault, 44); // copiar base
    msg[6] = 0x01;                       // mask: dynModel
    msg[7] = model;                      // modelo dinámico

    // Calcular checksum
    calcChecksum(&msg[2], 40, &msg[42], &msg[43]);
    sendUBXmsg(msg, 44);
*/
}

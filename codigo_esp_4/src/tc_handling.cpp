/*
Includes
*/
#include "tc_handling.h"
#include <Arduino.h>
#include <LoRa.h>
#include <string.h>
#include "clora.h"
#include "gps.h"
#include "data.h"

extern int loop_period_ms;

/*
Implementaciones de funciones
*/

// Función principal de TCs
void dispatch_telecommand(void)
{
    switch (data_tc_1.TC_Action_ID)
    {
    case 1:
        handle_reset_system();
        break;
    case 2:
        handle_loop_freq();
        break;

    case 10:
        handle_reconfigure_lora(DEFAULT_SF, DEFAULT_BW, DEFAULT_CR);
        break;

    case 11:
        handle_change_trans_power();
        break;

    default:
        Serial.print("⚠️ Código de TC no reconocido: ");
        Serial.println(data_tc_1.TC_Action_ID);
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

// Telecomando 2: Cambio de periodo del loop principal
void handle_loop_freq(void)
{
    loop_period_ms = data_tc_1.TC_Payload;
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
void handle_change_trans_power()
{
    Serial.println(" ....... ");

    /*
    Cambio de potencia
    */
}

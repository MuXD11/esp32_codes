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
Function implementation
*/

void dispatch_telecommand(const Data_TC_1 &tc)
{
    switch (tc.TC_Action_ID)
    {
    case 1:
        if (handle_reset_system())
        {
            // ERROR
            DEBUG_PRINTLN("Error execuring TC with ID: ");
            DEBUG_PRINTLN(tc.TC_ID);
        }
        break;
    case 2:
        if (handle_loop_freq(tc.TC_Payload))
        {
            // ERROR
            DEBUG_PRINTLN("Error execuring TC with ID: ");
            DEBUG_PRINTLN(tc.TC_ID);
        }
        break;

    case 10:
        if (handle_reconfigure_lora_sf(tc.TC_Payload))
        {
            // ERROR
            DEBUG_PRINTLN("Error execuring TC with ID: ");
            DEBUG_PRINTLN(tc.TC_ID);
        }
        break;

    case 11:
        if (handle_reconfigure_lora_bw(tc.TC_Payload))
        {
            // ERROR
            DEBUG_PRINTLN("Error execuring TC with ID: ");
            DEBUG_PRINTLN(tc.TC_ID);
        }
        break;

    case 12:
        if (handle_reconfigure_lora_cr(tc.TC_Payload))
        {
            // ERROR
            DEBUG_PRINTLN("Error execuring TC with ID: ");
            DEBUG_PRINTLN(tc.TC_ID);
        };
        break;

    case 13:
        if (handle_change_trans_power(tc.TC_Payload))
        {
            // ERROR
            DEBUG_PRINTLN("Error execuring TC with ID: ");
            DEBUG_PRINTLN(tc.TC_ID);
        };
        break;

    default:
        DEBUG_PRINTLN("Unrecognized TC code: ");
        DEBUG_PRINTLN(tc.TC_Action_ID);
        break;
    }
}

// TC 1: System reset
bool handle_reset_system(void)
{
    DEBUG_PRINTLN(" TC 1 recibido: Reiniciando sistema...");
    delay(1000);
    ESP.restart();
    return false;
}

// TC 2: Loop period change
bool handle_loop_freq(int TC_Payload)
{
    // TODO
    return false;
}

// TC 10: LoRa reconfig - Spreading Factor
bool handle_reconfigure_lora_sf(int TC_Payload)
{
    DEBUG_PRINTLN(" TC 10 recibido: Reconfigurando SF de LoRa...");

    // Local parameter
    int l_sf = DEFAULT_LORA_SF;

    // SX127x valid SF range: 6 to 12
    if (TC_Payload >= 6 && TC_Payload <= 12)
    {
        l_sf = TC_Payload;
    }
    else
    {
        DEBUG_PRINTLN("Configuration not allowed");
        return true;
    }

    // Change LoRa parameter
    LoRa.setSpreadingFactor(l_sf);
    return false;
}

// TC 11: LoRa reconfig - Bandwidth
bool handle_reconfigure_lora_bw(int TC_Payload)
{
    DEBUG_PRINTLN(" TC 11 recibido: Reconfigurando BW de LoRa...");

    // Local parameter
    long l_bw = DEFAULT_LORA_BW;

    // SX127x valid BW values (Hz)
    if (TC_Payload == 7800 || TC_Payload == 10400 || TC_Payload == 15600 ||
        TC_Payload == 20800 || TC_Payload == 31250 || TC_Payload == 41700 ||
        TC_Payload == 62500 || TC_Payload == 125000 || TC_Payload == 250000 ||
        TC_Payload == 500000)
    {
        l_bw = TC_Payload;
    }
    else
    {
        DEBUG_PRINTLN("Configuration not allowed");
        return true;
    }

    // Change LoRa parameter
    LoRa.setSignalBandwidth(l_bw);
    return false;
}

// TC 12: LoRa reconfig - Coding Rate
bool handle_reconfigure_lora_cr(int TC_Payload)
{

    // Local parameter
    int l_cr = DEFAULT_LORA_CR;

    // SX127x valid CR denominator: 5 (4/5), 6 (4/6), 7 (4/7), 8 (4/8)
    if (TC_Payload >= 5 && TC_Payload <= 8)
    {
        l_cr = TC_Payload;
    }
    else
    {
        DEBUG_PRINTLN("Configuration not allowed");
        return true;
    }

    // Change LoRa parameter
    LoRa.setCodingRate4(l_cr);
    return false;
}

// TC 13: Cambio de potencia de transmisión LoRa
bool handle_change_trans_power(int TC_Payload)
{

    // Local parameter
    int l_power = DEFAULT_LORA_POWER;

    // SX127x PA_BOOST valid TX power range: 2 to 20 dBm
    if (TC_Payload >= 2 && TC_Payload <= 20)
    {
        l_power = TC_Payload;
    }
    else
    {
        DEBUG_PRINTLN("Configuration not allowed");
        return true;
    }

    // Change LoRa parameter
    LoRa.setTxPower(l_power);
    return false;
}
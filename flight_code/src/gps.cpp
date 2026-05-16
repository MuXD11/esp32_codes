#include "gps.h"
#include "data.h"
/*
Variable definition
*/

TinyGPSPlus _gps;
HardwareSerial _serial_gps(GPS_SERIAL_NUM);

/*
Function definition
*/

void gps_read_loop(void)
{
    while (_serial_gps.available())
    {
        _gps.encode(_serial_gps.read());
    }
}

void calculateUBXChecksum(byte *msg, uint16_t length, byte *ck_a, byte *ck_b)
{

    *ck_a = 0;
    *ck_b = 0;
    for (uint16_t i = 2; i < length - 2; i++)
    { // Desde Class hasta último byte del payload
        *ck_a += msg[i];
        *ck_b += *ck_a;
    }
}

void sendUBXmsg(byte *msg, uint16_t length)
{
    // Step 1: Checksum calculation
    byte ck_a, ck_b;
    calculateUBXChecksum(msg, length, &ck_a, &ck_b);

    // Step 2: assign computed values
    msg[length - 2] = ck_a;
    msg[length - 1] = ck_b;

    // Paso 3: Loop for sending
    for (uint8_t i = 0; i < length; i++)
    {
        _serial_gps.write(msg[i]);
    }
    /*
        // Paso 4: Log the frame
        Serial.print(" Trama UBX (");
        Serial.print(length);
        Serial.print(" bytes):");

        for (uint16_t i = 0; i < length; i++)
        {
            if (msg[i] < 0x10)
                Serial.print("0");
            Serial.print(msg[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    */
    Serial.println("UBX-CFG-NAV5 sent ");
}

// Reads a specific type of message
void leerRespuestaNAV5(byte *p_dynstate_ext_ctrl_var)
{
    byte buffer[50];
    int i = 0;

    unsigned long start = millis();
    bool header_found = false;

    while (millis() - start < 3000)
    { // timeout de 3 segundos para la recepción
        if (_serial_gps.available())
        {
            byte c = _serial_gps.read();

            // Detectar inicio de trama UBX ([0xB5, 0x62])
            if (!header_found && c == 0xB5)
            {
                buffer[0] = c;
                while (!_serial_gps.available())
                    ;
                c = _serial_gps.read();
                if (c == 0x62)
                {
                    buffer[1] = c;
                    i = 2;
                    header_found = true;
                }
            }
            else if (header_found && i < sizeof(buffer))
            {
                buffer[i++] = c;

                if (i >= 8 && buffer[2] == 0x06 && buffer[3] == 0x24)
                {
                    uint16_t payload_len = buffer[4] | (buffer[5] << 8);
                    if (i >= payload_len + 8)
                    {
                        byte dynModel = buffer[8]; // offset 2 del payload
                        Serial.println(" Dynamic mode succesfully detected:");
                        Serial.print("🔍 dynModel = ");
                        Serial.println(dynModel);

                        // Modificar variable externa de control
                        *p_dynstate_ext_ctrl_var = dynModel;
                        return;
                    }
                }
            }
        }
    }
    Serial.println("Timeout reading repply NAV5");
}

void alt_proc(byte dyn_state)
{

    if (data_random.altitude < 10000 && data_random.baro_altitude < 10000)
    {
        if (dyn_state == 0)
        {
            // Mode agrees with the altitude
        }
        else if (dyn_state == 6)
        {

            sendUBXmsg(setAirbornedefault, 44);
        }
        else
        {
            Serial.println("... Unknown dynamic mode ...");
        }
    }
    else if ((data_random.altitude > 10000 && data_random.baro_altitude > 10000) && (data_random.altitude < 45000 && data_random.baro_altitude < 45000))
    {
        if (dyn_state == 0)
        {

            sendUBXmsg(setAirborne1g, 44);
        }
        else if (dyn_state == 6)
        {

            // Mode agrees with the altitude
        }
        else
        {
            Serial.println("... Unknown dynamic mode ...");
        }
    }
    else
    {
        Serial.println("... Altitude measurement is not correct. Acquiring GPS ...");
    }
}

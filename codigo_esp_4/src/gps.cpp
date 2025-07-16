#include "gps.h"
#include "data.h"
/*
Definición de variables
*/

TinyGPSPlus _gps; // Definición: crea la variable global real
HardwareSerial _serial_gps(GPS_SERIAL_NUM);

/*
Definición de funciones
*/

void gps_read_loop(void)
{
    while (_serial_gps.available())
    {
        _gps.encode(_serial_gps.read());
    }
}

// Calcula el checksum para una trama
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

// Realiza el envío de un mensaje
void sendUBXmsg(byte *msg, uint16_t length)
{
    // Paso 1: Cálculo del Checksum
    byte ck_a, ck_b;
    calculateUBXChecksum(msg, length, &ck_a, &ck_b);

    // Paso 2: asignar los valores calculados
    msg[length - 2] = ck_a;
    msg[length - 1] = ck_b;

    // Paso 3: Bucle de envío
    for (uint8_t i = 0; i < length; i++)
    {
        _serial_gps.write(msg[i]);
    }
    /*
        // Paso 4: Imprimir trama en consola
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
        Serial.println(); // salto de línea al final
    */
    Serial.println("UBX-CFG-NAV5 enviado ");
}

// Realiza la lectura de un mensaje. Función específica para cada tipo de mensaje
void leerRespuestaNAV5(byte *p_dynstate_ext_ctrl_var)
{
    byte buffer[50];
    int i = 0;

    unsigned long start = millis();
    bool header_found = false;

    while (millis() - start < 3000)
    { // timeout 1 segundo
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
                        Serial.println("✅ Dynamic mode succesfully detected:");
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
    Serial.println("⚠️ Timeout leyendo respuesta NAV5");
}

void alt_proc(byte dyn_state)
{

    if (data_random.altitude < 10000 && data_random.baro_altitude < 10000)
    {
        if (dyn_state == 0)
        {
            // Modo acorde con la altitud
        }
        else if (dyn_state == 6)
        {
            // Envío de la trama para cambiar el modo
            sendUBXmsg(setAirbornedefault, 44);
        }
        else
        {
            Serial.println("... Modo dinámico desconocido, alerta! ...");
        }
    }
    else if ((data_random.altitude > 10000 && data_random.baro_altitude > 10000) && (data_random.altitude < 45000 && data_random.baro_altitude < 45000))
    {
        if (dyn_state == 0)
        {
            // Envío de la trama para cambiar el modo
            sendUBXmsg(setAirborne1g, 44);
        }
        else if (dyn_state == 6)
        {

            // Modo acorde con la altitud
        }
        else
        {
            Serial.println("... Modo dinámico desconocido, alerta! ...");
        }
    }
    else
    {
        Serial.println("... Medida de altitud no es correcta. Adquiriendo GPS ...");
    }
}

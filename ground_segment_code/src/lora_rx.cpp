#include "lora_rx.h"
#include <LoRa.h>
#include "configuration.h"

SPIClass spiLora(VSPI);

byte lora_read_register(byte addr)
{
    spiLora.beginTransaction(SPISettings(1E6, MSBFIRST, SPI_MODE0));
    digitalWrite(NSS, LOW);
    delayMicroseconds(1);
    spiLora.transfer(addr & 0x7F);
    byte value = spiLora.transfer(0x00);
    digitalWrite(NSS, HIGH);
    spiLora.endTransaction();
    return value;
}

bool lora_init(void)
{
    // Configure pinout
    pinMode(NSS, OUTPUT);
    digitalWrite(NSS, HIGH);
    pinMode(RST, OUTPUT);
    pinMode(DIO0, OUTPUT);

    // Begin SPI
    spiLora.begin(SCK, MISO, MOSI, NSS);

    // Tell LoRa to communicate via this SPI bus
    LoRa.setSPI(spiLora);
    LoRa.setPins(NSS, RST, DIO0);

    // Manual reset of the LoRa chip
    digitalWrite(RST, LOW);
    delay(10);
    digitalWrite(RST, HIGH);
    delay(10);

    // Check chip version register
    byte version = lora_read_register(0x42);
    Serial.print("REG_VERSION: 0x");
    Serial.println(version, HEX);

    if (version != 0x12)
    {
        Serial.println("ERROR: LoRa is not responding correctly");
        return false;
    }

    if (!LoRa.begin(LORA_FREQ))
    {
        Serial.println("ERROR: LoRa did not initialize correctly");
        return false;
    }

    LoRa.setSpreadingFactor(LORA_SPREAD_fACTOR);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    // LoRa.setSyncWord(LORA_SYNCWORD);    TODO: UNAVAILABLE NOW

    Serial.println("LoRa configured and ready!");
    return true;
}

int lora_poll_packet(void *buffer, size_t buffer_size)
{
    int packetSize = LoRa.parsePacket();
    if (packetSize <= 0)
    {
        return 0;
    }

    Serial.print("LoRa packet received correctly. Size: ");
    Serial.println(packetSize, DEC);

    if (!LoRa.available())
    {
        return 0;
    }

    // Cap the read at the buffer size to avoid overflow
    int to_read = packetSize;
    if ((size_t)to_read > buffer_size)
    {
        to_read = (int)buffer_size;
    }

    LoRa.readBytes((byte *)buffer, to_read);
    return to_read;
}

void lora_send_packet(const void *data, size_t length)
{
    LoRa.beginPacket();
    LoRa.write((const byte *)data, length);
    LoRa.endPacket();
}
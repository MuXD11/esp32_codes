#include <Arduino.h>
#include <stdlib.h>
#include "configuration.h"
#include "data.h"
#include "lora_rx.h"
#include "server.h"
#include "flash_log.h"
#include "utils.h"

// loop counter for TC queries
static unsigned long sinc_last = 0;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  if (!lora_init())
  {
    while (1)
    {
    }
  }

  oled_init();

  server_wifi_connect();
  screen_print("WiFi connected");

  if (!flash_log_init())
  {
    Serial.println("Flash log init failed - continuing without log");
  }

  srand(time(NULL));
}

void loop()
{
  // Buffer the next packet straight into the global telemetry struct
  int received = lora_poll_packet(&Datos, sizeof(data));

  if (received > 0)
  {
    // Overwrite GPS coordinates with a fixed location (ground-test override)
    Datos.latitude = 41.6488;
    Datos.longitude = -0.9022;
    Datos.altitude = 189;
    Datos.ext_temperature = Datos.temperature + (rand() % 5 - 2) * 0.5;
    Datos.ext_temperature = round(Datos.ext_temperature * 2.0) / 2.0;

    // OLED notification
    screen_show_packet(Datos.seq);

    // Print packet contents in hex
    Serial.print("Seq:");
    Serial.print(Datos.seq, DEC);
    Serial.print(", Bytes: ");
    for (int i = 0; i < received; i++)
    {
      if (i > 0)
        Serial.print(", ");
      byte value = ((byte *)&Datos)[i];
      if (value < 0x10)
        Serial.print("0");
      Serial.print(value, HEX);
    }
    Serial.println();

    // Append to flash log
    flash_log_append_seq(Datos.seq);

    // Dump log every 5 packets
    if (Datos.seq % 5 == 0)
    {
      flash_log_dump_to_serial();
    }

    // Forward to the cloud server
    if (server_wifi_is_connected())
    {
      server_post_telemetry(Datos);
    }
    else
    {
      Serial.println("WiFi disconnected. Retrying...");
      server_wifi_reconnect();
      screen_print("Reconnecting WiFi");
    }
  }
  else
  {
    // Optional: poll server for pending commands when idle. Added functionality
    /*
    if (millis() - sinc_last > 2000)
    {
        sinc_last = millis();
        server_fetch_command();
    }
    */
  }
}
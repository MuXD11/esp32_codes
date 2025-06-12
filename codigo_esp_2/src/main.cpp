#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <LoRa.h>
#include "configuration.h"

// Define LORA SPI object
SPIClass spiLora(VSPI);

// Private functions declaration
float randomInRange(int min, int max);
void screen_print(const char *text);
byte readlorareg(byte adrss);

// WiFi config (2)
/*
const char *ssid = "Livebox6-53EF";
const char *password = "GhSGKhn2Q9R2";
*/
const char *ssid = "S2P";
const char *password = "PASSWORD";

// OLED (I2C)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool screen_initialized = false;

/*
 display.println -> print to buffer
 display.display() -> print to screen
       o
 screen_print("text");      screen_print("⏳ Iniciando...");
*/

// sequence packet counter
int seq_r = 1;

// Data structure
typedef struct
{
  byte TX_ID;
  byte RX_ID;
  int seq;
  float latitude;
  float longitude;
  float altitude;
  float hdop;
  float temperature;
  float pressure;
  float humidity;
  float baro_altitude;
  float ext_temperature;
} data;

data Datos;

// Base URL for API Flask on Render
const char *serverName = "https://iot-app-test1.onrender.com/api/datos";

// Declare WIFI client object
WiFiClientSecure client;

// LorRa chip version
byte version;

//------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // Init and config LORA

  // Config pinout. SPI pins are already config
  pinMode(NSS, OUTPUT);
  digitalWrite(NSS, HIGH);
  pinMode(RST, OUTPUT);
  pinMode(DIO0, OUTPUT);

  // Begin SPI protocol
  spiLora.begin(SCK, MISO, MOSI, NSS);

  // Tell LoRa chip that we are going to communicate via SPI
  LoRa.setSPI(spiLora);
  LoRa.setPins(NSS, RST, DIO0);

  // Manual reset of LoRa chip
  digitalWrite(RST, LOW);
  delay(10);
  digitalWrite(RST, HIGH);
  delay(10);

  // Check integrity of registers
  version = readlorareg(0x42); // 0X42 address contains the chip version
  Serial.print("REG_VERSION: 0x");
  Serial.print(version, HEX);

  if (version != 0x12) // Incorrect response
  {
    Serial.print("ERROR: LoRa no responde correctamente");
    while (1)
    {
    };
  }

  if (!LoRa.begin(LORA_FREQ))
  {
    Serial.print("ERROR: LoRa no se ha inicializado correctamente");
    while (1)
    {
    };
  }

  LoRa.setSpreadingFactor(LORA_SPREAD_fACTOR);
  LoRa.setSignalBandwidth(LORA_BANDWIDTH);
  LoRa.setCodingRate4(LORA_CODING_RATE);
  // LoRa.setSyncWord(LORA_SYNCWORD);    TODO: UNAVAILABLE NOW

  Serial.print("LoRa configured and ready!");

  // Init OLED
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    screen_initialized = true;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("⏳ Iniciando...");
    display.display();
  }
  else
  {
    Serial.println(" No se detectó pantalla OLED");
  }

  // Init WIFI
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Conectado a WiFi");
  if (screen_initialized) // If screen was initialized. var = true
  {
    display.println("✅ WiFi conectado");
    display.display();
  }

  // TODO: Solo para pruebas (ignora certificados SSL)
  client.setInsecure();

  srand(time(NULL));
}

void loop()
{

  int packetSize = LoRa.parsePacket();

  if (packetSize) // Packet received. Size should equal to data struct: 4*10(float and int) + 2*1(byte) = 42B
  {
    Serial.print("LoRa packet received. Size:");
    Serial.print(packetSize, DEC);

    if (LoRa.available()) // There is a byte in the reception buffer
    {
      LoRa.readBytes((byte *)&Datos, packetSize);

      // Clean the screen and notify the new packet
      display.clearDisplay();  // Borra el contenido de la pantalla
      display.setCursor(0, 0); // Reposiciona el cursor arriba a la izquierda
      char msg[32];
      snprintf(msg, sizeof(msg), "📦 Recibido #%d", Datos.seq);
      display.println(msg); // Escribe en buffer
      display.display();    // Refresca pantalla

      // Show received packets
      for (int i = 0; i < packetSize; i++)
      {
        Serial.print("Seq:");
        Serial.print(seq_r, DEC);
        Serial.print("Byte ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(((byte *)&Datos)[i], HEX); // Cast "Datos" direction to a buffer of bytes and iterate
        Serial.println();                       // \n
      }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient https;

      if (https.begin(client, serverName))
      {
        https.addHeader("Content-Type", "application/json");

        // Create JSON
        String jsonData = "[";
        jsonData += "{\"sensor\":\"TX_ID\",\"valor\":" + String(Datos.TX_ID) + "},";
        jsonData += "{\"sensor\":\"RX_ID\",\"valor\":" + String(Datos.RX_ID) + "},";
        jsonData += "{\"sensor\":\"seq\",\"valor\":" + String(Datos.seq) + "},";
        jsonData += "{\"sensor\":\"latitude\",\"valor\":" + String(Datos.latitude, 6) + "},";
        jsonData += "{\"sensor\":\"longitude\",\"valor\":" + String(Datos.longitude, 6) + "},";
        jsonData += "{\"sensor\":\"altitude\",\"valor\":" + String(Datos.altitude, 2) + "},";
        jsonData += "{\"sensor\":\"hdop\",\"valor\":" + String(Datos.hdop, 2) + "},";
        jsonData += "{\"sensor\":\"temperature\",\"valor\":" + String(Datos.temperature, 2) + "},";
        jsonData += "{\"sensor\":\"pressure\",\"valor\":" + String(Datos.pressure, 2) + "},";
        jsonData += "{\"sensor\":\"humidity\",\"valor\":" + String(Datos.humidity, 2) + "},";
        jsonData += "{\"sensor\":\"baro_altitude\",\"valor\":" + String(Datos.baro_altitude, 2) + "},";
        jsonData += "{\"sensor\":\"ext_temperature\",\"valor\":" + String(Datos.ext_temperature, 2) + "}";
        jsonData += "]";

        int httpResponseCode = https.POST(jsonData);

        Serial.print("📡 Enviando POST... Código: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode > 0)
        {
          String respuesta = https.getString();
          Serial.println("📥 Respuesta del servidor:");
          Serial.println(respuesta);
        }
        else
        {
          Serial.print("❌ Error en POST: ");
          Serial.println(https.errorToString(httpResponseCode));

          screen_print("❌ Error en POST");
        }

        https.end();
      }
      else
      {
        Serial.println("❌ Error al conectar al servidor");
        screen_print("❌ Error servidor");
      }
    }
    else
    {
      Serial.println("❌ WiFi desconectado. Reintentando...");
      WiFi.reconnect();
      screen_print("🔄 Reconectando WiFi");
    }

    seq_r++;
  }
  else
  {
    // Serial.println(".");
  }
}

/*
OTHER PRIVATE FUNCTIONS --------------------------------------------------------------------
*/

byte readlorareg(byte adrss)
{
  spiLora.beginTransaction(SPISettings(1E6, MSBFIRST, SPI_MODE0)); // Begin SPI communication with LoRa chip
  digitalWrite(NSS, LOW);                                          // Set the CS to active
  delayMicroseconds(1);
  spiLora.transfer(adrss & 0x7F);      //
  byte value = spiLora.transfer(0x00); // Empty transfer
  digitalWrite(NSS, HIGH);
  spiLora.endTransaction(); // End SPI comm
  return value;
}

float randomInRange(int min, int max)
{
  return min + rand() % (max - min + 1);
}

// 📺 Imprimir texto en pantalla OLED
void screen_print(const char *text)
{
  if (!screen_initialized)
    return;

  static int line = 0;
  display.setCursor(0, line * 10);
  display.println(text);
  display.display();
  line++;
  if (line >= 6)
  {
    line = 0;
    delay(2000);
    display.clearDisplay();
  }
}

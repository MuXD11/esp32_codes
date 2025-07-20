#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <LoRa.h>
#include <LittleFS.h>
#include "configuration.h"

// Define LORA SPI object
SPIClass spiLora(VSPI);

// Private functions declaration
float randomInRange(int min, int max);
void screen_print(const char *text);
byte readlorareg(byte adrss);
void obtenerComandoDesdeServidor();

// WiFi config (2)

const char *ssid = "Livebox6-53EF";
const char *password = "GhSGKhn2Q9R2";

/*
const char *ssid = "S2P";
const char *password = "PASSWORD";
*/

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

// corrupt paquet count
int packet_corrupt = 0;

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

typedef struct
{
  byte Type_of_message;
  int TC_Action_ID;
  float TC_Payload;

} Data_TC_1;

Data_TC_1 Datos_TC_1;

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
  Serial.println();

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

  Serial.print("✅ LoRa configured and ready!");
  Serial.println();

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
  Serial.println();

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

  // Init log file and LittleFS
  if (!LittleFS.begin())
  {
    Serial.println("Error montando LittleFS. Intentando formatear...");
    if (LittleFS.format())
    {
      Serial.println("Formato exitoso. Intentando montar otra vez...");
      if (!LittleFS.begin())
      {
        Serial.println("Fallo definitivo montando LittleFS.");
        return;
      }
    }
    else
    {
      Serial.println("Fallo al formatear LittleFS.");
      return;
    }
  }
  else
  {
    Serial.println("LittleFS montado correctamente.");
  }

  File f = LittleFS.open("/log.txt", FILE_WRITE);
  f.print("LOGGER DEL RECEPTOR V:");
  f.println(2);
  f.print("Fecha: ");
  f.println("30/06/2025");
  f.close();

  // Información sobre el sistema de ficheros:
  size_t total = LittleFS.totalBytes();
  Serial.printf("Total disponible para archivos: %u bytes\n", total);

  // TODO: Solo para pruebas (ignora certificados SSL)
  client.setInsecure();

  srand(time(NULL));
}

void loop()
{

  int packetSize = LoRa.parsePacket();

  if (packetSize) // Packet received. Size should equal to data struct: 4*10(float and int) + 2*1(byte) = 42B
  {
    Serial.print("✅ LoRa packet received correctly. Size:");
    Serial.print(packetSize, DEC);

    if (LoRa.available()) // There is a byte in the reception buffer
    {
      LoRa.readBytes((byte *)&Datos, packetSize);

      // Sobrescribir coordenadas GPS con ubicación fija
      Datos.latitude = 41.6488;
      Datos.longitude = -0.9022;
      Datos.altitude = 189;
      Datos.ext_temperature = Datos.temperature + (rand() % 5 - 2) * 0.5;
      Datos.ext_temperature = round(Datos.ext_temperature * 2.0) / 2.0;

      // Clean the screen and notify the new packet
      display.clearDisplay();  // Borra el contenido de la pantalla
      display.setCursor(0, 0); // Reposiciona el cursor arriba a la izquierda
      char msg[32];
      snprintf(msg, sizeof(msg), "📦 Recibido #%d", Datos.seq);
      display.println(msg); // Escribe en buffer
      display.display();    // Refresca pantalla

      // Show received packets
      Serial.print("Seq:");
      Serial.print(Datos.seq, DEC);
      Serial.print(", Bytes: ");

      for (int i = 0; i < packetSize; i++)
      {
        if (i > 0)
          Serial.print(", "); // Añade coma solo a partir del segundo byte
        byte value = ((byte *)&Datos)[i];
        if (value < 0x10)
          Serial.print("0"); // Asegura dos dígitos hexadecimales
        Serial.print(value, HEX);
      }
      Serial.println();

      // Escribir en el LOG (memoria Flash):
      if (Datos.seq > 0 && Datos.seq < 1000000) // Validar que el número de secuencia sea razonable
      {
        File f = LittleFS.open("/log.txt", FILE_APPEND);
        if (!f || f.isDirectory())
        {
          Serial.println("No se pudo abrir log.txt o es un directorio (ESCRITURA).");
        }
        else
        {
          f.println(Datos.seq);
          f.close();
          Serial.println("📝 Secuencia válida escrita en log.");
        }
      }
      else
      {
        packet_corrupt++;
        Serial.printf("⚠️ Valor de secuencia anómalo (%d), no se guarda.\n", Datos.seq);
      }

      // Apertura de ese log en modo lectura en múltiplos de 5, asi no se lee en cada iteración
      if (Datos.seq % 5 == 0)
      {

        Serial.println("Contenido de /log.txt:\n");

        File logFile = LittleFS.open("/log.txt", FILE_READ);
        if (!logFile || logFile.isDirectory())
        {
          Serial.println("No se pudo abrir log.txt o es un directorio (LECTURA).");
        }
        // Leer el log a la terminal
        while (logFile.available())
        {
          Serial.write(logFile.read());
        }
        logFile.close();
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
  }
  else
  {
    obtenerComandoDesdeServidor();
    //  delay(1000);
    //  Serial.println("No hay datos de telemetría recibidos");
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

// Imprimir texto en pantalla OLED
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

// Obtener comandos envíados al servidor
void obtenerComandoDesdeServidor()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("❌ WiFi no conectado. No se puede obtener comando.");
    return;
  }

  HTTPClient https;
  const char *comando_url = "https://iot-app-test1.onrender.com/api/comando";

  if (https.begin(client, comando_url)) // Prior HTTP connection ended! Important
  {
    int httpCode = https.GET();

    if (httpCode > 0)
    {
      String respuesta = https.getString();
      Serial.println("📥 Comando recibido del servidor:");
      Serial.println(respuesta);

      // Verifica si hay comando real (no null)
      if (respuesta.indexOf("null") == -1)
      {
        // ArduinoJson para parsear:
        String comando = respuesta.substring(respuesta.indexOf(":\"") + 2, respuesta.indexOf("\","));
        String valor = respuesta.substring(respuesta.lastIndexOf(":") + 1, respuesta.indexOf("}"));

        Serial.print("➡️ Comando: ");
        Serial.println(comando);
        Serial.print("➡️ Valor: ");
        Serial.println(valor);

        // Paso a variables para el envío LoRa
        Datos_TC_1.TC_Action_ID = 1;
        Datos_TC_1.Type_of_message = valor.toInt();
        Datos_TC_1.TC_Payload = 0.0;

        // Envío por LoRa a segmento vuelo
        LoRa.beginPacket();
        LoRa.write((byte *)&Datos_TC_1, sizeof(Data_TC_1));
        LoRa.endPacket();

        Serial.println("📤 Comando enviado por LoRa.");

        // Muestra por pantalla
        /*
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "TC: %s = %s", comando.c_str(), valor.c_str());
        screen_print(buffer);
        */
      }
      else
      {
        // Serial.println("ℹ️ No hay comandos pendientes.");
      }
    }
    else
    {
      Serial.printf("❌ Error en GET: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();
  }
  else
  {
    Serial.println("❌ No se pudo conectar al servidor de comandos.");
  }
}

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>

// Declaración de funciones a utilizar
float randomInRange(int min, int max);
void screen_print(const char *text);

// Configura tu red WiFi
const char *ssid = "Livebox6-53EF";
const char *password = "GhSGKhn2Q9R2";

// Pantalla OLED (I2C)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool screen_initialized = false;

// display.println -> print to buffer
// display.display() -> print to screen
//        o
// screen_print("text");      screen_print("⏳ Iniciando...");

// packet counter
int i = 1;

// data structure
typedef struct
{
  int seq;
  float temperature;
  float pressure;
  float humidity;
} data;

data Datos;

// URL API Flask on Render
const char *serverName = "https://iot-app-test1.onrender.com/api/datos";

WiFiClientSecure client;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // Inicializar pantalla OLED
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
    Serial.println("⚠️ No se detectó pantalla OLED");
  }

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

  // Solo para pruebas (ignora certificados SSL)
  client.setInsecure();

  srand(time(NULL));
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient https;

    if (https.begin(client, serverName))
    {
      https.addHeader("Content-Type", "application/json");

      // Create random data
      Datos.seq = i;
      Datos.temperature = randomInRange((int)10, (int)60);
      Datos.pressure = randomInRange((int)10, (int)1000);
      Datos.humidity = randomInRange((int)0, (int)100);

      // Create JSON
      String jsonData = "[";
      jsonData += "{\"sensor\":\"temperatura\",\"valor\":" + String(Datos.temperature, 2) + "},";
      jsonData += "{\"sensor\":\"presion\",\"valor\":" + String(Datos.pressure, 2) + "},";
      jsonData += "{\"sensor\":\"humedad\",\"valor\":" + String(Datos.humidity, 2) + "},";
      jsonData += "{\"sensor\":\"seq\",\"valor\":" + String(Datos.seq) + "}";
      jsonData += "]";

      int httpResponseCode = https.POST(jsonData);

      Serial.print("📡 Enviando POST... Código: ");
      Serial.println(httpResponseCode);

      char buffer[64];
      snprintf(buffer, sizeof(buffer), "Seq: %d", Datos.seq);
      screen_print(buffer);
      snprintf(buffer, sizeof(buffer), "T: %.1f°C", Datos.temperature);
      screen_print(buffer);
      snprintf(buffer, sizeof(buffer), "P: %.1f hPa", Datos.pressure);
      screen_print(buffer);
      snprintf(buffer, sizeof(buffer), "H: %.1f%%", Datos.humidity);
      screen_print(buffer);

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

  delay(10000); // Esperar antes de repetir
  i = i + 1;
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

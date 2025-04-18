#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme; // I2C

void bme280_setup()
{
  if (!bme.begin(0x76))
  {
    Serial.println("No se detecta el BME280. Verifica conexiones y dirección I2C.");
    while (1)
      ;
  }
}

// ⚙️ Configura tu red WiFi
const char *ssid = "Livebox6-53EF";
const char *password = "GhSGKhn2Q9R2";

int i = 1;

typedef struct
{
  int seq;
  float temperature;
  float pressure;
  float humidity;
} data;

data Datos;

// URL de tu API Flask en Render
const char *serverName = "https://iot-app-test1.onrender.com/api/datos";

WiFiClientSecure client;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ Conectado a WiFi");

  //  Solo para pruebas (ignora certificados SSL)
  client.setInsecure();

  // Setup periféricos:
  // Init BME 280
  bme280_setup();
  ;
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient https;

    if (https.begin(client, serverName))
    {
      https.addHeader("Content-Type", "application/json");

      Datos.seq = i;
      Datos.temperature = bme.readTemperature();
      Datos.pressure = bme.readPressure() / 100.0F;
      Datos.humidity = bme.readHumidity();

      // Crear el JSON
      String jsonData = "[";
      jsonData += "{\"sensor\":\"temperatura\",\"valor\":" + String(Datos.temperature, 2) + "},";
      jsonData += "{\"sensor\":\"presion\",\"valor\":" + String(Datos.pressure, 2) + "},";
      jsonData += "{\"sensor\":\"humedad\",\"valor\":" + String(Datos.humidity, 2) + "},";
      jsonData += "{\"sensor\":\"seq\",\"valor\":" + String(Datos.seq) + "}";
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
      }

      https.end();
    }
    else
    {
      Serial.println("❌ Error al conectar al servidor");
    }
  }
  else
  {
    Serial.println("❌ WiFi desconectado. Reintentando...");
    WiFi.reconnect();
  }

  delay(10000); // Esperar 5 segundos antes de repetir
  i = i + 1;
}

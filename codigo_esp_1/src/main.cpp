#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ⚙️ Configura tu red WiFi
const char *ssid = "S2P";
const char *password = "PASSWORD";
int i = 1;

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
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient https;

    if (https.begin(client, serverName))
    {
      https.addHeader("Content-Type", "application/json");

      // Datos de prueba. Puedes reemplazar los valores por mediciones reales.
      String jsonData = "[{\"sensor\":\"temperatura\",\"valor\":" + String(i) + "}, {\"sensor\":\"presion\",\"valor\":1000}]";

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

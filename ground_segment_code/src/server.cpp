#include "server.h"
#include "lora_rx.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

/*
WiFi credentials. Move to configuration.h or a separate secrets file if
you want to keep them out of source control.
*/
static const char *ssid = "Onbekend";
static const char *password = "3155TT78191";

/*
Server endpoints.
*/
static const char *telemetry_url = "https://iot-app-test1.onrender.com/api/datos";
static const char *command_url = "https://iot-app-test1.onrender.com/api/comando";

void server_wifi_connect(void)
{
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
}

bool server_wifi_is_connected(void)
{
    return WiFi.status() == WL_CONNECTED;
}

void server_wifi_reconnect(void)
{
    WiFi.reconnect();
}

int server_post_telemetry(const data &d)
{
    HTTPClient https;
    WiFiClientSecure tls_local;
    https.setTimeout(15000);
    tls_local.setInsecure();
    tls_local.setTimeout(15000);

    if (!https.begin(tls_local, telemetry_url))
    {
        Serial.println("Error connecting to the server");
        return -1;
    }

    https.addHeader("Content-Type", "application/json");

    // Build JSON payload
    String jsonData = "[";
    jsonData += "{\"sensor\":\"TX_ID\",\"valor\":" + String(d.TX_ID) + "},";
    jsonData += "{\"sensor\":\"RX_ID\",\"valor\":" + String(d.RX_ID) + "},";
    jsonData += "{\"sensor\":\"seq\",\"valor\":" + String(d.seq) + "},";
    jsonData += "{\"sensor\":\"latitude\",\"valor\":" + String(d.latitude, 6) + "},";
    jsonData += "{\"sensor\":\"longitude\",\"valor\":" + String(d.longitude, 6) + "},";
    jsonData += "{\"sensor\":\"altitude\",\"valor\":" + String(d.altitude, 2) + "},";
    jsonData += "{\"sensor\":\"hdop\",\"valor\":" + String(d.hdop, 2) + "},";
    jsonData += "{\"sensor\":\"temperature\",\"valor\":" + String(d.temperature, 2) + "},";
    jsonData += "{\"sensor\":\"pressure\",\"valor\":" + String(d.pressure, 2) + "},";
    jsonData += "{\"sensor\":\"humidity\",\"valor\":" + String(d.humidity, 2) + "},";
    jsonData += "{\"sensor\":\"baro_altitude\",\"valor\":" + String(d.baro_altitude, 2) + "},";
    jsonData += "{\"sensor\":\"ext_temperature\",\"valor\":" + String(d.ext_temperature, 2) + "}";
    jsonData += "]";

    int httpResponseCode = https.POST(jsonData);

    Serial.print("Sending POST... Code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0)
    {
        String response = https.getString();
        Serial.println("Server response:");
        Serial.println(response);
    }
    else
    {
        Serial.print("POST error: ");
        Serial.println(https.errorToString(httpResponseCode));
    }

    https.end();
    return httpResponseCode;
}

void server_fetch_command(void)
{
    if (!server_wifi_is_connected())
    {
        Serial.println("WiFi not connected. Cannot fetch command.");
        return;
    }

    WiFiClientSecure tls;
    tls.setInsecure();
    tls.setTimeout(15000);

    HTTPClient https;
    if (!https.begin(tls, command_url))
    {
        Serial.println("Could not start HTTPS connection.");
        return;
    }

    https.addHeader("Accept", "application/json");

    int httpCode = https.GET();

    if (httpCode < 0)
    {
        Serial.printf("Transport error on GET: %s\n", https.errorToString(httpCode).c_str());
        https.end();
        return;
    }

    // 204 No Content -> no pending command
    if (httpCode == 204)
    {
        https.end();
        return;
    }

    if (httpCode != 200)
    {
        Serial.printf("Unexpected HTTP response: %d\n", httpCode);
        https.end();
        return;
    }

    String body = https.getString();
    https.end();

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err)
    {
        Serial.print("Malformed JSON: ");
        Serial.println(err.c_str());
        Serial.println(body);
        return;
    }

    if (!doc.containsKey("Type_of_message") ||
        !doc.containsKey("TC_Action_ID") ||
        !doc.containsKey("TC_Payload"))
    {
        Serial.println("Response without a valid TC.");
        Serial.println(body);
        return;
    }

    int t = doc["Type_of_message"].as<int>();
    int a = doc["TC_Action_ID"].as<int>();
    long p = doc["TC_Payload"].as<long>();

    Datos_TC_1.Type_of_message = static_cast<uint8_t>(t);
    Datos_TC_1.TC_Action_ID = static_cast<uint8_t>(a);
    Datos_TC_1.TC_Payload = static_cast<int32_t>(p);

    Serial.println("Command received from server:");
    Serial.print("  Type_of_message: ");
    Serial.println(Datos_TC_1.Type_of_message);
    Serial.print("  TC_Action_ID:    ");
    Serial.println(Datos_TC_1.TC_Action_ID);
    Serial.print("  TC_Payload:      ");
    Serial.println(Datos_TC_1.TC_Payload);

    // Forward to the flight segment over LoRa
    lora_send_packet(&Datos_TC_1, sizeof(Data_TC_1_t));

    Serial.println("Command sent via LoRa.");
}
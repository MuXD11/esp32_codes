// --- Este codígo envía datos aleatorios empleando el protocolo LoRa
// --- Librerías necesarias ---
#include <SPI.h>
#include <LoRa.h>
#include <LittleFS.h> // Asegúrate de que esté habilitado

// --- Definición de pines SPI y control ---
#define SCK 5
#define MISO 19
#define MOSI 27
#define NSS 18
#define RST 23
#define DIO0 26

// --- Estructura a envíar ---
typedef struct
{
  byte TX_ID = 0x55;
  byte RX_ID = 0xAA;
  int seq;
  float latitude;
  float longitude;
  float altitude;
  float hdop;
  float temperature;
  float pressure;
  float humidity;
  float baro_altitude;
  float ext_temperature_ours;
} Data;

Data data_random;

int i;

// --- Setup ---
void setup()
{
  randomSeed(analogRead(0));

  Serial.begin(115200);
  while (!Serial)
    ;

  // Configurar pines
  pinMode(NSS, OUTPUT);
  digitalWrite(NSS, HIGH);
  pinMode(RST, OUTPUT);
  pinMode(DIO0, INPUT);

  Serial.print("VERSION 1 LOGGER");
  Serial.print("\n");

  // Init log file
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
  f.print("VERSION DEL LOGGER:");
  f.println(1);
  f.print("Fecha: ");
  f.println("29/06/2025");
  f.close();

  // Información sobre el sistema de ficheros:
  size_t total = LittleFS.totalBytes();
  Serial.printf("Total disponible para archivos: %u bytes\n", total);
}

// --- Loop principal ---
void loop()
{

  // Incrementar contador
  i++;

  // Generar datos aleatorios
  data_random.altitude = 271;
  data_random.baro_altitude = random(240, 258);
  data_random.latitude = 0.000000;   // data_random.latitude = 41.64532;
  data_random.longitude = -0.000000; // data_random.longitude = -0.896553;
  data_random.temperature = random(311, 325) / 10.0;
  data_random.ext_temperature_ours = random(301, 315) / 10.0;
  data_random.pressure = random(9883, 9961) / 10.0;
  data_random.humidity = random(700, 731) / 10.0;
  data_random.seq = i;
  data_random.hdop = 0;
  Serial.print("Transmitiendo trama. ");
  Serial.print("Size of data: ");
  Serial.println(sizeof(Data));
  Serial.print(" Trama con Seq:");
  Serial.println(data_random.seq);
  Serial.print("Latitude: ");
  Serial.println(data_random.latitude, 6);
  Serial.print("Longitude: ");
  Serial.println(data_random.longitude, 6);
  Serial.print("Altitude: ");
  Serial.println(data_random.altitude, 2);
  Serial.print("Baro Altitude: ");
  Serial.println(data_random.baro_altitude, 2);
  Serial.print("Temperature (int): ");
  Serial.println(data_random.temperature, 2);
  Serial.print("Temperature (ext): ");
  Serial.println(data_random.ext_temperature_ours, 2);
  Serial.print("Pressure: ");
  Serial.println(data_random.pressure, 2);
  Serial.print("HDOP: ");
  Serial.println(data_random.hdop, 2);
  Serial.print("Humidity: ");
  Serial.println(data_random.humidity, 2);

  // Escribir en el log
  File f = LittleFS.open("/log.txt", FILE_APPEND);
  if (!f || f.isDirectory())
  {
    Serial.println("No se pudo abrir log.txt o es un directorio (ESCRITURA).");
    return;
  }
  f.println(i);
  f.close();

  // Apertura de ese log en modo lectura en múltiplos de 10, asi no se lee en cada iteración
  if (i % 10 == 0)
  {

    Serial.println("Contenido de /log.txt:\n");

    File logFile = LittleFS.open("/log.txt", FILE_READ);
    if (!logFile || logFile.isDirectory())
    {
      Serial.println("No se pudo abrir log.txt o es un directorio (LECTURA).");
      return;
    }
    // Leer el log a la terminal
    while (logFile.available())
    {
      Serial.write(logFile.read());
    }
    logFile.close();
  }

  delay(10000); // Espera 5 segundos entre envíos
}

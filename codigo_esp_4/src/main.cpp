// --- Este codígo envía datos aleatorios empleando el protocolo LoRa
// --- Librerías necesarias ---
#include <SPI.h>
#include <LoRa.h>

// --- Definición de pines SPI y control ---
#define SCK 5
#define MISO 19
#define MOSI 27
#define NSS 18
#define RST 23
#define DIO0 26

// --- Instancia de SPI para LoRa ---
SPIClass spiLoRa(VSPI); // Usa HSPI si ya estás usando VSPI para otra cosa

// --- Función para leer un registro del LoRa ---
byte readLoRaRegister(byte address)
{
  spiLoRa.beginTransaction(SPISettings(1E6, MSBFIRST, SPI_MODE0));
  digitalWrite(NSS, LOW);
  delayMicroseconds(1);
  spiLoRa.transfer(address & 0x7F);    // Bit 7 = 0 (lectura)
  byte value = spiLoRa.transfer(0x00); // Leer valor
  digitalWrite(NSS, HIGH);
  spiLoRa.endTransaction();
  return value;
}

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

  Serial.println("Iniciando sistema...");
  Serial.println("✅BME280 Iniciado correctamente");
  Serial.println("✅Sonda térmica funciona correctamente");

  // Configurar pines
  pinMode(NSS, OUTPUT);
  digitalWrite(NSS, HIGH);
  pinMode(RST, OUTPUT);
  pinMode(DIO0, INPUT);

  // Inicializar SPI
  spiLoRa.begin(SCK, MISO, MOSI, NSS);

  // Configurar LoRa
  LoRa.setSPI(spiLoRa);
  LoRa.setPins(NSS, RST, DIO0);

  // Reset del módulo LoRa
  digitalWrite(RST, LOW);
  delay(10);
  digitalWrite(RST, HIGH);
  delay(10);

  // Leer versión
  byte version = readLoRaRegister(0x42);
  Serial.print("REG_VERSION: 0x");
  Serial.println(version, HEX);

  if (version != 0x12)
  {
    Serial.println("Error: LoRa no responde.");
    while (true)
      ;
  }

  // Inicializar módulo en 868 MHz
  if (!LoRa.begin(868E6))
  {
    Serial.println("Error: No se pudo inicializar LoRa.");
    while (true)
      ;
  }

  // --- Configuración de parámetros ---
  LoRa.setSpreadingFactor(10);    // ← Ajusta aquí (SF7–SF12)
  LoRa.setSignalBandwidth(125E3); // ← Ajusta aquí (BW: 125E3, 250E3, 500E3)
  LoRa.setCodingRate4(5);         // 4/5, 4/6, 4/7, 4/8 opcionales

  Serial.println("✅LoRa listo para transmitir.");
  Serial.println();
}

// --- Loop principal ---
void loop()
{

  // Incrementar contador
  i++;

  // Generar datos aleatorios
  data_random.altitude = 271;

  data_random.baro_altitude = random(240, 258);

  // data_random.latitude = 41.64532;
  data_random.latitude = 0.000000;

  // data_random.longitude = -0.896553;
  data_random.longitude = -0.000000;

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

  // Envío del paquete por LoRa
  LoRa.beginPacket();
  LoRa.write((byte *)&data_random, sizeof(Data));
  LoRa.endPacket();

  delay(10000); // Espera 5 segundos entre envíos
}

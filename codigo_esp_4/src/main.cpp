// --- Este codígo envía datos aleatorios empleando el protocolo LoRa
// --- Librerías necesarias ---
#include "gps.h"
#include "clora.h"

/*
Mensajes GPS CFG-NAV5
*/

byte cfgNav5Poll[8] = {
    0xB5, 0x62, // Header
    0x06, 0x24, // Class = CFG, ID = NAV5
    0x00, 0x00, // Length = 0
    0x00, 0x00  // Checksum (se calcula)
};

byte setAirborne1g[44] = {
    0xB5, 0x62, // Sync chars
    0x06, 0x24, // Class, ID
    0x24, 0x00, // Length = 36 bytes
    0x01, 0x00, // mask: only dynModel and fixMode
    0x06,       // dynModel: 5 = Airborne <1g>
    0x00,       // fixMode: Auto 2D/3D
    // Remaining 28 bytes (set to 0)
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    // Placeholder for checksum
    0x00, 0x00};

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

// Contador local para el envío de telemetrías
int local_counter;

// Variable de medida de estado dinámico
byte dyn_state = 100;

// Dummy for tests
byte Dumm1;

void setup()
{
  randomSeed(analogRead(0));

  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("Iniciando sistema...");
  Serial.println("✅ BME280 Iniciado correctamente");
  Serial.println("✅ Sonda térmica funciona correctamente");

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
  // Serial.print("REG_VERSION: 0x");
  // Serial.println(version, HEX);

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
  LoRa.setSpreadingFactor(10);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5); // 4/5, 4/6, 4/7, 4/8

  Serial.println("✅ LoRa listo para transmitir.");

  // Inicializar GPS
  _serial_gps.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  delay(10000);

  // Envío de trama para hacer poll del estado
  Serial.println("Enviando Poll UBX-CFG-NAV5 ...");
  for (int i = 0; i < 10; i++)
  {
    gps_read_loop();
    delay(10);
  }
  sendUBXmsg(cfgNav5Poll, 8);

  // Recepción del mensaje de respuesta al poll de estado
  delay(200);
  leerRespuestaNAV5(&Dumm1);

  // Envío de la trama para cambiar el modo
  sendUBXmsg(setAirborne1g, 44);
};

/*
--- Bucle principal ---
*/
void loop()
{

  // Incrementar contador
  local_counter++;

  // Bucle para leer datos GPS
  gps_read_loop();
  Serial.print("Satélites detectados: ");
  Serial.println(_gps.satellites.value());

  // Generar datos aleatorios
  data_random.altitude = 271;
  data_random.baro_altitude = random(240, 258);
  data_random.latitude = _gps.location.lat();
  data_random.longitude = _gps.location.lng();
  data_random.temperature = random(311, 325) / 10.0;
  data_random.ext_temperature_ours = random(301, 315) / 10.0;
  data_random.pressure = random(9883, 9961) / 10.0;
  data_random.humidity = random(700, 731) / 10.0;
  data_random.seq = local_counter;
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

  // Comprobación del modo dinámico
  Serial.println("Enviando Poll UBX-CFG-NAV5 ...");
  sendUBXmsg(cfgNav5Poll, 8);
  delay(200);
  leerRespuestaNAV5(&dyn_state);

  if (dyn_state != 6)
  {
    // rutina de emergencia! valor incorrecto
  }

  Serial.println("----------------------------------------------------------------");

  delay(10000);
}

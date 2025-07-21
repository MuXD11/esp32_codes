// --- Este codígo envía datos aleatorios empleando el protocolo LoRa
// --- Librerías necesarias ---
#include "gps.h"
#include "clora.h"
#include "data.h"
#include "tc_handling.h"
#include "configuration.h"

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
    0x06,       // dynModel: 6 = Airborne <1g>
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

byte setAirbornedefault[44] = {
    0xB5, 0x62, // Sync chars
    0x06, 0x24, // Class, ID
    0x24, 0x00, // Length = 36 bytes
    0x01, 0x00, // mask: only dynModel and fixMode
    0x00,       // dynModel: 0 = Portable
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
Data data_random;

// --- Estructuras de telecomandos
Data_TC_1 data_tc_1;

// Contador local para el envío de telemetrías
int local_counter;

// Variable de medida de estado dinámico
byte dyn_state = 100;

// Variable for TC control
bool TC_ARRIVED;

int loop_period_ms = default_local_period;

void setup()
{
  // Inicialziación de variables
  randomSeed(analogRead(0));
  TC_ARRIVED = false;

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

  if (version != LORA_CHIP_VER)
  {
    Serial.println("Error: LoRa no responde.");
    while (true)
      ;
  }

  // Inicializar módulo en 868 MHz
  if (!LoRa.begin(DEFAULT_LORA_FQ))
  {
    Serial.println("Error: No se pudo inicializar LoRa.");
    while (true)
      ;
  }

  // --- Configuración de parámetros ---
  LoRa.setSpreadingFactor(DEFAULT_LORA_SF);
  LoRa.setSignalBandwidth(DEFAULT_LORA_BW);
  LoRa.setCodingRate4(DEFAULT_LORA_CR);
  LoRa.setTxPower(DEFAULT_LORA_POWER);

  Serial.println("✅ LoRa listo para transmitir.");

  // Inicializar GPS
  _serial_gps.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  delay(10000);

  // Poll al GPS
  Serial.println("Enviando Poll UBX-CFG-NAV5 ...");

  // Activación previa de la UART para evitar errores
  for (int i = 0; i < 10; i++)
  {
    gps_read_loop();
    delay(10);
  }

  // Envío de trama para hacer poll
  sendUBXmsg(cfgNav5Poll, 8);

  // Recepción del mensaje de respuesta al poll de estado
  delay(200);
  leerRespuestaNAV5(&dyn_state);

  // Envío de la trama para cambiar el modo
  // sendUBXmsg(setAirborne1g, 44);
};

/*
--- Bucle principal ---
*/
void loop()
{
  // Incrementar contador de telemetría
  local_counter++;

  // Toma de medida de tiempo
  unsigned long t_inicio_loop = millis();

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /* fragmento de prueba: Inyección de TC por código identificador */
  /*
  if (local_counter == 5)
  {
    TC_ARRIVED = true;
    TC_code = 1;
  }
  else if (local_counter == 6)
  {
    TC_ARRIVED = false;
    TC_code = 0;
  }
  */
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // Comprobar los telecomandos recibidos
  if (TC_ARRIVED == true)
  {
    dispatch_telecommand();
    TC_ARRIVED = false;
  }

  // Bucle para leer datos GPS
  gps_read_loop();
  Serial.print("Satélites detectados: ");
  Serial.println(_gps.satellites.value());

  // Generar datos aleatorios
  data_random.altitude = 271;
  data_random.baro_altitude = random(240, 258);

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /* fragmento de prueba: Valor de altitud elevado para comprobar el cambio automático de modo dinámico del GPS*/
  if ((local_counter > 10) && (local_counter < 20))
  {
    data_random.altitude = 14400;
    data_random.baro_altitude = 14200;
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  data_random.latitude = _gps.location.lat();
  data_random.longitude = _gps.location.lng();
  data_random.hdop = _gps.hdop.hdop();
  data_random.temperature = random(311, 325) / 10.0;
  data_random.ext_temperature_ours = random(301, 315) / 10.0;
  data_random.pressure = random(9883, 9961) / 10.0;
  data_random.humidity = random(700, 731) / 10.0;
  data_random.seq = local_counter;

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

  // Comprobación del modo dinámico del GPS
  Serial.println("Enviando Poll UBX-CFG-NAV5 ...");
  sendUBXmsg(cfgNav5Poll, 8);
  delay(200);
  leerRespuestaNAV5(&dyn_state);

  // Comprobación de modo de altitud
  alt_proc(dyn_state);

  // Envío del paquete por LoRa
  LoRa.beginPacket();
  LoRa.write((byte *)&data_random, sizeof(Data));
  LoRa.endPacket();

  // Toma de medida de tiempo
  unsigned long t_end_loop = millis();
  unsigned long t_duration_loop = (t_end_loop - t_inicio_loop);

  if (t_duration_loop > loop_period_ms)
  {
    Serial.println("ERROR: LOOP TOOK MORE THAN 10 SECONDS");
  }

  unsigned long t_remaining = loop_period_ms - t_duration_loop; // Cantidad de tiempo restante del periodo

  Serial.println("ENTRANDO EN ESCUCHA ACTIVA");
  while ((millis() - t_end_loop) < t_remaining) // while there is period time remaining
  {

    // Escucha en LoRa
    int packetsize = LoRa.parsePacket();
    if (packetsize) // Packet received. Size should equal to data TODO: Define TC/TM system
    {
      Serial.println("Paquete recibido");
      Serial.print(packetsize, DEC);

      // Flag de recepción
      TC_ARRIVED = true;

      if (LoRa.available())
      {
        LoRa.readBytes((byte *)&data_tc_1, packetsize); // Al copiarlo a esa estructura, no hace falta pasar params, ya que todos los archivos tienen acceso a esa estructura        Serial.println("Acción a realizar");
        Serial.print(data_tc_1.TC_Action_ID, DEC);
      }
    }

    Serial.println("-");
    delay(1000);
  }
  Serial.println("SALIENDO DE ESCUCHA ACTIVA");

  Serial.println("----------------------------------------------------------------");
}

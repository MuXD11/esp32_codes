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

// --- Funciones auxiliares ---

// Leer un registro del LoRa (lectura segura con bit 7 = 0)
byte readLoRaRegister(byte address)
{
  spiLoRa.beginTransaction(SPISettings(1E6, MSBFIRST, SPI_MODE0));
  digitalWrite(NSS, LOW);
  delayMicroseconds(1);
  spiLoRa.transfer(address & 0x7F);    // Enviar dirección con bit 7 = 0 (lectura)
  byte value = spiLoRa.transfer(0x00); // Recibir el valor
  digitalWrite(NSS, HIGH);
  spiLoRa.endTransaction();
  return value;
}

// --- Setup ---
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("Iniciando sistema...");

  // Configurar pines
  pinMode(NSS, OUTPUT);
  digitalWrite(NSS, HIGH);
  pinMode(RST, OUTPUT);
  pinMode(DIO0, INPUT);

  // Inicializar SPI para LoRa
  spiLoRa.begin(SCK, MISO, MOSI, NSS);

  // Configurar librería LoRa
  LoRa.setSPI(spiLoRa);
  LoRa.setPins(NSS, RST, DIO0);

  // Reset manual del LoRa
  digitalWrite(RST, LOW);
  delay(10);
  digitalWrite(RST, HIGH);
  delay(10);

  // Comprobar REG_VERSION
  byte version = readLoRaRegister(0x42);
  Serial.print("REG_VERSION: 0x");
  Serial.println(version, HEX);

  if (version != 0x12)
  {
    Serial.println("Error: LoRa no responde correctamente.");
    while (true)
      ;
  }

  // Inicializar LoRa
  if (!LoRa.begin(868E6))
  {
    Serial.println("Error: No se pudo inicializar LoRa.");
    while (true)
      ;
  }

  // Configuración de parámetros de radio
  LoRa.setSpreadingFactor(7);     // SF7
  LoRa.setSignalBandwidth(125E3); // 125 kHz
  LoRa.setCodingRate4(5);         // 4/5
  // LoRa.setSyncWord(0x34);         // Sync Word por defecto

  // Activar recepción continua
  LoRa.receive();

  Serial.println("LoRa inicializado correctamente. Esperando paquetes...");
}

// --- Loop principal ---
void loop()
{
  int packetSize = LoRa.parsePacket();

  if (packetSize)
  {
    Serial.print("Paquete recibido: '");

    while (LoRa.available())
    {
      Serial.print((char)LoRa.read());
    }

    Serial.print("' | RSSI: ");
    Serial.println(LoRa.packetRssi());
  }
  // Sin delay para mantener escucha continua
}
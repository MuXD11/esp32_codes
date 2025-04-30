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
  LoRa.setSpreadingFactor(12);  // ← Ajusta aquí (SF7–SF12)
  LoRa.setSignalBandwidth(7E3); // ← Ajusta aquí (BW: 125E3, 250E3, 500E3)
  LoRa.setCodingRate4(5);       // 4/5, 4/6, 4/7, 4/8 opcionales

  Serial.println("LoRa listo para transmitir.");
}

// --- Loop principal ---
void loop()
{
  // Alternar bit 0 y 1 en cada transmisión (opcional)
  static bool bit = false;
  byte mensaje = bit ? 0x01 : 0x00;
  bit = !bit;

  Serial.print("Transmitiendo bit: ");
  Serial.println(mensaje);

  LoRa.beginPacket();
  LoRa.write(mensaje); // Transmitir un solo byte
  LoRa.endPacket();

  delay(5000); // Espera 5 segundos entre envíos
}

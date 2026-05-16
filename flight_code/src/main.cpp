// --- This code sends sensor data using the LoRa protocol ---
// --- Required libraries ---
#include "gps.h"
#include "clora.h"
#include "data.h"
#include "tc_handling.h"
#include "configuration.h"

/*
GPS CFG-NAV5 messages
*/

byte cfgNav5Poll[8] = {
    0xB5, 0x62, // Header
    0x06, 0x24, // Class = CFG, ID = NAV5
    0x00, 0x00, // Length = 0
    0x00, 0x00  // Checksum (computed)
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

// --- Telemetry structure ---
Data data_random;

// --- Telecommand structure ---
Data_TC_1 data_tc_1;

// ==== FIX 5: Explicit global initialization ====
// Local counter for telemetry sequence numbering
int local_counter = 0;

// GPS dynamic state variable (100 = unknown, set by first NAV5 poll)
byte dyn_state = 100;

// TC reception flag
bool TC_ARRIVED = false;

// Loop period in ms. Unsigned to match millis() arithmetic and avoid
// signed/unsigned comparison warnings against t_duration_loop.
unsigned long loop_period_ms = 10000;

void setup()
{
  // Variable initialization
  randomSeed(analogRead(0));

  Serial.begin(115200);
  while (!Serial)
    ;

  DEBUG_PRINTLN("Starting system...");

  // Initialize sensors (BME280 over I2C)
  if (!init_sensors())
  {
    DEBUG_PRINTLN("Sensor init failed — continuing without BME280");
    // Note: we don't halt here. The TC system and LoRa link should remain
    // functional even if the environmental sensors are dead, so the device
    // can still report status and accept reconfiguration commands.
  }

  // Configure pins
  pinMode(NSS, OUTPUT);
  digitalWrite(NSS, HIGH);
  pinMode(RST, OUTPUT);
  pinMode(DIO0, INPUT);

  // Initialize SPI
  spiLoRa.begin(SCK, MISO, MOSI, NSS);

  // Configure LoRa
  LoRa.setSPI(spiLoRa);
  LoRa.setPins(NSS, RST, DIO0);

  // Reset LoRa module
  digitalWrite(RST, LOW);
  delay(10);
  digitalWrite(RST, HIGH);
  delay(10);

  // Read version
  byte version = readLoRaRegister(0x42);

  if (version != LORA_CHIP_VER)
  {
    DEBUG_PRINTLN("Error: LoRa not responding.");
    while (true)
      ;
  }

  // Initialize module at 868 MHz
  if (!LoRa.begin(DEFAULT_LORA_FQ))
  {
    DEBUG_PRINTLN("Error: Could not initialize LoRa.");
    while (true)
      ;
  }

  // --- Parameter configuration ---
  LoRa.setSpreadingFactor(DEFAULT_LORA_SF);
  LoRa.setSignalBandwidth(DEFAULT_LORA_BW);
  LoRa.setCodingRate4(DEFAULT_LORA_CR);
  LoRa.setTxPower(DEFAULT_LORA_POWER);

  DEBUG_PRINTLN("LoRa ready to transmit.");

  // Initialize GPS
  _serial_gps.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  delay(10000);

  // GPS poll
  DEBUG_PRINTLN("Sending UBX-CFG-NAV5 poll...");

  // Prior UART activation to avoid errors
  for (int i = 0; i < 10; i++)
  {
    gps_read_loop();
    delay(10);
  }

  // Send poll frame
  sendUBXmsg(cfgNav5Poll, 8);

  // Receive response to status poll
  delay(200);
  leerRespuestaNAV5(&dyn_state);

  // Send frame to change mode
  // sendUBXmsg(setAirborne1g, 44);
};

/*
--- Main loop ---
*/
void loop()
{
  // Increment telemetry counter
  local_counter++;

  // Loop start timestamp
  unsigned long t_inicio_loop = millis();

  // ==== FIX 6: Test fragment for artificial TC injection, gated by flag ====
#if ENABLE_TC_TEST_INJECTION
  if (local_counter == 5)
  {
    TC_ARRIVED = true;
    data_tc_1.TC_Action_ID = 1;
    data_tc_1.TC_Payload = 0;
  }
  else if (local_counter == 6)
  {
    TC_ARRIVED = false;
    data_tc_1.TC_Action_ID = 0;
    data_tc_1.TC_Payload = 0;
  }
#endif

  // Check received TCs
  if (TC_ARRIVED == true)
  {
    dispatch_telecommand(data_tc_1); // Pass by reference as constant structure

    // Reset control variable and TC variable
    TC_ARRIVED = false;

    data_tc_1.Type_of_message = 0;
    data_tc_1.TC_Action_ID = 0;
    data_tc_1.TC_Payload = 0;
  }

  // GPS data read loop
  gps_read_loop();
  DEBUG_PRINT("Satellites detected: ");
  DEBUG_PRINTLN(_gps.satellites.value());

  // Update data
  gather_sensor_data();
  data_random.seq = local_counter;

  // ==== FIX 6: Test fragment for high altitude injection, gated by flag ====
#if ENABLE_ALTITUDE_TEST_INJECTION
  if ((local_counter > 10) && (local_counter < 20))
  {
    data_random.altitude = 14400;
    data_random.baro_altitude = 14200;
  }
#endif

  DEBUG_PRINT("Transmitting frame. ");
  DEBUG_PRINT("Size of data: ");
  DEBUG_PRINTLN(sizeof(Data));
  DEBUG_PRINT(" Frame with Seq: ");
  DEBUG_PRINTLN(data_random.seq);
  DEBUG_PRINT("Latitude: ");
  DEBUG_PRINTLN(data_random.latitude);
  DEBUG_PRINT("Longitude: ");
  DEBUG_PRINTLN(data_random.longitude);
  DEBUG_PRINT("Altitude: ");
  DEBUG_PRINTLN(data_random.altitude);
  DEBUG_PRINT("Baro Altitude: ");
  DEBUG_PRINTLN(data_random.baro_altitude);
  DEBUG_PRINT("Temperature (int): ");
  DEBUG_PRINTLN(data_random.temperature);
  DEBUG_PRINT("Temperature (ext): ");
  DEBUG_PRINTLN(data_random.ext_temperature_ours);
  DEBUG_PRINT("Pressure: ");
  DEBUG_PRINTLN(data_random.pressure);
  DEBUG_PRINT("HDOP: ");
  DEBUG_PRINTLN(data_random.hdop);
  DEBUG_PRINT("Humidity: ");
  DEBUG_PRINTLN(data_random.humidity);

  // GPS dynamic mode check
  DEBUG_PRINTLN("Sending UBX-CFG-NAV5 poll...");
  sendUBXmsg(cfgNav5Poll, 8);
  delay(200);
  leerRespuestaNAV5(&dyn_state);

  // Altitude mode check
  alt_proc(dyn_state);

  // Send packet over LoRa
  LoRa.beginPacket();
  LoRa.write((byte *)&data_random, sizeof(Data));
  LoRa.endPacket();

  // Loop end timestamp
  unsigned long t_end_loop = millis();
  unsigned long t_duration_loop = (t_end_loop - t_inicio_loop);

  // ==== FIX 1: Guard against unsigned underflow when loop overruns the period ====
  unsigned long t_remaining;
  if (t_duration_loop >= loop_period_ms)
  {
    DEBUG_PRINTLN("ERROR: LOOP TOOK MORE THAN PERIOD");
    t_remaining = 0;
  }
  else
  {
    t_remaining = loop_period_ms - t_duration_loop;
  }

  DEBUG_PRINTLN("ENTERING ACTIVE LISTENING");
  while ((millis() - t_end_loop) < t_remaining) // while there is period time remaining
  {

    // LoRa listening
    int packetsize = LoRa.parsePacket();

    // ==== FIX 2 + 7: Validate packet size before copying, use sizeof consistently ====
    if (packetsize == (int)sizeof(Data_TC_1))
    {
      DEBUG_PRINT("Packet received: ");
      DEBUG_PRINTLN(packetsize);

      if (LoRa.available())
      {
        LoRa.readBytes((byte *)&data_tc_1, sizeof(Data_TC_1));

        // Raise the flag only after the bytes are actually read
        TC_ARRIVED = true;

        DEBUG_PRINT("Action to perform: ");
        DEBUG_PRINTLN(data_tc_1.TC_Action_ID);
      }
    }
    else if (packetsize > 0)
    {
      // Unexpected size: drain the buffer and ignore
      DEBUG_PRINT("Packet received with unexpected size: ");
      DEBUG_PRINTLN(packetsize);
      while (LoRa.available())
      {
        LoRa.read();
      }
    }

    DEBUG_PRINTLN("-");
    delay(2000); // Adjustable period to atempt a new read (depends on the configuration of the server)
  }
  DEBUG_PRINTLN("EXITING ACTIVE LISTENING");

  DEBUG_PRINTLN("----------------------------------------------------------------");
}
#include "gps.h"
#include "data.h"
#include "configuration.h"
#include <Wire.h>
#include <Adafruit_BME280.h>

/*
Variable definitions
*/

// BME280 sensor instance (I2C). Default address is 0x76 on most modules,
// some boards use 0x77 — adjust BME280_I2C_ADDR in configuration.h if needed.
Adafruit_BME280 bme;

// Sea-level reference pressure in hPa for barometric altitude calculation.
// 1013.25 is the ISA standard; replace with the local QNH for better accuracy.
static float sea_level_pressure_hpa = 1013.25f;

/*
Function definitions
*/

void gather_sensor_data(void)
{
    if (CURRENT_MODE_DEBUG)
    {
        // Debug mode: simulated values, real GPS lat/lon/HDOP
        data_random.altitude = 271;
        data_random.baro_altitude = random(240, 258);
        data_random.latitude = _gps.location.lat();
        data_random.longitude = _gps.location.lng();
        data_random.hdop = _gps.hdop.hdop();
        data_random.temperature = random(311, 325) / 10.0;
        data_random.ext_temperature_ours = random(301, 315) / 10.0;
        data_random.pressure = random(9883, 9961) / 10.0;
        data_random.humidity = random(700, 731) / 10.0;
    }
    else if (CURRENT_MODE_DEBUG_SENSOR_ON)
    {
        // Real sensor sampling

        // Guard each field against invalid fixes so we don't transmit (0,0)
        // on cold boot or fix loss.
        if (_gps.location.isValid())
        {
            data_random.latitude = _gps.location.lat();
            data_random.longitude = _gps.location.lng();
        }
        else
        {
            data_random.latitude = 0.0f;
            data_random.longitude = 0.0f;
        }

        if (_gps.altitude.isValid())
        {
            data_random.altitude = _gps.altitude.meters();
        }
        else
        {
            data_random.altitude = 0.0f;
        }

        if (_gps.hdop.isValid())
        {
            data_random.hdop = _gps.hdop.hdop();
        }
        else
        {
            data_random.hdop = 99.99f; // Sentinel for "no fix"
        }

        // --- BME280: temperature, pressure, humidity, barometric altitude ---
        float t = bme.readTemperature(); // degC
        float p = bme.readPressure();    // Pa
        float h = bme.readHumidity();    // %RH

        if (!isnan(t))
        {
            // Internal and external temperature both come from the BME280. Replace ext_temperature_ours with the dedicated
            // external probe reading once that driver is integrated.
            data_random.temperature = t;
            data_random.ext_temperature_ours = t;
        }

        if (!isnan(p))
        {
            // Store pressure in hPa to match the units used in the rest of
            // the system
            data_random.pressure = p / 100.0f;

            // Barometric altitude from pressure using the international
            // barometric formula.
            data_random.baro_altitude = bme.readAltitude(sea_level_pressure_hpa);
        }

        if (!isnan(h))
        {
            data_random.humidity = h;
        }
    }
}

bool init_sensors(void)
{
    // I2C bus init. Pins come from configuration.h
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    if (!bme.begin(BME280_I2C_ADDR, &Wire))
    {
        DEBUG_PRINTLN("ERROR: BME280 not found at configured address");
        return false;
    }

    // Recommended settings for indoor and slow-changing-environment use
    bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                    Adafruit_BME280::SAMPLING_X2,  // temperature
                    Adafruit_BME280::SAMPLING_X16, // pressure
                    Adafruit_BME280::SAMPLING_X1,  // humidity
                    Adafruit_BME280::FILTER_X16,
                    Adafruit_BME280::STANDBY_MS_500);

    DEBUG_PRINTLN("BME280 initialized");
    return true;
}
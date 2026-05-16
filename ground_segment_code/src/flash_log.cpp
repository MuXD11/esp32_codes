#include "flash_log.h"
#include <LittleFS.h>

static const char *LOG_PATH = "/log.txt";

// Counter for packets rejected as corrupt by flash_log_append_seq.
static int packet_corrupt = 0;

bool flash_log_init(void)
{
    if (!LittleFS.begin())
    {
        Serial.println("Error mounting LittleFS. Trying to format...");
        if (LittleFS.format())
        {
            Serial.println("Format successful. Trying to mount again...");
            if (!LittleFS.begin())
            {
                Serial.println("Definitive failure mounting LittleFS.");
                return false;
            }
        }
        else
        {
            Serial.println("Failed to format LittleFS.");
            return false;
        }
    }
    else
    {
        Serial.println("LittleFS mounted correctly.");
    }

    File f = LittleFS.open(LOG_PATH, FILE_WRITE);
    if (!f)
    {
        Serial.println("Could not open log file for header write.");
        return false;
    }
    f.print("RECEIVER LOGGER V:");
    f.println(2);
    f.print("Date: ");
    f.println("30/06/2025");
    f.close();

    size_t total = LittleFS.totalBytes();
    Serial.printf("Total available for files: %u bytes\n", total);

    return true;
}

bool flash_log_append_seq(int seq)
{
    // Validate that the sequence number is reasonable
    if (seq <= 0 || seq >= 1000000)
    {
        packet_corrupt++;
        Serial.printf("Anomalous sequence value (%d), not saved.\n", seq);
        return false;
    }

    File f = LittleFS.open(LOG_PATH, FILE_APPEND);
    if (!f || f.isDirectory())
    {
        Serial.println("Could not open log.txt or it is a directory (WRITE).");
        return false;
    }

    f.println(seq);
    f.close();
    Serial.println("Valid sequence written to log.");
    return true;
}

void flash_log_dump_to_serial(void)
{
    Serial.println("Contents of /log.txt:\n");

    File logFile = LittleFS.open(LOG_PATH, FILE_READ);
    if (!logFile || logFile.isDirectory())
    {
        Serial.println("Could not open log.txt or it is a directory (READ).");
        return;
    }

    while (logFile.available())
    {
        Serial.write(logFile.read());
    }
    logFile.close();
}

int flash_log_corrupt_count(void)
{
    return packet_corrupt;
}
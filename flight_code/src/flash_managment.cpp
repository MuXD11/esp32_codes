#include "flash_managment.h"
#include "configuration.h"
#include <LittleFS.h>

/*
Function definitions
*/

bool init_flash_memory(void)
{
    DEBUG_PRINTLN("Initializing LittleFS...");

    // begin(formatOnFail = true): if the partition is unformatted on first
    // boot after flashing, format it automatically rather than failing.
    if (!LittleFS.begin(true))
    {
        DEBUG_PRINTLN("ERROR: Failed to mount LittleFS");
        return false;
    }

    DEBUG_PRINT("LittleFS mounted. Total: ");
    DEBUG_PRINT(LittleFS.totalBytes());
    DEBUG_PRINT(" bytes, Used: ");
    DEBUG_PRINT(LittleFS.usedBytes());
    DEBUG_PRINTLN(" bytes");

    return true;
}

int flash_write_bytes(const char *path, const uint8_t *data, size_t length)
{
    if (path == nullptr || data == nullptr || length == 0)
    {
        DEBUG_PRINTLN("ERROR: flash_write_bytes invalid arguments");
        return -1;
    }

    File f = LittleFS.open(path, CURRENT_USAGE_MODE);
    if (!f)
    {
        DEBUG_PRINT("ERROR: Could not open file for write: ");
        DEBUG_PRINTLN(path);
        return -1;
    }

    size_t written = f.write(data, length);
    f.close();

    if (written != length)
    {
        DEBUG_PRINT("WARN: Short write. Requested ");
        DEBUG_PRINT(length);
        DEBUG_PRINT(", wrote ");
        DEBUG_PRINTLN(written);
    }

    return (int)written;
}

int flash_read_bytes(const char *path, uint8_t *buffer, size_t max_length)
{
    if (path == nullptr || buffer == nullptr || max_length == 0)
    {
        DEBUG_PRINTLN("ERROR: flash_read_bytes invalid arguments");
        return -1;
    }

    File f = LittleFS.open(path, FILE_READ);
    if (!f)
    {
        DEBUG_PRINT("ERROR: Could not open file for read: ");
        DEBUG_PRINTLN(path);
        return -1;
    }

    size_t read_bytes = f.read(buffer, max_length);
    f.close();

    return (int)read_bytes;
}
#ifndef FLASHMANAGMENT_H
#define FLASHMANAGMENT_H

#include <Arduino.h>

// Usage mode

#define CURRENT_USAGE_MODE FILE_APPEND

#ifndef CURRENT_USAGE_MODE
#define CURRENT_USAGE_MODE FILE_WRITE
#endif

/*
Function declarations
*/

/**
 * @brief Initializes the LittleFS filesystem.
 *
 * Mounts LittleFS, formatting the partition automatically if it has not
 * been formatted yet. Must be called once from setup() before any
 * flash_write_bytes or flash_read_bytes call.
 *
 * @return true on success, false if the filesystem could not be mounted
 */
bool init_flash_memory(void);

/**
 * @brief Writes a buffer of bytes to a file.
 *
 * Opens the file in append mode, writes the buffer, and closes the file.
 * If the file does not exist it is created. Each call commits to flash
 * before returning.
 *
 * @param path     File path, must start with '/' (e.g. "/data.bin")
 * @param data     Pointer to the byte buffer to write
 * @param length   Number of bytes to write
 * @return Number of bytes actually written, or -1 on error
 */
int flash_write_bytes(const char *path, const uint8_t *data, size_t length);

/**
 * @brief Reads bytes from a file into a buffer.
 *
 * Opens the file in read mode and reads up to max_length bytes into the
 * provided buffer.
 *
 * @param path        File path, must start with '/'
 * @param buffer      Destination buffer
 * @param max_length  Maximum number of bytes to read (buffer capacity)
 * @return Number of bytes actually read, or -1 on error
 */
int flash_read_bytes(const char *path, uint8_t *buffer, size_t max_length);

#endif
#ifndef FLASH_LOG_H
#define FLASH_LOG_H

#include <Arduino.h>

/**
 * @brief Initializes LittleFS and writes the log header.
 *
 * Mounts the filesystem, formatting on failure as a recovery path.
 * Then opens /log.txt in write mode and writes a version + date header.
 *
 * @return true on success, false if mount + format both fail
 */
bool flash_log_init(void);

/**
 * @brief Appends a sequence number to the log file.
 *
 * Validates the sequence is in a reasonable range (1 to 999999) before
 * writing. Out-of-range values are rejected and the corrupt counter is
 * incremented.
 *
 * @param seq Sequence number to append
 * @return true if written, false if rejected or write failed
 */
bool flash_log_append_seq(int seq);

/**
 * @brief Dumps the full contents of /log.txt to the serial port.
 *
 * Intended for debug / verification. Called from the main loop every N
 * packets so the operator can see the log grow in real time.
 */
void flash_log_dump_to_serial(void);

/**
 * @brief Returns the number of corrupt packets seen so far.
 */
int flash_log_corrupt_count(void);

#endif
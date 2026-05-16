#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

/**
 * @brief Initializes the OLED display.
 *
 * @return true if the display was found and initialized, false otherwise
 */
bool oled_init(void);

/**
 * @brief Prints text on the OLED, wrapping every 6 lines.
 *
 * No-op if the display was not initialized successfully.
 */
void screen_print(const char *text);

/**
 * @brief Clears the OLED and prints a centered message at the top.
 *
 * Used to refresh the display each time a new packet arrives.
 */
void screen_show_packet(int seq);

/**
 * @brief Returns a random integer in [min, max] inclusive, as a float.
 */
float randomInRange(int min, int max);

#endif
#include "utils.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "configuration.h"
#include <stdlib.h>

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static bool screen_initialized = false;

bool oled_init(void)
{
    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        screen_initialized = true;
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Starting...");
        display.display();
        return true;
    }
    Serial.println("OLED display not detected");
    return false;
}

void screen_print(const char *text)
{
    if (!screen_initialized)
        return;

    static int line = 0;
    display.setCursor(0, line * 10);
    display.println(text);
    display.display();
    line++;
    if (line >= 6)
    {
        line = 0;
        delay(2000);
        display.clearDisplay();
    }
}

void screen_show_packet(int seq)
{
    if (!screen_initialized)
        return;

    display.clearDisplay();
    display.setCursor(0, 0);
    char msg[32];
    snprintf(msg, sizeof(msg), "Received #%d", seq);
    display.println(msg);
    display.display();
}

float randomInRange(int min, int max)
{
    return min + rand() % (max - min + 1);
}
#include "DevDashM5Core2.h"
#include <M5Core2.h>

bool DevDashM5Core2::begin() {
    // setup LVGL, WifiManager, SensorDashboard, etc.
    M5.begin();
    M5.Lcd.fillScreen(RED); // Clear the screen with red color
    return true;
}

void DevDashM5Core2::loop() {
    // handle UI tasks
}

void DevDashM5Core2::destroy() {
    // tear down components
}
#include <Arduino.h>
#include <M5Core2.h>
#include "DevDash.h"

void setup() {
    Serial.begin(115200);
    M5.begin();
    GestureTrigger::Config cfg{
        GestureTrigger::Type::LongPress,
        GestureTrigger::Button::B,
        1500
    };
    DevDash::begin("M5Core2", cfg);
}

void loop() {
    M5.update();
    DevDash::loop();
}

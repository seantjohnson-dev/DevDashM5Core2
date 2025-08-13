#include <DevDash.h>

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    GestureTrigger::Config cfg{
      GestureTrigger::Type::LongPress,
      GestureTrigger::Button::B,
      1500
    };
    DevDash::begin("M5Core2", cfg);
}

void loop() {
    DevDash::loop();
}

#include <DevDash.h>

void setup() {
  Serial.begin(115200);
  DevDash::begin("M5Core2");
}

void loop() {
  DevDash::loop();
}

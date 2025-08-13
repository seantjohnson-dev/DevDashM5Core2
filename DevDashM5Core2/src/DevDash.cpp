#include <Arduino.h>
#include "DevDash.h"
#include <WString.h>
#include "DevDashM5Core2/DevDashM5Core2.h"

String DevDash::deviceName = "M5Core2";
IDevice* DevDash::device   = nullptr;
GestureTrigger* DevDash::gesture = nullptr;

void DevDash::begin() {
    GestureTrigger::Config defaultCfg{
        GestureTrigger::Type::LongPress,
        GestureTrigger::Button::A,
        1000
    };
    begin(deviceName, defaultCfg);
}

void DevDash::begin(const String& devName) {
    deviceName = devName;
    GestureTrigger::Config defaultCfg{
        GestureTrigger::Type::LongPress,
        GestureTrigger::Button::A,
        1000
    };
    begin(deviceName, defaultCfg);
}

void DevDash::begin(const GestureTrigger::Config& gestureCfg) {
    begin(deviceName, gestureCfg);
}

void DevDash::begin(const String& devName, const GestureTrigger::Config& gestureCfg) {
    deviceName = devName;
    if (gesture) { gesture->destroy(); delete gesture; }
    gesture = new GestureTrigger(gestureCfg);
    gesture->begin();
}

void DevDash::loop() {
    if (!device && gesture && gesture->checkAndFire()) {
        if (deviceName == "M5Core2") {
            device = new DevDashM5Core2();
        } else {
            Serial.print("DevDash::begin: Unknown device \"");
            Serial.print(deviceName);
            Serial.println("\". Initialization failed.");
            return;
        }
        device->begin();
    }
    if (device) {
        device->loop();
    }
}

void DevDash::destroy() {
    if (device) { device->destroy(); delete device; device = nullptr; }
    if (gesture) { gesture->destroy(); delete gesture; gesture = nullptr; }
}
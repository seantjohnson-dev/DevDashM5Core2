#include <Arduino.h>
#include "DevDash.h"
#include <WString.h>
#include "DevDashM5Core2/DevDashM5Core2.h"

// Static member initialization
IDevice* DevDash::device = nullptr;
GestureTrigger* DevDash::gesture = new GestureTrigger(GestureTrigger::Type::DoubleTap);
ThemeManager* DevDash::theme = new ThemeManager();

void DevDash::begin(const String& deviceName) {
    // Instantiate device implementation once
    if (!device) {
        if (deviceName == "M5Core2") {
            device = new DevDashM5Core2();
        }
        // else if other device names in future
        else {
            Serial.print("DevDash::begin: Unknown device \"");
            Serial.print(deviceName);
            Serial.println("\". Initialization failed.");
            return;
        }
    }
    gesture->begin();
    theme->apply(theme->current());
    device->begin();
}

void DevDash::loop() {
    if (gesture->checkAndFire()) {
        device->begin();
    }
    if (device) {
        device->loop();
    }
}

void DevDash::destroy() {
    if (device) {
        device->destroy();
        delete device;
        device = nullptr;
    }
    if (gesture) {
        gesture->destroy();
        delete gesture;
        gesture = nullptr;
    }
    if (theme) {
        delete theme;
        theme = nullptr;
    }
}

void DevDash::setGestureType(GestureTrigger::Type t) {
    if (gesture) {
        gesture->destroy();
        delete gesture;
    }
    gesture = new GestureTrigger(t);
}

void DevDash::updateTheme() {
    if (!theme) return;
    auto current = theme->current();
    auto next = (current == ThemeManager::Mode::Light)
                ? ThemeManager::Mode::Dark
                : ThemeManager::Mode::Light;
    theme->apply(next);
}

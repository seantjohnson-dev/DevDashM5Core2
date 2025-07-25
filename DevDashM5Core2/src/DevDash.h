#pragma once
#include "IDevice.h"
#include "GestureTrigger.h"
#include "ThemeManager.h"
#include <WString.h>

class DevDash {
public:
    /**
     * Initialize the DevDash library for a given device name (e.g., "M5Core2").
     * Must be called once in setup().
     */
    static void begin(const String& deviceName);
    static void loop();
    static void destroy();

    static void setGestureType(GestureTrigger::Type t);
    static void updateTheme();  // toggles between light and dark modes

private:
    DevDash() = delete;
    ~DevDash() = delete;

    // Static members
    static IDevice* device;
    static GestureTrigger* gesture;
    static ThemeManager* theme;
};
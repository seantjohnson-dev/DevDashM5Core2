#pragma once
#include <Arduino.h>
#include "IDevice.h"
#include "GestureTrigger.h"
#include <WString.h>

class DevDash {
public:
    static void begin();
    static void begin(const String& deviceName);
    static void begin(const GestureTrigger::Config& gestureCfg);
    static void begin(const String& deviceName, const GestureTrigger::Config& gestureCfg);
    static void loop();
    static void destroy();

private:
    DevDash() = delete;
    ~DevDash() = delete;

    static String deviceName;
    static IDevice* device;
    static GestureTrigger* gesture;
};
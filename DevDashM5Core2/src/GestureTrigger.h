#pragma once
#include <cstdint>

class GestureTrigger {
public:
    enum class Type { DoubleTap, Shake, LongPress };
    enum class Button { A, B, C };

    struct Config {
        Type type;
        Button button;
        uint32_t durationMs;
    };

    GestureTrigger(const Config& cfg);
    void begin();
    bool checkAndFire();
    void destroy();

private:
    Config cfg;
    bool     wasDown;
    uint32_t pressStart;
    bool     firedOnce;
};
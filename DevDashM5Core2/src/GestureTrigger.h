#pragma once

class GestureTrigger {
public:
    enum class Type { DoubleTap, Shake, LongPress /* ... */ };
    GestureTrigger(Type t);
    void begin();
    bool checkAndFire();
    void destroy();

private:
    Type gestureType;
};
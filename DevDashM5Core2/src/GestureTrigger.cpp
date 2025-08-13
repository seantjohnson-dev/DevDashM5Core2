#include "GestureTrigger.h"
#include <Arduino.h>
#include <M5Core2.h>

GestureTrigger::GestureTrigger(const Config& c)
  : cfg(c), wasDown(false), pressStart(0), firedOnce(false) {}

void GestureTrigger::begin() {
    wasDown = false;
    firedOnce = false;
}

bool GestureTrigger::checkAndFire() {
    if (cfg.type == Type::LongPress) {
        bool isDown = false;
        switch (cfg.button) {
            case Button::A: {
                isDown = M5.BtnA.pressedFor(cfg.durationMs, cfg.durationMs);
                break;
            }
            case Button::B: {
                isDown = M5.BtnB.pressedFor(cfg.durationMs, cfg.durationMs);
                break;
            }
            case Button::C: {
                isDown = M5.BtnC.pressedFor(cfg.durationMs, cfg.durationMs);
                break;
            }
        }
        return isDown;
    }
    // TODO: other gestures
    return false;
}

void GestureTrigger::destroy() {}
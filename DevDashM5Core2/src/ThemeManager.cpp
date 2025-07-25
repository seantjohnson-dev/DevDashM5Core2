#include "ThemeManager.h"

ThemeManager::ThemeManager() : mode(Mode::Light) {}
void ThemeManager::apply(Mode m) {
    mode = m;
    // TODO: apply LVGL style changes based on mode (colors, fonts, etc.)
}
ThemeManager::Mode ThemeManager::current() const { return mode; }
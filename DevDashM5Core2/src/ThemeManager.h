#pragma once

class ThemeManager {
public:
    enum class Mode { Light, Dark };
    ThemeManager();
    void apply(Mode m);
    Mode current() const;

private:
    Mode mode;
};
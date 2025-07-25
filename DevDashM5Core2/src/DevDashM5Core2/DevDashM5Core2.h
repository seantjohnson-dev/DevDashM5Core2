#pragma once
#include "../IDevice.h"

class DevDashM5Core2 : public IDevice {
public:
    bool begin() override;
    void loop() override;
    void destroy() override;

private:
    // device-specific members
};
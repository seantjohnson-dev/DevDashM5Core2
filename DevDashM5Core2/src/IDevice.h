#pragma once

class IDevice {
public:
    virtual ~IDevice() {}
    virtual bool begin() = 0;
    virtual void loop() = 0;
    virtual void destroy() = 0;
};
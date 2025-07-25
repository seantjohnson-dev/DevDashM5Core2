#pragma once

class WifiManager {
public:
    bool begin();
    bool connect(const char* ssid, const char* password);
    void destroy();
};
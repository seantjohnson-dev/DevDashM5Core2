#include "WifiManager.h"
#include <SPIFFS.h>

WifiManager::WifiManager()
  : _lastError(WiFiError::None), _autoReconnect(false) {}

WifiManager::~WifiManager() {
    destroy();
}

bool WifiManager::begin() {
    if (!SPIFFS.begin(true)) {
        _lastError = WiFiError::LoadFailed;
        return false;
    }
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    return true;
}

std::vector<WiFiNetwork> WifiManager::scanNetworks(uint8_t maxCount) {
    std::vector<WiFiNetwork> out;
    int n = WiFi.scanNetworks();

    // track best RSSI per SSID using linear lookups (n is small)
    struct Best { int idx; int rssi; };
    std::vector<String> ssids;
    std::vector<Best>   bests;
    ssids.reserve(n); bests.reserve(n);

    for (int i = 0; i < n; ++i) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0) continue;
        int rssi = WiFi.RSSI(i);

        int k = -1;
        for (size_t j = 0; j < ssids.size(); ++j) if (ssids[j] == ssid) { k = (int)j; break; }
        if (k < 0) { ssids.push_back(ssid); bests.push_back({i, rssi}); }
        else if (rssi > bests[k].rssi) { bests[k] = {i, rssi}; }
    }

    for (size_t j = 0; j < ssids.size() && (int)out.size() < maxCount; ++j) {
        WiFiNetwork nw;
        nw.ssid = ssids[j];
        nw.rssi = bests[j].rssi;
        out.push_back(nw);
    }
    return out;
}


bool WifiManager::connect(const char* ssid, const char* password, uint32_t timeoutMs) {
    Serial.printf("Connecting to %s with password: %s\n", ssid, password);
    WiFi.begin(ssid, password);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > timeoutMs) {
            _lastError = WiFiError::Timeout;
            return false;
        }
        delay(100);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    _lastError = WiFiError::None;
    saveCredentials(ssid, password);
    return true;
}

void WifiManager::disconnect() {
    WiFi.disconnect(true);
}

bool WifiManager::isConnected() const {
    return (WiFi.status() == WL_CONNECTED);
}

String WifiManager::currentSSID() const {
    return WiFi.SSID();
}

int32_t WifiManager::rssi() const {
    return isConnected() ? WiFi.RSSI() : 0;
}

bool WifiManager::saveCredentials(const char* ssid, const char* password) {
    File file = SPIFFS.open("/wifi.cfg", FILE_WRITE);
    if (!file) {
        _lastError = WiFiError::SaveFailed;
        return false;
    }
    Serial.printf("Saving credentials: SSID=%s, Password=%s\n", ssid, password);
    file.printf("%s\n%s", ssid, password);
    file.close();
    Serial.println("Credentials saved.");
    _lastError = WiFiError::None;
    return true;
}

bool WifiManager::loadCredentials(String& ssidOut, String& passOut) {
    File file = SPIFFS.open("/wifi.cfg", FILE_READ);
    if (!file) {
        _lastError = WiFiError::LoadFailed;
        return false;
    }
    Serial.println("Loading credentials from /wifi.cfg");
    ssidOut = file.readStringUntil('\n');
    passOut = file.readStringUntil('\n');
    file.close();
    Serial.printf("Loaded SSID: %s, Password: %s\n", ssidOut.c_str(), passOut.c_str());
    _lastError = WiFiError::None;
    return true;
}

WiFiError WifiManager::lastError() const {
    return _lastError;
}

void WifiManager::setAutoReconnect(bool enable) {
    _autoReconnect = enable;
}

void WifiManager::loop() {
    if (_autoReconnect && !isConnected()) {
        String ssid, pass;
        if (loadCredentials(ssid, pass)) {
            connect(ssid.c_str(), pass.c_str());
        }
    }
}

void WifiManager::destroy() {
    disconnect();
    SPIFFS.end();
}
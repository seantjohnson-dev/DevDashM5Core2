#include "WifiManager.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

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
    _scannedNetworks = out; // store for later use
    return out;
}


bool WifiManager::connect(const char* ssid, const char* password, uint32_t timeoutMs) {
    Serial.printf("Connecting to %s \n", ssid);
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
    if (!ssid || !ssid[0]) { _lastError = WiFiError::SaveFailed; return false; }

    // Check if this SSID already exists in the saved list
    for (const auto& n : _savedNetworks) {
        if (n.ssid == ssid) {
            // SSID already present — no change required.
            // Still ensure the file exists/is up-to-date.
            return writeSavedNetworksToFile_();
        }
    }

    // Add new entry (allow empty password for open networks)
    SavedWiFiNetwork entry{ String(ssid), String(password ? password : "") };
    _savedNetworks.push_back(entry);

    if (!writeSavedNetworksToFile_()) { _lastError = WiFiError::SaveFailed; return false; }
    _lastError = WiFiError::None;
    return true;
}

bool WifiManager::writeSavedNetworksToFile_() {
    // Size: base + per-network overhead
    const size_t base = 1024;
    const size_t per  = 128;
    DynamicJsonDocument doc(base + (_savedNetworks.size() * per));

    JsonObject root = doc.to<JsonObject>();
    JsonArray arr = root.createNestedArray("networks");

    for (const auto& n : _savedNetworks) {
        if (!n.ssid.length()) continue; // skip empty
        JsonObject o = arr.createNestedObject();
        o["ssid"] = n.ssid;
        o["password"] = n.password;
    }

    File f = SPIFFS.open(kJsonPath, FILE_WRITE);
    if (!f) { return false; }
    if (serializeJson(doc, f) == 0) { f.close(); return false; }
    f.close();
    return true;
}

bool WifiManager::loadCredentials() {
    Serial.println("Loading WiFi credentials from SPIFFS...");
    if (!_credsLoaded) {
        File file = SPIFFS.open(kJsonPath, FILE_READ);
        if (!file) {
            _lastError = WiFiError::LoadFailed;
            return false;
        }
        Serial.println("Loading credentials from /wifi.json");
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, file);
        if (error) {
            Serial.print("Failed to parse JSON: ");
            Serial.println(error.c_str());
            return false;
        }
        file.close();
        JsonArray networks = doc["networks"].as<JsonArray>();
        for (JsonObject network : networks) {
            const String ssid = network["ssid"];
            const String password = network["password"];
            _savedNetworks.push_back({ssid, password});
        }
        _lastError = WiFiError::None;
        
        Serial.printf("Loaded %d saved networks.\n", _savedNetworks.size());
        _credsLoaded = true;
        return true;
    }
    return false;
}

WiFiError WifiManager::lastError() const {
    return _lastError;
}

void WifiManager::setAutoReconnect(bool enable) {
    _autoReconnect = enable;
}

void WifiManager::loop() {
    // Serial.println("AutoConnect: " + String(_autoReconnect));
    // Serial.println("isConnected: " + String(isConnected()));
    if (_autoReconnect && !isConnected()) {
        std::vector<WiFiNetwork> matchedNetworks;
        Serial.println("ScannedNetworks: " + String(_scannedNetworks.size()));
        Serial.println("SavedNetworks: " + String(_savedNetworks.size()));
        if (!_scannedNetworks.empty()) {
            for (const auto& scanned : _scannedNetworks) {
                for (const auto& saved : _savedNetworks) {
                    if (scanned.ssid == saved.ssid) {
                        matchedNetworks.push_back(scanned);
                    }
                }
            }
        }
        // Find strongest network from matchedNetworks
        WiFiNetwork strongestNetwork;
        SavedWiFiNetwork strongestSaved;
        bool hasStrongest = false;

        for (const auto& net : matchedNetworks) {
            if (!hasStrongest || net.rssi > strongestNetwork.rssi) {
                strongestNetwork = net;
                hasStrongest = true;
            }
        }

        // strongestNetwork now holds the network with the highest RSSI (if any matched)
        if (hasStrongest) {
            Serial.println("Strongest matched network: " + strongestNetwork.ssid + 
                        " (RSSI: " + String(strongestNetwork.rssi) + ")");
            // I need to find this network in saved networks to get the password
            for (const auto& saved : _savedNetworks) {
                if (saved.ssid == strongestNetwork.ssid) {
                    strongestSaved = saved;
                    break;
                }
            }
            connect(strongestSaved.ssid.c_str(), strongestSaved.password.c_str());
            Serial.println("Connected to " + strongestSaved.ssid + " successfully.");

        } else {
            Serial.println("No matched networks found.");
        }
    }
}

void WifiManager::destroy() {
    disconnect();
    SPIFFS.end();
}
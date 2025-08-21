#pragma once

#include <Arduino.h>
#include <vector>
#include <FS.h>
#include <WiFi.h>

/**
 * Error codes for WiFi operations
 */
enum class WiFiError {
    None,
    Timeout,
    AuthFailed,
    NoHardware,
    ScanFailed,
    SaveFailed,
    LoadFailed
};

/**
 * Representation of a WiFi network
 */
struct WiFiNetwork {
    String ssid;
    int32_t rssi;
};

struct SavedWiFiNetwork {
    String ssid;
    String password;
};

class WifiManager {
public:
    WifiManager();
    ~WifiManager();

    /** Initialize WiFi subsystem and SPIFFS */
    bool begin();

    /** Scan for available networks (up to maxCount) */
    std::vector<WiFiNetwork> scanNetworks(uint8_t maxCount = 10);

    /** Connect to the given SSID/password, blocking up to timeoutMs */
    bool connect(const char* ssid, const char* password, uint32_t timeoutMs = 10000);

    /** Disconnect from the current network */
    void disconnect();

    /** Check if currently connected */
    bool isConnected() const;

    /** Get the current SSID (empty if none) */
    String currentSSID() const;

    /** Get current RSSI (0 if not connected) */
    int32_t rssi() const;

    /** Save credentials to SPIFFS */
    bool saveCredentials(const char* ssid, const char* password);

    /** Load credentials from SPIFFS */
    bool loadCredentials();

    /** Get last error code */
    WiFiError lastError() const;

    /** Enable or disable automatic reconnect */
    void setAutoReconnect(bool enable);

    /** Call regularly to handle auto-reconnect logic */
    void loop();

    /** Clean up resources */
    void destroy();

    const std::vector<WiFiNetwork>& getScannedNetworks() const { return _scannedNetworks; }

    const std::vector<SavedWiFiNetwork>& getSavedNetworks() const { return _savedNetworks; }

private:
    WiFiError _lastError;
    bool      _autoReconnect;
    std::vector<SavedWiFiNetwork> _savedNetworks;
    std::vector<WiFiNetwork> _scannedNetworks;
    bool _credsLoaded = false;
    bool writeSavedNetworksToFile_();
    static constexpr const char* kJsonPath = "/wifi.json";
};
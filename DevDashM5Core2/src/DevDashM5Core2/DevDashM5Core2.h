#pragma once
#include "../IDevice.h"
#include "ThemeManager.h"
#include "LVGLRenderer.h"
#include "WifiManager.h"
#include <string>

#include <lvgl.h> // use LVGL types directly to avoid forward-decl/typedef conflicts

class DevDashM5Core2 : public IDevice {
public:
    bool begin() override;
    void loop() override;
    void destroy() override;

    // UI actions
    void showPasswordModal(const char* ssid);
    void hidePasswordModal();
    void populateWifiList(lv_obj_t* panel);

    // Backward-compatible handlers (not required by the new setup)
    void wifiRowEventHandler(lv_event_t* e);
    void connectBtnEventHandler(lv_event_t* e);

    static void updateTheme();

private:
    // Singletons owned elsewhere
    static ThemeManager* theme;
    static LVGLRenderer* renderer;
    static WifiManager*  manager;

    // Persistent UI elements (created once, reused)
    lv_obj_t* refresh_btn_      = nullptr;
    lv_obj_t* wifi_icon_        = nullptr;
    lv_obj_t* wifi_panel_       = nullptr;

    lv_obj_t* password_modal_   = nullptr; // modal container (hidden/shown)
    lv_obj_t* password_keyboard_= nullptr; // on-screen keyboard (hidden/shown)
    lv_obj_t* password_textarea_= nullptr; // input field
    lv_obj_t* ssid_label_       = nullptr; // label inside modal
    lv_obj_t* show_pw_btn_      = nullptr; // eye toggle
    lv_obj_t* connect_btn_      = nullptr;
    lv_obj_t* cancel_btn_       = nullptr;

    // State
    std::string current_ssid_;

    // Helpers
    void ensurePasswordUI_();   // lazy-create modal + keyboard once
    void resetPasswordUI_();    // update SSID label, reset TA each time
};
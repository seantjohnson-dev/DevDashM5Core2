// =============================
// DevDashM5Core2.cpp (LVGL 9.3)
// =============================
#include <M5Core2.h>
#include <lvgl.h>
#include "DevDashM5Core2.h"
#include "WifiManager.h"
#include "LVGLRenderer.h"
#include "ThemeManager.h"
#include "SensorDashboard.h"

/* -------------------- Setup and Loop -------------------- */

ThemeManager* DevDashM5Core2::theme    = new ThemeManager();
LVGLRenderer* DevDashM5Core2::renderer = new LVGLRenderer();
WifiManager*  DevDashM5Core2::manager  = new WifiManager();
SensorDashboard* DevDashM5Core2::sensorDashboard = new SensorDashboard();

bool DevDashM5Core2::begin() {
    // assume M5.begin() has been called in setup

    if (!renderer->begin()) {
        Serial.println("LVGLRenderer init failed");
        return false;
    }

    if (!theme->begin()) {
        Serial.println("ThemeManager init failed");
        return false;
    }
    theme->apply(theme->current());

    // if (!sensorDashboard->begin()) {
    //     Serial.println("SensorDashboard init failed");
    //     return false;
    // }

    if (!manager->begin()) {
        Serial.println("WifiManager init failed");
        return false;
    }
    manager->setAutoReconnect(true);
    manager->loadCredentials();

    // Root layout
    lv_obj_t* container = theme->createContainer(lv_screen_active());
    // lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN); // start hidden
    lv_obj_t* header = theme->createHeader(container, "WiFi Networks", &refresh_btn_, &wifi_icon_);
    (void)header; // silence unused if header isn't referenced further
    wifi_panel_ = theme->createPanel(container);

    // Refresh button re-scans and repopulates the list
    lv_obj_add_event_cb(refresh_btn_, [](lv_event_t* e){
        if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
        auto* self = static_cast<DevDashM5Core2*>(lv_event_get_user_data(e));
        if (!self || !self->wifi_panel_) return;
        lv_obj_clean(self->wifi_panel_);
        self->populateWifiList(self->wifi_panel_);
    }, LV_EVENT_ALL, this);

    // Initial population
    populateWifiList(wifi_panel_);
    return true;
}

void DevDashM5Core2::loop() {
    // handle UI tasks
    renderer->loop();
    static wl_status_t last_status = WL_IDLE_STATUS;
    wl_status_t current = WiFi.status();

    if (wifi_icon_ && current != last_status) {
        lv_color_t color = (current == WL_CONNECTED)
            ? lv_palette_main(LV_PALETTE_GREEN)
            : lv_palette_main(LV_PALETTE_GREY);
        lv_obj_set_style_text_color(wifi_icon_, color, 0);
        last_status = current;
    }
    manager->loop();
    theme->loop();
    // sensorDashboard->loop();
    delay(5);
}

void DevDashM5Core2::destroy() {
    // tear down components
    if (renderer) { delete renderer; renderer = nullptr; }
    if (theme)    { delete theme; theme = nullptr; }
    if (manager)  { delete manager; manager = nullptr; }
    if (sensorDashboard)  { delete sensorDashboard; sensorDashboard = nullptr; }
}

void DevDashM5Core2::updateTheme() {
    if (!theme) return;
    auto current = theme->current();
    auto next = (current == ThemeManager::Mode::Light)
                ? ThemeManager::Mode::Dark
                : ThemeManager::Mode::Light;
    theme->apply(next);
}

/* -------------------- Wi-Fi List -------------------- */

void DevDashM5Core2::populateWifiList(lv_obj_t* panel) {
    if (!panel) return;

    std::vector<WiFiNetwork> networks = manager->scanNetworks(20);
    std::vector<SavedWiFiNetwork> saved = manager->getSavedNetworks();

    if (networks.empty()) {
        lv_obj_t* label = lv_label_create(panel);
        lv_label_set_text(label, "No networks found.");
        return;
    }

    for (const auto& network : networks) {
        // Row container
        lv_obj_t* row = lv_obj_create(panel);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_height(row, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_row(row, 0, 0);
        lv_obj_set_style_pad_column(row, 10, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);

        // SSID label (child 0)
        lv_obj_t* ssid_label = lv_label_create(row);
        lv_label_set_text(ssid_label, network.ssid.c_str());
        lv_obj_set_flex_grow(ssid_label, 1);

        // RSSI label (child 1)
        lv_obj_t* rssi_label = lv_label_create(row);
        if (!saved.empty()) {
            for (const auto& saved_net : saved) {
                if (saved_net.ssid == network.ssid) {
                    // Highlight saved networks
                    lv_label_set_text_fmt(rssi_label, "%s %d dBm", LV_SYMBOL_SAVE, network.rssi);
                    break;
                } else {
                    lv_label_set_text_fmt(rssi_label, "%d dBm", network.rssi);
                }
            }
        } else {
            lv_label_set_text_fmt(rssi_label, "%d dBm", network.rssi);
        }
        lv_obj_set_style_text_color(rssi_label, lv_palette_main(LV_PALETTE_BLUE), 0);

        // Click handler: read SSID text from first child; no heap allocations
        lv_obj_add_event_cb(row,
            [](lv_event_t* e){
                if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
                auto* self = static_cast<DevDashM5Core2*>(lv_event_get_user_data(e));
                if (!self) return;
                lv_obj_t* row = static_cast<lv_obj_t*>(lv_event_get_target(e));
                lv_obj_t* lbl = lv_obj_get_child(row, 0);
                if (!lbl) return;
                const char* ssid_text = lv_label_get_text(lbl);
                std::vector<SavedWiFiNetwork> saved = self->manager->getSavedNetworks();
                if (!saved.empty()) {
                    for (const auto& net : saved) {
                        if (net.ssid == ssid_text) {
                            // Already saved, no need to show modal
                            Serial.println("Network already saved: " + String(ssid_text));
                            bool connected = self->manager->connect(ssid_text, net.password.c_str());
                            if (connected) {
                                Serial.println("Connected to: " + String(ssid_text));
                            } else {
                                Serial.println("Failed to connect to: " + String(ssid_text));
                            }
                            return;
                        } else {
                            self->showPasswordModal(ssid_text ? ssid_text : "");
                        }
                    }
                } else {
                    self->showPasswordModal(ssid_text ? ssid_text : "");
                }
            },
        LV_EVENT_ALL, this);
    }
}

/* -------------------- Password Modal (create once, reuse) -------------------- */

void DevDashM5Core2::ensurePasswordUI_() {
    if (password_modal_) return; // already built

    // Modal on the top layer
    password_modal_ = lv_obj_create(lv_layer_top());
    lv_obj_set_size(password_modal_, 320, 240);
    lv_obj_center(password_modal_);
    lv_obj_add_flag(password_modal_, LV_OBJ_FLAG_HIDDEN); // start hidden
    lv_obj_set_style_bg_color(password_modal_, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_pad_all(password_modal_, 10, 0);
    lv_obj_set_layout(password_modal_, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(password_modal_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(password_modal_, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(password_modal_, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_scrollbar_mode(password_modal_, LV_SCROLLBAR_MODE_OFF);

    // SSID label
    ssid_label_ = lv_label_create(password_modal_);

    // Password row container
    lv_obj_t* pw_row = lv_obj_create(password_modal_);
    lv_obj_set_width(pw_row, lv_pct(100));
    lv_obj_set_height(pw_row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(pw_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(pw_row, 10, 0);
    lv_obj_set_style_border_width(pw_row, 0, 0);
    lv_obj_set_style_bg_opa(pw_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(pw_row, 0, 0);

    // Textarea
    password_textarea_ = lv_textarea_create(pw_row);
    lv_textarea_set_password_mode(password_textarea_, true);
    lv_textarea_set_placeholder_text(password_textarea_, "Enter password");
    lv_textarea_set_max_length(password_textarea_, 64);
    lv_textarea_set_one_line(password_textarea_, true);
    lv_obj_set_flex_grow(password_textarea_, 1);
    lv_obj_set_height(password_textarea_, LV_SIZE_CONTENT);

    // Show/Hide Password Button
    show_pw_btn_ = lv_btn_create(pw_row);
    lv_obj_set_size(show_pw_btn_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    {
        lv_obj_t* show_pw_label = lv_label_create(show_pw_btn_);
        lv_label_set_text(show_pw_label, LV_SYMBOL_EYE_OPEN);
        lv_obj_center(show_pw_label);
    }
    // Toggle password visibility (non-capturing lambda)
    lv_obj_add_event_cb(show_pw_btn_, [](lv_event_t* e){
        if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
        lv_obj_t* btn = static_cast<lv_obj_t*>(lv_event_get_target(e));
        lv_obj_t* row = lv_obj_get_parent(btn);
        lv_obj_t* ta  = lv_obj_get_child(row, 0); // first child is textarea
        if (!ta) return;
        bool is_pw = lv_textarea_get_password_mode(ta);
        lv_textarea_set_password_mode(ta, !is_pw);
        lv_obj_t* lbl = lv_obj_get_child(btn, 0);
        lv_label_set_text(lbl, is_pw ? LV_SYMBOL_EYE_CLOSE : LV_SYMBOL_EYE_OPEN);
    }, LV_EVENT_ALL, nullptr);

    // Keyboard (top layer; hidden by default)
    password_keyboard_ = lv_keyboard_create(lv_layer_top());
    lv_obj_set_size(password_keyboard_, 320, 110);
    lv_obj_align(password_keyboard_, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(password_keyboard_, LV_OBJ_FLAG_HIDDEN);

    // Show keyboard on focus, hide on defocus
    lv_obj_add_event_cb(password_textarea_, [](lv_event_t* e){
        lv_event_code_t code = lv_event_get_code(e);
        DevDashM5Core2* self = static_cast<DevDashM5Core2*>(lv_event_get_user_data(e));
        if (!self) return;
        if (code == LV_EVENT_FOCUSED) {
            lv_keyboard_set_textarea(self->password_keyboard_, self->password_textarea_);
            lv_obj_clear_flag(self->password_keyboard_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(self->password_keyboard_);
        } else if (code == LV_EVENT_DEFOCUSED) {
            lv_keyboard_set_textarea(self->password_keyboard_, nullptr);
            lv_obj_add_flag(self->password_keyboard_, LV_OBJ_FLAG_HIDDEN);
        }
    }, LV_EVENT_ALL, this);

    // Button row
    lv_obj_t* btn_row = lv_obj_create(password_modal_);
    lv_obj_set_width(btn_row, lv_pct(100));
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(btn_row, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_column(btn_row, 10, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);

    // Cancel button
    cancel_btn_ = lv_btn_create(btn_row);
    lv_label_set_text(lv_label_create(cancel_btn_), "Cancel");
    lv_obj_add_event_cb(cancel_btn_, [](lv_event_t* e){
        if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
        DevDashM5Core2* self = static_cast<DevDashM5Core2*>(lv_event_get_user_data(e));
        if (!self) return;
        self->hidePasswordModal();
        if (self->password_textarea_) lv_textarea_set_text(self->password_textarea_, "");
    }, LV_EVENT_ALL, this);

    // Connect button
    connect_btn_ = lv_btn_create(btn_row);
    lv_label_set_text(lv_label_create(connect_btn_), "Connect");
    lv_obj_add_event_cb(connect_btn_, [](lv_event_t* e){
        if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
        DevDashM5Core2* self = static_cast<DevDashM5Core2*>(lv_event_get_user_data(e));
        if (!self) return;
        const char* pw = self->password_textarea_ ? lv_textarea_get_text(self->password_textarea_) : "";
        bool connected = self->manager->connect(self->current_ssid_.c_str(), pw);
        self->hidePasswordModal();
        if (self->password_textarea_) lv_textarea_set_text(self->password_textarea_, "");
    }, LV_EVENT_ALL, this);
}

void DevDashM5Core2::resetPasswordUI_() {
    if (ssid_label_)     lv_label_set_text_fmt(ssid_label_, "Connect to: %s", current_ssid_.c_str());
    if (password_textarea_) {
        lv_textarea_set_text(password_textarea_, "");
        lv_textarea_set_password_mode(password_textarea_, true);
    }
}

void DevDashM5Core2::showPasswordModal(const char* ssid) {
    current_ssid_ = ssid ? ssid : "";
    ensurePasswordUI_();
    resetPasswordUI_();

    lv_obj_clear_flag(password_modal_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(password_modal_);

    // Focus textarea to bring up keyboard immediately (if desired)
    lv_obj_add_state(password_textarea_, LV_STATE_FOCUSED);
}

void DevDashM5Core2::hidePasswordModal() {
    if (password_keyboard_) {
        lv_keyboard_set_textarea(password_keyboard_, nullptr);
        lv_obj_add_flag(password_keyboard_, LV_OBJ_FLAG_HIDDEN);
    }
    if (password_modal_) {
        lv_obj_add_flag(password_modal_, LV_OBJ_FLAG_HIDDEN);
    }
}

/* -------------------- Legacy-style handlers (optional) -------------------- */

void DevDashM5Core2::wifiRowEventHandler(lv_event_t* e) {
    // Fallback handler if used elsewhere: expects SSID in user_data
    const char* selected_ssid = static_cast<const char*>(lv_event_get_user_data(e));
    showPasswordModal(selected_ssid ? selected_ssid : "");
}

void DevDashM5Core2::connectBtnEventHandler(lv_event_t* /*e*/) {
    // Fallback: use persistent textarea and SSID
    const char* pw = password_textarea_ ? lv_textarea_get_text(password_textarea_) : "";
    manager->connect(current_ssid_.c_str(), pw);
    hidePasswordModal();
    if (password_textarea_) lv_textarea_set_text(password_textarea_, "");
}

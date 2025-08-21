#pragma once

#include <lvgl.h>

class ThemeManager {
public:
    enum class Mode { Light, Dark };

    ThemeManager();
    ~ThemeManager();

    /** Apply light or dark mode styles */
    bool begin(Mode mode = Mode::Light);
    void destroy();
    void apply(Mode m);
    void loop();
    Mode current() const;

    /** Create a standard header bar with title text */
    lv_obj_t* createHeader(lv_obj_t* parent, const char* title, lv_obj_t** out_refresh_btn, lv_obj_t** out_wifi_icon);

    /** Create a standard header bar with title text */
    lv_obj_t* createContainer(lv_obj_t* parent);

    lv_obj_t* createPanel(lv_obj_t* parent);

    /** Create a styled button with text and event callback */
    lv_obj_t* createButton(lv_obj_t* parent, const char* text, lv_event_cb_t event_cb, void* user_data);

    /** Create a scrollable flex list container */
    lv_obj_t* createList(lv_obj_t* parent);

    /** Add a styled list item to a list container */
    lv_obj_t* addListItem(lv_obj_t* list, const char* text, lv_event_cb_t event_cb, void* user_data);

private:
    Mode mode;
    lv_style_t headerStyle;
    lv_style_t containerStyle;
    lv_style_t panelStyle;
    lv_style_t buttonStyle;
    lv_style_t listStyle;
    lv_style_t listItemStyle;
};
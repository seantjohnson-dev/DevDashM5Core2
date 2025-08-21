#include "ThemeManager.h"
#include <lvgl.h>

// Color definitions
static lv_color_t LIGHT_BG   = lv_color_white();
static lv_color_t DARK_BG    = lv_color_hex(0x222222);
static lv_color_t LIGHT_TEXT = lv_color_black();
static lv_color_t DARK_TEXT  = lv_color_white();
static lv_color_t BUTTON_BG  = lv_color_hex(0x00AA00);


ThemeManager::ThemeManager() {}

ThemeManager::~ThemeManager() {}

bool ThemeManager::begin(Mode m) {
    // Initialize styles
    lv_style_init(&headerStyle);
    lv_style_init(&buttonStyle);
    lv_style_init(&listStyle);
    lv_style_init(&listItemStyle);
    lv_style_init(&containerStyle);
    lv_style_init(&panelStyle);

    if (m == Mode::Light || m == Mode::Dark) {
        mode = m;
    } else {
        mode = Mode::Light; // Default to light mode
    }

    return true; // Assume styles initialized successfully
}

void ThemeManager::destroy() {
    lv_style_reset(&headerStyle);
    lv_style_reset(&buttonStyle);
    lv_style_reset(&listStyle);
    lv_style_reset(&listItemStyle);
    lv_style_reset(&containerStyle);
    lv_style_reset(&panelStyle);
}

void ThemeManager::apply(Mode m) {
    mode = m;
    // Background and text colors based on mode
    lv_color_t bg   = (mode == Mode::Light) ? LIGHT_BG : DARK_BG;
    lv_color_t text = (mode == Mode::Light) ? LIGHT_TEXT : DARK_TEXT;

    // Header style
    lv_style_set_bg_color(&headerStyle, bg);
    lv_style_set_pad_all(&headerStyle, 10);
    lv_style_set_text_color(&headerStyle, text);

    // Button style
    lv_style_set_bg_color(&buttonStyle, BUTTON_BG);
    lv_style_set_radius(&buttonStyle, 8);
    lv_style_set_text_color(&buttonStyle, text);
    lv_style_set_pad_all(&buttonStyle, 8);

    // List container style
    lv_style_set_bg_color(&listStyle, bg);
    lv_style_set_pad_row(&listStyle, 4);
    lv_style_set_pad_column(&listStyle, 4);

    // List item style
    lv_style_set_bg_color(&listItemStyle, bg);
    lv_style_set_border_side(&listItemStyle, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_width(&listItemStyle, 1);
    lv_style_set_border_color(&listItemStyle, text);
    lv_style_set_pad_all(&listItemStyle, 10);
    lv_style_set_text_color(&listItemStyle, text);
}

void ThemeManager::loop() {
    // Placeholder for any periodic updates needed
}

ThemeManager::Mode ThemeManager::current() const {
    return mode;
}

lv_obj_t* ThemeManager::createHeader(lv_obj_t* parent, const char* title, lv_obj_t** out_refresh_btn, lv_obj_t** out_wifi_icon) {
    // lv_obj_t* cont = lv_obj_create(parent);
    // lv_obj_add_style(cont, &headerStyle, 0);
    // lv_obj_set_size(cont, LV_PCT(100), 40);
    // lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    // lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // lv_obj_t* lbl = lv_label_create(cont);
    // lv_label_set_text(lbl, title);
    // return cont;

    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_add_style(header, &headerStyle, 0);
    lv_obj_set_size(header, lv_pct(100), 40);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x333333), 0);
    // lv_obj_set_style_pad_all(header, 10, 0);
    lv_obj_set_style_pad_hor(header, 10, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Label
    lv_obj_t *label = lv_label_create(header);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_set_flex_grow(label, 1);  // Take all available space

    // Refresh Button
    lv_obj_t *refresh_btn = lv_btn_create(header);
    lv_obj_set_size(refresh_btn, 32, 32);
    lv_obj_t *refresh_icon = lv_label_create(refresh_btn);
    lv_label_set_text(refresh_icon, LV_SYMBOL_REFRESH);
    lv_obj_center(refresh_icon);

    if (out_refresh_btn) *out_refresh_btn = refresh_btn;

    lv_obj_t* wifi_icon = lv_label_create(header);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_icon, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_text_font(wifi_icon, &lv_font_montserrat_18, 0);
    lv_obj_center(wifi_icon);

    if (out_wifi_icon) *out_wifi_icon = wifi_icon;

    return header;
}

lv_obj_t* ThemeManager::createContainer(lv_obj_t *parent) {
    int32_t w = lv_obj_get_width(parent);
    int32_t h = lv_obj_get_height(parent);

    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, w, h);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(container, lv_color_hex(0xcccccc), 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    return container;
}

lv_obj_t* ThemeManager::createPanel(lv_obj_t *parent) {
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_size(panel, lv_pct(100), (lv_pct(100) - 20));
    lv_obj_center(panel);
    // lv_obj_set_style_margin_all(panel, 10, 0);
    lv_obj_set_style_margin_hor(panel, 10, 0);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_scroll_dir(panel, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(panel, 10, 0);
    return panel;
}

lv_obj_t* ThemeManager::createButton(lv_obj_t* parent, const char* text, lv_event_cb_t event_cb, void* user_data) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_add_style(btn, &buttonStyle, 0);
    lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, user_data);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_center(lbl);
    return btn;
}

lv_obj_t* ThemeManager::createList(lv_obj_t* parent) {
    lv_obj_t* list = lv_obj_create(parent);
    lv_obj_add_style(list, &listStyle, 0);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    return list;
}

lv_obj_t* ThemeManager::addListItem(lv_obj_t* list, const char* text, lv_event_cb_t event_cb, void* user_data) {
    lv_obj_t* row = lv_obj_create(list);
    lv_obj_add_style(row, &listItemStyle, 0);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_add_event_cb(row, event_cb, LV_EVENT_CLICKED, user_data);

    lv_obj_t* lbl = lv_label_create(row);
    lv_label_set_text(lbl, text);
    lv_obj_set_flex_grow(lbl, 1);
    return row;
}
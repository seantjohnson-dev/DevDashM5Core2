#pragma once

#include <lvgl.h>
#include <M5Core2.h>
#include <Arduino.h>

class LVGLRenderer {
public:
    LVGLRenderer();
    ~LVGLRenderer();

    bool begin();
    void loop();
    void destroy();
    static void display_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
    static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data);
    static uint32_t tick(void);
};

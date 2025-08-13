#include <lvgl.h>
#include <M5Core2.h>
#include <Arduino.h>
#include "draw/sw/lv_draw_sw.h" // Needed for lv_draw_sw_rgb565_swap
#include "LVGLRenderer.h"

// Font (assuming enabled in lv_conf.h)
// extern lv_font_t lv_font_montserrat_18;

/* Screen config */
#define TFT_HOR_RES   320
#define TFT_VER_RES   240
#define DRAW_BUF_SIZE (TFT_HOR_RES * 40 * (LV_COLOR_DEPTH / 8)) // Reduced buffer to save RAM

uint32_t* draw_buf;

/* LVGL logging (optional) */
#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}
#endif

LVGLRenderer::LVGLRenderer() {}

LVGLRenderer::~LVGLRenderer() {
    destroy();
}

bool LVGLRenderer::begin() {
    lv_init();
    lv_tick_set_cb(LVGLRenderer::tick);

#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print);
#endif

    lv_display_t *disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, LVGLRenderer::display_flush);

    draw_buf = (uint32_t*)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_SPIRAM);
    if (!draw_buf) {
        Serial.println("Draw buffer alloc failed.");
        while (1);
    }
    lv_display_set_buffers(disp, draw_buf, nullptr, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, LVGLRenderer::touchpad_read);
    return true;
}

void LVGLRenderer::loop() {
    lv_timer_handler();
}

void LVGLRenderer::destroy() {

}

/* Flush callback: push LVGL buffer to screen */
void LVGLRenderer::display_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t width = lv_area_get_width(area);
    uint32_t height = lv_area_get_height(area);

    // Swap color order to match M5Core2's RGB565 expectations
    lv_draw_sw_rgb565_swap(px_map, width * height);

    M5.Lcd.pushImage(area->x1, area->y1, width, height, (uint16_t *)px_map);
    lv_display_flush_ready(disp);
}


/* Read M5Core2 touch input */
void LVGLRenderer::touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    TouchPoint_t tp = M5.Touch.getPressPoint();
    if (tp.x >= 0 && tp.y >= 0) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = tp.x;
        data->point.y = tp.y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

uint32_t LVGLRenderer::tick(void) {
    return millis();
}
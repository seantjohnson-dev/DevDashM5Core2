#ifndef LVGL_RENDERER_H
#define LVGL_RENDERER_H

#include <lvgl.h>

namespace LVGLRenderer {
    lv_obj_t* createHeader(lv_obj_t* parent, const char* title, lv_color_t bg);
    lv_obj_t* createPanel(lv_obj_t* parent, lv_color_t bg, int margin);
    void applyStandardStyle(lv_obj_t* obj);
}

#endif // LVGL_RENDERER_H

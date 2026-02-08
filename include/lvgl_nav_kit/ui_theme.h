#ifndef LVGL_NAV_KIT_UI_THEME_H
#define LVGL_NAV_KIT_UI_THEME_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_theme {
    const lv_font_t *font_normal;
    const lv_font_t *font_title;
    const lv_font_t *font_icon;
    const lv_font_t *font_mono;
    uint32_t color_bg_white;
    uint32_t color_bg_light;
    uint32_t color_bg_dark;
    uint32_t color_border;
    uint32_t color_text_primary;
    uint32_t color_text_secondary;
    uint32_t color_text_white;
    uint32_t color_bg_overlay;
    uint32_t color_primary;
    uint32_t color_success;
    uint32_t color_danger;
    uint32_t color_warning;
    int card_radius;
    int dropdown_w;
    int input_h;
    int input_radius;
    int input_pad_v;
    int input_w_sm;
    int pad_h;
    int gap;
    int icon_offset;
    int btn_radius;
    int label_w;
    int dialog_radius;
    int shadow_w;
    /** Status bar height in px; 0 = none. */
    int status_bar_height;
} ui_theme_t;

const ui_theme_t *ui_theme_get_default(void);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_NAV_KIT_UI_THEME_H */

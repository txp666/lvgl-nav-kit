/**
 * @file ui_theme_default.c
 * Built-in default theme (no app fonts/colors).
 */

#include "lvgl_nav_kit/ui_theme.h"
#include "lvgl.h"

static ui_theme_t s_default_theme;
static int s_default_theme_initialized = 0;

static void init_default_theme(void) {
    if (s_default_theme_initialized) return;
    s_default_theme.font_normal = NULL;
    s_default_theme.font_title  = NULL;
    s_default_theme.font_icon   = NULL;
    s_default_theme.font_mono   = NULL;
    s_default_theme.color_bg_white     = 0xFFFFFF;
    s_default_theme.color_bg_light     = 0xF0F0F0;
    s_default_theme.color_bg_dark     = 0xE0E0E0;
    s_default_theme.color_border      = 0xDDDDDD;
    s_default_theme.color_text_primary   = 0x333333;
    s_default_theme.color_text_secondary = 0x666666;
    s_default_theme.color_text_white  = 0xFFFFFF;
    s_default_theme.color_bg_overlay  = 0x000000;
    s_default_theme.color_primary     = 0x2196F3;
    s_default_theme.color_success     = 0x4CAF50;
    s_default_theme.color_danger      = 0xF44336;
    s_default_theme.color_warning     = 0xFF9800;
    s_default_theme.card_radius   = 8;
    s_default_theme.dropdown_w    = 120;
    s_default_theme.input_h       = 36;
    s_default_theme.input_radius   = 4;
    s_default_theme.input_pad_v   = 6;
    s_default_theme.input_w_sm    = 80;
    s_default_theme.pad_h         = 12;
    s_default_theme.gap           = 6;
    s_default_theme.icon_offset   = 24;
    s_default_theme.btn_radius    = 6;
    s_default_theme.label_w        = 60;
    s_default_theme.dialog_radius = 12;
    s_default_theme.shadow_w      = 12;
    s_default_theme.status_bar_height = 24;  /* 0 = no status bar */
    s_default_theme_initialized = 1;
}

const ui_theme_t *ui_theme_get_default(void) {
    init_default_theme();
    return &s_default_theme;
}

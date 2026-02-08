#include "lvgl_nav_kit/page_base.h"
#include "lvgl_nav_kit/ui_theme.h"
#include <esp_log.h>

#define TAG "PageBase"

namespace ui {

#define T theme_
#define TC(p) (T ? (p) : 0)

PageBase::PageBase(const char *id) : id_(id) {}

PageBase::~PageBase() {
    DoDestroy();
}

void PageBase::DoCreate(lv_obj_t *parent, const ui_theme_t *theme) {
    if (state_ != PageState::Registered && state_ != PageState::Destroyed) {
        ESP_LOGW(TAG, "Page %s already created, state=%d", id_, (int)state_);
        return;
    }
    theme_ = theme ? theme : ui_theme_get_default();

    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_radius(container_, 0, 0);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_GESTURE_BUBBLE);

    OnCreate(container_);

    state_ = PageState::Created;
    ESP_LOGI(TAG, "Page %s created", id_);
}

void PageBase::DoDestroy() {
    if (state_ == PageState::Registered || state_ == PageState::Destroyed) return;
    OnDestroy();
    DeleteAllTimers();
    if (container_) {
        lv_obj_delete(container_);
        container_ = nullptr;
    }
    event_bindings_.clear();
    theme_ = nullptr;
    state_ = PageState::Destroyed;
    ESP_LOGI(TAG, "Page %s destroyed", id_);
}

lv_timer_t *PageBase::CreateTimer(lv_timer_cb_t cb, uint32_t period, void *user_data) {
    lv_timer_t *t = lv_timer_create(cb, period, user_data);
    if (t) timers_.push_back(t);
    return t;
}

void PageBase::DeleteAllTimers() {
    for (auto *t : timers_) { if (t) lv_timer_delete(t); }
    timers_.clear();
}

void PageBase::AddEventHandler(lv_obj_t *obj, lv_event_cb_t cb, lv_event_code_t code, void *user_data) {
    lv_obj_add_event_cb(obj, cb, code, user_data);
    event_bindings_.push_back({obj, cb});
}

lv_obj_t *PageBase::CreateLabel(lv_obj_t *parent, const char *text) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    if (T && T->font_normal) lv_obj_set_style_text_font(label, T->font_normal, 0);
    return label;
}

lv_obj_t *PageBase::CreateButton(lv_obj_t *parent, const char *text, lv_event_cb_t cb, void *user_data) {
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_t *lab = lv_label_create(btn);
    lv_label_set_text(lab, text);
    if (T && T->font_normal) lv_obj_set_style_text_font(lab, T->font_normal, 0);
    lv_obj_center(lab);
    if (cb) AddEventHandler(btn, cb, LV_EVENT_CLICKED, user_data);
    return btn;
}

lv_obj_t *PageBase::CreateCheckbox(lv_obj_t *parent, const char *text, lv_event_cb_t cb, void *user_data) {
    lv_obj_t *cb_obj = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb_obj, text);
    if (T && T->font_normal) lv_obj_set_style_text_font(cb_obj, T->font_normal, 0);
    if (cb) AddEventHandler(cb_obj, cb, LV_EVENT_VALUE_CHANGED, user_data);
    return cb_obj;
}

lv_obj_t *PageBase::CreateTextarea(lv_obj_t *parent, const char *placeholder) {
    lv_obj_t *ta = lv_textarea_create(parent);
    lv_textarea_set_placeholder_text(ta, placeholder);
    lv_textarea_set_one_line(ta, true);
    if (T && T->font_normal) lv_obj_set_style_text_font(ta, T->font_normal, 0);
    return ta;
}

lv_obj_t *PageBase::CreateCard(lv_obj_t *parent, int x, int y, int w, int h) {
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_bg_color(card, lv_color_hex(TC(T->color_bg_white)), 0);
    lv_obj_set_style_radius(card, TC(T->card_radius), 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

lv_obj_t *PageBase::CreateFlexCard(lv_obj_t *parent, int x, int y, int w, int h, lv_flex_flow_t flow) {
    lv_obj_t *card = CreateCard(parent, x, y, w, h);
    lv_obj_set_flex_flow(card, flow);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    return card;
}

lv_obj_t *PageBase::CreateDropdown(lv_obj_t *parent, const char *options, lv_event_cb_t cb, void *user_data) {
    lv_obj_t *dropdown = lv_dropdown_create(parent);
    lv_dropdown_set_options(dropdown, options);
    lv_obj_set_size(dropdown, TC(T->dropdown_w), TC(T->input_h));
    if (T && T->font_normal) lv_obj_set_style_text_font(dropdown, T->font_normal, 0);
    lv_obj_set_style_radius(dropdown, TC(T->input_radius), 0);
    lv_obj_set_style_border_color(dropdown, lv_color_hex(TC(T->color_border)), 0);
    lv_obj_set_style_pad_ver(dropdown, TC(T->input_pad_v), 0);
    lv_dropdown_open(dropdown);
    lv_obj_t *list = lv_dropdown_get_list(dropdown);
    if (list && T && T->font_normal) lv_obj_set_style_text_font(list, T->font_normal, 0);
    lv_dropdown_close(dropdown);
    if (cb) lv_obj_add_event_cb(dropdown, cb, LV_EVENT_VALUE_CHANGED, user_data);
    return dropdown;
}

lv_obj_t *PageBase::CreateSmallInput(lv_obj_t *parent, const char *placeholder, int max_len, lv_event_cb_t cb, void *user_data) {
    lv_obj_t *ta = lv_textarea_create(parent);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_text(ta, "");
    lv_textarea_set_placeholder_text(ta, placeholder);
    lv_textarea_set_max_length(ta, max_len);
    lv_obj_set_size(ta, TC(T->input_w_sm), TC(T->input_h));
    if (T && T->font_normal) lv_obj_set_style_text_font(ta, T->font_normal, 0);
    lv_obj_set_style_text_align(ta, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_border_width(ta, 1, 0);
    lv_obj_set_style_border_color(ta, lv_color_hex(TC(T->color_border)), 0);
    lv_obj_set_style_radius(ta, TC(T->input_radius), 0);
    lv_obj_set_style_pad_ver(ta, TC(T->input_pad_v), 0);
    if (cb) lv_obj_add_event_cb(ta, cb, LV_EVENT_ALL, user_data);
    return ta;
}

lv_obj_t *PageBase::CreateFlexInput(lv_obj_t *parent, const char *placeholder, int max_len, lv_event_cb_t cb, void *user_data) {
    lv_obj_t *ta = lv_textarea_create(parent);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_text(ta, "");
    lv_textarea_set_placeholder_text(ta, placeholder);
    lv_textarea_set_max_length(ta, max_len);
    lv_obj_set_flex_grow(ta, 1);
    lv_obj_set_height(ta, TC(T->input_h));
    if (T && T->font_normal) lv_obj_set_style_text_font(ta, T->font_normal, 0);
    lv_obj_set_style_border_width(ta, 1, 0);
    lv_obj_set_style_border_color(ta, lv_color_hex(TC(T->color_border)), 0);
    lv_obj_set_style_radius(ta, TC(T->input_radius), 0);
    lv_obj_set_style_pad_ver(ta, TC(T->input_pad_v), 0);
    if (cb) lv_obj_add_event_cb(ta, cb, LV_EVENT_ALL, user_data);
    return ta;
}

lv_obj_t *PageBase::CreateIconLabel(lv_obj_t *parent, const char *icon, const char *text, uint32_t icon_color) {
    lv_obj_t *icon_lab = lv_label_create(parent);
    lv_label_set_text(icon_lab, icon);
    if (T && T->font_icon) lv_obj_set_style_text_font(icon_lab, T->font_icon, 0);
    lv_obj_set_style_text_color(icon_lab, lv_color_hex(icon_color), 0);
    lv_obj_align(icon_lab, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_t *text_lab = lv_label_create(parent);
    lv_label_set_text(text_lab, text);
    if (T && T->font_normal) lv_obj_set_style_text_font(text_lab, T->font_normal, 0);
    lv_obj_set_style_text_color(text_lab, lv_color_hex(TC(T->color_text_primary)), 0);
    lv_obj_align(text_lab, LV_ALIGN_LEFT_MID, TC(T->icon_offset), 0);
    return text_lab;
}

void PageBase::SetPageBackground(lv_obj_t *parent, uint32_t color) {
    uint32_t c = color ? color : (T ? T->color_bg_light : 0xF0F0F0);
    lv_obj_set_style_bg_color(parent, lv_color_hex(c), 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
}

lv_obj_t *PageBase::CreateKeyboard(lv_obj_t *parent, lv_keyboard_mode_t mode) {
    lv_obj_t *kb = lv_keyboard_create(parent);
    lv_keyboard_set_mode(kb, mode);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    if (IsLargeScreen() && T && T->font_icon)
        lv_obj_set_height(kb, ScreenHeight() * 40 / 100),
        lv_obj_set_style_text_font(kb, T->font_icon, LV_PART_ITEMS);
    return kb;
}

lv_obj_t *PageBase::CreateDialog(int w, int h, uint32_t border_color) {
    uint32_t bc = border_color ? border_color : (T ? T->color_primary : 0x2196F3);
    lv_obj_t *dlg = lv_obj_create(lv_layer_top());
    lv_obj_set_size(dlg, w, h);
    lv_obj_center(dlg);
    lv_obj_set_style_bg_color(dlg, lv_color_hex(T ? T->color_bg_white : 0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(dlg, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(dlg, 2, 0);
    lv_obj_set_style_border_color(dlg, lv_color_hex(bc), 0);
    lv_obj_set_style_radius(dlg, TC(T->dialog_radius), 0);
    lv_obj_set_style_shadow_width(dlg, TC(T->shadow_w), 0);
    lv_obj_set_style_shadow_color(dlg, lv_color_hex(TC(T->color_bg_overlay)), 0);
    lv_obj_set_style_shadow_opa(dlg, LV_OPA_20, 0);
    lv_obj_set_style_pad_all(dlg, TC(T->pad_h), 0);
    lv_obj_set_scrollbar_mode(dlg, LV_SCROLLBAR_MODE_OFF);
    return dlg;
}

void PageBase::CreateInfoRow(lv_obj_t *parent, int y, const char *icon, const char *title, const char *value, uint32_t color, lv_obj_t **value_label) {
    int g = T ? T->gap : 6;
    int io = T ? T->icon_offset : 24;
    int lw = T ? T->label_w : 60;
    lv_obj_t *icon_l = lv_label_create(parent);
    lv_label_set_text(icon_l, icon);
    if (T && T->font_icon) lv_obj_set_style_text_font(icon_l, T->font_icon, 0);
    lv_obj_set_style_text_color(icon_l, lv_color_hex(color), 0);
    lv_obj_set_pos(icon_l, 0, y + g / 2);
    lv_obj_t *title_l = lv_label_create(parent);
    lv_label_set_text(title_l, title);
    if (T && T->font_normal) lv_obj_set_style_text_font(title_l, T->font_normal, 0);
    lv_obj_set_style_text_color(title_l, lv_color_hex(TC(T->color_text_secondary)), 0);
    lv_obj_set_pos(title_l, io, y);
    *value_label = lv_label_create(parent);
    lv_label_set_text(*value_label, value);
    if (T && T->font_normal) lv_obj_set_style_text_font(*value_label, T->font_normal, 0);
    lv_obj_set_style_text_color(*value_label, lv_color_hex(TC(T->color_text_primary)), 0);
    lv_obj_set_pos(*value_label, lw + io, y);
}

void PageBase::ShowLoading(const char *text) {
    if (loading_overlay_) return;
    const bool large = IsLargeScreen();
    loading_overlay_ = lv_obj_create(lv_layer_top());
    lv_obj_set_size(loading_overlay_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(loading_overlay_, 0, 0);
    lv_obj_set_style_bg_color(loading_overlay_, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(loading_overlay_, LV_OPA_40, 0);
    lv_obj_set_style_border_width(loading_overlay_, 0, 0);
    lv_obj_set_style_radius(loading_overlay_, 0, 0);
    lv_obj_clear_flag(loading_overlay_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(loading_overlay_, LV_OBJ_FLAG_CLICKABLE);
    int card_w = large ? 240 : 180;
    int card_h = large ? 130 : 95;
    int pad = T ? (large ? T->pad_h : T->gap * 2) : 12;
    lv_obj_t *card = lv_obj_create(loading_overlay_);
    lv_obj_set_size(card, card_w, card_h);
    lv_obj_center(card);
    lv_obj_set_style_bg_color(card, lv_color_hex(TC(T->color_bg_white)), 0);
    lv_obj_set_style_radius(card, TC(T->card_radius), 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_shadow_width(card, TC(T->shadow_w), 0);
    lv_obj_set_style_shadow_color(card, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_20, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(card, pad, 0);
    lv_obj_set_style_pad_row(card, T ? T->gap * 2 : 12, 0);
    lv_obj_t *spinner = lv_spinner_create(card);
    lv_obj_set_size(spinner, large ? 50 : 36, large ? 50 : 36);
    lv_spinner_set_anim_params(spinner, 1000, 200);
    int aw = large ? 5 : 3;
    lv_obj_set_style_arc_width(spinner, aw, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, aw, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(TC(T->color_primary)), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(TC(T->color_border)), LV_PART_MAIN);
    lv_obj_t *lab = lv_label_create(card);
    lv_label_set_text(lab, text);
    if (T && T->font_normal) lv_obj_set_style_text_font(lab, T->font_normal, 0);
    lv_obj_set_style_text_color(lab, lv_color_hex(TC(T->color_text_primary)), 0);
    lv_refr_now(NULL);
}

void PageBase::HideLoading() {
    if (loading_overlay_) {
        lv_obj_delete(loading_overlay_);
        loading_overlay_ = nullptr;
    }
}

int PageBase::GetStatusBarHeight() const {
    return (theme_ && theme_->status_bar_height > 0) ? theme_->status_bar_height : 0;
}

#undef T
#undef TC
} // namespace ui

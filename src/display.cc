#include "lvgl_nav_kit/display.h"
#include <esp_log.h>

#define TAG "Display"

namespace ui {

Display::Display() {}
Display::~Display() {}

void Display::SetStatus(const char *status) {
    (void)status;
    ESP_LOGD(TAG, "SetStatus: %s", status ? status : "");
}

void Display::ShowNotification(const std::string &notification, int duration_ms) {
    ShowNotification(notification.c_str(), duration_ms);
}

void Display::ShowNotification(const char *notification, int duration_ms) {
    (void)notification;
    (void)duration_ms;
    ESP_LOGD(TAG, "ShowNotification: %s", notification ? notification : "");
}

void Display::UpdateStatusBar(bool update_all) {
    (void)update_all;
}

void Display::SetPowerSaveMode(bool on) {
    (void)on;
    ESP_LOGD(TAG, "SetPowerSaveMode: %d", on);
}

} // namespace ui

lv_indev_t *lvgl_nav_kit_add_pointer_indev(lv_display_t *disp,
                                            lv_indev_read_cb_t read_cb,
                                            void *user_data) {
    if (disp == nullptr || read_cb == nullptr) {
        return nullptr;
    }
    lv_indev_t *indev = lv_indev_create();
    if (indev == nullptr) {
        return nullptr;
    }
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, read_cb);
    lv_indev_set_user_data(indev, user_data);
#if LVGL_VERSION_MAJOR >= 9
    lv_indev_set_display(indev, disp);
#endif
    return indev;
}

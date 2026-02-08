#ifndef LVGL_NAV_KIT_DISPLAY_H
#define LVGL_NAV_KIT_DISPLAY_H

#include <lvgl.h>
#include <esp_log.h>
#include <string>

class Display {
public:
    Display();
    virtual ~Display();
    virtual void SetStatus(const char *status);
    virtual void ShowNotification(const char *notification, int duration_ms = 3000);
    virtual void ShowNotification(const std::string &notification, int duration_ms = 3000);
    virtual void UpdateStatusBar(bool update_all = false);
    virtual void SetPowerSaveMode(bool on);
    virtual void SetAutoScreenOff(bool enabled, uint32_t timeout_ms) {}
    virtual void ShowOtaProgress(bool show, const char *status = nullptr, int progress = -1) {}
    virtual void UpdateOtaProgress(const char *status, int progress) {}
    int width() const { return width_; }
    int height() const { return height_; }
protected:
    int width_ = 0;
    int height_ = 0;
    friend class DisplayLockGuard;
    virtual bool Lock(int timeout_ms = 0) = 0;
    virtual void Unlock() = 0;
};

class DisplayLockGuard {
public:
    explicit DisplayLockGuard(Display *display) : display_(display) {
        if (display_ && !display_->Lock(30000)) ESP_LOGE("Display", "Failed to lock display");
    }
    ~DisplayLockGuard() { if (display_) display_->Unlock(); }
private:
    Display *display_;
};

class NoDisplay : public Display {
    bool Lock(int timeout_ms = 0) override { (void)timeout_ms; return true; }
    void Unlock() override {}
};

/** Optional: bind a pointer (touch) device to a display. Skip if using lvgl_port_add_touch. */
lv_indev_t *lvgl_nav_kit_add_pointer_indev(lv_display_t *disp,
                                           lv_indev_read_cb_t read_cb,
                                           void *user_data);

#endif /* LVGL_NAV_KIT_DISPLAY_H */

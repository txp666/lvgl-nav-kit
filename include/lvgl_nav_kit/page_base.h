#ifndef LVGL_NAV_KIT_PAGE_BASE_H
#define LVGL_NAV_KIT_PAGE_BASE_H

#include <functional>
#include <string>
#include <vector>
#include "lvgl.h"
#include "lvgl_nav_kit/ui_types.h"
#include "lvgl_nav_kit/ui_theme.h"

namespace ui {

class UIManager;
class PageRegistry;

class PageBase {
    friend class UIManager;
    friend class PageRegistry;
public:
    explicit PageBase(const char *id);
    virtual ~PageBase();
    virtual void OnCreate(lv_obj_t *parent) = 0;
    virtual void OnEnter() {}
    virtual void OnLeave() {}
    virtual void OnDestroy() {}
    const char *GetId() const { return id_.c_str(); }
    lv_obj_t *GetContainer() const { return container_; }
    PageState GetState() const { return state_; }
    const ui_theme_t *GetTheme() const { return theme_; }
protected:
    lv_timer_t *CreateTimer(lv_timer_cb_t cb, uint32_t period, void *user_data = nullptr);
    void DeleteAllTimers();
    void AddEventHandler(lv_obj_t *obj, lv_event_cb_t cb, lv_event_code_t code, void *user_data = nullptr);
    static bool IsLargeScreen() { return LV_HOR_RES >= 720; }
    static int ScreenWidth() { return LV_HOR_RES; }
    static int ScreenHeight() { return LV_VER_RES; }
    /** Content area top offset from theme status_bar_height (0 = no bar). */
    int GetStatusBarHeight() const;
    lv_obj_t *CreateLabel(lv_obj_t *parent, const char *text);
    lv_obj_t *CreateButton(lv_obj_t *parent, const char *text, lv_event_cb_t cb, void *user_data = nullptr);
    lv_obj_t *CreateCheckbox(lv_obj_t *parent, const char *text, lv_event_cb_t cb, void *user_data = nullptr);
    lv_obj_t *CreateTextarea(lv_obj_t *parent, const char *placeholder);
    lv_obj_t *CreateCard(lv_obj_t *parent, int x, int y, int w, int h);
    lv_obj_t *CreateFlexCard(lv_obj_t *parent, int x, int y, int w, int h, lv_flex_flow_t flow);
    lv_obj_t *CreateDropdown(lv_obj_t *parent, const char *options, lv_event_cb_t cb = nullptr, void *user_data = nullptr);
    lv_obj_t *CreateSmallInput(lv_obj_t *parent, const char *placeholder, int max_len, lv_event_cb_t cb = nullptr, void *user_data = nullptr);
    lv_obj_t *CreateFlexInput(lv_obj_t *parent, const char *placeholder, int max_len, lv_event_cb_t cb = nullptr, void *user_data = nullptr);
    lv_obj_t *CreateIconLabel(lv_obj_t *parent, const char *icon, const char *text, uint32_t icon_color);
    void SetPageBackground(lv_obj_t *parent, uint32_t color = 0);
    lv_obj_t *CreateKeyboard(lv_obj_t *parent, lv_keyboard_mode_t mode = LV_KEYBOARD_MODE_NUMBER);
    lv_obj_t *CreateDialog(int w, int h, uint32_t border_color = 0);
    void CreateInfoRow(lv_obj_t *parent, int y, const char *icon, const char *title, const char *value, uint32_t color, lv_obj_t **value_label);
    void ShowLoading(const char *text = "Loading...");
    void HideLoading();
    bool IsLoading() const { return loading_overlay_ != nullptr; }
protected:
    lv_obj_t *container_ = nullptr;
    std::string id_;
    PageState state_ = PageState::Registered;
    lv_obj_t *loading_overlay_ = nullptr;
private:
    void DoCreate(lv_obj_t *parent, const ui_theme_t *theme);
    void DoDestroy();
    const ui_theme_t *theme_ = nullptr;
    std::vector<lv_timer_t *> timers_;
    struct EventBinding { lv_obj_t *obj; lv_event_cb_t cb; };
    std::vector<EventBinding> event_bindings_;
};

} // namespace ui

#endif /* LVGL_NAV_KIT_PAGE_BASE_H */

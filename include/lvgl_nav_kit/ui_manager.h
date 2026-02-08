#ifndef LVGL_NAV_KIT_UI_MANAGER_H
#define LVGL_NAV_KIT_UI_MANAGER_H

#include <functional>
#include <string>
#include "lvgl.h"
#include "lvgl_nav_kit/page_registry.h"
#include "lvgl_nav_kit/ui_theme.h"
#include "lvgl_nav_kit/ui_types.h"

namespace ui {

class UIManager {
public:
    static UIManager &GetInstance();
    void Initialize(lv_obj_t *parent, const ui_theme_t *theme = nullptr);
    void Shutdown();
    bool IsInitialized() const { return initialized_; }
    PageRegistry &GetRegistry() { return registry_; }
    void NavigateTo(const char *page_id, Direction dir = Direction::Right, TransitionType type = TransitionType::Slide);
    void NavigateToWithFade(const char *page_id);
    void NavigateBack();
    PageBase *GetCurrentPage() const { return current_page_; }
    void SetTransitionType(TransitionType type);
    void SetTransitionDuration(uint32_t ms);
    void EnableGesture(bool enable);
    TransitionType GetTransitionType() const { return transition_type_; }
    uint32_t GetTransitionDuration() const { return transition_duration_; }
    const ui_theme_t *GetTheme() const { return theme_; }
private:
    UIManager();
    ~UIManager();
    UIManager(const UIManager &) = delete;
    UIManager &operator=(const UIManager &) = delete;
    void DoNavigate(PageBase *target, Direction dir, TransitionType type);
    void DoSlideTransition(lv_obj_t *old_obj, lv_obj_t *new_obj, PageBase *old_page, PageBase *target, Direction dir);
    void DoFadeTransition(lv_obj_t *old_obj, lv_obj_t *new_obj, PageBase *old_page, PageBase *target);
    void OnGestureDetected(Direction dir);
    static void GestureEventCb(lv_event_t *e);
    bool initialized_ = false;
    lv_obj_t *parent_ = nullptr;
    lv_obj_t *page_container_ = nullptr;
    const ui_theme_t *theme_ = nullptr;
    PageRegistry registry_;
    PageBase *current_page_ = nullptr;
    static constexpr int kMaxHistory = 10;
    std::string history_[kMaxHistory];
    int history_index_ = 0;
    bool gesture_enabled_ = true;
    TransitionType transition_type_ = TransitionType::Slide;
    uint32_t transition_duration_ = 300;
    bool is_animating_ = false;
};

} // namespace ui

#endif /* LVGL_NAV_KIT_UI_MANAGER_H */

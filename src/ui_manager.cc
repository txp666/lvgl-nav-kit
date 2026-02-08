#include "lvgl_nav_kit/ui_manager.h"
#include "lvgl_nav_kit/ui_theme.h"
#include <algorithm>
#include <esp_log.h>

#define TAG "UIManager"

namespace ui {

UIManager &UIManager::GetInstance() {
    static UIManager instance;
    return instance;
}

UIManager::UIManager() {}
UIManager::~UIManager() { Shutdown(); }

void UIManager::Initialize(lv_obj_t *parent, const ui_theme_t *theme) {
    if (initialized_) {
        ESP_LOGW(TAG, "UI Manager already initialized");
        return;
    }
    parent_ = parent;
    theme_ = theme ? theme : ui_theme_get_default();

    page_container_ = lv_obj_create(parent_);
    lv_obj_set_size(page_container_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(page_container_, 0, 0);
    lv_obj_set_style_pad_all(page_container_, 0, 0);
    lv_obj_set_style_border_width(page_container_, 0, 0);
    lv_obj_set_style_radius(page_container_, 0, 0);
    lv_obj_set_style_bg_color(page_container_, lv_color_white(), 0);
    lv_obj_clear_flag(page_container_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page_container_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(page_container_, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_flag(page_container_, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_event_cb(parent_, GestureEventCb, LV_EVENT_GESTURE, this);

    initialized_ = true;
    ESP_LOGI(TAG, "UI Manager initialized");
}

void UIManager::Shutdown() {
    if (!initialized_) return;
    if (current_page_) {
        current_page_->OnLeave();
        current_page_->DoDestroy();
        current_page_ = nullptr;
    }
    inactive_cache_.clear();
    registry_.Clear();
    if (page_container_) {
        lv_obj_delete(page_container_);
        page_container_ = nullptr;
    }
    history_index_ = 0;
    theme_ = nullptr;
    initialized_ = false;
    ESP_LOGI(TAG, "UI Manager shutdown");
}

void UIManager::NavigateTo(const char *page_id, Direction dir, TransitionType type) {
    if (!initialized_) { ESP_LOGE(TAG, "UI Manager not initialized"); return; }
    PageBase *target = registry_.GetPage(page_id);
    if (!target) { ESP_LOGW(TAG, "Page '%s' not found", page_id); return; }
    DoNavigate(target, dir, type);
}

void UIManager::NavigateToWithFade(const char *page_id) {
    NavigateTo(page_id, Direction::Right, TransitionType::Fade);
}

void UIManager::NavigateBack() {
    if (history_index_ > 0) {
        history_index_--;
        const HistoryEntry &entry = history_[history_index_];
        PageBase *target = registry_.GetPage(entry.page_id.c_str());
        if (target) {
            DoNavigate(target, GetOppositeDirection(entry.dir), entry.type, false);
        }
    }
}

void UIManager::SetTransitionType(TransitionType type) { transition_type_ = type; }
void UIManager::SetTransitionDuration(uint32_t ms) { transition_duration_ = ms; }
void UIManager::EnableGesture(bool enable) { gesture_enabled_ = enable; }
void UIManager::SetMaxCachedPages(int n) { max_cached_pages_ = n; }

void UIManager::DoNavigate(PageBase *target, Direction dir, TransitionType type, bool record_history) {
    if (!target || is_animating_) return;

    ESP_LOGI(TAG, "Navigating from '%s' to '%s' (dir: %s, type: %d)",
             current_page_ ? current_page_->GetId() : "none", target->GetId(), DirectionToString(dir), (int)type);

    if (record_history && current_page_) {
        if (history_index_ >= kMaxHistory) {
            /* Shift history, drop oldest entry */
            for (int i = 0; i < kMaxHistory - 1; i++) {
                history_[i] = std::move(history_[i + 1]);
            }
            history_index_ = kMaxHistory - 1;
            ESP_LOGW(TAG, "Navigation history full, oldest entry dropped");
        }
        history_[history_index_] = {current_page_->GetId(), dir, type};
        history_index_++;
    }

    PageBase *old_page = current_page_;

    /* Remove target from inactive cache if it was cached */
    auto cache_it = std::find(inactive_cache_.begin(), inactive_cache_.end(), target);
    if (cache_it != inactive_cache_.end()) inactive_cache_.erase(cache_it);

    if (target->GetState() == PageState::Registered || target->GetState() == PageState::Destroyed) {
        target->DoCreate(page_container_, theme_);
    }

    if (target->GetContainer()) {
        lv_obj_add_flag(target->GetContainer(), LV_OBJ_FLAG_HIDDEN);
    }

    if (type == TransitionType::None || !old_page) {
        if (old_page) {
            old_page->OnLeave();
            old_page->state_ = PageState::Inactive;
            lv_obj_add_flag(old_page->GetContainer(), LV_OBJ_FLAG_HIDDEN);
            inactive_cache_.push_back(old_page);
        }
        if (target->GetContainer()) lv_obj_clear_flag(target->GetContainer(), LV_OBJ_FLAG_HIDDEN);
        target->state_ = PageState::Active;
        target->OnEnter();
        current_page_ = target;
        CleanupInactivePages();
    } else if (type == TransitionType::Fade) {
        DoFadeTransition(old_page->GetContainer(), target->GetContainer(), old_page, target);
        current_page_ = target;
    } else if (type == TransitionType::SlideOver) {
        DoSlideOverTransition(old_page->GetContainer(), target->GetContainer(), old_page, target, dir);
        current_page_ = target;
    } else {
        DoSlideTransition(old_page->GetContainer(), target->GetContainer(), old_page, target, dir);
        current_page_ = target;
    }
}

void UIManager::OnAnimationComplete(PageBase *old_page, PageBase *new_page) {
    if (old_page) {
        old_page->OnLeave();
        old_page->state_ = PageState::Inactive;
        if (old_page->GetContainer()) {
            lv_obj_add_flag(old_page->GetContainer(), LV_OBJ_FLAG_HIDDEN);
        }
        inactive_cache_.push_back(old_page);
    }
    new_page->state_ = PageState::Active;
    new_page->OnEnter();
    is_animating_ = false;
    CleanupInactivePages();
}

void UIManager::CleanupInactivePages() {
    if (max_cached_pages_ < 0) return;
    while (static_cast<int>(inactive_cache_.size()) > max_cached_pages_) {
        PageBase *p = inactive_cache_.front();
        inactive_cache_.erase(inactive_cache_.begin());
        ESP_LOGI(TAG, "Destroying cached page '%s'", p->GetId());
        p->DoDestroy();
    }
}

void UIManager::DoSlideTransition(lv_obj_t *old_obj, lv_obj_t *new_obj, PageBase *old_page, PageBase *target, Direction dir) {
    is_animating_ = true;
    int32_t w = lv_obj_get_width(page_container_);
    int32_t h = lv_obj_get_height(page_container_);
    int32_t start_x = 0, start_y = 0, end_x = 0, end_y = 0;
    switch (dir) {
        case Direction::Left:  start_x = w;   end_x = -w; break;
        case Direction::Right: start_x = -w;  end_x = w;  break;
        case Direction::Up:    start_y = h;   end_y = -h; break;
        case Direction::Down:  start_y = -h;  end_y = h;  break;
    }

    lv_obj_set_pos(new_obj, start_x, start_y);
    lv_obj_clear_flag(new_obj, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t anim_new;
    lv_anim_init(&anim_new);
    lv_anim_set_var(&anim_new, new_obj);
    lv_anim_set_time(&anim_new, transition_duration_);
    if (dir == Direction::Left || dir == Direction::Right) {
        lv_anim_set_values(&anim_new, start_x, 0);
        lv_anim_set_exec_cb(&anim_new, [](void *obj, int32_t v) { lv_obj_set_x((lv_obj_t *)obj, v); });
    } else {
        lv_anim_set_values(&anim_new, start_y, 0);
        lv_anim_set_exec_cb(&anim_new, [](void *obj, int32_t v) { lv_obj_set_y((lv_obj_t *)obj, v); });
    }
    lv_anim_set_path_cb(&anim_new, lv_anim_path_ease_out);
    lv_anim_start(&anim_new);

    lv_anim_t anim_old;
    lv_anim_init(&anim_old);
    lv_anim_set_var(&anim_old, old_obj);
    lv_anim_set_time(&anim_old, transition_duration_);
    if (dir == Direction::Left || dir == Direction::Right) {
        lv_anim_set_values(&anim_old, 0, end_x);
        lv_anim_set_exec_cb(&anim_old, [](void *obj, int32_t v) { lv_obj_set_x((lv_obj_t *)obj, v); });
    } else {
        lv_anim_set_values(&anim_old, 0, end_y);
        lv_anim_set_exec_cb(&anim_old, [](void *obj, int32_t v) { lv_obj_set_y((lv_obj_t *)obj, v); });
    }
    lv_anim_set_path_cb(&anim_old, lv_anim_path_ease_out);

    anim_ctx_ = {this, old_page, target};
    lv_anim_set_completed_cb(&anim_old, [](lv_anim_t *a) {
        AnimContext *c = (AnimContext *)a->user_data;
        if (c->old_p && c->old_p->GetContainer()) {
            lv_obj_set_pos(c->old_p->GetContainer(), 0, 0);
        }
        c->mgr->OnAnimationComplete(c->old_p, c->new_p);
    });
    lv_anim_set_user_data(&anim_old, &anim_ctx_);
    lv_anim_start(&anim_old);
}

void UIManager::DoSlideOverTransition(lv_obj_t *old_obj, lv_obj_t *new_obj, PageBase *old_page, PageBase *target, Direction dir) {
    is_animating_ = true;
    int32_t w = lv_obj_get_width(page_container_);
    int32_t h = lv_obj_get_height(page_container_);
    int32_t start_x = 0, start_y = 0;
    switch (dir) {
        case Direction::Left:  start_x = w;   break;
        case Direction::Right: start_x = -w;  break;
        case Direction::Up:    start_y = h;   break;
        case Direction::Down:  start_y = -h;  break;
    }

    lv_obj_set_pos(new_obj, start_x, start_y);
    lv_obj_clear_flag(new_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_to_index(new_obj, -1); /* bring new page to front */

    lv_anim_t anim_new;
    lv_anim_init(&anim_new);
    lv_anim_set_var(&anim_new, new_obj);
    lv_anim_set_time(&anim_new, transition_duration_);
    if (dir == Direction::Left || dir == Direction::Right) {
        lv_anim_set_values(&anim_new, start_x, 0);
        lv_anim_set_exec_cb(&anim_new, [](void *obj, int32_t v) { lv_obj_set_x((lv_obj_t *)obj, v); });
    } else {
        lv_anim_set_values(&anim_new, start_y, 0);
        lv_anim_set_exec_cb(&anim_new, [](void *obj, int32_t v) { lv_obj_set_y((lv_obj_t *)obj, v); });
    }
    lv_anim_set_path_cb(&anim_new, lv_anim_path_ease_out);

    anim_ctx_ = {this, old_page, target};
    lv_anim_set_completed_cb(&anim_new, [](lv_anim_t *a) {
        AnimContext *c = (AnimContext *)a->user_data;
        c->mgr->OnAnimationComplete(c->old_p, c->new_p);
    });
    lv_anim_set_user_data(&anim_new, &anim_ctx_);
    lv_anim_start(&anim_new);
}

void UIManager::DoFadeTransition(lv_obj_t *old_obj, lv_obj_t *new_obj, PageBase *old_page, PageBase *target) {
    is_animating_ = true;
    lv_obj_set_pos(new_obj, 0, 0);
    lv_obj_set_style_opa(new_obj, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(new_obj, LV_OBJ_FLAG_HIDDEN);

    lv_anim_t anim_new;
    lv_anim_init(&anim_new);
    lv_anim_set_var(&anim_new, new_obj);
    lv_anim_set_values(&anim_new, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&anim_new, transition_duration_);
    lv_anim_set_exec_cb(&anim_new, [](void *obj, int32_t v) { lv_obj_set_style_opa((lv_obj_t *)obj, v, 0); });
    lv_anim_set_path_cb(&anim_new, lv_anim_path_ease_in_out);
    lv_anim_start(&anim_new);

    lv_anim_t anim_old;
    lv_anim_init(&anim_old);
    lv_anim_set_var(&anim_old, old_obj);
    lv_anim_set_values(&anim_old, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&anim_old, transition_duration_);
    lv_anim_set_exec_cb(&anim_old, [](void *obj, int32_t v) { lv_obj_set_style_opa((lv_obj_t *)obj, v, 0); });
    lv_anim_set_path_cb(&anim_old, lv_anim_path_ease_in_out);

    anim_ctx_ = {this, old_page, target};
    lv_anim_set_completed_cb(&anim_old, [](lv_anim_t *a) {
        AnimContext *c = (AnimContext *)a->user_data;
        if (c->old_p && c->old_p->GetContainer()) {
            lv_obj_set_style_opa(c->old_p->GetContainer(), LV_OPA_COVER, 0);
        }
        c->mgr->OnAnimationComplete(c->old_p, c->new_p);
    });
    lv_anim_set_user_data(&anim_old, &anim_ctx_);
    lv_anim_start(&anim_old);
}

void UIManager::OnGestureDetected(Direction dir) {
    if (!gesture_enabled_ || is_animating_ || !current_page_) return;
    lv_indev_wait_release(lv_indev_active());

    PageBase *target = nullptr;
    Direction anim_dir = dir;
    TransitionType anim_type = TransitionType::Slide;

    if (registry_.GetNavigationTarget(current_page_->GetId(), dir, target, anim_dir, anim_type)) {
        ESP_LOGI(TAG, "Gesture %s on '%s' -> '%s'", DirectionToString(dir), current_page_->GetId(), target->GetId());
        DoNavigate(target, anim_dir, anim_type);
    }
}

void UIManager::GestureEventCb(lv_event_t *e) {
    UIManager *self = (UIManager *)lv_event_get_user_data(e);
    lv_indev_t *indev = lv_indev_active();
    if (!indev) return;
    lv_dir_t gdir = lv_indev_get_gesture_dir(indev);
    Direction dir;
    switch (gdir) {
        case LV_DIR_LEFT:   dir = Direction::Left;   break;
        case LV_DIR_RIGHT:  dir = Direction::Right;  break;
        case LV_DIR_TOP:    dir = Direction::Up;     break;
        case LV_DIR_BOTTOM: dir = Direction::Down;   break;
        default: return;
    }
    self->OnGestureDetected(dir);
}

} // namespace ui

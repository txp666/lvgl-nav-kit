#include "lvgl_nav_kit/page_registry.h"
#include <algorithm>
#include <esp_log.h>

#define TAG "PageRegistry"

namespace ui {

PageRegistry::PageRegistry() {}
PageRegistry::~PageRegistry() { Clear(); }

void PageRegistry::RegisterPage(PageBase *page) {
    if (!page) {
        ESP_LOGE(TAG, "Cannot register null page");
        return;
    }
    auto it = id_map_.find(page->GetId());
    if (it != id_map_.end()) {
        ESP_LOGW(TAG, "Page ID '%s' already exists, replacing", page->GetId());
        PageBase *old = it->second;
        pages_.erase(std::find(pages_.begin(), pages_.end(), old));
        delete old;
    }
    pages_.push_back(page);
    id_map_[page->GetId()] = page;
    ESP_LOGI(TAG, "Registered page '%s'", page->GetId());
}

PageBase *PageRegistry::GetPage(const char *id) {
    auto it = id_map_.find(id);
    return it != id_map_.end() ? it->second : nullptr;
}

void PageRegistry::SetNavigation(const char *page_id, const PageNavigation &nav) {
    navigation_map_[page_id] = nav;
    ESP_LOGI(TAG, "Set navigation for '%s'", page_id);
}

bool PageRegistry::GetNavigationTarget(const char *page_id, Direction gesture_dir, PageBase *&out_target, Direction &out_anim_dir, TransitionType &out_anim_type) {
    auto it = navigation_map_.find(page_id);
    if (it == navigation_map_.end()) {
        out_target = nullptr;
        return false;
    }
    const PageNavigation &nav = it->second;
    const NavTarget *target = nullptr;
    switch (gesture_dir) {
        case Direction::Left:  target = &nav.left;  break;
        case Direction::Right: target = &nav.right; break;
        case Direction::Up:    target = &nav.up;    break;
        case Direction::Down:  target = &nav.down;  break;
    }
    if (!target || !target->page) {
        out_target = nullptr;
        return false;
    }
    out_target = GetPage(target->page);
    out_anim_dir = target->anim_dir;
    out_anim_type = target->anim_type;
    return out_target != nullptr;
}

void PageRegistry::Clear() {
    for (auto *p : pages_) delete p;
    pages_.clear();
    id_map_.clear();
    navigation_map_.clear();
}

} // namespace ui

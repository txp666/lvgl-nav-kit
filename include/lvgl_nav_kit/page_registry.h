#ifndef LVGL_NAV_KIT_PAGE_REGISTRY_H
#define LVGL_NAV_KIT_PAGE_REGISTRY_H

#include <map>
#include <string>
#include "lvgl_nav_kit/page_base.h"
#include "lvgl_nav_kit/ui_types.h"

namespace ui {

struct NavTarget {
    const char *page = nullptr;
    Direction anim_dir = Direction::Right;
    TransitionType anim_type = TransitionType::Slide;
    NavTarget() = default;
    NavTarget(const char *p) : page(p), anim_dir(Direction::Right), anim_type(TransitionType::Slide) {}
    NavTarget(const char *p, Direction dir) : page(p), anim_dir(dir), anim_type(TransitionType::Slide) {}
    NavTarget(const char *p, TransitionType type) : page(p), anim_dir(Direction::Right), anim_type(type) {}
    NavTarget(const char *p, Direction dir, TransitionType type) : page(p), anim_dir(dir), anim_type(type) {}
};

struct PageNavigation {
    NavTarget left, right, up, down;
};

class PageRegistry {
public:
    PageRegistry();
    ~PageRegistry();
    void RegisterPage(PageBase *page);
    PageBase *GetPage(const char *id);
    void SetNavigation(const char *page_id, const PageNavigation &nav);
    bool GetNavigationTarget(const char *page_id, Direction gesture_dir, PageBase *&out_target, Direction &out_anim_dir, TransitionType &out_anim_type);
    void Clear();
    size_t GetPageCount() const { return id_map_.size(); }
private:
    std::map<std::string, PageBase *> id_map_;
    std::map<std::string, PageNavigation> navigation_map_;
};

} // namespace ui

#endif /* LVGL_NAV_KIT_PAGE_REGISTRY_H */

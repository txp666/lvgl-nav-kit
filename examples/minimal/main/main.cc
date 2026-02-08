/**
 * LVGL Nav Kit minimal example: Home / Settings / List / Detail.
 * Requires: main REQUIRES lvgl_nav_kit lvgl log; LVGL + display inited before UI (lv_scr_act() valid).
 */

#include "lvgl_nav_kit/page_base.h"
#include "lvgl_nav_kit/page_registry.h"
#include "lvgl_nav_kit/ui_manager.h"
#include "lvgl_nav_kit/ui_theme.h"
#include "lvgl_nav_kit/ui_types.h"
#include "lvgl.h"
#include "esp_log.h"
#include <cstdio>

static const char *TAG = "lvgl_nav_kit_example";

namespace {

// Home
class HomePage : public ui::PageBase {
public:
    HomePage() : ui::PageBase("home") {}
    void OnCreate(lv_obj_t *parent) override {
        SetPageBackground(parent);
        int top = GetStatusBarHeight() + 16;
        lv_obj_t *title = CreateLabel(parent, "Home");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, top);
        top += 40;
        lv_obj_t *btn_settings = CreateButton(parent, "Go to Settings", OnSettingsClicked, this);
        lv_obj_align(btn_settings, LV_ALIGN_TOP_MID, 0, top);
        top += 50;
        lv_obj_t *btn_list = CreateButton(parent, "Go to List", OnListClicked, this);
        lv_obj_align(btn_list, LV_ALIGN_TOP_MID, 0, top);
    }
    void OnEnter() override { ESP_LOGI(TAG, "HomePage OnEnter"); }
    void OnLeave() override { ESP_LOGI(TAG, "HomePage OnLeave"); }
private:
    static void OnSettingsClicked(lv_event_t *e) {
        (void)e;
        ui::UIManager::GetInstance().NavigateTo("settings", ui::Direction::Left);
    }
    static void OnListClicked(lv_event_t *e) {
        (void)e;
        ui::UIManager::GetInstance().NavigateTo("list", ui::Direction::Left);
    }
};

// Settings
class SettingsPage : public ui::PageBase {
public:
    SettingsPage() : ui::PageBase("settings") {}
    void OnCreate(lv_obj_t *parent) override {
        SetPageBackground(parent);
        int top = GetStatusBarHeight() + 16;
        lv_obj_t *title = CreateLabel(parent, "Settings");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, top);
        top += 36;
        lv_obj_t *dropdown = CreateDropdown(parent, "Option A\nOption B\nOption C", OnDropdownChanged, this);
        lv_obj_align(dropdown, LV_ALIGN_TOP_LEFT, 24, top);
        top += 60;
        lv_obj_t *back = CreateButton(parent, "Back to Home", OnBackClicked, this);
        lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -24);
    }
    void OnEnter() override { ESP_LOGI(TAG, "SettingsPage OnEnter"); }
    void OnLeave() override { ESP_LOGI(TAG, "SettingsPage OnLeave"); }
private:
    static void OnDropdownChanged(lv_event_t *e) {
        lv_obj_t *dd = lv_event_get_target(e);
        uint16_t sel = lv_dropdown_get_selected(dd);
        ESP_LOGI(TAG, "Settings dropdown selected: %u", (unsigned)sel);
    }
    static void OnBackClicked(lv_event_t *e) {
        (void)e;
        ui::UIManager::GetInstance().NavigateBack();
    }
};

// List
static const char *kListItems[] = { "Item A", "Item B", "Item C" };
static const int kListCount = 3;
static int s_selected_list_index = 0;

class ListPage : public ui::PageBase {
public:
    ListPage() : ui::PageBase("list") {}
    void OnCreate(lv_obj_t *parent) override {
        SetPageBackground(parent);
        int top = GetStatusBarHeight() + 16;
        lv_obj_t *title = CreateLabel(parent, "List");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, top);
        top += 40;
        int card_w = ScreenWidth() - 48;
        int card_h = 52;
        for (int i = 0; i < kListCount; i++) {
            lv_obj_t *card = CreateCard(parent, 24, top + i * (card_h + 8), card_w, card_h);
            lv_obj_t *label = CreateLabel(card, kListItems[i]);
            lv_obj_center(label);
            card->user_data = reinterpret_cast<void *>(static_cast<intptr_t>(i));
            AddEventHandler(card, OnItemClicked, LV_EVENT_CLICKED, nullptr);
        }
        top += kListCount * (card_h + 8) + 16;
        lv_obj_t *back = CreateButton(parent, "Back to Home", OnBackClicked, this);
        lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -24);
    }
    void OnEnter() override { ESP_LOGI(TAG, "ListPage OnEnter"); }
    void OnLeave() override { ESP_LOGI(TAG, "ListPage OnLeave"); }
private:
    static void OnItemClicked(lv_event_t *e) {
        lv_obj_t *target = lv_event_get_target(e);
        int idx = static_cast<int>(reinterpret_cast<intptr_t>(target->user_data));
        if (idx >= 0 && idx < kListCount) {
            s_selected_list_index = idx;
            /* SlideOver: detail slides over the list page */
            ui::UIManager::GetInstance().NavigateTo("detail", ui::Direction::Left, ui::TransitionType::SlideOver);
        }
    }
    static void OnBackClicked(lv_event_t *e) {
        (void)e;
        ui::UIManager::GetInstance().NavigateBack();
    }
};

// Detail
class DetailPage : public ui::PageBase {
public:
    DetailPage() : ui::PageBase("detail") {}
    void OnCreate(lv_obj_t *parent) override {
        SetPageBackground(parent);
        int top = GetStatusBarHeight() + 16;
        lv_obj_t *title = CreateLabel(parent, "Detail");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, top);
        top += 40;
        lv_obj_t *value_label = nullptr;
        const char *item_name = (s_selected_list_index >= 0 && s_selected_list_index < kListCount)
            ? kListItems[s_selected_list_index] : "—";
        CreateInfoRow(parent, top, "", "Selected", item_name, 0, &value_label);
        top += 48;
        lv_obj_t *back = CreateButton(parent, "Back to List", OnBackClicked, this);
        lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -24);
    }
    void OnEnter() override { ESP_LOGI(TAG, "DetailPage OnEnter (index=%d)", s_selected_list_index); }
    void OnLeave() override { ESP_LOGI(TAG, "DetailPage OnLeave"); }
private:
    static void OnBackClicked(lv_event_t *e) {
        (void)e;
        ui::UIManager::GetInstance().NavigateBack();
    }
};

} // namespace

/* Optional custom theme; pass &s_my_theme to Initialize, or nullptr for default. */
static void init_custom_theme(ui_theme_t *out) {
    const ui_theme_t *def = ui_theme_get_default();
    *out = *def;
    out->color_primary = 0x0066CC;
    out->card_radius = 12;
    out->status_bar_height = 0;
}

extern void app_main(void) {
    ESP_LOGI(TAG, "LVGL Nav Kit example starting");
    lv_obj_t *screen = lv_scr_act();
    if (!screen) {
        ESP_LOGE(TAG, "lv_scr_act() is null. Init LVGL and display first (e.g. lv_init, lvgl_port_add_disp).");
        return;
    }

    // 1) Init UI and theme (nullptr = default)
    ui_theme_t custom_theme;
    init_custom_theme(&custom_theme);
    auto &mgr = ui::UIManager::GetInstance();
    mgr.Initialize(screen, &custom_theme);

    // 2) Register pages
    auto &reg = mgr.GetRegistry();
    reg.RegisterPage(new HomePage());
    reg.RegisterPage(new SettingsPage());
    reg.RegisterPage(new ListPage());
    reg.RegisterPage(new DetailPage());

    // 3) Gesture targets (left/right/up/down)
    using Nav = ui::PageNavigation;
    using D = ui::Direction;
    using T = ui::TransitionType;
    reg.SetNavigation("home",     Nav{ {"settings", D::Right}, {}, {}, {"list", D::Right} });
    reg.SetNavigation("settings", Nav{ {}, {"home", D::Left}, {}, {} });
    reg.SetNavigation("list",     Nav{ {}, {"home", D::Left}, {}, {"detail", D::Right, T::SlideOver} });
    reg.SetNavigation("detail",   Nav{ {}, {"list", D::Left}, {}, {} });

    // 4) Transition settings
    mgr.SetTransitionDuration(280);
    /* Keep at most 3 inactive pages in memory; -1 = unlimited (default), 0 = destroy immediately */
    mgr.SetMaxCachedPages(3);

    mgr.NavigateTo("home");
    ESP_LOGI(TAG, "Example ready. Use buttons or swipe to navigate.");
}

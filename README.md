# LVGL Nav Kit

Page navigation framework for **ESP-IDF + LVGL**: lifecycle, swipe gestures, slide/fade transitions, theme abstraction, and a display base class.

## Features

- **Page lifecycle** — `OnCreate` / `OnEnter` / `OnLeave` / `OnDestroy` on `PageBase`
- **Navigation** — `UIManager` singleton: page stack, gestures, `NavigateTo` / `NavigateBack`
- **Transitions** — Slide, Fade, None
- **Theme** — `ui_theme_t` for fonts, colors, spacing; optional `Display` base for status bar / notifications

## Requirements

- ESP-IDF 5.x
- LVGL 9.x (via idf_component or managed_components)

## Install

**Local:** Copy `components/lvgl_nav_kit` into your project `components/`, then:

```cmake
idf_component_register(...
    REQUIRES lvgl_nav_kit
)
```

**Registry:** In your project's `idf_component.yml`:

```yaml
dependencies:
  txp666/lvgl-nav-kit: "^1.0.0"
```

## Quick start

After LVGL and display are initialized (`lv_scr_act()` valid):

```cpp
#include "lvgl_nav_kit/ui_manager.h"
#include "lvgl_nav_kit/page_base.h"
#include "lvgl_nav_kit/page_registry.h"

// Init (nullptr = default theme)
ui::UIManager::GetInstance().Initialize(lv_scr_act(), nullptr);

auto &reg = ui::UIManager::GetInstance().GetRegistry();
reg.RegisterPage(new HomePage());
reg.RegisterPage(new SettingsPage());
// e.g. swipe left from "home" -> "settings"; swipe right from "settings" -> "home"
reg.SetNavigation("home",     {{{"settings", ui::Direction::Right}, {}, {}, {}}});
reg.SetNavigation("settings", {{{}, {"home", ui::Direction::Left}, {}, {}}});

ui::UIManager::GetInstance().NavigateTo("home");
```

Implement pages by subclassing `PageBase`, overriding `OnCreate(lv_obj_t *parent)` and using helpers like `CreateLabel`, `CreateButton`, `SetPageBackground`, `GetStatusBarHeight()`.

## API summary

| UIManager | PageBase (override) |
|-----------|---------------------|
| `Initialize(screen, theme)` | `OnCreate(parent)` required |
| `GetRegistry()` → `RegisterPage`, `SetNavigation` | `OnEnter`, `OnLeave`, `OnDestroy` |
| `NavigateTo(id, dir)`, `NavigateBack()` | `CreateLabel`, `CreateButton`, `CreateCard`, … |
| `SetTransitionDuration(ms)`, `EnableGesture(bool)` | `GetStatusBarHeight()`, `GetTheme()` |

**Display:** Subclass `Display` in your app for status bar/notifications; use `NoDisplay` when headless. Theme’s `status_bar_height` (0 = none) is used by `GetStatusBarHeight()`.

**Touch:** Display/touch hardware init stays in the app. For custom pointer input (e.g. FT6236), use `lvgl_nav_kit_add_pointer_indev(disp, read_cb, user_data)` from `lvgl_nav_kit/display.h`.

## Examples

- **examples/minimal** — Multi-page UI (Home / Settings / List / Detail). Copy `main/` into your project; ensure LVGL + display are inited first.
- **examples/esp32_lcd_touch** — Full runnable: ESP32-S3 + ST7789 LCD + FT6236 touch registration, then same UI. Copy `main/` and add deps (see example README).

## License

MIT.

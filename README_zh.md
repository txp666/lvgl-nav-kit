# LVGL Nav Kit

面向 **ESP-IDF + LVGL** 的页面导航框架：生命周期、滑动手势、滑动/淡入淡出过渡、主题抽象与显示基类。

## 功能

- **页面生命周期** — `PageBase` 提供 `OnCreate` / `OnEnter` / `OnLeave` / `OnDestroy`
- **导航** — `UIManager` 单例：页面栈、手势、`NavigateTo` / `NavigateBack`
- **过渡** — Slide、Fade、None
- **主题** — `ui_theme_t` 配置字体、颜色、间距；可选 `Display` 基类做状态栏/通知

## 依赖

- ESP-IDF 5.x
- LVGL 9.x（通过 idf_component 或 managed_components 引入）

## 安装

**本地：** 将 `components/lvgl_nav_kit` 拷贝到项目 `components/` 下，在 main 的 CMake 中：

```cmake
idf_component_register(...
    REQUIRES lvgl_nav_kit
)
```

**组件库：** 在 `idf_component.yml` 中：

```yaml
dependencies:
  txp666/lvgl-nav-kit: "^1.0.0"
```

## 快速开始

在 LVGL 与显示初始化完成（`lv_scr_act()` 有效）后：

```cpp
#include "lvgl_nav_kit/ui_manager.h"
#include "lvgl_nav_kit/page_base.h"
#include "lvgl_nav_kit/page_registry.h"

// 初始化（nullptr = 默认主题）
ui::UIManager::GetInstance().Initialize(lv_scr_act(), nullptr);

auto &reg = ui::UIManager::GetInstance().GetRegistry();
reg.RegisterPage(new HomePage());
reg.RegisterPage(new SettingsPage());
// 例如：从 home 左滑到 settings，从 settings 右滑回 home
reg.SetNavigation("home",     {{{"settings", ui::Direction::Right}, {}, {}, {}}});
reg.SetNavigation("settings", {{{}, {"home", ui::Direction::Left}, {}, {}}});

ui::UIManager::GetInstance().NavigateTo("home");
```

页面类继承 `PageBase`，实现 `OnCreate(lv_obj_t *parent)`，使用 `CreateLabel`、`CreateButton`、`SetPageBackground`、`GetStatusBarHeight()` 等辅助接口。

## API 摘要

| UIManager | PageBase（可重写） |
|-----------|---------------------|
| `Initialize(screen, theme)` | `OnCreate(parent)` 必实现 |
| `GetRegistry()` → `RegisterPage`、`SetNavigation` | `OnEnter`、`OnLeave`、`OnDestroy` |
| `NavigateTo(id, dir)`、`NavigateBack()` | `CreateLabel`、`CreateButton`、`CreateCard` 等 |
| `SetTransitionDuration(ms)`、`EnableGesture(bool)` | `GetStatusBarHeight()`、`GetTheme()` |

**Display：** 在应用中继承 `Display` 实现状态栏/通知；无屏或测试时用 `NoDisplay`。主题中 `status_bar_height`（0 表示无）供 `GetStatusBarHeight()` 使用。

**触摸：** 显示/触摸硬件初始化留在应用层。自定义指针输入（如 FT6236）可使用 `lvgl_nav_kit/display.h` 中的 `lvgl_nav_kit_add_pointer_indev(disp, read_cb, user_data)`。

## 例程

- **examples/minimal** — 多页面 UI（Home / Settings / List / Detail）。将 `main/` 拷入项目，先完成 LVGL 与显示初始化。
- **examples/esp32_lcd_touch** — 完整可运行：ESP32-S3 + ST7789 LCD + FT6236 触摸注册，再跑相同 UI。拷入 `main/` 并添加依赖（见例程内 README）。

## 许可证

MIT。

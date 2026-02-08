# LVGL Nav Kit

面向 **ESP-IDF + LVGL** 的页面导航框架：生命周期、滑动手势、滑动/覆盖滑动/淡入淡出过渡、主题抽象与显示基类。

## 功能

- **页面生命周期** — `PageBase` 提供 `OnCreate` / `OnEnter` / `OnLeave` / `OnDestroy`
- **导航** — `UIManager` 单例：页面栈、手势、`NavigateTo` / `NavigateBack`
- **过渡** — Slide、SlideOver、Fade、None
- **页面缓存** — 可配置非活跃页面上限，适用于内存受限设备
- **主题** — `ui_theme_t` 配置字体、颜色、间距；可选 `ui::Display` 基类做状态栏/通知

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
auto &mgr = ui::UIManager::GetInstance();
mgr.Initialize(lv_scr_act(), nullptr);

auto &reg = mgr.GetRegistry();
reg.RegisterPage(new HomePage());
reg.RegisterPage(new SettingsPage());

// 例如：从 home 左滑到 settings，从 settings 右滑回 home
reg.SetNavigation("home",     {{{"settings", ui::Direction::Right}, {}, {}, {}}});
reg.SetNavigation("settings", {{{}, {"home", ui::Direction::Left}, {}, {}}});

// 可选：限制内存中的非活跃页面数量（默认 -1 = 不限制）
mgr.SetMaxCachedPages(3);

mgr.NavigateTo("home");
```

页面类继承 `PageBase`，实现 `OnCreate(lv_obj_t *parent)`，使用 `CreateLabel`、`CreateButton`、`SetPageBackground`、`GetStatusBarHeight()` 等辅助接口。

## API 摘要

| UIManager | PageBase（可重写） |
|-----------|---------------------|
| `Initialize(screen, theme)` | `OnCreate(parent)` 必实现 |
| `GetRegistry()` → `RegisterPage`、`SetNavigation` | `OnEnter`、`OnLeave`、`OnDestroy` |
| `NavigateTo(id, dir, type)`、`NavigateBack()` | `CreateLabel`、`CreateButton`、`CreateCard` 等 |
| `SetTransitionDuration(ms)`、`EnableGesture(bool)` | `GetStatusBarHeight()`、`GetTheme()` |
| `SetMaxCachedPages(n)` — 页面内存管理 | `ShowLoading()`、`HideLoading()` |

**过渡类型：** `Slide`（新旧页面同时滑动）、`SlideOver`（新页面覆盖滑入，旧页面不动）、`Fade`、`None`。可通过 `NavTarget(page, dir, type)` 单独配置，或通过 `NavigateTo(id, dir, type)` 逐次指定。

**NavigateBack：** 自动反转动画方向，并使用与前进导航相同的过渡类型。

**Display：** 在应用中继承 `ui::Display` 实现状态栏/通知；无屏或测试时用 `ui::NoDisplay`。主题中 `status_bar_height`（0 表示无）供 `GetStatusBarHeight()` 使用。

**触摸：** 显示/触摸硬件初始化留在应用层。自定义指针输入（如 FT6236）可使用 `lvgl_nav_kit/display.h` 中的 `lvgl_nav_kit_add_pointer_indev(disp, read_cb, user_data)`。

**线程安全：** `UIManager` 所有公开方法必须在 LVGL 任务中调用（使用 `esp_lvgl_port` 时需持有 LVGL 锁）。

## 例程

- **examples/minimal** — 多页面 UI（Home / Settings / List / Detail），含 SlideOver 演示。将 `main/` 拷入项目，先完成 LVGL 与显示初始化。
- **examples/esp32_lcd_touch** — 完整可运行：ESP32-S3 + ST7789 LCD + FT6236 触摸注册，再跑相同 UI。拷入 `main/` 并添加依赖（见例程内 README）。

## 许可证

MIT。

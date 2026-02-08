/**
 * LVGL Nav Kit example: ESP32-S3 + ST7789 LCD + FT6236 touch. Registers display and touch, then same UI as minimal.
 * Copy main/ into your project; REQUIRES lvgl_nav_kit lvgl log esp_lvgl_port esp_lcd driver spi_master i2c_master esp_lcd_panel_io_additions.
 */

#include "board_config.h"
#include "lvgl_nav_kit/display.h"
#include "lvgl_nav_kit/page_base.h"
#include "lvgl_nav_kit/page_registry.h"
#include "lvgl_nav_kit/ui_manager.h"
#include "lvgl_nav_kit/ui_theme.h"
#include "lvgl_nav_kit/ui_types.h"

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_io_additions.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lvgl_port.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <driver/spi_master.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

#include <cstring>

static const char *TAG = "esp32_lcd_touch";

// Touch read callback (FT6236/FT6x36: read 16 bytes from reg 0x00)
static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    esp_lcd_panel_io_handle_t touch_io = (esp_lcd_panel_io_handle_t)lv_indev_get_user_data(indev);
    uint8_t buf[16];
    esp_err_t ret = esp_lcd_panel_io_rx_param(touch_io, 0x00, buf, sizeof(buf));
    if (ret != ESP_OK) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    uint8_t points = buf[0x02] & 0x0F;
    if (points > 0 && points < 3) {
        data->state = LV_INDEV_STATE_PRESSED;
        uint16_t x = (uint16_t)((buf[0x03] & 0x0F) << 8) | buf[0x04];
        uint16_t y = (uint16_t)((buf[0x05] & 0x0F) << 8) | buf[0x06];
#if LCD_SWAP_XY
        uint16_t t = x; x = y; y = t;
#endif
        if (x > (uint16_t)LCD_WIDTH)  x = LCD_WIDTH;
        if (y > (uint16_t)LCD_HEIGHT) y = LCD_HEIGHT;
        data->point.x = (int32_t)x;
        data->point.y = (int32_t)y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

// Display + touch init; returns default lv_display_t* or nullptr on failure
static lv_display_t *init_display_and_touch(void) {
    esp_lcd_panel_io_handle_t panel_io = nullptr;
    esp_lcd_panel_handle_t panel = nullptr;
    esp_lcd_panel_io_handle_t touch_io = nullptr;
    i2c_master_bus_handle_t touch_i2c_bus = nullptr;

    // SPI + LCD panel
    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = LCD_MOSI_PIN;
    bus_cfg.miso_io_num = GPIO_NUM_NC;
    bus_cfg.sclk_io_num = LCD_CLK_PIN;
    bus_cfg.quadwp_io_num = GPIO_NUM_NC;
    bus_cfg.quadhd_io_num = GPIO_NUM_NC;
    bus_cfg.max_transfer_sz = (size_t)LCD_WIDTH * LCD_HEIGHT * 2;
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_cfg = {};
    io_cfg.cs_gpio_num = LCD_CS_PIN;
    io_cfg.dc_gpio_num = LCD_DC_PIN;
    io_cfg.spi_mode = LCD_SPI_MODE;
    io_cfg.pclk_hz = 80 * 1000 * 1000;
    io_cfg.trans_queue_depth = 10;
    io_cfg.lcd_cmd_bits = 8;
    io_cfg.lcd_param_bits = 8;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(LCD_SPI_HOST, &io_cfg, &panel_io));

    esp_lcd_panel_dev_config_t panel_cfg = {};
    panel_cfg.reset_gpio_num = LCD_RST_PIN;
    panel_cfg.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
    panel_cfg.bits_per_pixel = 16;
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_cfg, &panel));

    esp_lcd_panel_reset(panel);
    esp_lcd_panel_init(panel);
    esp_lcd_panel_invert_color(panel, LCD_INVERT_COLORS);
    esp_lcd_panel_swap_xy(panel, LCD_SWAP_XY);
    esp_lcd_panel_mirror(panel, LCD_MIRROR_X, LCD_MIRROR_Y);

    uint16_t *line = (uint16_t *)heap_caps_malloc((size_t)LCD_WIDTH * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (line) {
        for (int i = 0; i < LCD_WIDTH; i++) line[i] = 0xFFFF;
        for (int y = 0; y < LCD_HEIGHT; y++) {
            esp_lcd_panel_draw_bitmap(panel, 0, y, LCD_WIDTH, y + 1, line);
        }
        heap_caps_free(line);
    }
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));
    ESP_LOGI(TAG, "LCD panel on");

    // LVGL port
    lv_init();
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    port_cfg.task_priority = 7;
    lvgl_port_init(&port_cfg);

    lvgl_port_display_cfg_t disp_cfg = {};
    disp_cfg.io_handle = panel_io;
    disp_cfg.panel_handle = panel;
    disp_cfg.control_handle = nullptr;
    disp_cfg.buffer_size = (uint32_t)(LCD_WIDTH * 20);
    disp_cfg.double_buffer = false;
    disp_cfg.trans_size = 0;
    disp_cfg.hres = (uint32_t)LCD_WIDTH;
    disp_cfg.vres = (uint32_t)LCD_HEIGHT;
    disp_cfg.monochrome = false;
    disp_cfg.rotation.swap_xy = LCD_SWAP_XY;
    disp_cfg.rotation.mirror_x = LCD_MIRROR_X;
    disp_cfg.rotation.mirror_y = LCD_MIRROR_Y;
    disp_cfg.color_format = LV_COLOR_FORMAT_RGB565;
    disp_cfg.flags.buff_dma = 1;
    disp_cfg.flags.buff_spiram = 0;
    disp_cfg.flags.sw_rotate = 0;
    disp_cfg.flags.swap_bytes = 1;
    disp_cfg.flags.full_refresh = 0;
    disp_cfg.flags.direct_mode = 0;

    lv_display_t *disp = lvgl_port_add_disp(&disp_cfg);
    if (!disp) {
        ESP_LOGE(TAG, "lvgl_port_add_disp failed");
        esp_lcd_panel_del(panel);
        esp_lcd_panel_io_del(panel_io);
        return nullptr;
    }
    if (LCD_OFFSET_X != 0 || LCD_OFFSET_Y != 0) {
        lv_display_set_offset(disp, LCD_OFFSET_X, LCD_OFFSET_Y);
    }

    // Backlight
    if (LCD_BACKLIGHT_PIN != GPIO_NUM_NC) {
        gpio_config_t io = {};
        io.pin_bit_mask = (1ULL << LCD_BACKLIGHT_PIN);
        io.mode = GPIO_MODE_OUTPUT;
        io.pull_up_en = GPIO_PULLUP_DISABLE;
        io.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io.intr_type = GPIO_INTR_DISABLE;
        gpio_config(&io);
        gpio_set_level(LCD_BACKLIGHT_PIN, 1);
    }

    // I2C touch
    i2c_master_bus_config_t i2c_cfg = {};
    i2c_cfg.i2c_port = TOUCH_I2C_NUM;
    i2c_cfg.sda_io_num = TOUCH_SDA_PIN;
    i2c_cfg.scl_io_num = TOUCH_SCL_PIN;
    i2c_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_cfg.glitch_ignore_cnt = 7;
    i2c_cfg.flags.enable_internal_pullup = 1;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_cfg, &touch_i2c_bus));

    esp_lcd_panel_io_i2c_config_t touch_io_cfg = {};
    touch_io_cfg.dev_addr = TOUCH_I2C_ADDR;
    touch_io_cfg.control_phase_bytes = 1;
    touch_io_cfg.dc_bit_offset = 0;
    touch_io_cfg.lcd_cmd_bits = 8;
    touch_io_cfg.lcd_param_bits = 8;
    touch_io_cfg.flags.disable_control_phase = 1;
    touch_io_cfg.scl_speed_hz = TOUCH_I2C_HZ;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c_v2(touch_i2c_bus, &touch_io_cfg, &touch_io));

    lv_indev_t *indev = lvgl_nav_kit_add_pointer_indev(disp, touch_read_cb, touch_io);
    if (indev) {
        ESP_LOGI(TAG, "Touch registered (FT6236/FT6x36)");
    } else {
        ESP_LOGW(TAG, "Touch indev register failed");
    }
    (void)indev;

    return disp;
}

// Pages (same as minimal, static callbacks)
namespace {

static const char *kItems[] = { "Item A", "Item B", "Item C" };
static const int kListCount = 3;
static int s_selected = 0;

class HomePage : public ui::PageBase {
public:
    HomePage() : ui::PageBase("home") {}
    void OnCreate(lv_obj_t *parent) override {
        SetPageBackground(parent);
        int top = GetStatusBarHeight() + 16;
        lv_obj_t *title = CreateLabel(parent, "Home");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, top);
        top += 40;
        lv_obj_t *b1 = CreateButton(parent, "Go to Settings", OnSettings, nullptr);
        lv_obj_align(b1, LV_ALIGN_TOP_MID, 0, top);
        top += 50;
        lv_obj_t *b2 = CreateButton(parent, "Go to List", OnList, nullptr);
        lv_obj_align(b2, LV_ALIGN_TOP_MID, 0, top);
    }
private:
    static void OnSettings(lv_event_t *e) {
        (void)e;
        ui::UIManager::GetInstance().NavigateTo("settings", ui::Direction::Left);
    }
    static void OnList(lv_event_t *e) {
        (void)e;
        ui::UIManager::GetInstance().NavigateTo("list", ui::Direction::Left);
    }
};

class SettingsPage : public ui::PageBase {
public:
    SettingsPage() : ui::PageBase("settings") {}
    void OnCreate(lv_obj_t *parent) override {
        SetPageBackground(parent);
        int top = GetStatusBarHeight() + 16;
        lv_obj_t *title = CreateLabel(parent, "Settings");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, top);
        top += 50;
        lv_obj_t *back = CreateButton(parent, "Back", OnBack, nullptr);
        lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -24);
    }
private:
    static void OnBack(lv_event_t *e) {
        (void)e;
        ui::UIManager::GetInstance().NavigateBack();
    }
};

class ListPage : public ui::PageBase {
public:
    ListPage() : ui::PageBase("list") {}
    void OnCreate(lv_obj_t *parent) override {
        SetPageBackground(parent);
        int top = GetStatusBarHeight() + 16;
        lv_obj_t *title = CreateLabel(parent, "List");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, top);
        top += 40;
        int w = ui::PageBase::ScreenWidth() - 48;
        int h = 52;
        for (int i = 0; i < kListCount; i++) {
            lv_obj_t *card = CreateCard(parent, 24, top + i * (h + 8), w, h);
            lv_obj_t *lab = CreateLabel(card, kItems[i]);
            lv_obj_center(lab);
            card->user_data = (void *)(intptr_t)i;
            AddEventHandler(card, OnItemClicked, LV_EVENT_CLICKED, nullptr);
        }
        lv_obj_t *back = CreateButton(parent, "Back", OnBack, nullptr);
        lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -24);
    }
private:
    static void OnItemClicked(lv_event_t *e) {
        lv_obj_t *target = lv_event_get_target(e);
        int idx = (int)(intptr_t)target->user_data;
        if (idx >= 0 && idx < kListCount) {
            s_selected = idx;
            /* SlideOver: detail slides over the list page */
            ui::UIManager::GetInstance().NavigateTo("detail", ui::Direction::Left, ui::TransitionType::SlideOver);
        }
    }
    static void OnBack(lv_event_t *e) {
        (void)e;
        ui::UIManager::GetInstance().NavigateBack();
    }
};

class DetailPage : public ui::PageBase {
public:
    DetailPage() : ui::PageBase("detail") {}
    void OnCreate(lv_obj_t *parent) override {
        SetPageBackground(parent);
        int top = GetStatusBarHeight() + 16;
        lv_obj_t *title = CreateLabel(parent, "Detail");
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, top);
        top += 40;
        lv_obj_t *val = nullptr;
        const char *name = (s_selected >= 0 && s_selected < kListCount) ? kItems[s_selected] : "—";
        CreateInfoRow(parent, top, "", "Selected", name, 0, &val);
        lv_obj_t *back = CreateButton(parent, "Back to List", OnBack, nullptr);
        lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -24);
    }
private:
    static void OnBack(lv_event_t *e) {
        (void)e;
        ui::UIManager::GetInstance().NavigateBack();
    }
};

} // namespace

extern void app_main(void) {
    ESP_LOGI(TAG, "ESP32 LCD + Touch example");
    lv_display_t *disp = init_display_and_touch();
    if (!disp) {
        ESP_LOGE(TAG, "Display init failed");
        return;
    }
    lv_obj_t *screen = lv_scr_act();
    if (!screen) {
        ESP_LOGE(TAG, "No screen (lv_scr_act is null)");
        return;
    }

    auto &mgr = ui::UIManager::GetInstance();
    mgr.Initialize(screen, nullptr);

    auto &reg = mgr.GetRegistry();
    reg.RegisterPage(new HomePage());
    reg.RegisterPage(new SettingsPage());
    reg.RegisterPage(new ListPage());
    reg.RegisterPage(new DetailPage());

    using Nav = ui::PageNavigation;
    using D = ui::Direction;
    using T = ui::TransitionType;
    reg.SetNavigation("home",     Nav{ {"settings", D::Right}, {}, {}, {"list", D::Right} });
    reg.SetNavigation("settings", Nav{ {}, {"home", D::Left}, {}, {} });
    reg.SetNavigation("list",     Nav{ {}, {"home", D::Left}, {}, {"detail", D::Right, T::SlideOver} });
    reg.SetNavigation("detail",   Nav{ {}, {"list", D::Left}, {}, {} });

    mgr.SetTransitionDuration(280);
    /* Keep at most 3 inactive pages in memory; -1 = unlimited (default), 0 = destroy immediately */
    mgr.SetMaxCachedPages(3);

    mgr.NavigateTo("home");
    ESP_LOGI(TAG, "UI ready. Use touch or swipe.");
}

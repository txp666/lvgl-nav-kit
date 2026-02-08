# lvgl_nav_kit: page lifecycle, gestures, transitions, theme. Requires lvgl, log.

idf_component_register(
    SRCS
        "src/display.cc"
        "src/page_base.cc"
        "src/page_registry.cc"
        "src/ui_manager.cc"
        "src/ui_theme_default.c"
    INCLUDE_DIRS
        "include"
        "include/lvgl_nav_kit"
    REQUIRES
        lvgl
        log
)

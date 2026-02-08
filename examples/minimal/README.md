# Minimal example

Four pages (Home, Settings, List, Detail) with buttons and swipe navigation. Uses `CreateLabel`, `CreateButton`, `CreateCard`, `CreateInfoRow`, optional custom theme.

**Run:** Copy `main/` into your project `main/`, add `REQUIRES lvgl_nav_kit lvgl log`, and ensure LVGL + display are initialized before the UI code (e.g. `lv_scr_act()` valid). Then `idf.py build flash monitor`.

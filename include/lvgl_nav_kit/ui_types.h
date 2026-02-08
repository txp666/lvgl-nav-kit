#ifndef LVGL_NAV_KIT_UI_TYPES_H
#define LVGL_NAV_KIT_UI_TYPES_H

#include "lvgl.h"

namespace ui {

enum class Direction { Up, Down, Left, Right };
enum class TransitionType { None, Slide, Fade, SlideOver };
enum class PageState { Registered, Created, Active, Inactive, Destroyed };

inline Direction GetOppositeDirection(Direction dir) {
    switch (dir) {
        case Direction::Up: return Direction::Down;
        case Direction::Down: return Direction::Up;
        case Direction::Left: return Direction::Right;
        case Direction::Right: return Direction::Left;
    }
    return Direction::Up;
}

inline const char *DirectionToString(Direction dir) {
    switch (dir) {
        case Direction::Up: return "Up";
        case Direction::Down: return "Down";
        case Direction::Left: return "Left";
        case Direction::Right: return "Right";
    }
    return "Unknown";
}

} // namespace ui

#endif /* LVGL_NAV_KIT_UI_TYPES_H */

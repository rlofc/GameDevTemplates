#ifndef GDT_PLATFORM_HEADER_INCLUDED
#define GDT_PLATFORM_HEADER_INCLUDED

#include <functional>
#include <string>

#include "utils/checks.hh"
#include "core/screen.hh"

namespace gdt {

// clang-format off
enum key {
    A=0, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    N1, N2, N3, N4, N5, N6, N7, N8, N9, N0, 
    UP, DOWN, LEFT, RIGHT,
    ENTER, TAB, ESC,  BACKSPACE, SPACE 
};
// clang-format on

namespace blueprints::platform {

/**
 * platform backend;
 */
class backend {
  public:
    gdt::screen _screen;
    virtual void create_window() = 0;
    virtual void update_window() = 0;
    virtual void update_keyboard() = 0;
    virtual bool capture_mouse() const = 0;
    virtual void release_mouse() const = 0;
    virtual void update_mouse() = 0;
    virtual bool process_events() = 0;
    virtual bool is_key_pressed(key k) const = 0;
    virtual bool is_button_pressed() const = 0;
    virtual void get_mouse(int *x, int *y) const = 0;
    gdt::screen & screen() { return _screen; }
    const gdt::screen & c_screen() const { return _screen; }
    std::function<void(int w, int h)> on_resize_callback;
    std::function<void(int k)> on_key_callback;
};

}
}
#endif  // GDT_PLATFORM_HEADER_INCLUDED

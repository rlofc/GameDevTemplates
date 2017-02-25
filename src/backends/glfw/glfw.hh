#ifndef GDT_GLFW_HEADER_INCLUDED
#define GDT_GLFW_HEADER_INCLUDED

#include <GLFW/glfw3.h>

#include "platform.hh"

namespace gdt::platform::glfw {

class platform_is_glfw : public blueprints::platform::backend {
  protected:
    GLFWwindow *s_pWindow;

  public:
    platform_is_glfw(bool no_api = true);
    virtual ~platform_is_glfw();
    bool process_events() override;
    void create_window() override;
    void update_window() override;
    void update_keyboard() override;
    bool capture_mouse() const override;
    void release_mouse() const override;
    void update_mouse() override;
    bool is_key_pressed(key k) const override;
    void get_mouse(int *x, int *y) const override;
    bool is_button_pressed() const override;
    static std::string read_file(std::string filename);
    GLFWwindow *getwnd() const {
        return s_pWindow;
    }
};


};
#endif  // GDT_GLFW_HEADER_INCLUDED

#ifndef backends_glfw_glfw_opengl_hh_INCLUDED
#define backends_glfw_glfw_opengl_hh_INCLUDED

#include <GLES3/gl31.h>
#include "glfw.hh"

namespace gdt::platform::glfw {

class backend_for_opengl : public platform_is_glfw {
public:
    backend_for_opengl();
    virtual ~backend_for_opengl();

    void update_window() override;
    void imgui_frame();
};

};

#endif // backends/glfw/glfw_opengl_hh_INCLUDED


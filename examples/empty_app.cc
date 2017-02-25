/* examples / empty_app.cc
 * =======================
 *
 * This is a barebone GDT application, showing just a blank
 * screen.
 */
#include "gdt.h"

#include "backends/sdl/sdl.hh"
#include "backends/opengl/opengl.hh"

using my_app = gdt::application<gdt::platform::sdl::backend_for_opengl,
                                gdt::graphics::opengl::backend,
                                gdt::no_audio,
                                gdt::no_physics,
                                gdt::no_networking,
                                gdt::context>;

int main()
{
    try {
        return std::make_unique<my_app>()->run<my_app::empty_scene>();
    } catch (const std::exception & e) {
        LOG_ERROR << e.what();
        return 1;
    }
}

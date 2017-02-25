It's your App
=============

gdt::application
----------------

.. doxygenclass:: gdt::application
        :project: GDT
        :members:


Usage
-----

To create a basic OpenGL game using SDL as the
platform backend:

.. code-block:: cpp

    #include "gdt.h"

    #include "bricks/opengl/opengl.hh"
    #include "bricks/sdl/sdl.hh"

    using my_game = gdt::application<
        gdt::platform::sdl::backend_for_opengl,
        gdt::graphics::opengl::backend,
        gdt::no_audio,
        gdt::no_physics,
        gdt::no_networking,
        gdt::context
    >;

    int main()
    {
        std::make_unique<my_app>()->run<my_app::empty_scene>();
    }


#ifndef GDT_OPENGL_HEADER_INCLUDED
#define GDT_OPENGL_HEADER_INCLUDED

#include <GLES3/gl31.h>

#include <iostream>
#include <vector>
#include "backends/blueprints/graphics.hh"
#include "core/light.hh"
#include "core/math.hh"
#include "core/mesh.hh"
#include "utils/checks.hh"

#include "opengl_buffer.hh"
#include "opengl_shaders.hh"
#include "opengl_surface.hh"
#include "opengl_text.hh"

namespace gdt::graphics::opengl {
/**
 * OpenGL backend
 */
template <typename PLATFORM>
class backend : public blueprints::graphics::backend<backend<PLATFORM>>, screen::subscriber {
  private:
    std::vector<math::mat4 *> _instance_bufs;

  public:
    using pipeline = opengl_pipeline<backend, PLATFORM>;
    using filter_pipeline = opengl_filter_pipeline<backend, PLATFORM>;
    using base_pipeline = opengl_pipeline<backend, PLATFORM>;
    using cbackend = backend<PLATFORM>;
    using surface = opengl_surface<cbackend>;
    using texture = opengl_texture<cbackend>;
    using frame_buffer = opengl_base_buffer<cbackend>;
    using back_buffer = opengl_back_buffer<cbackend>;
    using depth_enabled_frame_buffer = opengl_depth_frame_buffer<cbackend>;
    using rgb16_buffer = opengl_rgb16_buffer<cbackend>;
    using rgba_buffer = opengl_rgba_buffer<cbackend>;
    using text = opengl_text<cbackend>;

    static const opengl_render_pass_clear_cmd clear;
    static const opengl_screen_buffer<backend> screen_buffer;

    backend(PLATFORM *p, screen *screen)
    {
        screen->subscribe(this);
    }

    virtual ~backend()
    {
        for (math::mat4 *p : _instance_bufs) {
            delete[] p;
        }
    }

    void clear_screen() const override
    {
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
    }

    void blend_on() const
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void blend_off() const
    {
        glDisable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void cull_on() const
    {
        glEnable(GL_CULL_FACE);
    }

    void cull_off() const
    {
        glDisable(GL_CULL_FACE);
    }

    void on_screen_resize(unsigned int w, unsigned int h) override
    {
        glViewport(0, 0, w, h);
    }

    void update_frame() override
    {
    }

    template <typename... SHADER>
    void process_cmds(std::function<void(const SHADER &...)> cmds, const SHADER &... s)
    {
        cmds(s...);
    }

    void create_instance_buffer(math::mat4 **d, int c)
    {
        *d = new math::mat4[c];
        _instance_bufs.push_back(*d);
    }

    void update_instance_buffer(math::mat4 *data, int count)
    {
    }

    bool created = false;

    GLuint quadVAO = 0;
    GLuint quadVBO;
    void render_quad()
    {
        if (quadVAO == 0) {
            GLfloat quadVertices[] = {
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
            };
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                                  (GLvoid *)(3 * sizeof(GLfloat)));
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
};

template <typename PLATFORM>
const opengl_render_pass_clear_cmd backend<PLATFORM>::clear;

template <typename PLATFORM>
const opengl_screen_buffer<backend<PLATFORM>> backend<PLATFORM>::screen_buffer;
}
#endif  // GDT_OPENGL_HEADER_INCLUDED

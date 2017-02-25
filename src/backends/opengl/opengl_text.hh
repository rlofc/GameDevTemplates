#ifndef SRC_BRICKS_OPENGL_OPENGL_TEXT_HH_INCLUDED
#define SRC_BRICKS_OPENGL_OPENGL_TEXT_HH_INCLUDED

#include "backends/blueprints/graphics.hh"
#include "core/context.hh"
#include "core/font.hh"

namespace gdt::graphics::opengl {

template <typename GRAPHICS>
class opengl_text : public gdt::blueprints::graphics::text<GRAPHICS> {
  public:
    opengl_text(const graphics_context<GRAPHICS>& ctx, const font<GRAPHICS>& f,
                const char* text);
    virtual ~opengl_text();

    // Only method to implement from the blueprint - this will
    // draw the pre-generated text object.
    void draw(const graphics_context<GRAPHICS>& ctx,
              const text_pipeline<GRAPHICS>& shader,
              const gdt::math::mat4* transform) const override;

  private:
    GLuint _vertex_vbo;
    GLuint _triangle_vbo;
    std::uint32_t _n_triangles;
};

template <typename GRAPHICS>
opengl_text<GRAPHICS>::opengl_text(const graphics_context<GRAPHICS>& ctx,
                                   const font<GRAPHICS>& f, const char* text)
    : gdt::blueprints::graphics::text<GRAPHICS>(ctx, f, text)
{
    GL_CHECK(glGenBuffers(1, &this->_vertex_vbo));
    GL_CHECK(glGenBuffers(1, &this->_triangle_vbo));

    std::vector<gdt::vertex> vs;
    std::vector<uint32_t> triangles;

    const char* p;
    int32_t xpos, ypos;
    xpos = ypos = 0;
    uint32_t tid = 0;
    float zpos = 0;
    int prev = -1;
    for (p = text; *p; p++) {
        if (*p == '\n') {
            ypos -= f.get_glyph(32).orig_dim.y * 1.5;  // whitespace height X line spacing
            xpos = 0;
            continue;
        }
        zpos += 0.01;
        const typename font<GRAPHICS>::glyph_data& m = f.get_glyph(*p);
        gdt::vertex v1, v2, v3, v4;

        static const float BUF = 0;
        float offv = m.orig_dim.y - m.dim.y - m.offset.y;
        xpos += m.offset.x;

        v1.position.x = xpos;
        v1.position.y = ypos + offv;
        v1.position.z = zpos;
        v1.uvs.x = (m.pos.x - BUF) / f.get_atlas_width();
        v1.uvs.y = 1.0 - (m.pos.y + m.dim.y + BUF) / f.get_atlas_height();
        vs.push_back(v1);

        v2.position.x = xpos + m.dim.x;
        v2.position.y = ypos + offv;
        v2.position.z = zpos;
        v2.uvs.x = (m.pos.x + m.dim.x + BUF) / f.get_atlas_width();
        v2.uvs.y = 1.0 - (m.pos.y + m.dim.y + BUF) / f.get_atlas_height();
        vs.push_back(v2);

        v3.position.x = xpos + m.dim.x;
        v3.position.y = ypos + m.dim.y + offv;
        v3.position.z = zpos;
        v3.uvs.x = (m.pos.x + m.dim.x + BUF) / f.get_atlas_width();
        v3.uvs.y = 1.0 - (m.pos.y - BUF) / f.get_atlas_height();
        vs.push_back(v3);

        v4.position.x = xpos;
        v4.position.y = ypos + m.dim.y + offv;
        v4.position.z = zpos;
        v4.uvs.x = (m.pos.x - BUF) / f.get_atlas_width();
        v4.uvs.y = 1.0 - (m.pos.y - BUF) / f.get_atlas_height();
        vs.push_back(v4);

        xpos += m.orig_dim.x - m.offset.x - BUF + (prev != -1 ? f.get_kerning(prev, *p) : 0);

        triangles.push_back(tid + 0);
        triangles.push_back(tid + 1);
        triangles.push_back(tid + 2);
        triangles.push_back(tid + 0);
        triangles.push_back(tid + 2);
        triangles.push_back(tid + 3);

        tid = tid + 4;
        prev = *p;
    }

    float* vb_data = (float*)malloc(sizeof(float) * vs.size() * 18);
    for (int i = 0; i < vs.size(); i++) {
        vs[i].to_array(&vb_data[(i * 18)]);
    }

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->_vertex_vbo));
    GL_CHECK(
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18 * vs.size(), vb_data, GL_STATIC_DRAW));
    free(vb_data);
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_triangle_vbo));
    _n_triangles = triangles.size();
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * triangles.size(),
                          &triangles[0], GL_STATIC_DRAW));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

template <typename GRAPHICS>
opengl_text<GRAPHICS>::~opengl_text()
{
    glDeleteBuffers(1, &_vertex_vbo);
    glDeleteBuffers(1, &_triangle_vbo);
}

template <typename GRAPHICS>
void opengl_text<GRAPHICS>::draw(const graphics_context<GRAPHICS>& ctx,
                                 const text_pipeline<GRAPHICS>& shader,
                                 const gdt::math::mat4* transform) const
{
    shader.set_transform(transform[0]);
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->_vertex_vbo));
    shader.enable_vertex_attributes();
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->_triangle_vbo));
    GL_CHECK(glDrawElements(GL_TRIANGLES, this->_n_triangles, GL_UNSIGNED_INT, (void*)0));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    shader.disable_all_vertex_attribs();
}
}

#endif  // src/backends/opengl/opengl_text_hh_INCLUDED

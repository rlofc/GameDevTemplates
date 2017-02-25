#ifndef BRICKS_OPENGL_OPENGL_SURFACE_HH_INCLUDED
#define BRICKS_OPENGL_OPENGL_SURFACE_HH_INCLUDED

namespace gdt::graphics::opengl {

template <typename GRAPHICS>
struct opengl_surface : blueprints::graphics::surface<GRAPHICS, opengl_surface<GRAPHICS>> {
    GLuint vertex_vbo;
    GLuint triangle_vbo;
    GLuint transform_vbo;
    GLuint world_vbo;
    opengl_surface(const graphics_context<typename GRAPHICS::backend> &ctx, mesh *m);
    virtual ~opengl_surface();

    template <typename PIPELINE>
    void draw_instanced(const GRAPHICS &backend,
                        const PIPELINE &shader,
                        const math::mat4 *transforms,
                        int count) const
    {
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->transform_vbo));
        shader.bind_instances();
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(math::mat4) * count, transforms,
                              GL_DYNAMIC_DRAW));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->vertex_vbo));
        shader.enable_vertex_attributes();

        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->triangle_vbo));
        GL_CHECK(glDrawElementsInstanced(GL_TRIANGLES, this->n_triangles * 3, GL_UNSIGNED_INT,
                                         (void *)0, count));

        shader.disable_all_vertex_attribs();
        // IMGUI
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        // IMGUI
    }
};

template <typename GRAPHICS>
opengl_surface<GRAPHICS>::~opengl_surface()
{
    glDeleteBuffers(1, &vertex_vbo);
    glDeleteBuffers(1, &triangle_vbo);
    glDeleteBuffers(1, &transform_vbo);
    glDeleteBuffers(1, &world_vbo);
}

template <typename GRAPHICS>
opengl_surface<GRAPHICS>::opengl_surface(
    const graphics_context<typename GRAPHICS::backend> &ctx, mesh *m)
    : gdt::blueprints::graphics::surface<GRAPHICS, opengl_surface<GRAPHICS>>(ctx, m)
{
    int vertex_size = m->is_rigged ? 24 : 18;
    this->calc_bounds(m);
    glGenBuffers(1, &this->vertex_vbo);
    glGenBuffers(1, &this->triangle_vbo);
    glGenBuffers(1, &this->transform_vbo);
    glGenBuffers(1, &this->world_vbo);
    this->n_vertices = m->vertices.size();
    this->n_triangles = m->triangles.size() / 3;
    float *vb_data = (float *)malloc(sizeof(float) * this->n_vertices * vertex_size);
    for (int i = 0; i < this->n_vertices; i++) {
        m->vertices[i].to_array(&vb_data[(i * vertex_size)]);
        if (m->is_rigged) {
            vb_data[(i * vertex_size) + 18] = (float)m->weights[i].bone_ids[0];
            vb_data[(i * vertex_size) + 19] = (float)m->weights[i].bone_ids[1];
            vb_data[(i * vertex_size) + 20] = (float)m->weights[i].bone_ids[2];
            vb_data[(i * vertex_size) + 21] = m->weights[i].bone_weights[0];
            vb_data[(i * vertex_size) + 22] = m->weights[i].bone_weights[1];
            vb_data[(i * vertex_size) + 23] = m->weights[i].bone_weights[2];
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, this->vertex_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * this->n_vertices * vertex_size, vb_data,
                 GL_STATIC_DRAW);
    free(vb_data);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->triangle_vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * this->n_triangles * 3,
                 &m->triangles[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
}

#endif  // backends/opengl/opengl_surface_hh_INCLUDED

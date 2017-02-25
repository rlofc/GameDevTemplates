#ifndef GDT_BLUEPRINTS_GRAPHICS_INCLUDED
#define GDT_BLUEPRINTS_GRAPHICS_INCLUDED

#include "checks.hh"
#include "context.hh"
#include "math.hh"
#include "mesh.hh"
#include "font.hh"
namespace gdt::blueprints::graphics {

template <typename B>
class pipeline {
};

template <typename GRAPHICS, typename SURFACE>
struct surface {
    int n_vertices;
    int n_triangles;
    math::vec3 max_v;
    math::vec3 min_v;

    surface(const graphics_context<typename GRAPHICS::backend> &ctx, mesh *m)
    {
    }
    virtual ~surface()
    {
    }

    void calc_bounds(const mesh *m)
    {
        float max_x, min_x, max_y, min_y, max_z, min_z;
        max_x = min_x = max_y = min_y = max_z = min_z = 0;
        for (const auto &v : m->vertices) {
            math::vec3 p = v.position;
            max_x = std::max(max_x, p.x);
            max_y = std::max(max_y, p.y);
            max_z = std::max(max_z, p.z);
            min_x = std::min(min_x, p.x);
            min_y = std::min(min_y, p.y);
            min_z = std::min(min_z, p.z);
        }
        max_v = math::vec3(max_x, max_y, max_z);
        min_v = math::vec3(min_x, min_y, min_z);
    }

    template <typename PIPELINE>
    void draw_instanced(const GRAPHICS &backend,
                        const PIPELINE &shader,
                        const math::mat4 *transforms,
                        int count) const
    {
        static_cast<const SURFACE *>(this)->draw_instanced(backend, shader, transforms, count);
    }
};

// TODO: batch multiple draw calls into one global text draw call.
template <typename GRAPHICS>
class text {
  public:
    text(const graphics_context<GRAPHICS>& ctx, const font<GRAPHICS>& f, const char* text){};

    virtual void draw(const graphics_context<GRAPHICS>& ctx, const text_pipeline<GRAPHICS>& shader,
                      const gdt::math::mat4* transform) const = 0;
};

template <typename BACKEND>
class backend {
  public:
    backend()
    {
        static_assert(std::is_base_of<pipeline<BACKEND>,
                      typename BACKEND::pipeline>::value);
        static_assert(std::is_base_of<pipeline<BACKEND>,
                      typename BACKEND::filter_pipeline>::value);
        static_assert(std::is_base_of<pipeline<BACKEND>,
                      typename BACKEND::base_pipeline>::value);
        static_assert(std::is_base_of<surface<BACKEND, typename BACKEND::surface>,
                                      typename BACKEND::surface>::value);
        static_assert(std::is_base_of<text<BACKEND>,
                      typename BACKEND::text>::value);
    }
    virtual void clear_screen() const = 0;
    virtual void update_frame() = 0;
};
};
#endif  // GDT_BLUEPRINTS_GRAPHICS_INCLUDED

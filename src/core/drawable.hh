#ifndef GDT_DRAWABLE_HEADER_INCLUDED
#define GDT_DRAWABLE_HEADER_INCLUDED
#define GL_EXT_PROTOTYPES

#include <memory>
#include <vector>

#include "checks.hh"
#include "graphics.hh"
#include "loader.hh"
#include "math.hh"
#include "mesh.hh"
#include "traits.hh"

namespace gdt {

/**
 * A drawable is an **almost** ready to draw 3D entity you load from a file, holding
 * the required surface data you need to provide your rendering pipeline.
 *
 * Why almost?
 *
 * In GDT, a drawable object contains the 3D vertices information of a model, 
 * without any material properties or texture map data. 
 * These attributes are stored in gdt::material objects or being set directly
 * using gdt::pipeline instances.
 *
 * Another part missing from gdt::drawable objects in the specific instance
 * information about the position, rotation and scale of the model in
 * the 3D world containing it. This information is being stored in
 * the gdt::instance or gdt::instances template that can contain
 * drawables or other type of GDT assets.
 *
 * Here's how you can subclass gdt::drawable to create your own assets:
 *
 *     class zombie : public my_app::asset<zombie>,
 *                    public my_app::drawable<zombie> {
 *       public:
 *         moon(const my_app::context& ctx):
 *             my_app::drawable<zombie>(ctx, "res/zombie.smd") {}
 *     };
 *
 *
 * And here's how you could instantiate a drawable in your scene class:
 *
 *     gdt::instance<zombie> _zombie;
 *     gdt::instances<zombie, 1000> _army_of_zombies;
 *
 * Now, if you want direct control over your drawable instances, you can
 * use gdt::driven together with a gdt::direct_driver or other drivers
 * to help you manipulate your instances transformations:
 *
 *     gdt::driven<gdt::instance<zombie>, gdt::direct_driver> _movable_zombie;
 *
 */
template <typename GRAPHICS, typename ACTUAL>
class drawable : public is_drawable<ACTUAL> {
  public:
    /**
     * Construct a drawable from the provided SMD model filename.
     *
     * @param ctx a graphics_context compatible context object
     * @param filename full path to a valid SMD model
     * @param loader file loader - SMD file loader by default
     */
    drawable(const graphics_context<GRAPHICS> &ctx, std::string filename,
                std::function<std::unique_ptr<model>(const char *filename)> loader = read_smd);

    virtual ~drawable();

    template <typename PIPELINE>
    void draw_instances(const graphics_context<GRAPHICS> &ctx, const PIPELINE &s,
                        const typename PIPELINE::material &_material,
                        const math::mat4 *transforms, int count) const;

    template <typename PIPELINE>
    void draw_instances(const graphics_context<GRAPHICS> &ctx, const PIPELINE &s,
                        const math::mat4 *transforms, int count) const;

    math::vec3 get_bounds() const;

  private:
    std::vector<std::unique_ptr<
        gdt::blueprints::graphics::surface<GRAPHICS, typename GRAPHICS::surface>>>
        _surfaces;
};

template <typename GRAPHICS, typename ACTUAL>
template <typename PIPELINE>
void drawable<GRAPHICS, ACTUAL>::draw_instances(const graphics_context<GRAPHICS> &ctx,
                                        const PIPELINE &s,
                                        const typename PIPELINE::material &_material,
                                        const math::mat4 *transforms,
                                        int count) const
{
    _material.bind(ctx, s);
    for (const auto &surf : _surfaces) {
        surf.get()->draw_instanced(*ctx.graphics, s, transforms, count);
    }
}

template <typename GRAPHICS, typename ACTUAL>
template <typename PIPELINE>
void drawable<GRAPHICS, ACTUAL>::draw_instances(const graphics_context<GRAPHICS> &ctx,
                                        const PIPELINE &s,
                                        const math::mat4 *transforms,
                                        int count) const
{
    for (const auto &surf : _surfaces) {
        surf.get()->draw_instanced(*ctx.graphics, s, transforms, count);
    }
}

template <typename GRAPHICS, typename ACTUAL>
math::vec3 drawable<GRAPHICS, ACTUAL>::get_bounds() const
{
    float max_x, min_x, max_y, min_y, max_z, min_z;
    max_x = min_x = max_y = min_y = max_z = min_z = 0;
    for (const auto &surf : _surfaces) {
        max_x = std::max(max_x, surf->max_v.x);
        max_y = std::max(max_y, surf->max_v.y);
        max_z = std::max(max_z, surf->max_v.z);
        min_x = std::min(min_x, surf->min_v.x);
        min_y = std::min(min_y, surf->min_v.x);
        min_z = std::min(min_z, surf->min_v.x);
    }
    math::vec3 ret = math::vec3((max_x - min_x) / 2, (max_y - min_y) / 2, (max_z - min_z) / 2);
    LOG_DEBUG << "drawable bounds are " << ret;
    return ret;
}

template <typename GRAPHICS, typename ACTUAL>
drawable<GRAPHICS, ACTUAL>::drawable(const graphics_context<GRAPHICS> &ctx,
                            std::string filename,
                            std::function<std::unique_ptr<model>(const char *filename)> loader)
{
    auto model = loader(filename.c_str());
    for (auto &m : model->meshes) {
        auto s = std::make_unique<typename GRAPHICS::surface>(ctx, m.get());
        _surfaces.push_back(std::move(s));
    }
}

template <typename GRAPHICS, typename ACTUAL>
drawable<GRAPHICS, ACTUAL>::~drawable()
{
}

}
#endif  // GDT_DRAWABLE_HEADER_INCLUDED

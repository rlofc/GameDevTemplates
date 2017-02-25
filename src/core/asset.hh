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

template <typename BACKEND>
class asset_impl {
  public:
    asset_impl(
        const graphics_context<BACKEND> &ctx, std::string filename,
        std::function<std::unique_ptr<model>(const char *filename)> loader = read_smd);
    virtual ~asset_impl();

    template <typename PIPELINE>
    void draw_instances(const graphics_context<BACKEND> &ctx, const PIPELINE &s,
                        const typename PIPELINE::material &_material,
                        const math::mat4 *transforms, int count) const;

    template <typename PIPELINE>
    void draw_instances(const graphics_context<BACKEND> &ctx, const PIPELINE &s,
                        const math::mat4 *transforms, int count) const;

    math::vec3 get_bounds() const;

  private:
    std::vector<std::unique_ptr<
        gdt::blueprints::graphics::surface<BACKEND, typename BACKEND::surface>>>
        _surfaces;
};

template <typename BACKEND>
class bare_asset : public asset_impl<BACKEND>,
                 public is_entity<bare_asset<BACKEND>>,
                 public is_drivable<bare_asset<BACKEND>>,
                 public is_drawable<bare_asset<BACKEND>>,
                 public is_non_animatable<bare_asset<BACKEND>>,
                 public is_non_collidable {
  public:
    bare_asset(const graphics_context<BACKEND> &ctx, std::string filename,
             std::function<std::unique_ptr<model>(const char *filename)> loader = read_smd)
        : asset_impl<BACKEND>(ctx, filename, loader)
    {
    }
    virtual ~bare_asset()
    {
    }
};

/**
 * An asset is an **almost** drawable 3D entity you load from a file, holding
 * the required surface data you need to provide your rendering pipeline.
 *
 * Why almost?
 *
 * In GDT, an asset object contains the 3D vertices information of a model, 
 * without any material properties or texture map data. 
 * These attributes are stored in gdt::material objects or being set directly
 * using gdt::pipeline instances.
 *
 * Another part missing from gdt::asset objects in the specific instance
 * information about the position, rotation and scale of the model in
 * the 3D world containing it. This information is being stored in
 * the gdt::instance or gdt::instances template that can contain
 * assets or other type of GDT entities.
 *
 */
template <typename ACTUAL>
class asset : public is_entity<ACTUAL>,
              public is_drivable<ACTUAL>
{
};

template <typename BACKEND,
          typename ACTUAL>
class drawable : public asset_impl<BACKEND>,
    public is_drawable<ACTUAL> {
  public:
    /**
     * Construct an asset from the provided SMD model filename.
     *
     * @param ctx a graphics_context compatible context object
     * @param filename full path to a valid SMD model
     * @param loader file loader - SMD file loader by default
     */
    drawable(const graphics_context<BACKEND> &ctx, std::string filename,
                std::function<std::unique_ptr<model>(const char *filename)> loader = read_smd)
        : asset_impl<BACKEND>(ctx, filename, loader)
    {
    }
    virtual ~drawable()
    {
    }
};


template <typename ACTUAL>
class animatable :
    public animixer,
                       public is_animatable<ACTUAL>
{
  public:
    /**
     * TODO
     */
    animatable(const skeleton& s)
        : animixer(s)
    {
    }
    virtual ~animatable()
    {
    }
};

template <typename BACKEND>
template <typename PIPELINE>
void asset_impl<BACKEND>::draw_instances(const graphics_context<BACKEND> &ctx,
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

template <typename BACKEND>
template <typename PIPELINE>
void asset_impl<BACKEND>::draw_instances(const graphics_context<BACKEND> &ctx,
                                            const PIPELINE &s,
                                            const math::mat4 *transforms,
                                            int count) const
{
    for (const auto &surf : _surfaces) {
        surf.get()->draw_instanced(*ctx.graphics, s, transforms, count);
    }
}

template <typename BACKEND>
math::vec3 asset_impl<BACKEND>::get_bounds() const
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
    LOG_DEBUG << "asset bounds are " << ret;
    return ret;
}

template <typename BACKEND>
asset_impl<BACKEND>::asset_impl(
    const graphics_context<BACKEND> &ctx, std::string filename,
    std::function<std::unique_ptr<model>(const char *filename)> loader)
{
    auto model = loader(filename.c_str());
    for (auto &m : model->meshes) {
        auto s = std::make_unique<typename BACKEND::surface>(ctx, m.get());
        _surfaces.push_back(std::move(s));
    }
}

template <typename BACKEND>
asset_impl<BACKEND>::~asset_impl()
{
}
}
#endif  // GDT_DRAWABLE_HEADER_INCLUDED

#ifndef GDT_CONSTRUCTS_RIGID_BODY_INCLUDED
#define GDT_CONSTRUCTS_RIGID_BODY_INCLUDED

#include "context.hh"
#include "math.hh"
#include "physics.hh"
#include "traits.hh"

namespace gdt {

/**
 * A proxy template that provides a bounding box collidable 
 * for objects in a physics domain.
 *
 * @tparam PHYSICS the physics backend to use
 * @tparam DRAWABLE the entity to use as a collidable object
 *
 */
template <typename PHYSICS, typename DRAWABLE>
class box_proxy : public container<DRAWABLE>,
                  public is_drivable<box_proxy<PHYSICS, DRAWABLE>>,
                  public is_collidable<box_proxy<PHYSICS, DRAWABLE>>,
                  public may_have_drawable<DRAWABLE, box_proxy<PHYSICS, DRAWABLE>>,
                  public may_have_animatable<DRAWABLE, box_proxy<PHYSICS, DRAWABLE>> {
  public:
    /**
     * TODO: document me
     */
    template <typename CONTEXT, typename... ZZ>
    box_proxy(const CONTEXT& ctx, float mass, ZZ&... rest);

    /**
     * TODO: document me
     */
    const typename PHYSICS::shape& get_shape() const;

    /**
     * TODO: document me
     */
    PHYSICS* get_physics();

    /**
     * TODO: document me
     */
    float get_mass() const;

  private:
    PHYSICS* _pd;
    float _mass;
    typename PHYSICS::shape _shape;
};

/**
 * TODO: document me
 */
template <typename PHYSICS, typename DRAWABLE>
class sphere_proxy : public container<DRAWABLE>,
                     public is_collidable<sphere_proxy<PHYSICS, DRAWABLE>>,
                     public may_have_drawable<DRAWABLE, sphere_proxy<PHYSICS, DRAWABLE>>,
                     public may_have_animatable<DRAWABLE, sphere_proxy<PHYSICS, DRAWABLE>>
{
  public:
    template <typename CONTEXT, typename... ZZ>
    sphere_proxy(const CONTEXT& ctx, float mass, ZZ&... rest);
    const typename PHYSICS::shape& get_shape() const;
    PHYSICS* get_physics();
    float get_mass() const;

  private:
    PHYSICS* _pd;
    float _mass;
    typename PHYSICS::shape _shape;
};

/**
 * TODO: document me
 */
template <typename PHYSICS, typename SHAPED>
class rigid_body_driver {
  public:
    struct initializer {
        math::vec3 pos;
        math::quat r;
    };
    rigid_body_driver(const physics_context<PHYSICS>& ctx,
                      math::mat4* transform,
                      SHAPED* shaped, initializer i);
    void update();
    void reuse(const physics_context<PHYSICS>& ctx, math::mat4* transform);

  private:
    math::mat4* _driven_transform;
    typename PHYSICS::body _body;
};

//------------------------------------------------------------------------------------------------
// BOX_PROXY
//------------------------------------------------------------------------------------------------

template <typename PHYSICS, typename DRAWABLE>
template <typename CONTEXT, typename... ZZ>
box_proxy<PHYSICS, DRAWABLE>::box_proxy(const CONTEXT& ctx, float mass, ZZ&... rest)
    : container<DRAWABLE>(ctx, rest...),
      _shape(ctx.physics->make_box_shape(this->content()->get_bounds())),
      _pd(ctx.physics),
      _mass(mass)
{
}

template <typename PHYSICS, typename DRAWABLE>
const typename PHYSICS::shape& box_proxy<PHYSICS, DRAWABLE>::get_shape() const
{
    return _shape;
}

template <typename PHYSICS, typename DRAWABLE>
PHYSICS* box_proxy<PHYSICS, DRAWABLE>::get_physics()
{
    return _pd;
}

template <typename PHYSICS, typename DRAWABLE>
float box_proxy<PHYSICS, DRAWABLE>::get_mass() const
{
    return _mass;
}

//------------------------------------------------------------------------------------------------
// SPHERE_PROXY
//------------------------------------------------------------------------------------------------

template <typename PHYSICS, typename DRAWABLE>
template <typename CONTEXT, typename... ZZ>
sphere_proxy<PHYSICS, DRAWABLE>::sphere_proxy(const CONTEXT& ctx, float mass, ZZ&... rest)
    : _shape(ctx.physics->make_sphere_shape(this->content()->get_radius())),
      _pd(ctx.physics),
      _mass(mass)
{
}

template <typename PHYSICS, typename DRAWABLE>
const typename PHYSICS::shape& sphere_proxy<PHYSICS, DRAWABLE>::get_shape() const
{
    return _shape;
}

template <typename PHYSICS, typename DRAWABLE>
PHYSICS* sphere_proxy<PHYSICS, DRAWABLE>::get_physics()
{
    return _pd;
}

template <typename PHYSICS, typename DRAWABLE>
float sphere_proxy<PHYSICS, DRAWABLE>::get_mass() const
{
    return _mass;
}

//------------------------------------------------------------------------------------------------
// RIGID_BODY_DRIVER
//------------------------------------------------------------------------------------------------

template <typename PHYSICS, typename SHAPED>
rigid_body_driver<PHYSICS, SHAPED>::rigid_body_driver(const physics_context<PHYSICS>& ctx,
                                                      math::mat4* transform,
                                                      SHAPED* shaped, initializer i)
    : _driven_transform(transform)
{
    //math::vec3 pos = math::vec3(transform->wx, transform->wy,
    //                            transform->wz);  // = get position from transform
    //math::quat r = transform->as_quat();
    _body =
        shaped->get_physics()->make_rigid_body(shaped->get_shape(), i.pos, i.r, shaped->get_mass());
    *_driven_transform = math::mat4::translation(i.pos).transpose();
}

template <typename PHYSICS, typename SHAPED>
void rigid_body_driver<PHYSICS, SHAPED>::update()
{
    _body.update_transform(_driven_transform);
}

template <typename PHYSICS, typename SHAPED>
void rigid_body_driver<PHYSICS, SHAPED>::reuse(const physics_context<PHYSICS>& ctx,
                                               math::mat4* transform)
{
    math::vec3 pos = math::vec3(transform->wx, transform->wy,
                                transform->wz);  // = get position from transform
    _body.reposition(pos);
    *_driven_transform = math::mat4::translation(pos).transpose();
}
};
#endif  // GDT_CONSTRUCTS_RIGID_BODY_INCLUDED

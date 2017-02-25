#ifndef GDT_PHYSICS_HEADER_INCLUDED
#define GDT_PHYSICS_HEADER_INCLUDED

#include "instances.hh"
#include "math.hh"

namespace gdt::blueprints::physics {

class shape {
};

class static_body {
};

class rigid_body {
  public:
    virtual void update_transform(math::mat4* t) = 0;
    virtual void set_gravity(math::vec3 g) = 0;
    virtual void set_actor_params() = 0;
    virtual void stop() = 0;
    virtual void impulse(math::vec3 v) = 0;
    virtual float nearest_collision(math::vec3 from, math::vec3 to) = 0;
    virtual math::vec3 get_pos() = 0;
};

template <typename RIGID_BODY, typename STATIC_BODY, typename SHAPE>
class backend {
  public:
    static_assert(std::is_base_of<gdt::blueprints::physics::rigid_body, RIGID_BODY>::value,
                  "physics body should derive from gdt::blueprints::physics::rigid_body");
    using body = RIGID_BODY;
    static_assert(std::is_base_of<gdt::blueprints::physics::static_body, STATIC_BODY>::value,
                  "physics wall should derive from gdt::blueprints::physics::static_body");
    using wall = STATIC_BODY;
    static_assert(std::is_base_of<gdt::blueprints::physics::shape, SHAPE>::value,
                  "physics shape should derive from gdt::blueprints::physics::shape");
    using shape = SHAPE;
    virtual SHAPE make_box_shape(math::vec3 dimensions) = 0;
    virtual SHAPE make_sphere_shape(float r) = 0;
    virtual RIGID_BODY make_rigid_body(const SHAPE& shape, math::vec3 pos, math::quat r, float m) = 0;
    virtual STATIC_BODY make_wall(math::vec3 plane, math::vec3 pos) = 0;
    virtual math::vec3 lase_normal(math::vec3 from, math::vec3 to) = 0;
    virtual math::vec3 lase_pos(math::vec3 from, math::vec3 to) = 0;
    virtual void update(const core_context& ctx) = 0;
};
};
#endif  // GDT_PHYSICS_HEADER_INCLUDED

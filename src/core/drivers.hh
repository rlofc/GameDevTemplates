#ifndef GDT_DRIVER_HEADER_INCLUDED
#define GDT_DRIVER_HEADER_INCLUDED

#include "camera.hh"
#include "context.hh"
#include "instances.hh"
#include "math.hh"
#include "physics.hh"
#include "backends/blueprints/platform.hh"

namespace gdt {

/**
* drivers are attached to instances and references and act
* as controllers to manipulate their transformations.
*/
class driver {
  public:
    virtual math::mat4 get_transform() const = 0;
    virtual void update() = 0;
    virtual void reuse(){};
};

class drivable {
  public:
    virtual driver *get_driver_ptr() = 0;
};

class pov_driver : public driver {
  public:
    struct initializer {
        math::vec3 pos = { 50, 50, 50};
        math::vec3 tgt = {0, 0, 0};
    };
    static std::function<initializer(int)> look_at(math::vec3 pos, math::vec3 tgt) {
        return [pos, tgt](int j) -> initializer {
            return initializer{pos, tgt};
        };
    }
    static initializer look_at2(int j) {
            return initializer{};
    }
    virtual void dolly(float d) = 0;
    virtual void truck(float d) = 0;
    virtual void follow(float a1, float a2) = 0;
    virtual void pan(float a1) = 0;
    virtual void jump() = 0;
    virtual void stop() = 0;
    virtual void pedestal(float d) = 0;
};

template <typename DRIVABLE>
class hover_driver : public pov_driver {
  public:
    hover_driver(const core_context &context, math::mat4 *transform, pov *pov, pov_driver::initializer i);

    void update() override;
    void dolly(float d) override;
    void truck(float d) override;
    void follow(float a1, float a2) override;
    void pan(float a1) override;
    void jump() override;
    void stop() override;
    void pedestal(float d) override;
    math::mat4 get_transform() const override;

  private:
    math::vec3 *_driven_pos;
    math::vec3 *_driven_tgt;
    math::mat4 *_driven_transform;
    pov *_pov;

    void retransform();
};

template <typename PHYSICS, typename DRIVABLE>
class fps_driver : public pov_driver {
  public:
    fps_driver(physics_context<PHYSICS> const &ctx, math::mat4 *transform, pov *pov, pov_driver::initializer i);
    void set_up(math::vec3 v);
    void set_gravity(math::vec3 v);
    math::vec3 up() const;
    math::vec3 gravity() const;

    virtual void stop() override;
    void retransform();
    void dolly(float d) override;
    void truck(float d) override;
    void jet(float d);
    void jet_dir(math::vec3 dir, float d);
    virtual bool is_on_obj();
    bool is_on_obj_int(float x, float z);
    virtual void jump() override;
    void update() override;
    void follow(float a1, float a2) override;
    void pan(float a1) override;
    void pedestal(float d) override;

    math::mat4 get_transform() const override;

    float get_height() const
    {
        return _height;
    }

  protected:
    typename PHYSICS::body _body;
    math::vec3 *_driven_pos;
    math::vec3 *_driven_tgt;
    math::mat4 *_driven_transform;
    math::vec3 _cam_dir;
    typename PHYSICS::shape _shape;
    pov *_pov;
    math::vec3 _gravity{0, -100, 0};
    math::vec3 _up{0, 1.0f, 0};
    float _height{1};
};

template <typename DRIVABLE>
class dummy_driver {
  public:
    using initializer = math::mat4;
    math::mat4 get_transform() const;
    dummy_driver(const core_context &context, math::mat4 *transform, pov *pov, initializer i);
    void update();

  private:
    math::mat4 *_driven_transform;
    pov *_pov;

    void retransform();
};

template <typename DRIVABLE>
class direct_driver {
  public:
    using initializer = math::mat4;
    math::mat4 get_transform() const;
    direct_driver(const core_context &context, math::mat4 *transform, DRIVABLE *d, math::mat4 im);
    void update();
    void reset();
    void rotate(math::vec3 v);
    void rotate(math::quat q);
    void lookat(math::vec3 pos, math::vec3 tgt, math::vec3 up);
    void scale(math::vec3 v);
    void translate(math::vec3 v);
    math::mat4 *direct()
    {
        return _driven_transform;
    }

  private:
    math::mat4 *_driven_transform;
    math::mat4 _transform;

    math::vec3 _translation;
    math::mat4 _rotation;
    math::vec3 _scale;
};

class wsad_controller {
  private:
    bool _mouse_active{false};

  public:
    template <typename D, typename P>
    void update(const gdt::platform_context<P> &ctx, D *pi, bool stop_on_nothing = true)
    {
        bool press = false;
        if (ctx.get_platform()->is_key_pressed(gdt::key::SPACE)) {
            pi->jump();
            press = true;
        }
        if (ctx.get_platform()->is_key_pressed(gdt::key::W)) {
            pi->dolly(1);
            press = true;
        }
        if (ctx.get_platform()->is_key_pressed(gdt::key::S)) {
            pi->dolly(-1);
            press = true;
        }
        if (ctx.get_platform()->is_key_pressed(gdt::key::D)) {
            pi->truck(1);
            press = true;
        }
        if (ctx.get_platform()->is_key_pressed(gdt::key::A)) {
            pi->truck(-1);
            press = true;
        }
        if (ctx.get_platform()->is_key_pressed(gdt::key::ESC)) {
            _mouse_active = false;
            ctx.get_platform()->release_mouse();
        }
        if (ctx.get_platform()->is_button_pressed()) {
            if (ctx.get_platform()->capture_mouse()) {
                _mouse_active = true;
            }
        }
        if (!press && stop_on_nothing) pi->stop();

        int mouse_x, mouse_y;
        ctx.get_platform()->get_mouse(&mouse_x, &mouse_y);
        if (_mouse_active) {
            float a1 = -(float)mouse_x * 0.001;
            float a2 = (float)mouse_y * 0.001;
            pi->follow(a1, a2);
        }
    }
};

// IMPLEMENTATIONS

template <typename DRIVABLE>
math::mat4 hover_driver<DRIVABLE>::get_transform() const
{
    return *_driven_transform;
}

template <typename DRIVABLE>
hover_driver<DRIVABLE>::hover_driver(const core_context &context, math::mat4 *transform,
                                     pov *pov, pov_driver::initializer i)
    : _driven_transform(transform), _driven_pos(&pov->pos), _driven_tgt(&pov->tgt), _pov(pov)
{

    *_driven_pos = i.pos;
    *_driven_tgt = i.tgt;
    retransform();
}

template <typename DRIVABLE>
void hover_driver<DRIVABLE>::update()
{
    *_driven_transform =
        math::mat4().view_look_at(*_driven_pos, *_driven_tgt, math::vec3(0.0f, 1.0f, 0.0f));
}

template <typename DRIVABLE>
void hover_driver<DRIVABLE>::dolly(float d)
{
    math::vec3 cam_dir = (*_driven_tgt - *_driven_pos).normalize();
    *_driven_pos = *_driven_pos + (cam_dir * d);
    *_driven_tgt = *_driven_tgt + (cam_dir * d);
    this->retransform();
}

template <typename DRIVABLE>
void hover_driver<DRIVABLE>::truck(float d)
{
    math::vec3 cam_dir = (*_driven_tgt - *_driven_pos).normalize();
    math::vec3 side_dir = cam_dir.cross(math::vec3(0, 1, 0)).normalize();
    *_driven_pos = *_driven_pos + (side_dir * d);
    *_driven_tgt = *_driven_tgt + (side_dir * d);
    this->retransform();
}

template <typename DRIVABLE>
void hover_driver<DRIVABLE>::follow(float a1, float a2)
{
    math::vec3 cam_dir = (*_driven_tgt - *_driven_pos).normalize();
    cam_dir.y += -a2;
    math::vec3 side_dir = cam_dir.cross(math::vec3(0, 1, 0)).normalize();
    cam_dir = cam_dir + (side_dir * -a1);
    cam_dir = cam_dir.normalize();
    *_driven_tgt = *_driven_pos + cam_dir;
    this->retransform();
}

template <typename DRIVABLE>
void hover_driver<DRIVABLE>::pan(float a1)
{
    math::vec3 cam_dir = (*_driven_tgt - *_driven_pos).normalize();
    math::vec3 side_dir = cam_dir.cross(math::vec3(0, 1, 0)).normalize();
    cam_dir = cam_dir + (side_dir * -a1);
    cam_dir = cam_dir.normalize();
    *_driven_tgt = (*_driven_pos + cam_dir);
    this->retransform();
}

template <typename DRIVABLE>
void hover_driver<DRIVABLE>::jump()
{
}

template <typename DRIVABLE>
void hover_driver<DRIVABLE>::stop()
{
}

template <typename DRIVABLE>
void hover_driver<DRIVABLE>::pedestal(float d)
{
    math::vec3 cam_dir = (*_driven_tgt - *_driven_pos).normalize();
    math::vec3 side_dir = cam_dir.cross(math::vec3(0, 0, 1)).normalize();
    *_driven_pos = *_driven_pos + (side_dir * d);
    *_driven_tgt = *_driven_tgt + (side_dir * d);
    this->retransform();
}

template <typename DRIVABLE>
void hover_driver<DRIVABLE>::retransform()
{
    *_driven_transform =
        math::mat4().view_look_at(*_driven_pos, *_driven_tgt, math::vec3(0.0f, 1.0f, 0.0f));
}

template <typename PHYSICS, typename DRIVABLE>
fps_driver<PHYSICS, DRIVABLE>::fps_driver(const physics_context<PHYSICS> &ctx,
                                          math::mat4 *transform, pov *pov, pov_driver::initializer i)
    : _driven_transform(transform),
      _driven_pos(&pov->pos),
      _driven_tgt(&pov->tgt),
      _pov(pov),
      _shape(ctx.physics->make_sphere_shape(3))
{
    //math::vec3 pos = math::vec3(transform->wx, transform->wy,
    //                            transform->wz);  // = get position from transform

    *_driven_pos = i.pos;
    *_driven_tgt = i.tgt;
    //retransform();
    //*_driven_pos = pos;
    _body = ctx.physics->make_rigid_body(_shape, *_driven_pos, math::quat(0, 0, 0, 1), 1);
    _body.set_gravity(_gravity);
    _body.set_actor_params();
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::set_up(math::vec3 v)
{
    _up = v;
}

template <typename PHYSICS, typename DRIVABLE>
math::vec3 fps_driver<PHYSICS, DRIVABLE>::up() const
{
    return _up;
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::set_gravity(math::vec3 v)
{
    _gravity = v;
    _body.set_gravity(_gravity);
}
template <typename PHYSICS, typename DRIVABLE>
math::vec3 fps_driver<PHYSICS, DRIVABLE>::gravity() const
{
    return _gravity;
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::stop()
{
    if (!is_on_obj()) return;
    _body.stop();
}

template <typename PHYSICS, typename DRIVABLE>
math::mat4 fps_driver<PHYSICS, DRIVABLE>::get_transform() const
{
    return *_driven_transform;
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::retransform()
{
    math::vec3 p = _pov->pos;
    math::vec3 t = _pov->tgt;
    p.y += _height;
    t.y += _height;
    *_driven_transform = math::mat4().view_look_at(p, t, _up);
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::dolly(float d)
{
    if (!is_on_obj()) return;
    math::vec3 cam_dir = (*_driven_tgt - *_driven_pos).normalize();
    cam_dir.y = 0;
    math::vec3 force = cam_dir * (d * 3); 
    _body.impulse(force);
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::truck(float d)
{
    if (!is_on_obj()) return;
    math::vec3 cam_dir = (*_driven_tgt - *_driven_pos).normalize();
    math::vec3 side_dir = cam_dir.cross(_up).normalize();
    side_dir.y = 0;
    math::vec3 force = side_dir * (d * 3);
    _body.impulse(force);
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::jet(float d)
{
    math::vec3 cam_dir = (*_driven_tgt - *_driven_pos).normalize();
    math::vec3 force = cam_dir * (d * 3);
    _body.impulse(force);
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::jet_dir(math::vec3 dir, float d)
{
    math::vec3 force = dir * (d * 3);
    _body.impulse(force);
}

template <typename PHYSICS, typename DRIVABLE>
bool fps_driver<PHYSICS, DRIVABLE>::is_on_obj()
{
    return is_on_obj_int(0, 0) || is_on_obj_int(1, 1) || is_on_obj_int(-1, 1) ||
           is_on_obj_int(1, -1) || is_on_obj_int(-1, -1);
}

template <typename PHYSICS, typename DRIVABLE>
bool fps_driver<PHYSICS, DRIVABLE>::is_on_obj_int(float x, float z)
{
    math::vec3 g = _gravity.normalize() * -1.0f;
    math::vec3 from = *_driven_pos + g * 3;
    math::vec3 to = *_driven_pos;
    to += g * -10000.0;
    return _body.nearest_collision(from, to) < 3;
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::jump()
{
    if (!is_on_obj()) return;
    math::vec3 force = (_gravity.normalize() * -1) * 50;
    _body.impulse(force);
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::update()
{
    math::vec3 cam_dir = (*_driven_tgt - *_driven_pos).normalize();
    math::vec3 new_pos = _body.get_pos();
    math::vec3 delta = *_driven_pos - new_pos;
    *_driven_pos = new_pos;
    *_driven_tgt = (new_pos + cam_dir);
    this->retransform();
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::follow(float a1, float a2)
{
    math::vec3 cam_dir = (*_driven_tgt - *_driven_pos).normalize();
    cam_dir += _up * -a2;
    math::vec3 side_dir = cam_dir.cross(_up).normalize();
    cam_dir = cam_dir + (side_dir * -a1);
    cam_dir = cam_dir.normalize();
    *_driven_tgt = *_driven_pos + cam_dir;
    this->retransform();
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::pan(float a1)
{
}

template <typename PHYSICS, typename DRIVABLE>
void fps_driver<PHYSICS, DRIVABLE>::pedestal(float d)
{
}

template <typename DRIVABLE>
math::mat4 direct_driver<DRIVABLE>::get_transform() const
{
    return *_driven_transform;
}

template <typename DRIVABLE>
direct_driver<DRIVABLE>::direct_driver(const core_context &context, math::mat4 *transform,
                                       DRIVABLE *d, math::mat4 im)
    : _driven_transform(transform)
{
    reset();
}

template <typename DRIVABLE>
void direct_driver<DRIVABLE>::update()
{
    _transform = math::mat4::translation(_translation);
    _transform *= _rotation;
    _transform *= math::mat4::scale(_scale);
    *_driven_transform = _transform.transpose();
}

template <typename DRIVABLE>
void direct_driver<DRIVABLE>::reset()
{
    _translation = math::vec3();
    _rotation = math::mat4::rotation_eular({0, 0, 0});
    _scale = math::vec3({1, 1, 1});
    update();
}

template <typename DRIVABLE>
void direct_driver<DRIVABLE>::rotate(math::vec3 v)
{
    _rotation = math::mat4::rotation_eular(v);
    update();
}

template <typename DRIVABLE>
void direct_driver<DRIVABLE>::rotate(math::quat q)
{
    _rotation = math::mat4::rotation_quat(q);
    update();
}
template <typename DRIVABLE>
void direct_driver<DRIVABLE>::lookat(math::vec3 pos, math::vec3 tgt, math::vec3 up)
{
    _rotation = math::mat4().view_look_at(pos, tgt, up);
    update();
}

template <typename DRIVABLE>
void direct_driver<DRIVABLE>::scale(math::vec3 v)
{
    _scale = v;
    update();
}

template <typename DRIVABLE>
void direct_driver<DRIVABLE>::translate(math::vec3 v)
{
    _translation = v;
    update();
}

template <typename DRIVABLE>
void dummy_driver<DRIVABLE>::retransform()
{
}

template <typename DRIVABLE>
math::mat4 dummy_driver<DRIVABLE>::get_transform() const
{
    return *_driven_transform;
}

template <typename DRIVABLE>
dummy_driver<DRIVABLE>::dummy_driver(const core_context &context, math::mat4 *transform,
                                     pov *pov, initializer i)
    : _driven_transform(transform), _pov(pov)
{
}

template <typename DRIVABLE>
void dummy_driver<DRIVABLE>::update()
{
    *_driven_transform = math::mat4::id();
}
};
#endif  // GDT_DRIVER_HEADER_INCLUDED

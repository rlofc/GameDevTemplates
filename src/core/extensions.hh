#ifndef GDT_INSTANCES_HEADER_INCLUDED
#define GDT_INSTANCES_HEADER_INCLUDED

#include <memory>
#include <vector>

#include "context.hh"
#include "math.hh"
#include "traits.hh"
#include "utils/checks.hh"

namespace gdt {

template <typename DRIVABLE>
class no_driver {
  public:
    no_driver(const core_context &ctx, math::mat4 *t, DRIVABLE *d)
    {
        UNUSED(ctx);
        UNUSED(t);
        UNUSED(d);
    }
};

static math::mat4 center_pos(int j)
{
    return math::mat4::id().transpose();
}

namespace pos {

static math::mat4 origin(int j)
{
    return math::mat4::id().transpose();
}

static std::function<math::mat4(int)> look_at(math::vec3 pos, math::vec3 tgt)
{
    return [pos, tgt](int) -> math::mat4 {
        return math::mat4::view_look_at(pos, tgt, {0, 1, 0});
    };
}

}

/**
 * represents one or more concrete entities - that is - entities
 * with a usable transformation matrix.
 */
template <int C = 1>
class transforms : public is_transformable<transforms<C>> {
  protected:
    math::mat4 *_transforms;

  public:
    template <typename CONTEXT>
    transforms(const CONTEXT &ctx, std::function<math::mat4(int)> pos_callback = [](int i) {
        UNUSED(i);
        return math::mat4().transpose();
    })
    {
        ctx.graphics->create_instance_buffer(&_transforms, C);
        for (int j = 0; j < C; j++) {
            _transforms[j] = pos_callback(j);
        }
    }
    template <typename CONTEXT>
    void reuse(const CONTEXT &ctx, std::function<math::mat4(int)> pos_callback = [](int i) {
        UNUSED(i);
        return math::mat4().transpose();
    })
    {
        for (int j = 0; j < C; j++) {
            _transforms[j] = pos_callback(j);
        }
    }

    unsigned int size() const
    {
        return C;
    }

    math::mat4 *begin()
    {
        return _transforms;
    }

    math::mat4 *end()
    {
        return &_transforms[C];
    }

    operator std::array<math::mat4, C>() const
    {
        return *_transforms;
    }

    const math::mat4 *get_transforms() const
    {
        return _transforms;
    }

    math::mat4 *get_transform_ptr(int i)
    {
        return &_transforms[i];
    }

    void set_transforms(std::function<math::mat4(int index)> f, int begin = 0, int end = C)
    {
        for (int j = begin; j < end; j++) {
            _transforms[j] = f(j);
        }
    }

    template <typename CONTEXT>
    void exp_update_instance_buffer(const CONTEXT &ctx)
    {
        ctx.graphics->update_instance_buffer(_transforms, C);
    }

    template <typename CONTEXT>
    void update(const CONTEXT &ctx)
    {
        exp_update_instance_buffer(ctx);
    }
};

template <typename T, int C = 1>
class references : public transforms<C>,
                   public ptr_container<T>,
                   public may_have_drawable<T, references<T, C>>,
                   public may_have_drivable<T, references<T, C>>,
                   public may_have_animatable<T, references<T, C>>,
                   public may_have_collidable<T, references<T, C>> {
  public:
    using etype = T;
    template <typename CONTEXT, typename... ARG>
    references(const CONTEXT &ctx, std::function<math::mat4(int)> pos_callback =
                                       [](int j) {
                                           UNUSED(j);
                                           return math::mat4().transpose();
                                       },
               T *e = nullptr)
        : ptr_container<T>(e), transforms<C>(ctx, pos_callback)
    {
    }
    int size() const
    {
        return C;
    }
};

/**
 * gdt::instances is a special template container for gdt::is_entity types such
 * as gdt::asset and gdt::camera. By default, gdt::is_entity types doesn't have
 * any 3D world transformation data. gdt::instances augment entities with 3D
 * transformation and can also support graphical instancing of entities during
 * rendering.
 *
 * An instances object will manage a set of 3D transformation gdt::math::mat4
 * instances and a single object of your entity type.
 *
 * You will almost always have to use gdt::instances or gdt::instance (and their
 * related siblings, gdt::references and gdt::reference) to store your asset type:
 *
 *     gdt::instances<zombie, 1000> _my_army_of_zombies;
 *     gdt::instance<hero> _my_single_hero;
 *
 * When constructing an instances object, you will also get the opportunity to
 * construct your entity object by passing its constructor paramters from the
 * third argument onwards. See gdt::instances::instances for an example.
 *
 */
template <typename T, int C = 1>
class instances : public transforms<C>,
                  public container<T>,
                  public may_have_drawable<T, instances<T, C>>,
                  public may_have_drivable<T, instances<T, C>>,
                  public may_have_animatable<T, instances<T, C>>,
                  public may_have_collidable<T, instances<T, C>> {
  public:
    /**
     * Constructing an instances object involves setting the instances
     * preliminary transformation and then constructing the contained entity
     * object:
     *
     *     class my_scene : public my_game::scene {
     *       private:
     *         gdt::instance<hero> _my_hero;
     *    
     *       public:
     *         my_scene(...) : _my_hero(ctx, gdt::pos::origin, "my hero name")
     *         {
     *         }
     *     };
     *
     */
    template <typename CONTEXT, typename... ARG>
    instances(const CONTEXT &ctx, std::function<math::mat4(int)> pos_callback =
                                      [](int j) {
                                          UNUSED(j);
                                          return math::mat4().transpose();
                                      },
              const ARG &... a)
        : container<T>(ctx, a...), transforms<C>(ctx, pos_callback)
    {
    }
    int size() const
    {
        return C;
    }
};

template <typename T>
using instance = instances<T, 1>;

template <typename T>
using reference = references<T, 1>;

template <typename T, template <typename> typename DRIVER = no_driver>
class driven : public container<T>,
               public may_have_drawable<T, driven<T, DRIVER>>,
               public may_have_animatable<T, driven<T, DRIVER>>,
               public may_have_drivable<T, driven<T, DRIVER>>,
               public may_have_collidable<T, driven<T, DRIVER>>,
               public may_have_transformable<T, driven<T, DRIVER>> {
  private:
    std::vector<std::unique_ptr<DRIVER<typename T::t_drivable>>> _drivers;

  public:
    template <typename CONTEXT, typename... ARG>
    driven(const CONTEXT &ctx,
           const std::function<typename DRIVER<typename T::t_drivable>::initializer(int)>
               &driver_callback = [](int i) ->
           typename DRIVER<typename T::t_drivable>::initializer {
               UNUSED(i);
               return typename DRIVER<typename T::t_drivable>::initializer();
           },
           const ARG &... a)
        : container<T>(ctx, pos::origin, a...)
    {
        for (int j = 0; j < this->content()->size(); j++) {
            _drivers.push_back(std::make_unique<DRIVER<typename T::t_drivable>>(
                ctx, this->get_transformable_ptr()->get_transform_ptr(j),
                this->get_drivable_ptr(), driver_callback(j)));
        }
    }

    template <typename CONTEXT>
    void reuse(const CONTEXT &ctx, std::function<math::mat4(int)> pos_callback = [](int i) {
        UNUSED(i);
        return math::mat4().transpose();
    })
    {
        this->get_transformable_ptr()->reuse(ctx, pos_callback);
        for (int j = 0; j < this->content()->size(); j++) {
            _drivers[j]->reuse(ctx, this->get_transformable_ptr()->get_transform_ptr(j));
        }
    }

    const DRIVER<typename T::etype> &get_driver(int index = 0) const
    {
        return *_drivers[index].get();
    }

    DRIVER<typename T::etype> *get_driver_ptr(int index = 0)
    {
        return _drivers[index].get();
    }

    template <typename CONTEXT>
    void update(const CONTEXT &ctx)
    {
        for (int j = 0; j < this->content()->size(); j++) {
            _drivers[j]->update();
        }
        this->content()->update(ctx);
    }
    const T &drivable() const
    {
        return *this->ccontent();
    }

    T *drivable_ptr()
    {
        return this->content();
    }

    operator T()
    {
        return *this->content();
    }
};
}

#endif  // GDT_INSTANCES_HEADER_INCLUDED

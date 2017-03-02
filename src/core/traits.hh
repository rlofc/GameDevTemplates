#ifndef GDT_SRC_CORE_TRAITS_HH_INCLUDED
#define GDT_SRC_CORE_TRAITS_HH_INCLUDED

#include <type_traits>
namespace gdt {

// Thanks to David Rodríguez-dribeas for http://stackoverflow.com/a/11814074
template <bool Condition, typename Then, typename Else = void>
struct if_ {
    typedef Then type;
};
template <typename Then, typename Else>
struct if_<false, Then, Else> {
    typedef Else type;
};

struct no_impl {
};

/**
 * A generic container allowing the structuring of complex types by using composition
 * rather than inheritance.
 * A user defined container type can use additional GDT type traits
 * to provide direct access to objects implementing a certain functionality,
 * using compile time polymorphism. See gdt::instances and gdt::driven as relevant
 * examples.
 */
template <typename CONTENT>
struct container {
    using etype = typename CONTENT::etype;
    CONTENT _content;
    CONTENT *content()
    {
        return &_content;
    }
    const CONTENT *ccontent() const
    {
        return &_content;
    }
    template <typename CONTEXT, typename... ZZ>
    container(const CONTEXT &ctx, ZZ &... rest) : _content(ctx, rest...)
    {
    }

    const etype &entity() const
    {
        return static_cast<const etype &>(_content.entity());
    };

    etype *entity_ptr()
    {
        return static_cast<etype *>(_content.entity_ptr());
    };

    operator etype() const
    {
        return *static_cast<etype *>(_content.entity_ptr());
    }
};

/**
 * Same purpose as gdt:container only this time the container holds a pointer
 * rather then owning the contained object.
 */
template <typename CONTENT>
struct ptr_container {
    using etype = typename CONTENT::etype;
    CONTENT *_content;
    CONTENT *content()
    {
        return _content;
    }
    const CONTENT *ccontent() const
    {
        return _content;
    }
    ptr_container(CONTENT *content) : _content{content}
    {
    }
    const etype &entity() const
    {
        return static_cast<const etype &>(_content->entity());
    };

    etype *entity_ptr()
    {
        return static_cast<etype *>(_content->entity_ptr());
    };

    operator etype() const
    {
        return *static_cast<etype *>(_content->entity_ptr());
    }
};

/**
 * Types subclassing from gdt::is_entity are leaf objects within a container
 * chain. If you'd like to access objects of your type directly, no matter how deep
 * they are within the containing chain, subclass from is_entity and you'll be able
 * to use any container in the chain to pull out your entity object.
 */
template <typename ENTITY>
struct is_entity {
    using etype = ENTITY;
    const etype &entity() const
    {
        return static_cast<const etype &>(*this);
    };

    etype *entity_ptr()
    {
        return static_cast<etype *>(this);
    };

    operator etype() const
    {
        return *static_cast<etype *>(this);
    }
};

/**
 * Used by drawable types to allow pipelines a direct access to their draw
 * methods.
 */
template <typename DRAWABLE>
struct is_drawable {
    using t_drawable = DRAWABLE;
    const t_drawable &get_drawable() const
    {
        return static_cast<const DRAWABLE &>(*this);
    }
    t_drawable *get_drawable_ptr()
    {
        return static_cast<DRAWABLE *>(this);
    }
};

/**
 * SFINAE default container base type used when its content is not actually
 * a drawable.
 */
template <typename DRAWABLE, typename ME, typename Enable = void>
struct may_have_drawable {
};

/**
 * SFINAE container base type used only if the container's content is
 * of type gdt::is_drawable.
 */
template <typename DRAWABLE, typename ME>
struct may_have_drawable<DRAWABLE, ME,
                         typename if_<false, typename DRAWABLE::t_drawable>::type> {
    using t_drawable = typename DRAWABLE::t_drawable;
    const t_drawable &get_drawable() const
    {
        return static_cast<const ME *>(this)->ccontent()->get_drawable();
    }
    t_drawable *get_drawable_ptr()
    {
        return static_cast<ME *>(this)->content()->get_drawable_ptr();
    }
};

/**
 * Subclass this for animatable (skeleton animation) types to allow pipelines a direct access
 * to their bind method.
 */
template <typename ANIMATABLE>
struct is_animatable {
    using t_animatable = ANIMATABLE;
    const t_animatable &get_animatable() const
    {
        return static_cast<const ANIMATABLE &>(*this);
    }
    t_animatable *get_animatable_ptr()
    {
        return static_cast<ANIMATABLE *>(this);
    }
};

/**
 * SFINAE default container base type used when its content is not actually
 * an animatable.
 */
template <typename ANIMATABLE, typename ME, typename Enable = void>
struct may_have_animatable {
};

/**
 * SFINAE container base type used only if the container's content is
 * of type gdt::is_animatable.
 */
template <typename ANIMATABLE, typename ME>
struct may_have_animatable<ANIMATABLE, ME,
                           typename if_<false, typename ANIMATABLE::t_animatable>::type> {
    using t_animatable = typename ANIMATABLE::t_animatable;

    const t_animatable &get_animatable() const
    {
        return static_cast<const ME *>(this)->ccontent()->get_animatable();
    }
    t_animatable *get_animatable_ptr()
    {
        return static_cast<ME *>(this)->content()->get_animatable_ptr();
    }
};

/**
 * Subclass this for type that can be driven (as in, gdt::driven is
 * a valid container for them).
 */
template <typename DRIVABLE>
struct is_drivable {
    using t_drivable = DRIVABLE;
    const t_drivable *get_drivable() const
    {
        return static_cast<const DRIVABLE *>(this);
    }
    t_drivable *get_drivable_ptr()
    {
        return static_cast<DRIVABLE *>(this);
    }
};

/**
 * SFINAE default container base type used when its content is not actually
 * a drivable.
 */
template <typename DRIVABLE, typename ME, typename Enable = void>
struct may_have_drivable {
};

/**
 * SFINAE container base type used only if the container's content is
 * of type gdt::is_drivable.
 */
template <typename DRIVABLE, typename ME>
struct may_have_drivable<DRIVABLE, ME,
                         typename if_<false, typename DRIVABLE::t_drivable>::type> {
    using t_drivable = typename DRIVABLE::t_drivable;
    const t_drivable *get_drivable() const
    {
        return static_cast<const ME *>(this)->ccontent()->get_drivable();
    }
    t_drivable *get_drivable_ptr()
    {
        return static_cast<ME *>(this)->content()->get_drivable_ptr();
    }
};

/**
 * Subclass this if your type is a collidable. This will allow the physics
 * backend to access your type's get_shape method.
 */
template <typename COLLIDABLE>
struct is_collidable {
    using t_collidable = COLLIDABLE;
    const t_collidable &get_collidable() const
    {
        return static_cast<const COLLIDABLE &>(*this);
    }
};

/**
 * SFINAE default container base type used when its content is not actually
 * a collidable.
 */
template <typename COLLIDABLE, typename ME, typename Enable = void>
struct may_have_collidable {
};

/**
 * SFINAE container base type used only if the container's content is
 * of type gdt::is_collidable.
 */
template <typename COLLIDABLE, typename ME>
struct may_have_collidable<COLLIDABLE, ME,
                           typename if_<false, typename COLLIDABLE::t_collidable>::type> {
    using t_collidable = typename COLLIDABLE::t_collidable;
    const t_collidable &get_collidable() const
    {
        return static_cast<const ME *>(this)->ccontent()->get_collidable();
    }
};

/**
 * Subclass this if your type is a transformable type, meaning, it contains
 * one or more transformation matrices, provide access through a `get_transforms`
 * method.
 */
template <typename ME>
struct is_transformable {
    using t_transformable = ME;
    const t_transformable &get_transformable() const
    {
        return static_cast<const ME &>(*this);
    }
    t_transformable *get_transformable_ptr()
    {
        return static_cast<ME *>(this);
    }
};

/**
 * SFINAE default container base type used when its content is not actually
 * a transformable.
 */
template <typename TRANSFORMABLE, typename ME, typename Enable = void>
struct may_have_transformable {
};

/**
 * SFINAE container base type used only if the container's content is
 * of type gdt::is_transformable.
 */
template <typename TRANSFORMABLE, typename ME>
struct may_have_transformable<
    TRANSFORMABLE, ME, typename if_<false, typename TRANSFORMABLE::t_transformable>::type> {
    using t_transformable = typename TRANSFORMABLE::t_transformable;
    const t_transformable &get_transformable() const
    {
        return static_cast<const ME *>(this)->ccontent()->get_transformable();
    }
    t_transformable *get_transformable_ptr()
    {
        return static_cast<ME *>(this)->content()->get_transformable_ptr();
    }
};

template <typename ACTUAL>
class asset : public is_entity<ACTUAL>,
              public is_drivable<ACTUAL>
{
};

}
#endif /* GDT_SRC_CORE_TRAITS_HH_INCLUDED */

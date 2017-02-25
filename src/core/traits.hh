#ifndef NODES_HH_OJJUMXMW
#define NODES_HH_OJJUMXMW

#include <type_traits>
namespace gdt {
    //
// Thanks to David Rodríguez-dribeas for http://stackoverflow.com/a/11814074
template <bool Condition, typename Then, typename Else = void>
struct if_ {
   typedef Then type;
};
template <typename Then, typename Else>
struct if_<false, Then, Else > {
   typedef Else type;
};

struct no_impl {};

template <typename CONTENT>
struct container
{
    using etype = typename CONTENT::etype;
    CONTENT _content;
    CONTENT * content() { return &_content; }
    const CONTENT * ccontent() const { return &_content; }
    template <typename CONTEXT, typename... ZZ>
    container(const CONTEXT & ctx, ZZ&... rest) : _content(ctx, rest...)
    {
    }

    const etype &entity() const
    {
        return static_cast<const etype&>(_content.entity());
    };

    etype *entity_ptr()
    {
        return static_cast<etype*>(_content.entity_ptr());
    };

    operator etype() const
    {
        return *static_cast<etype*>(_content.entity_ptr());
    }
};

template <typename CONTENT>
struct ptr_container
{
    using etype = typename CONTENT::etype;
    CONTENT * _content;
    CONTENT * content() { return _content; }
    const CONTENT * ccontent() const { return _content; }
    ptr_container(CONTENT * content) : _content{content} {}
    const etype &entity() const
    {
        return static_cast<const etype&>(_content->entity());
    };

    etype *entity_ptr()
    {
        return static_cast<etype*>(_content->entity_ptr());
    };

    operator etype() const
    {
        return *static_cast<etype*>(_content->entity_ptr());
    }
};

template <typename ENTITY>
struct is_entity {
    using etype = ENTITY;
    const etype & entity() const
    {
        return static_cast<const etype&>(*this);
    };

    etype *entity_ptr()
    {
        return static_cast<etype*>(this);
    };

    operator etype() const
    {
        return *static_cast<etype*>(this);
    }

};

template <typename DRAWABLE>
struct is_drawable {
    using t_drawable = DRAWABLE;
    const t_drawable & get_drawable() const{
        return static_cast<const DRAWABLE&>(*this);
    }
    t_drawable * get_drawable_ptr() {
        return static_cast<DRAWABLE*>(this);
    }
};

template <typename DRAWABLE, typename ME, typename Enable = void> 
struct may_have_drawable
{};

template <typename DRAWABLE, typename ME>
struct may_have_drawable<DRAWABLE, ME, typename if_<false,typename DRAWABLE::t_drawable>::type> {
    using t_drawable = typename DRAWABLE::t_drawable;
    const t_drawable & get_drawable() const {
        return static_cast<const ME*>(this)->ccontent()->get_drawable();
    }
    t_drawable * get_drawable_ptr() {
        return static_cast<ME*>(this)->content()->get_drawable_ptr();
    }
};

template <typename T>
struct is_non_drawable {
    using t_drawable = no_impl;
};

template <typename ANIMATABLE>
struct is_animatable {
    using t_animatable = ANIMATABLE;
    const t_animatable & get_animatable() const{
        return static_cast<const ANIMATABLE&>(*this);
    }
    t_animatable * get_animatable_ptr() {
        return static_cast<ANIMATABLE*>(this);
    }
};


template <typename ANIMATABLE, typename ME, typename Enable = void> 
struct may_have_animatable
{};

template <typename ANIMATABLE, typename ME>
struct may_have_animatable<ANIMATABLE, ME, typename if_<false,typename ANIMATABLE::t_animatable>::type>
{
    using t_animatable = typename ANIMATABLE::t_animatable;

    const t_animatable & get_animatable() const {
        return static_cast<const ME*>(this)->ccontent()->get_animatable();
    }
    t_animatable * get_animatable_ptr() {
        return static_cast<ME*>(this)->content()->get_animatable_ptr();
    }
};

template <typename T>
struct is_non_animatable {
    using t_animatable = no_impl;
};

template <typename DRIVABLE>
struct is_drivable {
    using t_drivable = DRIVABLE;
    const t_drivable * get_drivable() const{
        return static_cast<const DRIVABLE*>(this);
    }
    t_drivable * get_drivable_ptr() {
        return static_cast<DRIVABLE*>(this);
    }
};

template <typename DRIVABLE, typename ME>
struct may_have_drivable {
    using t_drivable = typename DRIVABLE::t_drivable;
    const t_drivable * get_drivable() const {
        return static_cast<const ME*>(this)->ccontent()->get_drivable();
    }
    t_drivable * get_drivable_ptr() {
        return static_cast<ME*>(this)->content()->get_drivable_ptr();
    }
};

template <typename COLLIDABLE>
struct is_collidable {
    using t_collidable = COLLIDABLE;
    const t_collidable & get_collidable() const {
        return static_cast<const COLLIDABLE&>(*this);
    }
};

template <typename T, typename Z, typename Enable = void> 
struct may_have_collidable {};

template <typename COLLIDABLE, typename ME>
struct may_have_collidable<COLLIDABLE, ME, typename std::enable_if<std::is_base_of<is_collidable<COLLIDABLE>, COLLIDABLE>::value>>
{
    using t_collidable = typename COLLIDABLE::t_collidable;
    const t_collidable & get_collidable() const {
        return static_cast<const ME*>(this)->ccontent()->get_collidable();
    }
};


struct is_non_collidable {
    using t_collidable = no_impl;
};

template <typename ME>
struct is_transformable {
    using t_transformable = ME;
    const t_transformable & get_transformable() const {
        return static_cast<const ME&>(*this);
    }
    t_transformable * get_transformable_ptr() {
        return static_cast<ME*>(this);
    }
};

template <typename TRANSFORMABLE, typename ME>
struct may_have_transformable {
    using t_transformable = typename TRANSFORMABLE::t_transformable;
    const t_transformable & get_transformable() const {
        return static_cast<const ME*>(this)->ccontent()->get_transformable();
    }
    t_transformable * get_transformable_ptr() {
        return static_cast<ME*>(this)->content()->get_transformable_ptr();
    }
};
}
#endif /* end of include guard: NODES_HH_OJJUMXMW */

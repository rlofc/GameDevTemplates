#ifndef TEXT_HH_LXXM6T2K
#define TEXT_HH_LXXM6T2K

#include "backends/blueprints/graphics.hh"

#include "context.hh"
#include "font.hh"
#include "traits.hh"

namespace gdt {

template <typename GRAPHICS>
class text : public GRAPHICS::text,
    public is_entity<text<GRAPHICS>>,
    public is_drivable<text<GRAPHICS>>,
    public is_drawable<text<GRAPHICS>>,
    public is_non_animatable<text<GRAPHICS>>,
    public is_non_collidable
{
    public:
        text(const graphics_context<GRAPHICS>& ctx, const font<GRAPHICS>& f, const char* text) :
        GRAPHICS::text(ctx, f, text){
        }
        virtual ~text() {}

};
}

#endif /* end of include guard: TEXT_HH_LXXM6T2K */

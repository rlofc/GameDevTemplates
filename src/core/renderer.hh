#ifndef GDT_RENDERER_HEADER_INCLUDED
#define GDT_RENDERER_HEADER_INCLUDED

#include "asset.hh"
#include "light.hh"
#include "shaders.hh"

namespace gdt {

/**
* renderer is a CRTP template that dictates the only method
* renderers should implement - record - which takes a set of
* render pass callbacks.
*/
template <typename GRAPHICS, typename RENDERER>
class renderer {
  public:
    template <typename... PIPELINE>
    void record(const graphics_context<GRAPHICS>& ctx,
                std::function<void(const PIPELINE &...)> cmds) const
    {
        static_cast<const RENDERER*>(this)->record(ctx, cmds); 
    }
};


/**
* render_pass allows a set of pipelines to be used on a selected
* frame buffer, either the screen or an offscreen buffer.
*     
*     gdt::render_pass test;
*
*
* blah
*/
template <typename GRAPHICS>
class render_pass {
  private:
    const graphics_context<GRAPHICS>& _ctx;
  public:
    render_pass(const graphics_context<GRAPHICS>& ctx) : _ctx(ctx)
    {
    }

    /**
    * Target any pipeline operations and drawings to the specified frame
    * buffer.
    *
    * @param fb test
    */
    const render_pass & target(const typename GRAPHICS::frame_buffer & fb) const {
        fb.bind();
        return *this;
    }

    /**
    * Clear the active target buffer.
    */
    const render_pass & clear(math::vec4 color = {0.9, 0.9, 0.9, 0.0}) const {
        GRAPHICS::clear.apply(color);
        return *this;
    }

    /**
    * Chaining any following calls to the selected pipeline.
    * This is useful for simple filter pipelines.
    */
    template <typename T>
    const T & filter(const T & pipeline) const {
        pipeline.use(_ctx);
        return pipeline;
    }

    template <typename... PIPELINE>
    void process_cmds(std::function<void(const PIPELINE &...)> cmds, const PIPELINE &... s) const
    {
        _ctx.graphics->process_cmds(cmds, s...);
    }
};

}
#endif  // GDT_RENDERER_HEADER_INCLUDED

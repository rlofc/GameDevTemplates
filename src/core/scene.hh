#ifndef SCENE_HH_PFPKEXHF
#define SCENE_HH_PFPKEXHF

#include "backends/blueprints/platform.hh"
#include "core/screen.hh"

namespace gdt {

/**
 * Scenes are how you divide your game into logical and memory managable parts.
 * You will usually subclass this template through your specified application type:
 *
 *     using my_game = gdt::application<...>;
 *
 *     class my_scene : public my_game::scene {
 *         ...
 *     };
 *
 * @tparam CONTEXT the user-specified context type to use.
 *
 */
template <typename CONTEXT>
class scene {
  public:
    scene() {}
    virtual ~scene() {}

    /**
     * You must provide your own update implementation. Put all your per-frame
     * scene update logic here and make sure you invoke a call to
     * gdt::scene::render. Don't directly draw stuff in this method.
     *
     *     class my_scene : public my_app::scene {
     *       public:
     *         ...
     *         void update(const my_app::context & ctx) override {
     *             // Update your objects
     *             ...
     *
     *             // Once you're done, call render to do the actual drawing
     *             render(ctx);
     *         }
     *     };
     */
    virtual void update(const CONTEXT& ctx) = 0;

    /**
     * You must provide your own rendering implementation. Make sure you draw
     * only in this method, as this method could also be called by the engine.
     *
     * You will usually want to reuse your render pass setup in a 
     * separate renderer class, but you can also directly
     * code everything in your render method implementation:
     *
     *
     *     class my_scene : public my_app::scene {
     *       public:
     *         ...
     *         void render(const my_app::context & ctx) override {
     *             my_app::render_pass(ctx)
     *                 .target(my_app::graphics::screen_buffer)
     *                 .clear();
     *             my_app::forward_pipeline()
     *                 .use(ctx)
     *                     .set_material(_material)
     *                     .set_pov(_camera)
     *                     .draw(_zombie);
     *         }
     *     };
     *
     */
    virtual void render(const CONTEXT& ctx) = 0;

    /**
     * If you have custom imgui code, implement this method and place it here.
     */
    virtual void imgui(const CONTEXT& ctx){};

    virtual void on_screen_resize(const CONTEXT& ctx)
    {
        render(ctx);
    }
};

/**
 * This empty scene can be used as a placeholder for scaffolding
 * an app.
 *
 * TODO: make it more interesting than just a blank screen..
 */
template <typename CONTEXT>
class empty_scene : public scene<CONTEXT> {
    public:
    empty_scene(const CONTEXT& ctx, gdt::screen* screen) {
    }
    virtual ~empty_scene() {}
    void update(const CONTEXT& ctx) override
    {
        if (ctx.get_platform()->is_key_pressed(key::ESC)) {
            ctx.quit();
        }
    }
    void render(const CONTEXT& ctx) override
    {
    }
};

}
#endif /* end of include guard: SCENE_HH_PFPKEXHF */

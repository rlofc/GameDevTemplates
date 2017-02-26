#ifndef GDT_APPLICATION_HEADER_INCLUDED
#define GDT_APPLICATION_HEADER_INCLUDED

#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

#include "backends/blueprints/audio.hh"
#include "backends/blueprints/platform.hh"
#include "backends/blueprints/physics.hh"

#include "core/camera.hh"
#include "core/asset.hh"
#include "core/drivers.hh"
#include "core/font.hh"
#include "core/renderer.hh"
#include "core/physics.hh"
#include "core/shaders.hh"
#include "core/scene.hh"
#include "core/text.hh"

#include "imgui/imgui_gdt.hh"

namespace gdt {

struct no_physics {
    using body = int;
    using wall = int;
    using shape = int;
};

using no_networking = void*;

/**
 * The entry point to your game is a single gdt::application based object.
 * GDT lets you specify your game's graphics, platform, physics and other services
 * through this template, for example:
 *
 *     using my_game = gdt::application<
 *         gdt::platform::sdl::backend_for_opengl,
 *         gdt::graphics::opengl::backend,
 *         gdt::no_audio,
 *         gdt::no_physics,
 *         gdt::no_networking,
 *         gdt::context
 *     >;
 *
 * Once your application type is specified, you can instantiate
 * an object (for example, by using `std::make_unique`) and use
 * the gdt::application::run method to start it.
 *
 * You will also be able (and encouraged) to use the many other templates
 * GDT has through your speficied application template.
 *
 * For example, here's how you subclass a scene:
 *
 *     class intro : public my_game::scene {
 *         ...
 *     }
 *
 * For the full list of provided types, see the specification below or read
 * through the rest of the docs for more examples.
 *
 * @tparam PLATFORM an implementation of gdt::blueprints::platform::backend
 * @tparam GRAPHICS an implementation of gdt::blueprints::graphics::backend
 * @tparam AUDIO an implementation of gdt::blueprints::audio::backend
 * @tparam PHYSICS an implementation of gdt::blueprints::network::backend
 * @tparam NETWORKING an implementation of the still missing networking backend
 * @tparam CONTEXT either gdt::context or a user subclass of it
 */
template <typename PLATFORM,
          template <typename> typename GRAPHICS,
          typename AUDIO,
          typename PHYSICS,
          typename NETWORKING,
          template <typename, typename, typename, typename> typename CONTEXT>
class application {
  public:
    // PLATFORM
    /**
     * The specified implemetation of gdt::blueprints::platform::backend.
     */
    static_assert(std::is_base_of<gdt::blueprints::platform::backend, PLATFORM>::value,
                  "platform");
    using platform = PLATFORM;

    /**
     * The specified implemetation of gdt::blueprints::graphics::backend.
     */
    static_assert(std::is_base_of<gdt::blueprints::graphics::backend<GRAPHICS<platform>>,
                                  GRAPHICS<platform>>::value,
                  "platform");
    using graphics = GRAPHICS<platform>;

    /**
     * A gdt::asset CRTP template you can use to subclass your own assets.
     * You will normally have additional traits set for your assets.
     */
    template <typename ACTUAL_ASSET>
    using asset = gdt::asset<ACTUAL_ASSET>;

    /**
     * A gdt::drawable CRTP template you subclass to create your own drawable
     * assets:
     *
     *     class zombie : public my_game::asset<zombie>,
     *                    public my_game::drawable<zombie> {
     *       public:
     *         zombie(const my_game::context & ctx):
     *           my_game::drawable<zombie>(ctx, "res/zombie.smd") {}
     *     };
     *
     * Drawable assets can be used as arguments in pipeline `draw` calls:
     *
     *     _pipeline.use(ctx)
     *         .draw(_zombie);
     */
    template <typename ACTUAL_ASSET>
    using drawable = gdt::drawable<GRAPHICS<platform>, ACTUAL_ASSET>;

    /**
     * A gdt::animatable CRTP template you can subclass to set up skeletal
     * animation support for an asset:
     *
     *     class zombie : public my_game::asset<zombie>,
     *                    public my_game::drawable<zombie>,
     *                    public my_game::animatable<zombie> {
     *       public:
     *         zombie(const my_game::context & ctx):
     *           my_game::drawable<zombie>(ctx, "res/zombie.smd"),
     *           my_game::animatable<zombie>(gdt::read_skeleton("res/zombie.smd")) {}
     *     };
     *
     * You can then play animation objects through the gdt::animatable::play
     * and have the model animated using one of the pipelies supporting
     * skeletal animations (for example, gdt::rigged_pipeline).
     */
    template <typename ACTUAL_ASSET>
    using animatable = gdt::animatable<ACTUAL_ASSET>;

    /**
     * A basic model asset you can use to compose your own more complex asset
     * types.
     *
     * For example:
     *
     *     struct zombie {
     *         my_game::model _model;
     *         zombie(const my_game::context & ctx):
     *           _model{ctx, "res/zombie.smd") {}
     *     };
     *
     */
    using model = gdt::bare_asset<GRAPHICS<platform>>;

    /**
     * Image-based texture type you can feed into pipeline materials.
     */
    using texture = typename GRAPHICS<platform>::texture;

    /**
     * SDF image-based font type you can initialize gdt::text objects with.
     */
    using font = gdt::font<graphics>;

    /**
     * Text type you can use with gdt::application::text_pipeline
     */
    using text = gdt::text<graphics>;

    /**
     * A read-to-use gdt::pipeline_proxy, used to ensure proper pipeline usage.
     */
    template <typename S>
    using pipeline_proxy = gdt::pipeline_proxy<graphics, S>;

    /**
     * A ready to use pipeline type for you to subclass from when you want
     * to create your own vertex and fragment shaders pipeline.
     */
    template <typename S>
    using pipeline = gdt::pipeline<graphics, S>;

    /**
     * Forward rendering shaders pipeline.
     */
    using forward_pipeline = gdt::forward_pipeline<graphics>;

    /**
     * Forward rendering shaders pipeline with skeletal animation support. 
     *
     */
    using rigged_pipeline = gdt::rigged_pipeline<graphics>;

    /**
     * GBuffer for use with deferred rendering pipelines.
     */
    using g_buffer = gdt::g_buffer<graphics>;

    /**
     * Geometry pipeline for deferred renderering.
     */
    using geom_pipeline = gdt::geom_pipeline<graphics>;

    /**
     * Geometry pipeline for deferred renderering - with skeleton animation support. 
     *
     */
    using rigged_geom_pipeline = gdt::rigged_geom_pipeline<graphics>;

    /**
     * Lighting pipeline (second phase) for deferred renderering
     */
    using light_pipeline = gdt::light_pipeline<graphics>;

    /**
     * Text rendering shaders pipeline.
     *
     */
    using text_pipeline = gdt::text_pipeline<graphics>;

    /**
     * FXAA antialiasing pipeline.
     *
     */
    using fxaa_pipeline = gdt::fxaa_pipeline<graphics>;

    /**
     * A standard, ready-to-use gdt::material (with diffuse, specular and normal textures).
     *
     */
    using material = gdt::material<graphics>;

    /**
     * TODO: document this
     *
     */
    using render_pass = gdt::render_pass<graphics>;

    /**
     * TODO: document this
     *
     */
    template <typename RENDERER>
    using renderer = gdt::renderer<graphics, RENDERER>;

    // PHYSICS

    /**
     * TODO: document this
     *
     */
    static_assert(
        std::is_same<no_physics, PHYSICS>::value ||
            std::is_base_of<gdt::blueprints::physics::backend<typename PHYSICS::body,
                                                               typename PHYSICS::wall,
                                                               typename PHYSICS::shape>,
                            PHYSICS>::value,
        "physics");
    using physics = PHYSICS;

    /**
     * A specified bounding box collision proxy template.
     *
     */
    template <typename E>
    using box_proxy = gdt::box_proxy<physics, E>;

    /**
     * A specified bounding sphere collision proxy template.
     *
     */
    template <typename E>
    using sphere_proxy = gdt::sphere_proxy<physics, E>;

    /**
     * Rigid body driver you can use with one of the collision proxies.
     *
     */
    template <typename E>
    using rigid_body_driver = gdt::rigid_body_driver<physics, E>;

    /**
     * An FPS physics-based driver you can use with your camera.
     *
     */
    template <typename DRIVABLE>
    using fps_driver = gdt::fps_driver<physics, DRIVABLE>;

    // AUDIO

    /**
     * A specified implementation of gdt::blueprints::audio::backend.
     */
    using audio = AUDIO;

    /**
     * A specified implementation of gdt::blueprints::audio::sound.
     */
    using sound = typename audio::sound;

    /**
     * A specified, ready-to-use context type, potentially user-subclassed.
     */
    using context = CONTEXT<platform, graphics, audio, physics>;

    /**
     * A specified gdt::scene template ready for you to subclass your scenes from
     */
    using scene = gdt::scene<context>;

    /**
     * Just a basic empty scene you can test your applications with.
     *
     */
    using empty_scene = gdt::empty_scene<context>;

    application() : _graphics(&_platform, &_platform._screen)
    {
        LOG_DEBUG << "application setup";
        _platform.on_resize_callback = [this](int w, int h) {
            _platform._screen.notify_subscribers();
            // this is added to allow rebuilding the command buffers
            // on scree resize since we rebuild the swapchain and
            // the command buffers need to be rebuilt as well.
            _active_scene->on_screen_resize(this->_ctx);
        };
        _platform.on_key_callback = [this](int k) { this->on_key(k); };
        _ctx.quit = [this]() { this->quit(); };
        _ctx.p = &_platform;
        _ctx.graphics = &_graphics;
        _ctx.physics = &_physics;
        _ctx.audio = &_audio;
        set_imgui_style();
    }

    virtual ~application()
    {
        LOG_DEBUG << "application cleanup";
    }

    /**
     * Use run to start your application. Make sure you provide a scene
     * class as your template argument so GDT will create your initial
     * scene object.
     *
     * For example:
     *
     *     int main()
     *     {
     *         try {
     *             return std::make_unique<my_app>()->run<my_app::empty_scene>();
     *         } catch (const std::exception & e) {
     *             LOG_ERROR << e.what();
     *             return 1;
     *         }
     *     }
     *
     * @tparam FIRST_SCENE the first gdt::scene subclass your game will initialize and run
     *
     */
    template <typename FIRST_SCENE>
    int run()
    {
        LOG_DEBUG << "application run initiated";
        _active_scene = std::make_unique<FIRST_SCENE>(_ctx, &_platform._screen);
        std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
        start = std::chrono::high_resolution_clock::now();
        _ctx.elapsed = 0;
        while (_platform.process_events() && !_quit) {
            _ctx.measure("core updates").begin();
            _graphics.update_frame();
            _platform.update_window();
            _platform.update_keyboard();
            _platform.update_mouse();
            _ctx.measure("core updates").end();
            this->update(_ctx);
            end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            float x = ms.count() / 1000000.0f;
            _ctx.elapsed = x;
            start = std::chrono::high_resolution_clock::now();
        }
        LOG_DEBUG << "application run ended";
        return 0;
    }

  private: 
    virtual void update(const context& _ctx)
    {
        clear_frame();
        _active_scene.get()->update(_ctx);
        _ctx.measure("core imgui").begin();
        _ctx.p->imgui_frame();
        _active_scene.get()->imgui(_ctx);
        ImGui::Render();
        _ctx.measure("core imgui").end();
    }

    virtual void on_key(int k)
    {
    }

    void quit()
    {
        _quit = true;
    }

    void clear_frame() const
    {
        _graphics.clear_screen();
    }

  private:
    platform _platform;
    graphics _graphics;
    audio _audio;
    physics _physics;
    context _ctx;
    std::unique_ptr<scene> _active_scene;
    bool _quit = false;

};
}
#endif  // GDT_APPLICATION_HEADER_INCLUDED

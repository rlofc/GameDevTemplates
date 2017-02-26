/* examples / physics_instancing.cc
 * ================================
 *
 * In this example we will use bullet physics to simulate a
 * set of instanced crates falling on the ground.
 *
 * Let's begin by including GDT:
 */
#include "gdt.h"

/* Specifying our context and app
 * ------------------------------
 *
 * We'll use OpenGL with GLFW this time, together with Bullet
 * Physics.
 *
 */

#include "backends/glfw/glfw_opengl.hh"
#include "backends/opengl/opengl.hh"
#include "backends/bullet/bullet.hh"

#include <algorithm>

using my_app =
    gdt::application<gdt::platform::glfw::backend_for_opengl,
                     gdt::graphics::opengl::backend,
                     gdt::no_audio,
                     gdt::physics::bullet::backend,
                     gdt::no_networking,
                     gdt::context>;

class crate : public my_app::asset<crate>,
                        public my_app::drawable<crate> {
  private:
    my_app::texture _diffuse_map, _normal_map, _specular_map;
    my_app::geom_pipeline::material _material;

  public:
    crate(const my_app::context& ctx)
        : my_app::drawable<crate>{ctx, "res/examples/crate2.smd"},
          _diffuse_map{ctx, "res/examples/crate2_d.png"},
          _normal_map{ctx, "res/examples/crate2_n.png"},
          _specular_map{ctx, "res/examples/crate2_s.png"},
          _material{ctx, &_diffuse_map, &_normal_map, &_specular_map}
    {
    }
    const my_app::geom_pipeline::material& get_material() const
    {
        return _material;
    }
};

/* Building a reusable deferred renderer
 * -------------------------------------
 * 
 * In some cases you may want to reuse your rendering code
 * in different scenes. A good example for this is a deferred
 * rendering processes. Deferred rendering involves
 * rendering a geometery pass into a geometery buffer (g-buffer)
 * and then using this buffer in a lighting pass.
 * In this example we take things one step further by applying an
 * FXAA anti-aliasing pass as well.
 */
class deferred_renderer : public my_app::renderer<deferred_renderer>,
                          public gdt::screen::subscriber {
  /* We'll need three different pipelines to structure our
   * rendering process: (a) a geometery pipeline, (b) a lighting
   * pipeline and (c) an FXAA pipeline.
   * We'll also need a back_buffer and a g_buffer.
   */
  private:
      my_app::geom_pipeline _geom_pipeline;
      my_app::light_pipeline _light_pipeline;
      my_app::fxaa_pipeline  _fxaa_pipeline;
      my_app::g_buffer _g_buffer;
      my_app::graphics::back_buffer _buffer;

  public:
    /* Since we'll have to resize the off screen buffers if the game
     * window size is changed, we'll subscribe to any screen changes.
     */
    deferred_renderer(const my_app::context& ctx, gdt::screen* screen)
        : _geom_pipeline(ctx),
          _light_pipeline(ctx),
          _fxaa_pipeline(ctx),
          _g_buffer(ctx, 640, 480),
          _buffer(ctx, 640, 480)
    {
        screen->subscribe(this);
    }

    virtual ~deferred_renderer()
    {
    }

    void on_screen_resize(unsigned int w, unsigned int h) override
    {
        _g_buffer.resize(w, h);
        _buffer.resize(w, h);
    }

    /* We'll provide our renderer users with a configuration callback method
     * that will allow them to setup the 2 deferred rendering pipelines before
     * drawing each frame.
     */
    void configure(
        const my_app::context& ctx,
        std::function<void(const my_app::pipeline_proxy<my_app::geom_pipeline>&,
                           const my_app::pipeline_proxy<my_app::light_pipeline>&)>
            callback)
    {
        callback(my_app::pipeline_proxy<my_app::geom_pipeline>(_geom_pipeline),
                 my_app::pipeline_proxy<my_app::light_pipeline>(_light_pipeline));
    }

    /* We'll provide our users with a record callback to allow drawing using
     * our pipelines on each frame.
     * While you may find using the verb `record` weird, it is actually a
     * forward thinking move for when GDT will support Vulkan. Vulkan (as well
     * as other modern graphics APIs) support the notion of pre-recording drawing
     * code and then replaying it on each frame - for increased performance.
     */
    void record(
        const my_app::context& ctx,
        std::function<void(const my_app::pipeline_proxy<my_app::geom_pipeline>&,
                           const my_app::pipeline_proxy<my_app::light_pipeline>&)>
            cmds)
    {
        /* Here's what's going on:
         * We begin by setting up our geometery pass, targeting `_g_buffer`,
         * and clearing it.
         */
        my_app::render_pass(ctx).target(_g_buffer).clear();
        ctx.graphics->cull_on();

        /* We delegate the cmds callback provided by the user to the graphics
         * backend. For OpenGL, this does nothing special but executing the
         * callback. The user callback should take care of drawing things
         * onto the g-buffer.
         */
        ctx.graphics->process_cmds(
            cmds, my_app::pipeline_proxy<my_app::geom_pipeline>(_geom_pipeline),
            my_app::pipeline_proxy<my_app::light_pipeline>(_light_pipeline));

        /* Once we have everything needed done in the geometery pass, we can
         * move on to the lighting pass.
         * We set up a new render pass, this time targeting the back buffer
         * and setting up `_light_pipeline` as a filter. A filter is a pipeline
         * designed to take an input buffer (`_g_buffer` in our case) and manipulate
         * it by rendering a quad.
         */
        my_app::render_pass(ctx)
            .target(_buffer)
            .clear()
            .filter(_light_pipeline)
            .bind_input(_g_buffer);
        ctx.graphics->render_quad();

        /* Finally, we can use the FXAA pipeline for anti-aliasing.
         */
        my_app::render_pass(ctx)
            .target(my_app::graphics::screen_buffer)
            .clear()
            .filter(_fxaa_pipeline)
            .bind_input(_buffer)
            .set_buf_size(
                gdt::math::vec2(_buffer._color_buffer.width, _buffer._color_buffer.height));
        ctx.graphics->render_quad();
        my_app::graphics::screen_buffer.copy_depth_from(_g_buffer);
    }
};

/* The physics scene
 * -----------------
 *
 */
class physics_scene : public my_app::scene {
  private:

    /* Let's define everything we need for the scene. For this example we'll
     * manage two cameras and let the user switch between them.
     */
    deferred_renderer _renderer;
    std::vector<gdt::light> _lights;
    gdt::driven<
        gdt::instances<
            my_app::box_proxy<crate>, 200>,
                my_app::rigid_body_driver> _crates;
    my_app::physics::wall _floor;

    gdt::driven<gdt::instance<gdt::camera>, my_app::fps_driver> _camera1;
    gdt::driven<gdt::instance<gdt::camera>, gdt::hover_driver> _camera2;

    gdt::instance<gdt::camera>* _active_camera_instance;
    gdt::pov_driver* _active_camera_driver;

    gdt::wsad_controller _wsad;

  public:
    /* Note how we initialize the crates using an initializator callback.
     * 
     */
    physics_scene(const my_app::context& ctx, gdt::screen* screen)
        : _renderer(ctx, screen),
          _lights(32),
          _crates(
              ctx,
              [](int j) {
                  my_app::rigid_body_driver<my_app::box_proxy<crate>>::initializer i;
                  i.pos = gdt::math::vec3::random(100, 400, 100) +
                          gdt::math::vec3(0, 350, 0);
                  return i;
              },
              2),
          _camera1{ctx, gdt::pov_driver::look_at({-100,0,-100},{0,100,0}), screen},
          _camera2{ctx, gdt::pov_driver::look_at({-100,200,-100},{0,0,0}), screen}
    {
        _floor = ctx.physics->make_wall(gdt::math::vec3(0, 1, 0), gdt::math::vec3(0, -1, 0));
        _crates.update(ctx);

        std::generate(_lights.begin(), _lights.end(),
                      []() { return gdt::light::randomize(300, 400, 300); });
        activate_camera1();
    }

    virtual ~physics_scene()
    {
    }

    void activate_camera1()
    {
        _active_camera_instance = _camera1.drivable_ptr();
        _active_camera_driver = _camera1.get_driver_ptr();
    }

    void activate_camera2()
    {
        _active_camera_instance = _camera2.drivable_ptr();
        _active_camera_driver = _camera2.get_driver_ptr();
    }

    /* Our frame update code involves updating the `_crates` instances,
     * the camera controller and the physics backend, as well as checking
     * for user input. Finally, we call `render` to draw the frame.
     */
    void update(const my_app::context& ctx) override
    {
        _crates.update(ctx);
        _wsad.update(ctx, _active_camera_driver);
        ctx.physics->update(ctx);
        if (ctx.get_platform()->is_key_pressed(gdt::key::N1)) {
            activate_camera1();
        }
        if (ctx.get_platform()->is_key_pressed(gdt::key::N2)) {
            activate_camera2();
        }
        if (ctx.get_platform()->is_key_pressed(gdt::key::Q)) {
            ctx.quit();
        }
        render(ctx);
    }

    /* We'll use the deferred renderer we've built to draw each frame.
     * First, we'll provide a configuration callback to setup the deferred
     * rendering pipelines.
     * Then, we'll provide a drawing callback to perform the actual drawing.
     */
    void render(const my_app::context& ctx) override
    {
        _renderer.configure(ctx, [this, ctx](auto& geom_pipeline, auto& light_pipeline) {
            geom_pipeline.use(ctx)
                .set_camera(*_active_camera_instance);
            light_pipeline.use(ctx)
                .set_eyepos(_active_camera_instance->entity().pos)
                .set_lights(this->_lights);
        });

        _renderer.record(ctx, [this, ctx](auto& geom_pipeline, auto& light_pipeline) {
            geom_pipeline.use(ctx)
                .set_imgui_overrides()
                .set_material(_crates.get_drawable().get_material())
                .draw(_crates);
        });
    }

    /* ImGui UI for this example:
     */
    void imgui(const my_app::context& ctx) override
    {
        if (ImGui::CollapsingHeader("light")) {
            for (auto& l : _lights) {
                l.imgui();
            }
        }
        _active_camera_instance->entity_ptr()->imgui();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
};

int main()
{
    try {
        my_app().run<physics_scene>();
    }
    catch (const std::exception& e) {
        LOG_ERROR << e.what();
    }
}

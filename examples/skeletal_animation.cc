/* examples / skeletal_animation.cc
 * ================================
 *
 * In this example we'll animate Imrod, .... amazing model.
 *
 * Let's begin by including GDT:
 */

#include "gdt.h"

/* Specifying our app
 * ------------------
 *
 * We'll use SDL as the platform backend with OpenGL as the graphics backend:
 */

#include "backends/sdl/sdl.hh"
#include "backends/opengl/opengl.hh"

using my_app = gdt::application<gdt::platform::sdl::backend_for_opengl,
                                 gdt::graphics::opengl::backend,
                                 gdt::no_audio,
                                 gdt::no_physics, gdt::no_networking, gdt::context>;


/* Our rendering pipeline is going to be a forward rendering pipeline with
 * skeletal animation support:
 */
using rigged_pipeline = gdt::rigged_pipeline<my_app::graphics>;


/* The imrod asset class
 * -----------------------
 *
 * Our main asset in this example is Imrod, so we'll
 * create an asset class, subclassing from both gdt::drawable
 * and gdt::animatable types. This will provide us with
 * the ability to have a skeleton animatable and drawable model.
 */
class imrod : public my_app::asset<imrod>,
              public my_app::drawable<imrod>,
              public my_app::animatable<imrod> {
  /* Member variables
   * ~~~~~~~~~~~~~~~~
   *
   * There are just a few more things we need in our asset class, such
   * as our material assets and an animation object to hold our single
   * animation for Imrod.
   */
  private:
    my_app::texture _diffuse_map, _normal_map, _specular_map;
    rigged_pipeline::material _material;
    gdt::animation _anim;

  public:
    /* Constructing imrod
     * ~~~~~~~~~~~~~~~~~~
     *
     * Our constructor has some work to do before we can use the imrod
     * class:
     *   * Construct drawable_asset with our imrod model file,
     *   * Construct animatable_asset with a skeleton read from the imrod model file,
     *   * Construct our texture maps,
     *   * Construct our material; and
     *   * Construct the animation object with our animation sequence for this example.
     */
    imrod(const my_app::context& ctx)
        : my_app::drawable< imrod>(ctx, "res/examples/imrod.smd"),
          my_app::animatable<imrod>(gdt::read_skeleton("res/examples/imrod.smd")),
          _diffuse_map(ctx, "res/examples/imrod.png"),
          _normal_map(ctx, "res/examples/imrod_nm.png"),
          _specular_map(ctx, "res/examples/imrod_s.png"),
          _material(ctx, &_diffuse_map, &_normal_map, &_specular_map),
          _anim("res/examples/imrod.ani", get_skeleton())
    {
        play(&_anim,0);
    }

    const rigged_pipeline::material& get_material() const
    {
        return _material;
    }

};

/* The imrod_scene class
 * -----------------------
 *
 * Our imrod scene is going to set up everything needed to render our asset
 * using a camera we can control with the mouse and keyboard.
 *
 * Let's take a look at the private members first:
 */
class imrod_scene : public my_app::scene {
  private:
    /* Member variables
     * ~~~~~~~~~~~~~~~~
     *
     * First, we set up our `_imrod` asset as a driven (gdt::driven) instance
     * (gdt::instance) of our imrod class. We use gdt::direct_driver
     * to allow code-based control over the asset location, scale and rotation.
     */
    gdt::driven<gdt::instance<imrod>, gdt::direct_driver> _imrod;
    /* We also set up our `_camera`, again as a driven instance, but this time
     * using gdt::hover_driver to allow the camera to move freely in 3D using
     * standard camera control semantics (pan, track, dolly, etc..)
     */
    gdt::driven<gdt::instance<gdt::camera>, gdt::hover_driver> _camera;
    /* Instead of controlling the camera through code, we'll allow the user
     * to control it using gdt::wsad_controller
     */
    gdt::wsad_controller _wsad;
    /* For rendering, we'll use a forward rendering pipeline with skeletal
     * animation support.
     */
    rigged_pipeline _pipeline;
    float _ambient_light = 0.3;
    gdt::math::vec3 _light_direction = {-0.7, 0.5, 0.9};

  public:
    /* Note how the constructure initializes both `_imrod` and `_camera`
     * using a driver initializator - a special function object designed to
     * provide initializers for the drivers instantiated by gdt::driven.
     */
    imrod_scene(const my_app::context& ctx, gdt::screen* screen) : 
          _imrod(ctx, gdt::pos::origin),
          _camera(ctx, gdt::pov_driver::look_at({-60, 90, -60}, {0, 0, 0}), screen),
          _pipeline(ctx)
    {
        /* Our model is rotated 90 degrees on the X axis, so we fix this:
         */
        _imrod.get_driver_ptr()->rotate({gdt::math::PI / 2, 0, 0});
    }

    /* Updating
     * ~~~~~~~~
     * Update is called on every frame and this is where you 
     * would usually begin running your own scene logic,
     * as well as call other update methods for assets, controllers or other
     * GDT objects you manage in the scene.
     *
     * In our case, we'll update our gdt::wsad_controller, our imrod asset
     * object, check if Q was pressed to exit and finally, call render to draw
     * a frame.
     */
    void update(const my_app::context& ctx) override
    {
        _wsad.update(ctx, _camera.get_driver_ptr());
        _imrod.get_animatable_ptr()->update(ctx);
        if (ctx.get_platform()->is_key_pressed(gdt::key::Q)) {
            ctx.quit();
        }
        render(ctx);
    }

    /* Rendering
     * ~~~~~~~~~
     *
     * To render Imrod, we'll create a render pass targeting the screen
     * and clear it. We'll then use the forward rendering pipeline,
     * set the material to use, the camera and some lighting information
     * we can control with our ImGui UI.
     * Finally, we'll draw our imrod asset.
     */
    void render(const my_app::context& ctx) override
    {
        my_app::render_pass(ctx)
            .target(my_app::graphics::screen_buffer)
            .clear();
        _pipeline.use(ctx)
            .set_material(_imrod.get_drawable().get_material())
            .set_camera(_camera)
            .set_ambient_additive(_ambient_light)
            .set_light_direction(_light_direction)
            .draw(_imrod);
    }

    /* ImGui
     * ~~~~~
     *
     * Overriding the imgui method allows you to use ImGui as a development
     * and debugging tool.
     */
    void imgui(const my_app::context& ctx) override
    {
        _camera.entity_ptr()->imgui();
        ImGui::InputFloat4("x",&_camera.get_transformable_ptr()->get_transform_ptr(0)->xx);
        ImGui::InputFloat4("y",&_camera.get_transformable_ptr()->get_transform_ptr(0)->yx);
        ImGui::InputFloat4("z",&_camera.get_transformable_ptr()->get_transform_ptr(0)->zx);
        ImGui::InputFloat4("w",&_camera.get_transformable_ptr()->get_transform_ptr(0)->wx);
        if (ImGui::CollapsingHeader("shader")) {
            ImGui::SliderFloat("Ambient", &_ambient_light, 0.0f, 1.0f);
            ImGui::SliderFloat3("Light Direction", &_light_direction.x, -1.0f, 1.0f);
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
};

/* Running the app
 * ---------------
 *
 * Nothing too special here. Just create the app object
 * and use the template method `run` to start our one and only
 * scene.
 */
int main()
{
    try {
        my_app().run<imrod_scene>();
    } catch (const std::exception & e) {
        LOG_ERROR << e.what();
    }
}

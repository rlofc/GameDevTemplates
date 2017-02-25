/* examples / basic_rendering.cc
 * =============================
 *
 * In this examples we'll go through the basics of setting up a GDT
 * application that renders a moon in a fixed position from using a
 * fixed camera.
 */
#include "gdt.h"

/* Specifying our app
 * ------------------
 *
 * We'll use an SDL platform backend with an OpenGL graphics
 * backend only.
 */
#include "backends/sdl/sdl.hh"
#include "backends/opengl/opengl.hh"

using my_app = gdt::application<gdt::platform::sdl::backend_for_opengl,
                                gdt::graphics::opengl::backend, 
                                gdt::no_audio,
                                gdt::no_physics,
                                gdt::no_networking,
                                gdt::context>;

/* Moon asset class
 * ----------------
 *
 * We have just one asset in this example, our moon object.
 * It is a drawable asset, loaded from an SMD file and
 * we also bundle in its texture maps and material, although
 * if this material was shared between several assets, then it
 * would have made more sense to place it outside of this class.
 */
class moon : public my_app::asset<moon>,
             public my_app::drawable<moon> {

  private:
    my_app::texture _diffuse_map, _normal_map, _specular_map;
    my_app::material _material;

  public:

    /* The moon asset constructor receives our app context and uses its
     * members constructures to load the SMD model and the three
     * PNG texture files.
     */
    moon(const my_app::context& ctx)
        : my_app::drawable<moon>(ctx, "res/examples/moon.smd"),
          _diffuse_map(ctx, "res/examples/moon_d.png"),
          _normal_map(ctx, "res/examples/moon_nm.png"),
          _specular_map(ctx, "res/examples/moon_s.png"),
          _material(ctx, &_diffuse_map, &_normal_map, &_specular_map)
    {
    }

    /* We also provide a simple getter for our material object. Note
     * that in this case we have the material bundled within the asset, but
     * you are free to separate materials from individual assets if it makes
     * sense in your case.
     */
    const my_app::material& get_material() const
    {
        return _material;
    }
};

/* Our space_scene
 * ---------------
 *
 * space_scene is a subclass of our application type scene class.
 * We will store our moon asset and a basic camera as members of the scene.
 * We will also override the required methods to update and render the scene.
 *
 * Note how we use gtd::instance to extend both our moon asset and the camera
 * with valid transformation data.
 */
class space_scene : public my_app::scene {
    gdt::instance<moon> _moon;
    gdt::instance<gdt::camera> _camera;
    my_app::forward_pipeline _pipeline;

  public:

    /* When constructing the scene, we will also construct our moon asset
     * and the camera instance
     */
    space_scene(const my_app::context& ctx, gdt::screen* screen)
        : _moon(ctx, gdt::pos::origin),
          _camera(ctx, gdt::pos::look_at({10, 0, -20}, {0, 0, 0}), screen),
          _pipeline(ctx)
    {
    }

    /* Updating the scene does nothing more than checking for our 
     * exit trigger and rendering.
     */
    void update(const my_app::context& ctx) override
    {
        if (ctx.get_platform()->is_key_pressed(gdt::key::Q)) {
            ctx.quit();
        }
        render(ctx);
    }

    /* The render method is where the actual drawing takes place.
     */
    void render(const my_app::context& ctx) override
    {
        /* First, we set up our render pass, designating the screen buffer
        * as the target, and clearing it with a black background.
        */
        my_app::render_pass(ctx)
            .target(my_app::graphics::screen_buffer)
            .clear({0, 0, 0, 1});
        /* Then we use a forward rendering pipeline, set the material to our
         * moon material, the camera and then invoke a single draw call
         * to draw the moon on the screen.
         */
        _pipeline.use(ctx)
            .set_material(_moon.get_drawable().get_material())
            .set_camera(_camera)
            .draw(_moon);
    }
};

/* Running the app
 * ---------------
 *
 * To run the application, we create an instance from `my_app`
 * and invoke the template method `run` with our scene class as its
 * template argument. GDT will instantiate the scene object for us
 * and will initiate the main loop.
 */
int main()
{
    try {
        my_app().run<space_scene>();
    } catch (const std::exception & e) {
        LOG_ERROR << e.what();
    }
}

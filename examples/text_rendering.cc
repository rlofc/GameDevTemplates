#include "gdt.h"

#include "backends/openal/openal.hh"
#include "backends/opengl/opengl.hh"
#include "backends/sdl/sdl.hh"

using my_app = gdt::application<gdt::platform::sdl::backend_for_opengl,
                                 gdt::graphics::opengl::backend,
                                 gdt::audio::openal::backend,
                                 gdt::no_physics,
                                 gdt::no_networking,
                                 gdt::context>;

class text_renderer : public my_app::renderer<text_renderer> {
    my_app::text_pipeline _text_pipeline;

  public:
    text_renderer(const my_app::context& ctx) : _text_pipeline(ctx)
    {
    }

    void record(const my_app::context& ctx,
                std::function<void(const my_app::pipeline_proxy<my_app::text_pipeline>&)>
                    cmds)
    {
        ctx.graphics->blend_on();
        ctx.graphics->cull_on();
        gdt::render_pass<my_app::graphics>(ctx)
            .target(my_app::graphics::screen_buffer)
            .clear({0.9, 0.9, 0.9, 1.0});
        ctx.graphics->process_cmds(
            cmds, my_app::pipeline_proxy<my_app::text_pipeline>(_text_pipeline));
    }
};

class text_scene : public my_app::scene {
    text_renderer _renderer;
    gdt::driven<gdt::instance<gdt::camera2dpp>, gdt::dummy_driver> _camera;
    my_app::font _f;
    gdt::driven<gdt::instance<my_app::text>, gdt::direct_driver> _text;
    float _text_scale = 1.0;  // 0.25;

  public:
    text_scene(const my_app::context& ctx, gdt::screen* screen)
        : _renderer(ctx),
          _camera(ctx, gdt::center_pos, screen),
          _f(ctx),
          _text(ctx, gdt::center_pos, _f,
                /* from "The Raven" by Edgar Allan Poe */
                "Deep into that darkness peering, long I stood there wondering, fearing,\n"
                "Doubting, dreaming dreams no mortal ever dared to dream before;\n"
                "But the silence was unbroken, and the stillness gave no token,\n"
                "And the only word there spoken was the whispered word, \"Lenore?\"\n")

    {
    }

    gdt::math::vec3 _pos;

    void update(const my_app::context& ctx) override
    {
        _camera.update(ctx);
        _text.get_driver_ptr()->translate(_pos);
        _text.get_driver_ptr()->scale(gdt::math::vec3() + _text_scale);
        _text.update(ctx);
        if (ctx.get_platform()->is_key_pressed(gdt::key::Q)) {
            ctx.quit();
        }
        render(ctx);
    }

    float _tweak = 1.0;

    void render(const my_app::context& ctx) override
    {
        _renderer.record(ctx, [this, ctx](auto& shader) {
            shader.use(ctx)
                .set_scale(_text_scale * _tweak)
                .set_color({0,0,0,1})
                .set_camera(_camera)
                .set_material(_f.material())
                .draw(_text.drivable());
        });
    }

    void imgui(const my_app::context& ctx) override
    {
        ImGui::SliderFloat3("pos", &_pos.x, -ctx.p->screen().w, ctx.p->screen().w);
        ImGui::SliderFloat("scale", &_text_scale, 0.1f, 5.0f);
        ImGui::SliderFloat("tweak", &_tweak, 0.5f, 2.0f);
    }
};

int main()
{
    std::make_unique<my_app>()->run<text_scene>();
}

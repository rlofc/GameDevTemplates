#ifndef SRC_CONSTRUCTS_SHADERS_HH_INCLUDED
#define SRC_CONSTRUCTS_SHADERS_HH_INCLUDED

#include "imgui/imgui.h"
#include "extensions.hh"
#include "light.hh"
#include "camera.hh"
namespace gdt {

/**
 * Use this class to provide any drawing code with a shader,
 * instead of directly passing a shader reference. This way
 * you can ensure the user will call gdt::pipeline::use because
 * this is the only way the user would be able to access the wrapped
 * shader reference:
 *
 *     my_game::pipeline_proxy<my_game::text_pipeline> get_text_shader() const {
 *         return my_game::pipeline_proxy<my_game::text_pipeline>(_text_pipeline);
 *     }
 *
 *     ...
 *
 *     void render(const my_game::context & ctx) {
 *         get_text_shader()
 *             .use(ctx) // now we have the actual text_pipeline object
 *             .draw(_text);
 *     }
 *
 */
template <typename GRAPHICS, typename S>
class pipeline_proxy {
  private:
    const S& _s;

  public:
    pipeline_proxy(const S& s) : _s(s)
    {
    }
    const S& use(const graphics_context<GRAPHICS>& ctx) const
    {
        _s.use(ctx);
        return _s;
    }
};

template <typename GRAPHICS>
struct material {
    material(const graphics_context<GRAPHICS>& ctx,
             const typename GRAPHICS::texture* d,
             const typename GRAPHICS::texture* n,
             const typename GRAPHICS::texture* s)
        : diffuse(d), normal(n), specular(s)
    {
    }

    template <typename SHADER>
    void bind(const graphics_context<GRAPHICS>& ctx, const SHADER& s) const
    {
        s.set_material(*this);
    }
    const typename GRAPHICS::texture* diffuse;
    const typename GRAPHICS::texture* normal;
    const typename GRAPHICS::texture* specular;
};

template <typename GRAPHICS, typename T>
class pipeline : public GRAPHICS::base_pipeline {
  public:
    pipeline(std::string resource_base_name) : GRAPHICS::base_pipeline(resource_base_name)
    {
    }
    pipeline_proxy<GRAPHICS, T> by_proxy()
    {
        return pipeline_proxy<GRAPHICS, T>(static_cast<const T&>(*this));
    }
    const T & use(const graphics_context<GRAPHICS> & ctx) const {
        GRAPHICS::base_pipeline::use(ctx);
        return static_cast<const T&>(*this);
    }
    typedef pipeline_proxy<GRAPHICS, T> proxy;
};

template <typename GRAPHICS>
class forward_pipeline : public pipeline<GRAPHICS, forward_pipeline<GRAPHICS>> {
  private:
    typename GRAPHICS::base_pipeline::sampler _tex;
    typename GRAPHICS::base_pipeline::sampler _ntex;
    typename GRAPHICS::base_pipeline::sampler _stex;

    typename GRAPHICS::base_pipeline::uniform _mvp;
    typename GRAPHICS::base_pipeline::uniform _eye;

    typename GRAPHICS::base_pipeline::attrib _vpos;
    typename GRAPHICS::base_pipeline::attrib _vnor;
    typename GRAPHICS::base_pipeline::attrib _vtan;
    typename GRAPHICS::base_pipeline::attrib _vuvs;

    typename GRAPHICS::base_pipeline::attrib _itfm;

  public:
    using material = gdt::material<GRAPHICS>;
    forward_pipeline(const graphics_context<GRAPHICS>& ctx)
        : pipeline<GRAPHICS, forward_pipeline<GRAPHICS>>("res/shaders/forward")
    {
        this->use(ctx);
        _tex = this->add_sampler("tex_diffuse");
        _ntex = this->add_sampler("tex_normal");
        _stex = this->add_sampler("tex_specular");
        _mvp = this->add_uniform("um4_mvp");
        _eye = this->add_uniform("uv3_eyepos");
        _vpos = this->add_attrib("av4_position");
        _vnor = this->add_attrib("av3_normal");
        _vtan = this->add_attrib("av3_tangent");
        _vuvs = this->add_attrib("av2_texcoord");
        _itfm = this->add_attrib("am4_transform");
    }

    template <typename SOMETHING>
    const forward_pipeline& draw(const SOMETHING& what) const
    {
        what.get_drawable().draw_instances(this->adhoc_context(),
                                           *this,
                                           what.get_transformable().get_transforms(),
                                           what.get_transformable().size());
        return *this;
    }

    template <typename CONTEXT, typename ENTITY>
    const forward_pipeline& user_draw(const CONTEXT& ctx, const ENTITY& what) const
    {
        what.draw(ctx, *this);
        return *this;
    }

    void disable_all_vertex_attribs() const
    {
        this->disable_attrib(_vpos);
        this->disable_attrib(_vnor);
        this->disable_attrib(_vtan);
        this->disable_attrib(_vuvs);
        this->disable_attrib(_itfm);
        this->disable_attrib(_itfm + 1);
        this->disable_attrib(_itfm + 2);
        this->disable_attrib(_itfm + 3);
    }

    template <typename PLUGABLE>
    auto& operator<<(const PLUGABLE& cc) const
    {
        this->set_material(cc);
        return *this;
    }

    const forward_pipeline& set_material(const material& solid_mat) const
    {
        this->bind_sampler(_tex, solid_mat.diffuse);
        this->bind_sampler(_ntex, solid_mat.normal);
        this->bind_sampler(_stex, solid_mat.specular);
        return *this;
    }

    void bind_vertex_attribs() const
    {
        this->bind_float_attrib(_vpos,
                                3,          // vec3
                                18,         // total floats per vertex
                                (void*)0);  // position in vertex
        this->bind_float_attrib(_vnor,
                                3,                            // vec2
                                18,                           // total floats per vertex
                                (void*)(sizeof(float) * 3));  // position in vertex
        this->bind_float_attrib(_vtan,
                                3,                            // vec2
                                18,                           // total floats per vertex
                                (void*)(sizeof(float) * 6));  // position in vertex
        this->bind_float_attrib(_vuvs,
                                2,                             // vec2
                                18,                            // total floats per vertex
                                (void*)(sizeof(float) * 12));  // position in vertex
    }

    void bind_instances() const
    {
        this->bind_instances_data(_itfm);
    }

    const forward_pipeline& set_modelview(gdt::math::mat4 mvp) const
    {
        this->bind_uniform(_mvp, mvp);
        return *this;
    }

    const forward_pipeline& set_eyepos(gdt::math::vec3 eye) const
    {
        this->bind_uniform(_eye, eye);
        return *this;
    }

    template <typename CAMERA>
    const forward_pipeline& set_camera(const CAMERA & c) const
    {
        set_eyepos(c.entity().pos);
        set_modelview(c.entity().proj * c.get_transformable().get_transforms()[0]);
        return *this;
    }

    void enable_vertex_attributes() const  // override
    {
        this->bind_vertex_attribs();
    };
};

template <typename GRAPHICS>
class geom_pipeline : public pipeline<GRAPHICS, geom_pipeline<GRAPHICS>> {
  private:
    typename GRAPHICS::base_pipeline::sampler _tex;
    typename GRAPHICS::base_pipeline::sampler _ntex;
    typename GRAPHICS::base_pipeline::sampler _stex;

    typename GRAPHICS::base_pipeline::uniform _mvp;
    typename GRAPHICS::base_pipeline::uniform _fog_density;

    typename GRAPHICS::base_pipeline::attrib _vpos;
    typename GRAPHICS::base_pipeline::attrib _vnor;
    typename GRAPHICS::base_pipeline::attrib _vtan;
    typename GRAPHICS::base_pipeline::attrib _vuvs;

    typename GRAPHICS::base_pipeline::attrib _itfm;

  public:
    geom_pipeline(const graphics_context<GRAPHICS>& ctx) : pipeline<GRAPHICS, geom_pipeline<GRAPHICS>>("res/shaders/geom")
    {
        this->use(ctx);
        _tex = this->add_sampler("tex_diffuse");
        _ntex = this->add_sampler("tex_normal");
        _stex = this->add_sampler("tex_specular");
        _mvp = this->add_uniform("um4_mvp");
        _fog_density = this->add_uniform("fog_density");
        _vpos = this->add_attrib("av4_position");
        _vnor = this->add_attrib("av3_normal");
        _vtan = this->add_attrib("av3_tangent");
        _vuvs = this->add_attrib("av2_texcoord");
        _itfm = this->add_attrib("am4_transform");
    }
    void disable_all_vertex_attribs() const
    {
        this->disable_attrib(_vpos);
        this->disable_attrib(_vnor);
        this->disable_attrib(_vtan);
        this->disable_attrib(_vuvs);
        this->disable_attrib(_itfm);
        this->disable_attrib(_itfm + 1);
        this->disable_attrib(_itfm + 2);
        this->disable_attrib(_itfm + 3);
    }

    using material = gdt::material<GRAPHICS>;

    template <typename SOMETHING>
    const geom_pipeline& draw(const SOMETHING& what) const
    {
        what.get_drawable().draw_instances(this->adhoc_context(),
                                           *this,
                                           what.get_transformable().get_transforms(),
                                           what.get_transformable().size());
        return *this;
    }

    template <typename CONTEXT, typename ENTITY>
    const geom_pipeline& user_draw(const CONTEXT& ctx, const ENTITY& what) const
    {
        what.draw(ctx, *this);
        return *this;
    }

    const geom_pipeline& set_material(const material& solid_mat) const
    {
        this->bind_sampler(_tex, solid_mat.diffuse);
        this->bind_sampler(_ntex, solid_mat.normal);
        this->bind_sampler(_stex, solid_mat.specular);
        return *this;
    }

    void bind_vertex_attribs() const
    {
        this->bind_float_attrib(_vpos,
                                3,          // vec3
                                18,         // total floats per vertex
                                (void*)0);  // position in vertex
        this->bind_float_attrib(_vnor,
                                3,                            // vec2
                                18,                           // total floats per vertex
                                (void*)(sizeof(float) * 3));  // position in vertex

        this->bind_float_attrib(_vtan,
                                3,                            // vec2
                                18,                           // total floats per vertex
                                (void*)(sizeof(float) * 6));  // position in vertex

        this->bind_float_attrib(_vuvs,
                                2,                             // vec2
                                18,                            // total floats per vertex
                                (void*)(sizeof(float) * 12));  // position in vertex
    }

    void bind_instances() const
    {
        this->bind_instances_data(_itfm);
    }

    const geom_pipeline& set_modelview(gdt::math::mat4 mvp) const
    {
        this->bind_uniform(_mvp, mvp);
        return *this;
    }

    template <typename CAMERA>
    const geom_pipeline& set_camera(const CAMERA & c) const
    {
        set_modelview(c.entity().proj * c.get_transformable().get_transforms()[0]);
        return *this;
    }

    const geom_pipeline& set_fog_density(float fd) const
    {
        this->bind_uniform(_fog_density, fd);
        return *this;
    }

    void enable_vertex_attributes() const  // override
    {
        this->bind_vertex_attribs();
    };

    bool _override = false;
    float _fog_density_override = 0.0f;

    const geom_pipeline& set_imgui_overrides() const
    {
        if (_override) {
            set_fog_density(_fog_density_override);
        }
        return *this;
    }

    void imgui()
    {
        if (ImGui::CollapsingHeader("deferred geom shader")) {
            ImGui::Checkbox("override", &_override);
            ImGui::SliderFloat("fog density", &_fog_density_override, 0.0f, 0.005f, "%.5f");
        }
    }
};

template <typename GRAPHICS>
class rigged_geom_pipeline : public pipeline<GRAPHICS, rigged_geom_pipeline<GRAPHICS>> {
  private:
    typename GRAPHICS::base_pipeline::sampler _tex;
    typename GRAPHICS::base_pipeline::sampler _ntex;
    typename GRAPHICS::base_pipeline::sampler _stex;

    typename GRAPHICS::base_pipeline::uniform _mvp;
    typename GRAPHICS::base_pipeline::uniform _fog_density;

    typename GRAPHICS::base_pipeline::attrib _vpos;
    typename GRAPHICS::base_pipeline::attrib _vnor;
    typename GRAPHICS::base_pipeline::attrib _vtan;
    typename GRAPHICS::base_pipeline::attrib _vuvs;

    typename GRAPHICS::base_pipeline::attrib _itfm;

    typename GRAPHICS::base_pipeline::uniform _quat_reals;
    typename GRAPHICS::base_pipeline::uniform _quat_duals;

    typename GRAPHICS::base_pipeline::attrib _vbi;
    typename GRAPHICS::base_pipeline::attrib _vbw;

  public:
    rigged_geom_pipeline(const graphics_context<GRAPHICS>& ctx)
        : pipeline<GRAPHICS, rigged_geom_pipeline<GRAPHICS>>("res/shaders/geom_rigged")
    {
        this->use(ctx);
        _tex = this->add_sampler("tex_diffuse");
        _ntex = this->add_sampler("tex_normal");
        _stex = this->add_sampler("tex_specular");
        _mvp = this->add_uniform("um4_mvp");
        _fog_density = this->add_uniform("fog_density");
        _vpos = this->add_attrib("av4_position");
        _vnor = this->add_attrib("av3_normal");
        _vtan = this->add_attrib("av3_tangent");
        _vuvs = this->add_attrib("av2_texcoord");
        _itfm = this->add_attrib("av4_transform");

        _quat_reals = this->add_uniform("uv4_quat_reals");
        _quat_duals = this->add_uniform("uv4_quat_duals");
        _vbi = this->add_attrib("av3_bindices");
        _vbw = this->add_attrib("av3_bweights");
    }
    void disable_all_vertex_attribs() const
    {
        this->disable_attrib(_vpos);
        this->disable_attrib(_vnor);
        this->disable_attrib(_vtan);
        this->disable_attrib(_vuvs);
        this->disable_attrib(_vbi);
        this->disable_attrib(_vbw);
        this->disable_attrib(_itfm);
        this->disable_attrib(_itfm + 1);
        this->disable_attrib(_itfm + 2);
        this->disable_attrib(_itfm + 3);
    }

    using material = gdt::material<GRAPHICS>;

    template <typename SOMETHING>
    const rigged_geom_pipeline& draw(const SOMETHING& what) const
    {
        what.get_animatable().bind(*this);
        what.get_drawable().draw_instances(this->adhoc_context(),
                                           *this,
                                           what.get_transformable().get_transforms(),
                                           what.get_transformable().size());
        return *this;
    }

    template <typename CONTEXT, typename ENTITY>
    const rigged_geom_pipeline& user_draw(const CONTEXT& ctx, const ENTITY& what) const
    {
        what.draw(ctx, *this);
        return *this;
    }

    const rigged_geom_pipeline& set_material(const material& solid_mat) const
    {
        this->bind_sampler(_tex, solid_mat.diffuse);
        this->bind_sampler(_ntex, solid_mat.normal);
        this->bind_sampler(_stex, solid_mat.specular);
        return *this;
    }

    void bind_vertex_attribs() const
    {
        this->bind_float_attrib(_vpos,
                                3,          // vec3
                                24,         // total floats per vertex
                                (void*)0);  // position in vertex
        this->bind_float_attrib(_vnor,
                                3,                            // vec2
                                24,                           // total floats per vertex
                                (void*)(sizeof(float) * 3));  // position in vertex
        this->bind_float_attrib(_vtan,
                                3,                            // vec2
                                24,                           // total floats per vertex
                                (void*)(sizeof(float) * 6));  // position in vertex
        this->bind_float_attrib(_vuvs,
                                2,                             // vec2
                                24,                            // total floats per vertex
                                (void*)(sizeof(float) * 12));  // position in vertex
        this->bind_float_attrib(_vbi,
                                3,                             // vec2
                                24,                            // total floats per vertex
                                (void*)(sizeof(float) * 18));  // position in vertex
        this->bind_float_attrib(_vbw,
                                3,                             // vec2
                                24,                            // total floats per vertex
                                (void*)(sizeof(float) * 21));  // position in vertex
    }

    void bind_instances() const
    {
        this->bind_instances_data(_itfm);
    }

    const rigged_geom_pipeline& set_modelview(gdt::math::mat4 mvp) const
    {
        this->bind_uniform(_mvp, mvp);
        return *this;
    }

    template <typename CAMERA>
    const rigged_geom_pipeline& set_camera(const CAMERA & c) const
    {
        set_modelview(c.entity().proj * c.get_transformable().get_transforms()[0]);
        return *this;
    }

    const rigged_geom_pipeline& set_fog_density(float fd) const
    {
        this->bind_uniform(_fog_density, fd);
        return *this;
    }

    void bind_reals(math::vec4* reals, int c) const
    {
        this->bind_uniform(_quat_reals, reals, 64);  // c);
    }
    void bind_duals(math::vec4* duals, int c) const
    {
        this->bind_uniform(_quat_duals, duals, 64);  // c);
    }

    void enable_vertex_attributes() const  // override
    {
        this->bind_vertex_attribs();
    };

    bool _override = false;
    float _fog_density_override = 0.0f;

    const rigged_geom_pipeline& set_imgui_overrides() const
    {
        if (_override) {
            set_fog_density(_fog_density_override);
        }
        return *this;
    }

    void imgui()
    {
        if (ImGui::CollapsingHeader("deferred geom shader")) {
            ImGui::Checkbox("override", &_override);
            ImGui::SliderFloat("fog density", &_fog_density_override, 0.0f, 0.005f, "%.5f");
        }
    }
};

template <typename GRAPHICS>
class g_buffer : public GRAPHICS::depth_enabled_frame_buffer {
  public:
    typename GRAPHICS::rgb16_buffer _position;
    typename GRAPHICS::rgb16_buffer _normal;
    typename GRAPHICS::rgba_buffer _albedospec;
    typename GRAPHICS::rgba_buffer _fog;

    g_buffer(const graphics_context<GRAPHICS>& ctx, unsigned int w, unsigned int h)
        : GRAPHICS::depth_enabled_frame_buffer(ctx, w, h),
          _position(ctx),
          _normal(ctx),
          _albedospec(ctx),
          _fog(ctx)
    {
        this->attachments({&_position, &_normal, &_albedospec, &_fog}, w, h);
    }
    virtual ~g_buffer()
    {
    }
};

template <typename GRAPHICS>
class light_pipeline : public pipeline<GRAPHICS, light_pipeline<GRAPHICS>> {
  private:
    struct light_attrs {
        typename GRAPHICS::base_pipeline::uniform pos;
        typename GRAPHICS::base_pipeline::uniform color;
        typename GRAPHICS::base_pipeline::uniform linear;
        typename GRAPHICS::base_pipeline::uniform quadric;
    } lights_attrs[32];

    typename GRAPHICS::base_pipeline::sampler _pos_buf;
    typename GRAPHICS::base_pipeline::sampler _nor_buf;
    typename GRAPHICS::base_pipeline::sampler _alb_buf;
    typename GRAPHICS::base_pipeline::sampler _fog_buf;

    typename GRAPHICS::base_pipeline::uniform _eye;

  public:
    light_pipeline(const graphics_context<GRAPHICS>& ctx) : pipeline<GRAPHICS, light_pipeline<GRAPHICS>>("res/shaders/light")
    {
        this->use(ctx);
        _pos_buf = this->add_sampler("s2d_position");
        _nor_buf = this->add_sampler("s2d_normal");
        _alb_buf = this->add_sampler("s2d_albedospec");
        _fog_buf = this->add_sampler("s2d_fog");

        _eye = this->add_uniform("uv3_viewpos");

        for (int i = 0; i < 32; i++) {
            this->lights_attrs[i].pos =
                this->add_uniform("lights[" + std::to_string(i) + "].position");
            this->lights_attrs[i].color =
                this->add_uniform("lights[" + std::to_string(i) + "].color");
            this->lights_attrs[i].linear =
                this->add_uniform("lights[" + std::to_string(i) + "].linear");
            this->lights_attrs[i].quadric =
                this->add_uniform("lights[" + std::to_string(i) + "].quadratic");
        }
    }

    void disable_all_vertex_attribs() const
    {
    }

    const light_pipeline& bind_input(const g_buffer<GRAPHICS>& gb) const
    {
        this->bind_sampler(_pos_buf, &gb._position);
        this->bind_sampler(_nor_buf, &gb._normal);
        this->bind_sampler(_alb_buf, &gb._albedospec);
        this->bind_sampler(_fog_buf, &gb._fog);
        return *this;
    }

    const light_pipeline& set_lights(const std::vector<light>& lights) const
    {
        if (lights.size() > 32) throw std::runtime_error("too many lights");
        int index = 0;
        for (const light& l : lights) {
            this->bind_uniform(lights_attrs[index].pos, l.pos);
            this->bind_uniform(lights_attrs[index].color, l.color);
            this->bind_uniform(lights_attrs[index].linear, l.linear);
            this->bind_uniform(lights_attrs[index].quadric, l.quadric);
            index++;
        }
        return *this;
    }

    const light_pipeline& set_eyepos(gdt::math::vec3 eye) const
    {
        this->bind_uniform(_eye, eye);
        return *this;
    }
};

template <typename GRAPHICS>
class text_pipeline : public pipeline<GRAPHICS, text_pipeline<GRAPHICS>>{
  private:
    typename GRAPHICS::base_pipeline::sampler _tex;
    typename GRAPHICS::base_pipeline::uniform _mvp;
    typename GRAPHICS::base_pipeline::uniform _otr;
    typename GRAPHICS::base_pipeline::uniform _smoothing;
    typename GRAPHICS::base_pipeline::uniform _outline;
    typename GRAPHICS::base_pipeline::uniform _color;
    typename GRAPHICS::base_pipeline::attrib _vpos;
    typename GRAPHICS::base_pipeline::attrib _vuvs;

  public:
    text_pipeline(const graphics_context<GRAPHICS>& ctx) : pipeline<GRAPHICS, text_pipeline<GRAPHICS>>("res/shaders/text")
    {
        this->use(ctx);
        _tex = this->add_sampler("tex");
        _mvp = this->add_uniform("mvp");
        _otr = this->add_uniform("otr");
        _smoothing = this->add_uniform("smoothing");
        _outline = this->add_uniform("outline");
        _color = this->add_uniform("color");
        _vpos = this->add_attrib("av4_position");
        _vuvs = this->add_attrib("av2_texcoord");
    }

    void disable_all_vertex_attribs() const
    {
        this->disable_attrib(_vpos);
        this->disable_attrib(_vuvs);
    }

    struct material {
        const typename GRAPHICS::texture* texts;
    };

    const text_pipeline& set_material(const material& text_mat) const
    {
        this->bind_sampler(_tex, text_mat.texts);
        return *this;
    }

    void bind_vertex_attribs() const
    {
        this->bind_float_attrib(_vpos,
                                3,          // vec3
                                18,         // total floats per vertex
                                (void*)0);  // position in vertex
        this->bind_float_attrib(_vuvs,
                                2,                             // vec2
                                18,                            // total floats per vertex
                                (void*)(sizeof(float) * 12));  // position in vertex
    }

    template <typename ENTITY>
    const text_pipeline& draw(const ENTITY& what) const
    {
        // TODO: consider static_cast to text
        what.entity().draw(this->adhoc_context(), *this, what.get_transforms());
        return *this;
    }

    const text_pipeline& set_modelview(gdt::math::mat4 mvp) const
    {
        this->bind_uniform(_mvp, mvp);
        return *this;
    }

    template <typename CAMERA>
    const text_pipeline& set_camera(const CAMERA & c) const
    {
        set_modelview(c.entity().proj * c.get_transformable().get_transforms()[0]);
        return *this;
    }

    const text_pipeline& set_transform(gdt::math::mat4 otr) const
    {
        this->bind_uniform(_otr, otr.transpose());
        return *this;
    }
    const text_pipeline& set_smoothing(float smoothing) const
    {
        this->bind_uniform(_smoothing, smoothing);
        return *this;
    }
    const text_pipeline& set_outline(float outline) const
    {
        this->bind_uniform(_outline, outline);
        return *this;
    }
    const text_pipeline& set_color(gdt::math::vec4 color) const
    {
        this->bind_uniform(_color, color);
        return *this;
    }
    const text_pipeline& set_scale(float scale) const
    {
        float spread = 10.0;  // 8.0
        float outline = 1.0;
        this->set_smoothing(0.25f / (spread * scale));
        this->set_outline((outline * scale) / (spread * scale));
        return *this;
    }

    void enable_vertex_attributes() const  // override
    {
        this->bind_vertex_attribs();
    };
};

template <typename GRAPHICS>
class rigged_pipeline : public pipeline<GRAPHICS, rigged_pipeline<GRAPHICS>>{
  private:
    typename GRAPHICS::base_pipeline::sampler _tex;
    typename GRAPHICS::base_pipeline::sampler _ntex;
    typename GRAPHICS::base_pipeline::sampler _stex;

    typename GRAPHICS::base_pipeline::uniform _mvp;
    typename GRAPHICS::base_pipeline::uniform _eye;

    typename GRAPHICS::base_pipeline::uniform _ambient_additive;
    typename GRAPHICS::base_pipeline::uniform _light_direction;

    typename GRAPHICS::base_pipeline::uniform _quat_reals;
    typename GRAPHICS::base_pipeline::uniform _quat_duals;

    typename GRAPHICS::base_pipeline::attrib _vpos;
    typename GRAPHICS::base_pipeline::attrib _vnor;
    typename GRAPHICS::base_pipeline::attrib _vtan;
    typename GRAPHICS::base_pipeline::attrib _vuvs;
    typename GRAPHICS::base_pipeline::attrib _vbi;
    typename GRAPHICS::base_pipeline::attrib _vbw;

    typename GRAPHICS::base_pipeline::attrib _itfm;

  public:
    rigged_pipeline(const graphics_context<GRAPHICS>& ctx)
        : pipeline<GRAPHICS, rigged_pipeline<GRAPHICS>>("res/shaders/forward_rigged")
    {
        this->use(ctx);
        _tex = this->add_sampler("tex");
        _ntex = this->add_sampler("ntex");
        _stex = this->add_sampler("stex");
        _mvp = this->add_uniform("mvp");
        _eye = this->add_uniform("gEyeWorldPos");
        _ambient_additive = this->add_uniform("gAmbientAdditive");
        _light_direction = this->add_uniform("gLightDirection");
        _quat_reals = this->add_uniform("quat_reals");
        _quat_duals = this->add_uniform("quat_duals");
        _vpos = this->add_attrib("av4position");
        _vnor = this->add_attrib("av3normal");
        _vtan = this->add_attrib("av3tangent");
        _vuvs = this->add_attrib("av2texcoord");
        _vbi = this->add_attrib("av3bindices");
        _vbw = this->add_attrib("av3bweights");
        _itfm = this->add_attrib("av4transform");
        this->unuse(ctx);
    }

    void disable_all_vertex_attribs() const
    {
        this->disable_attrib(_vpos);
        this->disable_attrib(_vnor);
        this->disable_attrib(_vtan);
        this->disable_attrib(_vuvs);
        this->disable_attrib(_vbi);
        this->disable_attrib(_vbw);
        this->disable_attrib(_itfm);
        this->disable_attrib(_itfm + 1);
        this->disable_attrib(_itfm + 2);
        this->disable_attrib(_itfm + 3);
    }

    using material = gdt::material<GRAPHICS>;

    const rigged_pipeline& set_material(const material& solid_mat) const
    {
        this->bind_sampler(_tex, solid_mat.diffuse);
        this->bind_sampler(_ntex, solid_mat.normal);
        this->bind_sampler(_stex, solid_mat.specular);
        return *this;
    }

    void bind_vertex_attribs() const
    {
        this->bind_float_attrib(_vpos,
                                3,          // vec3
                                24,         // total floats per vertex
                                (void*)0);  // position in vertex
        this->bind_float_attrib(_vnor,
                                3,                            // vec2
                                24,                           // total floats per vertex
                                (void*)(sizeof(float) * 3));  // position in vertex
        this->bind_float_attrib(_vtan,
                                3,                            // vec2
                                24,                           // total floats per vertex
                                (void*)(sizeof(float) * 6));  // position in vertex
        this->bind_float_attrib(_vuvs,
                                2,                             // vec2
                                24,                            // total floats per vertex
                                (void*)(sizeof(float) * 12));  // position in vertex
        this->bind_float_attrib(_vbi,
                                3,                             // vec2
                                24,                            // total floats per vertex
                                (void*)(sizeof(float) * 18));  // position in vertex
        this->bind_float_attrib(_vbw,
                                3,                             // vec2
                                24,                            // total floats per vertex
                                (void*)(sizeof(float) * 21));  // position in vertex
    }

    void bind_instances() const
    {
        this->bind_instances_data(_itfm);
    }

    const rigged_pipeline& set_modelview(gdt::math::mat4 mvp) const
    {
        this->bind_uniform(_mvp, mvp);
        return *this;
    }

    template <typename CAMERA>
    const rigged_pipeline& set_camera(const CAMERA & c) const
    {
        set_eyepos(c.entity().pos);
        set_modelview(c.entity().proj * c.get_transformable().get_transforms()[0]);
        return *this;
    }
    const rigged_pipeline& set_eyepos(gdt::math::vec3 eye) const
    {
        this->bind_uniform(_eye, eye);
        return *this;
    }
    const rigged_pipeline& set_light_direction(gdt::math::vec3 dir) const
    {
        this->bind_uniform(_light_direction, dir);
        return *this;
    }
    const rigged_pipeline& set_ambient_additive(float aa) const
    {
        this->bind_uniform(_ambient_additive, aa);
        return *this;
    }
    void bind_reals(math::vec4* reals, int c) const
    {
        this->bind_uniform(_quat_reals, reals, c);
    }
    void bind_duals(math::vec4* duals, int c) const
    {
        this->bind_uniform(_quat_duals, duals, c);
    }

    void enable_vertex_attributes() const  // override
    {
        this->bind_vertex_attribs();
    };

    template <typename SOMETHING>
    const rigged_pipeline& draw(const SOMETHING& what) const
    {
        what.get_animatable().bind(*this);
        what.get_drawable().draw_instances(this->adhoc_context(),
                                           *this,
                                           what.get_transformable().get_transforms(),
                                           what.get_transformable().size());
        return *this;
    }

    template <typename CONTEXT, typename ENTITY>
    const rigged_pipeline& user_draw(const CONTEXT& ctx, const ENTITY& what) const
    {
        what.draw(ctx, *this);
        return *this;
    }
};

template <typename GRAPHICS>
class fxaa_pipeline : public pipeline<GRAPHICS, fxaa_pipeline<GRAPHICS>>{
  private:
    typename GRAPHICS::base_pipeline::sampler _buf;
    typename GRAPHICS::base_pipeline::uniform _buf_size;

  public:
    fxaa_pipeline(const graphics_context<GRAPHICS>& ctx) : pipeline<GRAPHICS, fxaa_pipeline<GRAPHICS>>("res/shaders/fxaa")
    {
        this->use(ctx);
        _buf = this->add_sampler("s2d_buf");
        _buf_size = this->add_uniform("uv2_framebufsize");
    }

    void disable_all_vertex_attribs() const
    {
    }

    const fxaa_pipeline& bind_input(const typename GRAPHICS::back_buffer& bb) const
    {
        this->bind_sampler(_buf, &bb._color_buffer);
        return *this;
    }

    const fxaa_pipeline& set_buf_size(math::vec2 size) const
    {
        this->bind_uniform(_buf_size, size);
        return *this;
    }
};
}
#endif  // SRC_CONSTRUCTS_SHADERS_HH_INCLUDED

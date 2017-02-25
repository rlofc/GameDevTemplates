#ifndef BRICKS_OPENGL_OPENGL_SHADERS_HH_INCLUDED
#define BRICKS_OPENGL_OPENGL_SHADERS_HH_INCLUDED

#include "opengl_surface.hh"

namespace gdt::graphics::opengl {

class opengl_render_pass_clear_cmd {
  public:
    opengl_render_pass_clear_cmd() {}
    void apply(math::vec4 c={0.9, 0.9, 0.9, 0.0}) const
    {
        GL_CHECK(glClearColor(c.x, c.y, c.z, c.w));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
    }
};

template <typename GRAPHICS, typename PLATFORM>
class opengl_pipeline : public blueprints::graphics::pipeline<GRAPHICS> {
    friend struct opengl_surface<GRAPHICS>;

  private:
    mutable GRAPHICS * _graphics;
    void check_shader(GLuint shader) const;
    void check_program(GLuint program) const;
    void load_shaders(GLuint *shader, std::string buf, GLint shader_type);

  protected:
    GLuint shader_program, shader_frag, shader_vert, last_program;

  public:
    opengl_pipeline(std::string fragment, std::string vertex);
    opengl_pipeline(std::string name);
    virtual ~opengl_pipeline();

    void use(const graphics_context<GRAPHICS> &ctx) const;
    void unuse(const graphics_context<GRAPHICS> &ctx) const;

    using sampler = GLint;
    using uniform = GLint;
    using attrib = GLint;

    opengl_pipeline(const opengl_pipeline &) = delete;
    opengl_pipeline &operator=(const opengl_pipeline &) = delete;

    sampler add_sampler(std::string id) const
    {
        GLint ret = GL_CHECK(glGetUniformLocation(this->shader_program, id.c_str()));
        if (ret == -1) LOG_WARNING << "cannot find shader sampler named: " << id;
        return ret;
    }

    uniform add_uniform(std::string id) const
    {
        GLint ret = GL_CHECK(glGetUniformLocation(this->shader_program, id.c_str()));
        if (ret == -1) LOG_WARNING <<"cannot find shader uniform named: " << id;
        return ret;
    }

    attrib add_attrib(std::string id) const
    {
        GLint ret = GL_CHECK(glGetAttribLocation(this->shader_program, id.c_str()));
        if (ret == -1) LOG_WARNING << "cannot find shader attribute named: " << id;
        return ret;
    }

    void bind_sampler(sampler id, const opengl_color_buffer<GRAPHICS> *t) const
    {
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + t->unit));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, t->tex));
        GL_CHECK(glUniform1i(id, t->unit));
    }

    void bind_uniform(uniform id, gdt::math::mat4 mat) const
    {
        float buf[16];
        gdt::math::mat4_to_array(mat, buf);
        GL_CHECK(glUniformMatrix4fv(id, 1, GL_FALSE, buf));
    }

    void bind_uniform(uniform id, gdt::math::vec4 v) const
    {
        float buf[4];
        math::vec4_to_array(v, buf);
        GL_CHECK(glUniform4fv(id, 1, buf));
    }

    void bind_uniform(uniform id, gdt::math::vec3 v) const
    {
        float buf[3];
        math::vec3_to_array(v, buf);
        GL_CHECK(glUniform3fv(id, 1, buf));
    }

    void bind_uniform(uniform id, gdt::math::vec2 v) const
    {
        float buf[2];
        math::vec2_to_array(v, buf);
        GL_CHECK(glUniform2fv(id, 1, buf));
    }

    void bind_uniform(uniform id, std::vector<gdt::math::vec4> v) const
    {
        float buf[4 * v.size()];
        float *ptr = buf;
        for (auto vec : v) {
            math::vec4_to_array(vec, ptr += 4);
        }
        GL_CHECK(glUniform4fv(id, v.size(), buf));
    }

    void bind_uniform(uniform id, gdt::math::vec4 *v, int c) const
    {
        float buf[4 * c];
        float *ptr = buf;
        for (int j = 0; j < c; j++) {
            math::vec4_to_array(v[j], ptr);
            ptr += 4;
        }
        GL_CHECK(glUniform4fv(id, c, buf));
    }

    void bind_uniform(uniform id, float v) const
    {
        float buf[3];
        GL_CHECK(glUniform1f(id, v));
    }

    void bind_float_attrib(attrib id, int count, int stride, void *ptr) const
    {
        GL_CHECK(
            glVertexAttribPointer(id, count, GL_FLOAT, GL_FALSE, sizeof(float) * stride, ptr));
        GL_CHECK(glEnableVertexAttribArray(id));
    }

    void bind_input(const opengl_render_pass_clear_cmd &cmd) const
    {
        cmd.apply();
    }

    void disable_attrib(attrib id) const
    {
        glDisableVertexAttribArray(id);
    }

    void bind_instances_data(attrib id) const
    {
        for (unsigned int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(id + i);
            glVertexAttribPointer(id + i, 4, GL_FLOAT, GL_FALSE, sizeof(gdt::math::mat4),
                                  (const GLvoid *)(sizeof(GLfloat) * i * 4));
            glVertexAttribDivisor(id + i, 1);
        }
    }
  protected:
    graphics_context<GRAPHICS> adhoc_context() const {
        return graphics_context<GRAPHICS>{_graphics};
    }
};

class opengl_clearable {
  public:
    void bind_input(const opengl_render_pass_clear_cmd &cmd)
    {
        cmd.apply();
    }
};

template <typename GRAPHICS, typename PLATFORM>
opengl_pipeline<GRAPHICS, PLATFORM>::opengl_pipeline(std::string fragment, std::string vertex)
{
    LOG_DEBUG << "initializing opengl pipeline with fragment: " << fragment
              << " vertex: " << vertex;
    load_shaders(&this->shader_vert, PLATFORM::read_file(vertex), GL_VERTEX_SHADER);
    load_shaders(&this->shader_frag, PLATFORM::read_file(fragment), GL_FRAGMENT_SHADER);
    this->shader_program = GL_CHECK(glCreateProgram());
    GL_CHECK(glAttachShader(this->shader_program, this->shader_vert));
    GL_CHECK(glAttachShader(this->shader_program, this->shader_frag));
    GL_CHECK(glLinkProgram(this->shader_program));
}

template <typename GRAPHICS, typename PLATFORM>
opengl_pipeline<GRAPHICS, PLATFORM>::opengl_pipeline(std::string base_name)
    : opengl_pipeline(base_name + ".frag", base_name + ".vert")
{
}

template <typename GRAPHICS, typename PLATFORM>
opengl_pipeline<GRAPHICS, PLATFORM>::~opengl_pipeline()
{
    glDetachShader(this->shader_program, this->shader_frag);
    glDetachShader(this->shader_program, this->shader_vert);
    glDeleteProgram(this->shader_program);
    glDeleteShader(this->shader_frag);
    glDeleteShader(this->shader_vert);
}

template <typename GRAPHICS, typename PLATFORM>
void opengl_pipeline<GRAPHICS, PLATFORM>::load_shaders(GLuint *shader, std::string buf,
                                                     GLint shader_type)
{
    *shader = GL_CHECK(glCreateShader(shader_type));
    const char *cbuf = buf.c_str();
    GL_CHECK(glShaderSource(*shader, 1, &cbuf, NULL));
    GL_CHECK(glCompileShader(*shader));
    check_shader(*shader);
}

template <typename GRAPHICS, typename PLATFORM>
void opengl_pipeline<GRAPHICS, PLATFORM>::check_shader(GLuint shader) const
{
    GLint status = 1;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char *infoLog = (char *)malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            LOG_ERROR << "Error compiling shader: " << infoLog;
            free(infoLog);
            exit(1);
        }
    }
}

template <typename GRAPHICS, typename PLATFORM>
void opengl_pipeline<GRAPHICS, PLATFORM>::check_program(GLuint program) const
{
    GLint status = 1;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char *infoLog = (char *)malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(program, infoLen, NULL, infoLog);
            LOG_ERROR << "Error linking program: " << infoLog;
            free(infoLog);
            exit(1);
        }
    }
}

template <typename GRAPHICS, typename PLATFORM>
void opengl_pipeline<GRAPHICS, PLATFORM>::use(const graphics_context<GRAPHICS> &ctx) const
{
    GL_CHECK(glGetIntegerv(GL_CURRENT_PROGRAM, (GLint *)&this->last_program));
    GL_CHECK(glUseProgram(this->shader_program));
    // This is our opportunity to store the graphics backend for any consecutive
    // calls to this pipeline;
    _graphics = ctx.graphics;
}

template <typename GRAPHICS, typename PLATFORM>
void opengl_pipeline<GRAPHICS, PLATFORM>::unuse(const graphics_context<GRAPHICS> &ctx) const
{
    GL_CHECK(glUseProgram(this->last_program));
    _graphics = nullptr;
}

template <typename GRAPHICS, typename PLATFORM>
class opengl_filter_pipeline : public opengl_pipeline<GRAPHICS, PLATFORM>,
                             public opengl_clearable,
                             public screen::subscriber {
    enum e_textures { G_COLOR, NUM_OF_TEXTURES };
    GLint samplers[1];

  public:
    opengl_filter_pipeline(const graphics_context<GRAPHICS> &ctx, std::string frag);
    virtual ~opengl_filter_pipeline();

    using opengl_clearable::bind_input;
    void bind_input(const opengl_back_buffer<GRAPHICS> &gb) const
    {
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, gb._color_buffer.tex);
        glUniform1i(samplers[0], 0);
    }

    void on_screen_resize(unsigned int w, unsigned int h) override
    {
    }
};

template <typename GRAPHICS, typename PLATFORM>
opengl_filter_pipeline<GRAPHICS, PLATFORM>::opengl_filter_pipeline(const graphics_context<GRAPHICS> &ctx,
                                                             std::string frag)
    : opengl_pipeline<GRAPHICS, PLATFORM>(std::string("res/shaders/") + frag + ".frag", "res/shaders/filter.vert")
{
    this->samplers[G_COLOR] = GL_CHECK(glGetUniformLocation(this->shader_program, "gColor"));
}

template <typename GRAPHICS, typename PLATFORM>
opengl_filter_pipeline<GRAPHICS, PLATFORM>::~opengl_filter_pipeline()
{
}

}

#endif  // backends/opengl/opengl_pipelines_hh_INCLUDED

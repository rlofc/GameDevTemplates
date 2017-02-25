#ifndef BRICKS_OPENGL_OPENGL_BUFFER_HH_INCLUDED
#define BRICKS_OPENGL_OPENGL_BUFFER_HH_INCLUDED

#include <vector>

#include "loader.hh"

namespace gdt::graphics::opengl {

template <typename B>
struct opengl_color_buffer {
    static int counter;
    GLuint tex;
    GLint unit;
    unsigned int width;
    unsigned int height;

    opengl_color_buffer(const graphics_context<B> &ctx);

    virtual ~opengl_color_buffer();

    virtual void create(unsigned int w, unsigned int h)
    {
    }
};

template <typename GRAPHICS>
struct opengl_rgba_buffer : opengl_color_buffer<GRAPHICS> {
    opengl_rgba_buffer(const graphics_context<GRAPHICS> &ctx)
        : opengl_color_buffer<GRAPHICS>(ctx){};
    virtual ~opengl_rgba_buffer()
    {
    }

    void create(unsigned int w, unsigned int h) override
    {
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, this->tex));
        GL_CHECK(
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        this->width = w;
        this->height = h;
    }
};

template <typename GRAPHICS>
struct opengl_rgb16_buffer : opengl_color_buffer<GRAPHICS> {
    opengl_rgb16_buffer(const graphics_context<GRAPHICS> &ctx)
        : opengl_color_buffer<GRAPHICS>(ctx){};
    virtual ~opengl_rgb16_buffer()
    {
    }

    void create(unsigned int w, unsigned int h) override
    {
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, this->tex));
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, NULL));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        this->width = w;
        this->height = h;
    }
};

template <typename B>
int opengl_color_buffer<B>::counter = 0;

template <typename B>
struct opengl_texture : opengl_color_buffer<B> {
    opengl_texture(const graphics_context<B> &ctx, std::string filename);
    void create_texture(const char *filename);
};

template <typename B>
opengl_color_buffer<B>::opengl_color_buffer(const graphics_context<B> &ctx)
{
    this->unit = opengl_color_buffer<B>::counter;
    opengl_color_buffer<B>::counter++;
    GL_CHECK(glGenTextures(1, &this->tex));
}

template <typename B>
opengl_texture<B>::opengl_texture(const graphics_context<B> &ctx, std::string filename)
    : opengl_color_buffer<B>(ctx)
{
    create_texture(filename.c_str());
}

template <typename B>
opengl_color_buffer<B>::~opengl_color_buffer()
{
    GL_CHECK(glDeleteTextures(1, &tex));
}

template <typename B>
void opengl_texture<B>::create_texture(const char *filename)
{
    unsigned char *img;
    unsigned int ww, hh;
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, this->tex));
    if (load_png_for_texture(&img, &ww, &hh, filename)) {
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_CHECK(
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ww, hh, 0, GL_RGBA, GL_UNSIGNED_BYTE, img));
        free(img);
        this->width = ww;
        this->height = hh;
    }
}

template <typename GRAPHICS>
class opengl_base_buffer {
  protected:
    GLuint _buffer = 0;
    unsigned int _w, _h;

  public:
    opengl_base_buffer()
    {
    }
    virtual ~opengl_base_buffer()
    {
    }
    unsigned int width() const
    {
        return _w;
    }
    unsigned int height() const
    {
        return _h;
    }
    void bind() const
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _buffer));
    }
    void bind_for_read() const
    {
        GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, _buffer));
    }
    void bind_for_draw() const
    {
        GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _buffer));
    }
    void copy_depth_from(const opengl_base_buffer<GRAPHICS> &b) const
    {
        b.bind_for_read();
        bind_for_draw();
        glBlitFramebuffer(0, 0, b.width(), b.height(), 0, 0, b.width(), b.height(),
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        bind();
    }
    virtual void resize(unsigned int w, unsigned int h)
    {
        _w = w;
        _h = h;
    }
};

template <typename GRAPHICS>
class opengl_frame_buffer : public opengl_base_buffer<GRAPHICS> {
  protected:
    int n_attachments = 0;

  public:
    opengl_frame_buffer()
    {
        GL_CHECK(glGenFramebuffers(1, &this->_buffer));
    };
    virtual ~opengl_frame_buffer()
    {
        GL_CHECK(glDeleteFramebuffers(1, &this->_buffer));
    }
    GLuint attach(const opengl_color_buffer<GRAPHICS> &cb)
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, this->_buffer));
        GLuint aid = GL_COLOR_ATTACHMENT0 + n_attachments++;
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, aid, GL_TEXTURE_2D, cb.tex, 0));
        return aid;
    }
    virtual void resize(unsigned int w, unsigned int h) override
    {
        opengl_base_buffer<GRAPHICS>::resize(w, h);
        for (auto *b : _attachments) {
            b->create(w, h);
        }
    }
    std::vector<opengl_color_buffer<GRAPHICS> *> _attachments;
    void attachments(std::vector<opengl_color_buffer<GRAPHICS> *> attachments, unsigned int w,
                     unsigned int h)
    {
        std::vector<GLuint> attach_ids;
        for (auto *b : attachments) {
            b->create(w, h);
            attach_ids.push_back(this->attach(*b));
        }

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, this->_buffer));
        GL_CHECK(glDrawBuffers(attach_ids.size(), &attach_ids.at(0)));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        _attachments = attachments;
    }
};

template <typename GRAPHICS>
class opengl_depth_frame_buffer : public opengl_frame_buffer<GRAPHICS> {
    GLuint rbo_depth;

  public:
    opengl_depth_frame_buffer(const graphics_context<GRAPHICS> &ctx, unsigned int w,
                              unsigned int h)
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, this->_buffer));
        GL_CHECK(glGenRenderbuffers(1, &rbo_depth));
        GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth));
        GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h));
        GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                           rbo_depth));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    virtual void resize(unsigned int w, unsigned int h)
    {
        opengl_frame_buffer<GRAPHICS>::resize(w, h);
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, this->_buffer));
        GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth));
        GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    virtual ~opengl_depth_frame_buffer()
    {
        LOG_DEBUG << "cleaning up frame buffer";
        GL_CHECK(glDeleteRenderbuffers(1, &rbo_depth));
    }
};

template <typename GRAPHICS>
class opengl_screen_buffer : public opengl_base_buffer<GRAPHICS> {
};

template <typename GRAPHICS>
class opengl_g_buffer : public opengl_depth_frame_buffer<GRAPHICS> {
  public:
    opengl_rgb16_buffer<GRAPHICS> _position;
    opengl_rgb16_buffer<GRAPHICS> _normal;
    opengl_rgba_buffer<GRAPHICS> _albedospec;
    opengl_rgba_buffer<GRAPHICS> _fog;

    opengl_g_buffer(const graphics_context<GRAPHICS> &ctx, unsigned int w, unsigned int h)
        : opengl_depth_frame_buffer<GRAPHICS>(ctx, w, h),
          _position(ctx),
          _normal(ctx),
          _albedospec(ctx),
          _fog(ctx)
    {
        this->attachments({&_position, &_normal, &_albedospec, &_fog}, w, h);
    }
    virtual ~opengl_g_buffer()
    {
    }
};

template <typename GRAPHICS>
class opengl_back_buffer : public opengl_depth_frame_buffer<GRAPHICS> {
  public:
    opengl_rgba_buffer<GRAPHICS> _color_buffer;

    opengl_back_buffer(const graphics_context<GRAPHICS> &ctx, unsigned int w, unsigned int h)
        : opengl_depth_frame_buffer<GRAPHICS>(ctx, w, h), _color_buffer(ctx)
    {
        this->attachments({&_color_buffer}, w, h);
    }

    virtual ~opengl_back_buffer()
    {
    }
};
}
#endif  // BRICKS_OPENGL_OPENGL_BUFFER_HH_INCLUDED

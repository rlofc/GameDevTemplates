#ifndef GDT_CAMERA_HEADER_INCLUDED
#define GDT_CAMERA_HEADER_INCLUDED

#include "imgui/imgui.h"
#include "math.hh"
#include "traits.hh"
#include "screen.hh"

namespace gdt {

/**
 * Basic point of view data.
 */
struct pov {
    math::vec3 pos;   // position of view point
    math::vec3 tgt;   // position of target (not direction vector)
    math::mat4 proj;  // projection matrix
};

/**
 * A basic perspective camera.
 */
class camera : public pov,
               private screen::subscriber,
               public is_entity<camera>,
               public is_drivable<camera> {
  public:
    template <typename CONTEXT>
    camera(const CONTEXT& ctx, screen* screen, float fov = 0.28f, float near = 0.1,
           float far = 1024.0f);
    void imgui();

  protected:
    float _fov, _near, _far;

  private:
    void on_screen_resize(unsigned int width, unsigned int height) override;
};

/**
 * The basic 2D camera, X spanning from -1.0 to 1.0.
 */
class camera2d : public camera {
  public:
    template <typename CONTEXT>
    camera2d(const CONTEXT& ctx, screen* screen, float fov = 0.95f, float near = 0.1,
             float far = 1024.0f);
  private:
    void on_screen_resize(unsigned int width, unsigned int height) override;
};

/**
 * Pixel-space 2D camera, X spanning from -width/2 to width/2.
 */
class camera2dpp : public camera {
  public:
    template <typename CONTEXT>
    camera2dpp(const CONTEXT& ctx, screen* screen, float fov = 0.95f, float near = 0.1,
               float far = 1024.0f);
  private:
    float _width, _height;
    void on_screen_resize(unsigned int width, unsigned int height) override;
};

template <typename CONTEXT>
camera::camera(const CONTEXT& ctx, screen* screen, float fov, float near, float far)
    : _fov(fov), _near(near), _far(far)
{
    pos = math::vec3(0.0f, 0.0f, 0.0f);
    proj = math::mat4();
    tgt = math::vec3();
    screen->subscribe(this);
}

template <typename CONTEXT>
camera2d::camera2d(const CONTEXT& ctx, screen* screen, float fov, float near, float far)
    : camera(ctx, screen, fov, near, far)
{
}

template <typename CONTEXT>
camera2dpp::camera2dpp(const CONTEXT& ctx, screen* screen, float fov, float near, float far)
    : camera(ctx, screen, fov, near, far)
{
}


};
#endif  // GDT_CAMERA_HEADER_INCLUDED

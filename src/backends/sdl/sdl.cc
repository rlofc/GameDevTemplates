#include "screen.hh"
#include "sdl.hh"
#include "imgui/imgui.h"
#include "imgui_impl_sdl.h"

namespace gdt::platform::sdl {

bool platform_is_sdl::is_key_pressed(key k) const
{
    return _kbstate[YAGL_TO_SDL_KEY[k]];
}

void platform_is_sdl::update_keyboard()
{
    _kbstate = SDL_GetKeyboardState(NULL);
}

void platform_is_sdl::update_mouse()
{
    mstate = SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
}

void platform_is_sdl::get_mouse(int* x, int* y) const
{
    *x = mouse_x;
    *y = mouse_y;
}

bool platform_is_sdl::is_button_pressed() const
{
    return (mstate & SDL_BUTTON(1));
}

bool platform_is_sdl::capture_mouse() const
{
     if (ImGui::IsMouseHoveringAnyWindow()==false && ImGui::GetIO().WantCaptureMouse == false) {
         SDL_SetRelativeMouseMode(SDL_TRUE);
         return true;
     } else return false;
}

void platform_is_sdl::release_mouse() const
{
     SDL_SetRelativeMouseMode(SDL_FALSE);
}

platform_is_sdl::platform_is_sdl(Uint32 ext_flags)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::runtime_error("Unable to initialize SDL");
    }
    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(0, &mode);
#ifdef ANDROID
    _screen.w = mode.w;
    _screen.h = mode.h;
#else
    _screen.w = (int)(mode.h - 200) / 1.5;
    _screen.h = mode.h - 200;
#endif
    float zoom = 1.0f;
    _mainwindow = SDL_CreateWindow("Simple texture moving", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, _screen.w, _screen.h,
                                   ext_flags | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!_mainwindow) {
        std::runtime_error("Unable to create window");
    }
#ifdef ANDROID
    SDL_SetWindowFull_screen(_mainwindow, SDL_TRUE);
#endif
}

platform_is_sdl::~platform_is_sdl()
{
    SDL_DestroyWindow(_mainwindow);
    SDL_Quit();
}

bool platform_is_sdl::process_events()
{
    bool done = false;
    SDL_Event event;
    while (SDL_PollEvent(&event) && !done) {
        if (ImGui_ImplSdl_ProcessEvent(&event)) continue;
        switch (event.type) {
            case SDL_QUIT:
                done = true;
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                        on_window_resize(&event);
                        break;
                }
            case SDL_KEYDOWN:
                on_key_callback(event.key.keysym.sym);
                break;
        }
    }
    return !done;
}

void platform_is_sdl::create_window()
{
}

std::string platform_is_sdl::read_file(std::string filename)
{
    std::string buf;
    SDL_RWops* sdlIO = SDL_RWFromFile(filename.c_str(), "r");
    if (sdlIO != NULL) {
        char data;
        std::size_t amountRead;
        while ((amountRead = SDL_RWread(sdlIO, &data, 1, 1)) > 0) {
            buf += data;
        }

        SDL_RWclose(sdlIO);
    }
    return buf;
};

backend_for_opengl::backend_for_opengl() : platform_is_sdl(SDL_WINDOW_OPENGL)
{
    //    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
    //    SDL_GL_CONTEXT_PROFILE_ES);
    //    TODO: Figure out how to deal with instanced rendering fallback in
    //    opengles2

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    maincontext = SDL_GL_CreateContext(_mainwindow);
    if (!maincontext) {
        std::runtime_error("Unable to create GL context");
    }
    ImGui_ImplSdl_Init(_mainwindow);
    glViewport(0, 0, _screen.w, _screen.h);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(_mainwindow);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GL_CHECK(glEnable(GL_CULL_FACE));
    GL_CHECK(glEnable(GL_DEPTH_TEST));
}

backend_for_opengl::~backend_for_opengl()
{
    ImGui_ImplSdl_Shutdown();
    SDL_GL_DeleteContext(maincontext);
}

void backend_for_opengl::on_window_resize(SDL_Event const* event)
{
    SDL_GL_GetDrawableSize(_mainwindow, &_screen.w, &_screen.h);
    on_resize_callback(_screen.w, _screen.h);
}

void backend_for_opengl::update_window()
{
    SDL_GL_SwapWindow(_mainwindow);
}
void backend_for_opengl::imgui_frame() {
   glActiveTexture(GL_TEXTURE0);
   glUseProgram(0);
   ImGui_ImplSdl_NewFrame(_mainwindow);
}
}

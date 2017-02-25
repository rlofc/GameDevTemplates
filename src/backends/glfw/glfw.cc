#define BACKEND_IS_GLFW

#include <fstream>
#include <iostream>

#include "checks.hh"
#include "glfw.hh"
#include "glfw_opengl.hh"
#include "screen.hh"
#include "imgui/imgui.h"
#include "imgui_impl_glfw.h"

namespace gdt::platform::glfw {

static int GDT_TO_GLFW_KEY[1024] =
    {[key::A] = GLFW_KEY_A,         [key::B] = GLFW_KEY_B,
     [key::C] = GLFW_KEY_C,         [key::D] = GLFW_KEY_D,
     [key::E] = GLFW_KEY_E,         [key::F] = GLFW_KEY_F,
     [key::G] = GLFW_KEY_G,         [key::H] = GLFW_KEY_H,
     [key::I] = GLFW_KEY_I,         [key::J] = GLFW_KEY_J,
     [key::K] = GLFW_KEY_K,         [key::L] = GLFW_KEY_L,
     [key::M] = GLFW_KEY_M,         [key::N] = GLFW_KEY_N,
     [key::O] = GLFW_KEY_O,         [key::P] = GLFW_KEY_P,
     [key::Q] = GLFW_KEY_Q,         [key::R] = GLFW_KEY_R,
     [key::S] = GLFW_KEY_S,         [key::T] = GLFW_KEY_T,
     [key::U] = GLFW_KEY_U,         [key::V] = GLFW_KEY_V,
     [key::W] = GLFW_KEY_W,         [key::X] = GLFW_KEY_X,
     [key::Y] = GLFW_KEY_Y,         [key::Z] = GLFW_KEY_Z,
     [key::N1] = GLFW_KEY_1,        [key::N2] = GLFW_KEY_2,
     [key::N3] = GLFW_KEY_3,        [key::N4] = GLFW_KEY_4,
     [key::N5] = GLFW_KEY_5,        [key::N6] = GLFW_KEY_6,
     [key::N7] = GLFW_KEY_7,        [key::N8] = GLFW_KEY_8,
     [key::N9] = GLFW_KEY_9,        [key::N0] = GLFW_KEY_0,
     [key::UP] = GLFW_KEY_UP,       [key::DOWN] = GLFW_KEY_DOWN,
     [key::LEFT] = GLFW_KEY_LEFT,   [key::RIGHT] = GLFW_KEY_RIGHT,
     [key::ENTER] = GLFW_KEY_ENTER, [key::TAB] = GLFW_KEY_TAB,
     [key::ESC] = GLFW_KEY_ESCAPE,  [key::BACKSPACE] = GLFW_KEY_BACKSPACE,
     [key::SPACE] = GLFW_KEY_SPACE

};

bool platform_is_glfw::is_key_pressed(key k) const
{
    int state = glfwGetKey(s_pWindow, GDT_TO_GLFW_KEY[k]);
    return (state == GLFW_PRESS);
}

void platform_is_glfw::update_keyboard()
{
    //    _kbstate = SDL_GetKeyboardState(NULL);
}

void platform_is_glfw::update_mouse()
{
    //  mstate = SDL_GetRelativeMouseState(&mouse_x, &mouse_y);
}

void platform_is_glfw::get_mouse(int *x, int *y) const
{
    double xx, yy;
    static double bx = 0;
    static double by = 0;
    glfwGetCursorPos(s_pWindow, &xx, &yy);
    int state = glfwGetMouseButton(s_pWindow, GLFW_MOUSE_BUTTON_LEFT);
    int x1 = bx - xx;
    int y1 = by - yy;
    bx = xx;
    by = yy;
    *x = x1 * -1;
    *y = y1 * -1;
}

bool platform_is_glfw::is_button_pressed() const
{
    int state = glfwGetMouseButton(s_pWindow, GLFW_MOUSE_BUTTON_LEFT);
    return (state == GLFW_PRESS);
}
static void on_window_resize(GLFWwindow *win, int w, int h)
{
    platform_is_glfw *pp = static_cast<platform_is_glfw *>(glfwGetWindowUserPointer(win));
    pp->screen().w = w;
    pp->screen().h = h;
    pp->on_resize_callback(pp->screen().w, pp->screen().h);
}

bool platform_is_glfw::capture_mouse() const
{
     if (ImGui::IsMouseHoveringAnyWindow()==false && ImGui::GetIO().WantCaptureMouse == false) {
        glfwSetInputMode(s_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
         return true;
     } else return false;
}

void platform_is_glfw::release_mouse() const
{
    glfwSetInputMode(s_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

platform_is_glfw::platform_is_glfw(bool no_api)
{
    if (glfwInit() != 1) {
        throw std::runtime_error("error initializing GLFW");
    }

    int Major, Minor, Rev;

    glfwGetVersion(&Major, &Minor, &Rev);

    LOG_DEBUG << "GLFW " << Major << "." << Minor << "." << Rev << " initialized";


    GLFWmonitor *pMonitor = false ? glfwGetPrimaryMonitor() : NULL;

    if (no_api) glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    s_pWindow = glfwCreateWindow(640, 480, "", pMonitor, NULL);
    _screen.w = 640;
    _screen.h = 480;
    glfwSetWindowUserPointer(s_pWindow, this);
    if (!s_pWindow) {
        throw std::runtime_error("error creating window");
    }

    glfwSetWindowSizeCallback(s_pWindow, on_window_resize);
    ImGui_ImplGlfw_Init(s_pWindow, true);
}

platform_is_glfw::~platform_is_glfw()
{
    ImGui_ImplGlfw_Shutdown();
    glfwDestroyWindow(s_pWindow);
    glfwTerminate();
}

bool platform_is_glfw::process_events()
{
    glfwPollEvents();
    return true;
}
void platform_is_glfw::create_window()
{
}
void platform_is_glfw::update_window()
{
}

std::string platform_is_glfw::read_file(std::string filename)
{
    std::ifstream infile{filename};
    std::string file_contents{std::istreambuf_iterator<char>(infile),
                              std::istreambuf_iterator<char>()};
    return file_contents;
}

backend_for_opengl::backend_for_opengl() : platform_is_glfw(false)
{
    glfwMakeContextCurrent(s_pWindow);

    glViewport(0, 0, _screen.w, _screen.h);
    glClear(GL_COLOR_BUFFER_BIT);
    this->update_window();
    glClear(GL_COLOR_BUFFER_BIT);
    GL_CHECK(glEnable(GL_DEPTH_TEST));
}
backend_for_opengl::~backend_for_opengl()
{
}

void backend_for_opengl::update_window()
{
    glfwSwapBuffers(s_pWindow);
}
void backend_for_opengl::imgui_frame() {
   glActiveTexture(GL_TEXTURE0);
   glUseProgram(0);
    ImGui_ImplGlfw_NewFrame();
}
}

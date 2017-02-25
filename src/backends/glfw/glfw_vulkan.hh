#ifndef glfw_vulkan_hh_INCLUDED
#define glfw_vulkan_hh_INCLUDED

#define GLFW_INCLUDE_VULKAN
#include "glfw.hh"

namespace gdt::platform::glfw {
class platform_is_glfw_for_vulkan : public platform_is_glfw {
public:
    platform_is_glfw_for_vulkan() {}
    virtual ~platform_is_glfw_for_vulkan() {}
    void create_surface(VkInstance instance,VkSurfaceKHR * surf) {
        if (glfwCreateWindowSurface(instance, getwnd(), nullptr,
                                    surf) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
};
};

#endif // glfw_vulkan_hh_INCLUDED


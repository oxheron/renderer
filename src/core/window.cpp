#include "window.h"

// external
#include "GLFW/glfw3.h"
#ifdef __linux__
#define GLFW_EXPOSE_NATIVE_X11
#elif _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#elif __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include "GLFW/glfw3native.h"

// std
#include <stdexcept>

static GLFWwindow* window = nullptr;
int window_width = 0;
int window_height = 0;
bool window_resized = false;

void window_init(const std::string& title, int width, int height)
{
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window_width = width;
    window_height = height;

    window = glfwCreateWindow(window_width, window_height, title.c_str(), nullptr, nullptr);
    if (!window)
        throw std::runtime_error("Failed to create GLFW window");
}

void window_update()
{
    glfwPollEvents();
    
    // Handle resizing
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (width != window_width || height != window_height) window_resized = true;
    else window_resized = false;
    window_width = width;
    window_height = height;
}

bool window_should_close()
{
    return glfwWindowShouldClose(window);
}

void* get_native_window()
{
#ifdef __linux__
    return (void*) glfwGetX11Window(window);
#elif _WIN32
    return (void*) glfwGetWin32Window(window);
#elif __APPLE__
    return (void*) glfwGetCocoaWindow(window);
#endif
    return nullptr;
}

void* get_native_display()
{
#ifdef __linux__
    return (void*) glfwGetX11Display();
#endif
    return nullptr;
}
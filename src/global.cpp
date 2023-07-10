#include "global.h"

// internal
#include "core/window.h"
#include "core/bgfx_handler.h"

// std
#include <iostream>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) global->kb[key].update_pressed();
    else if (action == GLFW_RELEASE) global->kb[key].update_released();
}

void Global::init()
{
    window_init("bgfx"); 
    allocator = new bx::DefaultAllocator();
    bgfx = new BGFXHandler(get_native_window(), get_native_display(), window_width, window_height, allocator);
    glfwSetKeyCallback((GLFWwindow*) get_window(), key_callback); 
}

Global::~Global()
{
    delete bgfx;
    delete allocator;
}

Global* global;

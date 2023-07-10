#pragma once

// internal
#include "core/window.h"

// external
#include <GLFW/glfw3.h>

class Mouse
{
private:
    double x, y;
    double last_x, last_y;
public:
    Mouse() = default;
    Mouse(double sx, double sy)
        : x(sx), y(sy), last_x(sx), last_y(sy)
    {}

    void set_mode(int mode) { glfwSetInputMode((GLFWwindow*) get_window(), GLFW_CURSOR, mode); }

    void update(double x, double y)
    {
        last_x = this->x;
        last_y = this->y;
        this->x = x;
        this->y = y;
    }

    void poll_mouse()
    {
        last_x = this->x;
        last_y = this->y;
        glfwGetCursorPos((GLFWwindow*) get_window(), &x, &y);
    }

    double get_x() const { return x; }
    double get_y() const { return y; }
    double get_last_x() const { return last_x; }
    double get_last_y() const { return last_y; }
    double get_delta_x() const { return last_x - x; }
    double get_delta_y() const { return last_y - y; }  
};

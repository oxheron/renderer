#pragma once

// std
#include <string>

extern int window_width;
extern int window_height;
extern bool window_resized;

// The init function for the window, not for bgfx
void window_init(const std::string& title, int width = 1280, int height = 720);

// The update function for the window, not for bgfx
void window_update();

// If the window should close
bool window_should_close();

// The native window handle
void* get_native_window();

// The native display handle
void* get_native_display();
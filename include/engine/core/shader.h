#pragma once

// external
#include <bgfx/bgfx.h>

// std
#include <string>

bgfx::ShaderHandle load_shader(const std::string& name);

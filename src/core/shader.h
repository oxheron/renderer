#pragma once

// external
#include <bgfx/bgfx.h>

// std
#include <string>

bgfx::ShaderHandle load_shader(const std::string& name);
bgfx::ProgramHandle load_program(const std::string& vertex, 
    const std::string& fragment);

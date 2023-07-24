#pragma once

// external
#include <bx/bx.h>
#include <bgfx/bgfx.h>

// std
#include <string>

std::string read_file(const std::string& filepath);
bgfx::Memory* read_file_raw(const std::string& filepath);
void write_file(const std::string& filepath, const std::string& data);
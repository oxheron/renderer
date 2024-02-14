#include "util.h"

// std
#include <fstream>
#include <string>

std::string read_file(const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Cannot find file " + filepath);

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string str(size, '\0');
    file.read(str.data(), size);
    file.close();

    return str;
}

// Allocates raw memory for a file and returns a pointer to it
bgfx::Memory* read_file_raw(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios_base::binary | std::ios_base::ate);
    if (!file.is_open()) throw std::runtime_error("Cannot find file " + filepath);

    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    bgfx::Memory* data = (bgfx::Memory*) bgfx::alloc(size);
    file.read((char*) data->data, size);
    file.close();

    return data;
}

void write_file(const std::string& filepath, const std::string& data)
{
    std::ofstream file(filepath, std::ios_base::trunc);
    file.write(data.data(), data.size());
    file.close();
}

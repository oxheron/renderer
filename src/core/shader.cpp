#include "shader.h"

// std
#include <fstream>
#include <stdexcept>

#define SHADER_DIR "resources/shaders"

bgfx::ShaderHandle load_shader(const std::string& name) 
{
    std::string type; 
    switch (bgfx::getRendererType())
	{
		case bgfx::RendererType::Direct3D9:
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12:
			type = "s_5_0";
			break;
		case bgfx::RendererType::OpenGL:
			type = "440";
			break;
		case bgfx::RendererType::Vulkan:
			type = "spirv";
			break;
		case bgfx::RendererType::Metal:
			type = "metal";
			break;
		default:
			throw std::runtime_error("Unsupported renderer type");
	 }


    std::string path = std::string(SHADER_DIR) + std::string("/") + type + std::string("/") + name + std::string(".bin");
    char* data = nullptr;
    std::ifstream file;
    size_t size = 0;

    file.open(path);
    if (file.is_open()) 
    {
        file.seekg(0, std::ios::end);
        size = (size_t) file.tellg() + 1;
        file.seekg(0, std::ios::beg);
		data = new char[size];
        file.read(data, size);
        file.close();
    }
    else
    {
    	throw std::runtime_error("Cannot find file " + path);
    }

    const bgfx::Memory* mem = bgfx::copy(data, size);
    mem->data[mem->size - 1] = 0;
    bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, name.c_str());
	if (data) delete[] data;
    return handle;
}

bgfx::ProgramHandle create_program(const std::string& vertex_path, 
    const std::string& fragment_path)
{
    bgfx::ShaderHandle vsh = load_shader(vertex_path);
    bgfx::ShaderHandle fsh = load_shader(fragment_path);
    return bgfx::createProgram(vsh, fsh, true);
}

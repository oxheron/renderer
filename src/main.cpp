// internal
#include "core/window.h"
#include "core/bgfx_handler.h"
#include "global.h"
#include "texture/texture.h"
#include "model/mesh.h"
#include "world/camera.h"

// external
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
#include <bx/math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define SHADER_DIR "resources/shaders"

bgfx::ShaderHandle loadShader(const std::string& name) 
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

int main(void)
{   
    global.init();
    window_init("bgfx");

    BGFXHandler bgfx_handler(get_native_window(), get_native_display(), window_width, window_height);

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);

    StandardMesh mesh;
    mesh.load_model("resources/models/cube.gltf");
    mesh.load_texture("resources/textures/square_swirls.png");
    mesh.set_handles();

    bgfx::ShaderHandle vsh = loadShader("vertex.vs");
    bgfx::ShaderHandle fsh = loadShader("fragment.fs");
    bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, true);
    
    bgfx::touch(0);

    Camera camera;

    while(!window_should_close()) 
    {
        global.tm.update();
		window_update();
        bgfx_handler.update();

        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, window_width, window_height);

        glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float) window_width / (float) window_height, 0.1f, 100.0f);
		bgfx::setViewTransform(0, glm::value_ptr(camera.get_view()), &proj);

        glm::mat4 model = glm::mat4(1.0f);
        mesh.set_modelmat(model);
        
        mesh.draw(program);

        bgfx::frame();
        global.tm.hold_at_fps();
        if (global.tm.is_second()) std::cout << global.tm.get_fps() << std::endl;
    }

    bgfx::destroy(program);

    return 0;
}
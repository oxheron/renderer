#include "core/window.h"
#include "core/bgfx_handler.h"
#include "global.h"
#include "texture/texture.h"
#include "model/mesh.h"
#include "world/camera.h"
#include "core/shader.h"
#include "renderer/batch.h"

// external
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/bx.h>
#include <bx/math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <iostream>
#include <string>
#include <vector>

int main(void)
{   
    GlobalInitalizer init;
    // global->mouse.set_mode(GLFW_CURSOR_DISABLED);

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f, 0);
    bgfx::VertexLayout layout;
    bgfx::VertexLayout model_layout;

    layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    model_layout.begin()
        .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord2, 4, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord3, 4, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .end();

    BatchManager manager(2048, 2048, layout, model_layout, "fill_compute.cs");
    manager.set_program("batch_vertex.vs", "batch_fragment.fs");

    StandardMesh mesh;
    mesh.set_batchmanager(&manager);
    mesh.load_model("resources/models/cube.gltf");
    // mesh.load_texture("resources/textures/square_swirls.png");
    glm::mat4 model = glm::mat4(1.0f);
    mesh.set_modelmat(model);
    mesh.upload();

    StandardMesh mesh2;
    mesh2.set_batchmanager(&manager);
    mesh2.load_model("resources/models/cube.gltf");
    mesh2.load_texture("resources/textures/square_swirls.png");
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 5.0f));
    mesh2.set_modelmat(model);
    mesh2.upload();

    bgfx::touch(0);

    Camera camera;

    while(!window_should_close()) 
    {
        if (global->kb[GLFW_KEY_W].held) camera.movez(0.000000001 * global->tm.get_delta_time().count());
        if (global->kb[GLFW_KEY_S].held) camera.movez(-0.000000001 * global->tm.get_delta_time().count());
        if (global->kb[GLFW_KEY_A].held) camera.movex(-0.00000001 * global->tm.get_delta_time().count());
        if (global->kb[GLFW_KEY_D].held) camera.movex(0.00000001 * global->tm.get_delta_time().count());
        if (global->kb[GLFW_KEY_ESCAPE].get_pressed()) break;
        
        float sens = 0.1f;
        float x_change = global->mouse.get_delta_x() * sens;
        float y_change = global->mouse.get_delta_y() * sens;

        // camera.add_yaw(x_change);
        // camera.add_pitch(y_change);

        global->tm.update();
        global->mouse.poll_mouse();
		window_update();
        global->bgfx->update();

        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, window_width, window_height);

        glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float) window_width / (float) window_height, 0.1f, 100.0f);
		bgfx::setViewTransform(0, glm::value_ptr(camera.get_view()), glm::value_ptr(proj));

        manager.draw();
        
        bgfx::frame();
        global->tm.hold_at_fps();
        if (global->tm.is_second()) std::cout << global->tm.get_fps() << std::endl;
    }

    return 0;
}
  

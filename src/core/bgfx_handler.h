#pragma once

// external 
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"

class BGFXHandler
{
public:
    BGFXHandler(void* window, void* display, int width, int height, bgfx::RendererType::Enum type = bgfx::RendererType::Count);
    ~BGFXHandler() { bgfx::shutdown(); }

    void update();
    void resize(int width, int height);
private:
    void init(const bgfx::PlatformData& pd, int width, int height, bgfx::RendererType::Enum type);
};
#include "bgfx_handler.h"

// internal
#include "core/window.h"
#include "global.h"

BGFXHandler::BGFXHandler(void* window, void* display, int width, int height, bgfx::RendererType::Enum type)
{
    bgfx::PlatformData pd;
    pd.nwh = window;
    pd.ndt = display;
    init(pd, width, height, type);
}

void BGFXHandler::init(const bgfx::PlatformData& pd, int width, int height, bgfx::RendererType::Enum type)
{
    bgfx::Init bgfxInit;
    bgfxInit.type = type;
    bgfxInit.resolution.width = width;
    bgfxInit.resolution.height = height;
    bgfxInit.resolution.reset = BGFX_RESET_NONE;
    bgfxInit.platformData = pd;
    bgfxInit.allocator = global.allocator;
    bgfx::init(bgfxInit);
}

void BGFXHandler::update()
{
    if (window_resized)
    {
        resize(window_width, window_height);
        window_resized = false;
    }
}

void BGFXHandler::resize(int width, int height)
{
    bgfx::reset(width, height, BGFX_RESET_NONE);
}


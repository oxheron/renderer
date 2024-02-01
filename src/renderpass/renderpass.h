#pragma once

// external
#include <bgfx/bgfx.h>

// Doesn't hold program (as one subpass could contain many draws)

class SubpassManager
{
private:
    bgfx::ViewId current = 0;
public:
    // Create a temporary pass
    inline bgfx::ViewId get_pass(bgfx::FrameBufferHandle handle)
    {
        if (current == 255) render();
        setViewFrameBuffer(current, handle);
        return current++;
    }

    // Render
    inline void render() 
    {
        bgfx::frame();
        current = 0;
    };
};

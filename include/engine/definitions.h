#include <bgfx/bgfx.h>

bgfx::VertexLayout pos_tex_norm()
{
    static bgfx::VertexLayout layout;
    static bool initialized = false;
    if (!initialized)
    {
        layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
            .end();
        initialized = true;
    }

    return layout;
}

bgfx::VertexLayout model_tex_instance()
{
    static bgfx::VertexLayout layout;
    static bool initialized = false;
    if (!initialized)
    {
       layout.begin() 
            .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord2, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord3, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord4, 4, bgfx::AttribType::Float)
            .end();   
    }

    return layout;
}

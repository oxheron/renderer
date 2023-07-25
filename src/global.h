#pragma once

// internal
#include "core/bgfx_handler.h"
#include "util/timer.h"
#include "core/input/keyboard.h"
#include "core/input/mouse.h"
#include "renderer/batch.h"

// external
#include <bx/allocator.h>

// Handles all global things, such as allocators, etc.
struct Global
{
    BGFXHandler* bgfx; 
    TimeManager tm;
    Keyboard kb;
    Mouse mouse;

    bx::AllocatorI* allocator;

    void init();
    ~Global();
};

extern Global* global;

class GlobalInitializer
{
public:
    GlobalInitializer() { global = new Global; global->init(); }
    ~GlobalInitializer() { delete global; }
};

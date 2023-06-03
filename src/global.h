#pragma once

// internal
#include "util/timer.h"

// external
#include <bx/allocator.h>

// Handles all global things, such as alllocators, etc.
struct Global
{
    TimeManager tm;

    bx::AllocatorI* allocator;

    void init();
};

extern Global global;
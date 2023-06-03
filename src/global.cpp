#include "global.h"

void Global::init()
{
    allocator = new bx::DefaultAllocator();
}

Global global;
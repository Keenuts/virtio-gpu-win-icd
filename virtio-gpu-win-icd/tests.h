#pragma once

#include "win_types.h"

#define EXPORT extern "C"

namespace Tests
{
    extern bool test_enabled;

    EXPORT int WINAPI initialize_test_mode(const char* path);
    EXPORT void WINAPI finish_tests();

    void dumpCommandBuffer(VOID *data, UINT32 size);
}

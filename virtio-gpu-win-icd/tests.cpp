#include <cassert>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "debug.h"
#include "driver_api.h"
#include "virgl.h"
#include "win_types.h"

namespace Tests
{
    //Used on the driver_api.cpp to call dumpCommandBuffer or Escape function
    bool test_enabled = false;
    std::ofstream *output_file;

    int WINAPI initialize_test_mode(const char* path)
    {
        TRACE_IN();

        test_enabled = true;

        output_file = new std::ofstream();
        output_file->open(path, std::ios::out | std::ios::binary);

        sendCommand(NULL, 0);
        assert(output_file->is_open());
        if (!output_file->is_open())
            test_enabled = false;

        TRACE_OUT();
        return !test_enabled;
    }

    void WINAPI finish_tests()
    {
        TRACE_IN();

        if (!test_enabled)
            return;

        output_file->close();
        delete output_file;
        test_enabled = false;

        TRACE_OUT();
    }

    void dumpCommandBuffer(VOID *data, UINT32 size)
    {
        TRACE_IN();

        output_file->write((char*)&size, sizeof(UINT32));
        output_file->write((char*)data, size);

        TRACE_OUT();
    }
}

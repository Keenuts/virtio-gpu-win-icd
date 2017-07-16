#pragma once

#include "debug.h"

#define STOP()                                                                      \
    do {                                                                            \
        DbgPrint(TRACE_LEVEL_SEVERE, ("[!] %s: Will stop now.\n", __FUNCTION__));   \
        volatile int *a = 0;                                                        \
        volatile int b = *a + 2;                                                    \
        UNREFERENCED_PARAMETER(b);                                                  \
    } while (0)
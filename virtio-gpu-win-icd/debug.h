#pragma once

#include <cassert>
#include <stdio.h>

#define TRACE_LEVEL_INFO 0
#define TRACE_LEVEL_WARNING 1
#define TRACE_LEVEL_ERROR 2
#define TRACE_LEVEL_SEVERE 3

#define TRACE_LEVEL TRACE_LEVEL_SEVERE

//Warning disabled: constant comparaison
#define DbgPrint(Level, Line)                              \
    do {                                                   \
    __pragma(warning(push))                                \
    __pragma(warning(disable:4127))                        \
        if ((Level) >= TRACE_LEVEL)                        \
            printf Line;                                   \
    __pragma(warning(pop))                                 \
        FlushFileBuffers(GetStdHandle(STD_OUTPUT_HANDLE)); \
    } while (0)

#define TRACE_IN() \
    DbgPrint(TRACE_LEVEL_INFO, ("--> %s\n", __FUNCTION__))

#define TRACE_OUT() \
    DbgPrint(TRACE_LEVEL_INFO, ("<-- %s\n", __FUNCTION__))

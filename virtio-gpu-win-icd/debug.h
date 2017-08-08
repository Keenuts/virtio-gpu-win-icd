#pragma once

#include <cassert>
#include <stdio.h>

#define TRACE_LEVEL_VERBOSE 0
#define TRACE_LEVEL_INFO 1
#define TRACE_LEVEL_WARNING 2
#define TRACE_LEVEL_ERROR 3
#define TRACE_LEVEL_SEVERE 4

#ifdef _DEBUG
    #define TRACE_LEVEL TRACE_LEVEL_WARNING
#else
    #define TRACE_LEVEL TRACE_LEVEL_VERBOSE
#endif

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
    DbgPrint(TRACE_LEVEL_VERBOSE, ("--> %s\n", __FUNCTION__))

#define TRACE_OUT() \
    DbgPrint(TRACE_LEVEL_VERBOSE, ("<-- %s\n", __FUNCTION__))

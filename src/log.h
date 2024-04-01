#pragma once

#include <Windows.h>
#include <cstdarg>
#include <cstdio>

inline void debugLog(const char *format, ...) {
    const int bufferSize = 4096;
    char buffer[bufferSize];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, bufferSize, format, args);
    va_end(args);
    strcat_s(buffer, bufferSize, "\n");

#if GLBAMBOOZLE_LOG_TO_CONSOLE
    puts(buffer);
#else
   OutputDebugStringA(buffer);
#endif
}

// TODO eliminate warnings when there are no va args
#define DEBUG_LOG(str, ...)         \
    do {                            \
        debugLog(str, __VA_ARGS__); \
    } while (false)

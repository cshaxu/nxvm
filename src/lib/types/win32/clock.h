#ifndef LIB_TYPES_WIN32_CLOCK_H
#define LIB_TYPES_WIN32_CLOCK_H

#include <windows.h>

typedef LARGE_INTEGER lib_win32_counter;
#define lib_win32_query_performance_counter QueryPerformanceCounter
#define lib_win32_query_performance_frequency QueryPerformanceFrequency

#endif

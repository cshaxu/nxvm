#ifndef LIB_TYPES_LINUX_CLOCK_H
#define LIB_TYPES_LINUX_CLOCK_H

#include <time.h>

typedef struct timespec lib_linux_timespec;
#define lib_linux_clock_gettime clock_gettime

typedef time_t lib_linux_time_t;
#define lib_linux_nanosleep nanosleep
#define LIB_LINUX_CLOCK_REALTIME CLOCK_REALTIME
#define LIB_LINUX_CLOCK_MONOTONIC CLOCK_MONOTONIC

#endif

#ifndef LIB_TYPES_LINUX_PROCESS_H
#define LIB_TYPES_LINUX_PROCESS_H

#include <limits.h>
#include <unistd.h>

typedef ssize_t lib_linux_ssize_t;

#define LIB_LINUX_SSIZE_MAX SSIZE_MAX
#define lib_linux_readlink readlink

#endif

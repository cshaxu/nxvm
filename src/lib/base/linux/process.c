#include "lib/base/process.h"

#include <unistd.h>

lib_status base_process_platform_executable_path(char *output, lib_size capacity)
{
    ssize_t length;

    if (output == LIB_NULL || capacity < 2u || capacity > (lib_size)SSIZE_MAX)
        return LIB_STATUS_INVALID_ARGUMENT;
    length = readlink("/proc/self/exe", output, capacity - 1u);
    if (length < 0 || (lib_size)length >= capacity - 1u) return LIB_STATUS_IO_ERROR;
    output[length] = '\0';
    return LIB_STATUS_OK;
}

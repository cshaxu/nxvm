#include "lib/base/process.h"
#include "lib/types/linux/process.h"

lib_status base_process_platform_executable_path(char *output, lib_size capacity)
{
    lib_linux_ssize_t length;

    if (output == LIB_NULL || capacity < 2u || capacity > (lib_size)LIB_LINUX_SSIZE_MAX)
        return LIB_STATUS_INVALID_ARGUMENT;
    length = lib_linux_readlink("/proc/self/exe", output, capacity - 1u);
    if (length < 0 || (lib_size)length >= capacity - 1u) return LIB_STATUS_IO_ERROR;
    output[length] = '\0';
    return LIB_STATUS_OK;
}

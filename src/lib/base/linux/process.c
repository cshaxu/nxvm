#include "lib/base/process.h"
#include "lib/types/linux/process.h"
#include "lib/base/text_interface.h"

lib_status base_process_platform_executable_directory(char *output, lib_size capacity)
{
    lib_linux_ssize_t length;
    char *separator;

    if (output == LIB_NULL || capacity < 2u || capacity > (lib_size)LIB_LINUX_SSIZE_MAX)
        return LIB_STATUS_INVALID_ARGUMENT;
    length = lib_linux_readlink("/proc/self/exe", output, capacity);
    if (length < 0) return LIB_STATUS_IO_ERROR;
    if ((lib_size)length >= capacity) return LIB_STATUS_LIMIT_EXCEEDED;
    output[length] = '\0';
    if (!base_text_utf8_valid(output)) return LIB_STATUS_UNSUPPORTED;
    separator = output + length;
    while (separator != output && separator[-1] != '/') --separator;
    if (separator == output) return LIB_STATUS_IO_ERROR;
    if (separator > output + 1) --separator;
    *separator = '\0';
    return LIB_STATUS_OK;
}

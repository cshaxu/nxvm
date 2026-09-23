#include "lib/base/process.h"

#include <windows.h>

lib_status base_process_platform_executable_path(char *output, lib_size capacity)
{
    DWORD length;

    if (output == LIB_NULL || capacity > 0xffffffffu) return LIB_STATUS_INVALID_ARGUMENT;
    length = GetModuleFileNameA(NULL, output, (DWORD)capacity);
    if (length == 0u || length >= capacity) return LIB_STATUS_IO_ERROR;
    return LIB_STATUS_OK;
}

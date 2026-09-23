#include "lib/base/process.h"
#include "lib/types/win32/process.h"

lib_status base_process_platform_executable_path(char *output, lib_size capacity)
{
    lib_win32_dword length;

    if (output == LIB_NULL || capacity > 0xffffffffu) return LIB_STATUS_INVALID_ARGUMENT;
    length = lib_win32_get_module_file_name_a(LIB_NULL, output, (lib_win32_dword)capacity);
    if (length == 0u || length >= capacity) return LIB_STATUS_IO_ERROR;
    return LIB_STATUS_OK;
}

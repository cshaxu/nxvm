#include "lib/types/types_interface.h"
#include "app-nxvm/product/startup.h"

#include "lib/base/process_interface.h"

lib_status vm_app_ini_executable_path(lib_u8 *path, lib_size capacity)
{
    lib_size length;

    if (path == LIB_NULL || capacity < sizeof("NXVM.ini") + 1u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (base_process_executable_directory(path, capacity) != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    length = lib_text_length(path);
    if (length + 1u + sizeof("NXVM.ini") > capacity) return LIB_STATUS_INTERNAL_ERROR;
    path[length] = '\\';
    lib_memory_copy(path + length + 1u, "NXVM.ini", sizeof("NXVM.ini"));
    return LIB_STATUS_OK;
}

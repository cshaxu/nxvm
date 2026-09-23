#include "lib/types/types_interface.h"
#include "app-nxvm/product/startup.h"

#include "lib/base/process_interface.h"

type_status vm_app_ini_executable_path(C_CHAR *path, lib_size capacity)
{
    lib_size length;

    if (path == LIB_NULL || capacity < sizeof("NXVM.ini") + 1u)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (base_process_executable_directory(path, capacity) != LIB_STATUS_OK)
        return TYPE_STATUS_FAULT;
    length = lib_text_length(path);
    if (length + 1u + sizeof("NXVM.ini") > capacity) return TYPE_STATUS_FAULT;
    path[length] = '\\';
    lib_memory_copy(path + length + 1u, "NXVM.ini", sizeof("NXVM.ini"));
    return TYPE_STATUS_OK;
}

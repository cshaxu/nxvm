#include "app-nxvm/product/startup.h"

#include "lib/base/process_interface.h"

type_status vm_app_ini_executable_path(C_CHAR *path, STD_SIZE_T capacity)
{
    STD_SIZE_T length;

    if (path == STD_NULL || capacity < sizeof("NXVM.ini") + 1u)
        return TYPE_STATUS_INVALID_ARGUMENT;
    if (base_process_executable_directory(path, capacity) != LIB_STATUS_OK)
        return TYPE_STATUS_FAULT;
    length = STD_STRLEN(path);
    if (length + 1u + sizeof("NXVM.ini") > capacity) return TYPE_STATUS_FAULT;
    path[length] = '\\';
    STD_MEMCPY(path + length + 1u, "NXVM.ini", sizeof("NXVM.ini"));
    return TYPE_STATUS_OK;
}

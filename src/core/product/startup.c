#include "lib/types/types_interface.h"
#include "core/product/startup_interface.h"
#include "core/product/ini_interface.h"

#include "lib/base/process_interface.h"
#include "lib/types/file.h"

lib_status vm_app_ini_executable_path(const char *name, lib_u8 *path,
    lib_size capacity)
{
    lib_size length;
    lib_size name_length;

    if (name == LIB_NULL || path == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    name_length = lib_text_length(name);
    if (name_length == 0u || name_length + 2u > capacity)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (base_process_executable_directory((char *)path, capacity) != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    length = lib_text_length((const char *)path);
    if (length + 1u + name_length + 1u > capacity) return LIB_STATUS_INTERNAL_ERROR;
    path[length] = '\\';
    lib_memory_copy(path + length + 1u, name, name_length + 1u);
    return LIB_STATUS_OK;
}

lib_status vm_app_ini_load_request(const char *name,
    vm_session_request *out_request)
{
    lib_u8 path[1024];
    lib_status status;

    if (out_request == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_app_ini_executable_path(name, path, sizeof(path));
    if (status != LIB_STATUS_OK)
        return status;
    return vm_app_ini_load(path, out_request);
}

lib_i32 vm_app_run(const char *name, const char *banner,
    const vm_app_machine_binding *binding)
{
    vm_session_request request = {0};
    product_surface_definition definition = {0};

    if (name == LIB_NULL || banner == LIB_NULL || binding == LIB_NULL ||
        vm_app_ini_load_request("nxvm.ini", &request) != LIB_STATUS_OK ||
        vm_app_configure_ui(&request, &definition.ui) != LIB_STATUS_OK ||
        vm_app_compose_machine(binding, &request, &definition.machine) != LIB_STATUS_OK)
        return 1;
    definition.name = name;
    definition.banner = banner;
    definition.configure_extensions = vm_app_configure_standard_extensions;
    return product_surface_run(&definition);
}

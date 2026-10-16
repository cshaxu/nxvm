#include "lib/types/types_interface.h"
#include "x86-nxvm/product/startup_interface.h"
#include "x86-nxvm/product/ini_interface.h"
#include "x86-nxvm/product/version.h"

#include "lib/base/process_interface.h"
#include "lib/types/file.h"

/* The run-scoped NXVM descriptor owns the real machine. The PC shell only
 * borrows it for lifetime callbacks and extension configuration. Driver
 * callbacks retain their original machine context. */
static lib_status x86_nxvm_product_bind_composed(void *opaque, emulator_machine *emulator)
{
    x86_nxvm_product_machine *machine = opaque;
    return machine->composition.bind(machine->composition.machine, emulator);
}

static lib_status x86_nxvm_product_destroy_composed(void *opaque)
{
    x86_nxvm_product_machine *machine = opaque;
    return machine->composition.destroy(machine->composition.machine);
}

static lib_status x86_nxvm_product_configure_extensions(app_composed_machine *machine,
    x86_ibmpc_product_command_extensions *extensions)
{
    return x86_nxvm_product_configure_standard_extensions(machine->composition.machine,
        extensions);
}

lib_status x86_nxvm_product_ini_executable_path(const char *name, lib_u8 *path,
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
    if (length != 0u && path[length - 1u] != '/' && path[length - 1u] != '\\') {
        if (length + 1u + name_length + 1u > capacity) return LIB_STATUS_INTERNAL_ERROR;
        path[length++] = '\\';
    }
    if (length + name_length + 1u > capacity) return LIB_STATUS_INTERNAL_ERROR;
    lib_memory_copy(path + length, name, name_length + 1u);
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_product_ini_load_request(const char *name,
    x86_nxvm_product_request *out_request)
{
    lib_u8 path[1024];
    lib_status status;

    if (out_request == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_product_ini_executable_path(name, path, sizeof(path));
    if (status != LIB_STATUS_OK)
        return status;
    return x86_nxvm_product_ini_load(path, out_request);
}

lib_i32 x86_nxvm_product_run(const char *name, const char *ini_name,
    const x86_nxvm_product_machine_binding *binding)
{
    x86_nxvm_product_request request = {0};
    x86_ibmpc_product_definition definition = {0};
    x86_nxvm_product_machine machine = {0};
    char banner[256];
    lib_i32 written;

    if (name == LIB_NULL || binding == LIB_NULL ||
        x86_nxvm_product_ini_load_request(ini_name, &request) != LIB_STATUS_OK ||
        x86_nxvm_product_configure_ui(&request, &definition.ui) != LIB_STATUS_OK ||
        x86_nxvm_product_compose_machine(binding, &request, &machine) != LIB_STATUS_OK)
        return 1;
    written = lib_c_snprintf(banner, sizeof(banner), "%s [%s]\n%s", name,
        X86_NXVM_PRODUCT_VERSION, X86_NXVM_PRODUCT_COPYRIGHT);
    if (written < 0 || (lib_size)written >= sizeof(banner)) {
        (void)machine.composition.destroy(machine.composition.machine);
        return 1;
    }
    definition.machine.composition = machine.composition;
    definition.machine.composition.machine = &machine;
    definition.machine.composition.bind = x86_nxvm_product_bind_composed;
    definition.machine.composition.destroy = x86_nxvm_product_destroy_composed;
    definition.name = name;
    definition.banner = banner;
    definition.configure_extensions = x86_nxvm_product_configure_extensions;
    return x86_ibmpc_product_run(&definition);
}

/* Copyright 2012-2026 Neko. */
#include "lib/types/file.h"
#include "ibmpc/product/entry_interface.h"
#include "ibmpc/product/composition_interface.h"
#include "ibmpc/product/command_interface.h"
#include "ibmpc/product/startup_interface.h"

lib_i32 vm_app_run(const vm_app_definition *definition)
{
    vm_app *session = LIB_NULL;
    vm_app_console_context *console_context = LIB_NULL;
    lib_u8 ini_path[1024];
    lib_status status;
    lib_status destroy_status;

    if (definition == LIB_NULL || definition->name == LIB_NULL ||
        definition->version == LIB_NULL || definition->copyright == LIB_NULL ||
        definition->build_time == LIB_NULL) return 1;
    lib_c_printf("%s [%s]\n%s\n\nBuilt on %s\n", definition->name,
        definition->version, definition->copyright, definition->build_time);
    if (vm_app_ini_executable_path(ini_path, sizeof(ini_path)) != LIB_STATUS_OK) {
        lib_c_printf("Unable to determine NXVM.ini path.\n");
        return 1;
    }
    if (vm_app_create(&definition->factory, &session) != LIB_STATUS_OK ||
        vm_app_console_context_create(&console_context) != LIB_STATUS_OK) {
        (void)vm_app_destroy(session);
        return 1;
    }
    status = vm_app_console_main(console_context, session, ini_path);
    vm_app_console_context_destroy(console_context);
    destroy_status = vm_app_destroy(session);
    return status == LIB_STATUS_OK && destroy_status == LIB_STATUS_OK ? 0 : 1;
}

#include "x86-nxvm/product/startup_interface.h"
#include "x86-nxvm/product/ini_interface.h"

static lib_u8 candidate;
static lib_u32 binds;
static lib_u32 unbinds;
static lib_u32 destroys;
static lib_u32 runs;
static lib_bool reject_bind;
static lib_bool reject_configuration;
static lib_bool valid;
static const x86_nxvm_product_machine_binding binding = {.name = "fixture"};

static lib_status bind_machine(void *machine, emulator_machine *emulator)
{
    valid = valid && machine == &candidate;
    if (emulator == LIB_NULL) ++unbinds;
    else ++binds;
    return reject_bind && emulator != LIB_NULL ?
        LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

static lib_status destroy_machine(void *machine)
{
    valid = valid && machine == &candidate;
    ++destroys;
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_product_ini_load(const lib_u8 *path, x86_nxvm_product_request *request)
{
    lib_size length = lib_text_length((const char *)path);
    valid = valid && length >= 8u &&
        lib_text_compare((const char *)path + length - 8u, "nxvm.ini") == 0;
    *request = (x86_nxvm_product_request){.display = "window"};
    return reject_configuration ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

lib_status x86_nxvm_product_configure_ui(const x86_nxvm_product_request *request,
    app_composed_ui *ui)
{
    valid = valid && lib_text_compare((const char *)request->display, "window") == 0;
    *ui = (app_composed_ui){.display = EMULATOR_SESSION_DISPLAY_WINDOW};
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_product_compose_machine(const x86_nxvm_product_machine_binding *selected,
    const x86_nxvm_product_request *request, x86_nxvm_product_machine *machine)
{
    (void)request;
    valid = valid && selected == &binding;
    *machine = (x86_nxvm_product_machine){
        .composition = {.machine = &candidate,
            .driver.context = &candidate,
            .bind = bind_machine, .destroy = destroy_machine},
        .context = selected};
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_product_configure_standard_extensions(x86_nxvm_product_machine *machine,
    x86_ibmpc_product_command_extensions *extensions)
{
    valid = valid && machine->context == &binding &&
        machine->composition.machine == &candidate;
    *extensions = (x86_ibmpc_product_command_extensions){.context = machine};
    return LIB_STATUS_OK;
}

lib_i32 x86_ibmpc_product_run(const x86_ibmpc_product_definition *definition)
{
    x86_ibmpc_product_command_extensions extensions = {0};
    app_composed_machine composed = definition->machine;
    emulator_product_machine machine = definition->machine.composition;
    lib_status status;

    ++runs;
    valid = valid && lib_text_compare(definition->name, "fixture") == 0 &&
        definition->banner != LIB_NULL &&
        machine.driver.context == &candidate && machine.machine != &candidate;
    valid = valid && definition->configure_extensions(
        &composed, &extensions) == LIB_STATUS_OK &&
        extensions.context == machine.machine;
    status = machine.bind(machine.machine, (emulator_machine *)&candidate);
    if (status == LIB_STATUS_OK)
        valid = valid && machine.bind(machine.machine, LIB_NULL) == LIB_STATUS_OK;
    valid = valid && machine.destroy(machine.machine) == LIB_STATUS_OK;
    return status == LIB_STATUS_OK ? 0 : 1;
}

lib_i32 main(void)
{
    char long_name[300];

    valid = LIB_TRUE;
    if (x86_nxvm_product_run("fixture", "nxvm.ini", &binding) != 0 ||
        !valid || runs != 1u || binds != 1u || unbinds != 1u || destroys != 1u)
        return 1;
    reject_bind = LIB_TRUE;
    if (x86_nxvm_product_run("fixture", "nxvm.ini", &binding) != 1 ||
        !valid || runs != 2u || binds != 2u || unbinds != 1u || destroys != 2u)
        return 2;
    reject_configuration = LIB_TRUE;
    if (x86_nxvm_product_run("fixture", "nxvm.ini", &binding) != 1 || runs != 2u || destroys != 2u)
        return 3;
    reject_configuration = LIB_FALSE;
    lib_memory_set(long_name, 'x', sizeof(long_name) - 1u);
    long_name[sizeof(long_name) - 1u] = '\0';
    if (x86_nxvm_product_run(long_name, "nxvm.ini", &binding) != 1 ||
        !valid || runs != 2u || destroys != 3u)
        return 4;
    return 0;
}

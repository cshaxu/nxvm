#include "type.h"

#include "vm/composition/console_target.h"


#include "vm/composition/session_lifecycle.h"

#include "vm/composition/debug_target.h"

#include "vm/composition/providers.h"

#include "vm/composition/session_control.h"

#include "core/machine/memory.h"

#include "vm/machine/fdd.h"

#include "vm/machine/hdd.h"

#include "vm/machine/debug.h"

#include "vm/profile/default_profile/firmware/bios.h"

#include "vm/platform/platform.h"

#include "core/product/debug/debug.h"

static C_VOID vm_session_console_initialize(C_VOID *context)
{
    vm_session *machine = (vm_session *)context;
    vm_session_initialize(machine);
#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
    vm_machine_fdd_insert_for(machine->fdd, "d:/fd.img");
    vm_machine_hdd_insert(machine->hdd, "d:/hd.img");
#else
    vm_machine_fdd_insert_for(machine->fdd, "/Users/xha/fd.img");
    vm_machine_hdd_insert(machine->hdd, "/Users/xha/hd.img");
#endif
}

static C_VOID vm_session_console_finalize(C_VOID *context) { vm_session_finalize((vm_session *)context); }
static C_INT vm_session_console_is_running(C_VOID *context) { return vm_session_control_is_running(((vm_session *)context)->control); }
static C_VOID vm_session_console_print_machine(C_VOID *context) { vm_session_print_machine((vm_session *)context); }
static C_INT vm_session_console_get_window_display(C_VOID *context) { return vm_platform_run_context_get_window_display(((vm_session *)context)->platform_run_context); }
static C_VOID vm_session_console_set_window_display(C_VOID *context, C_INT enabled) { vm_platform_run_context_set_window_display(((vm_session *)context)->platform_run_context, enabled); }
static C_VOID vm_session_console_print_bios(C_VOID *context) { vm_profile_default_bios_print(((vm_session *)context)->default_bios); }
static C_VOID vm_session_console_print_status(C_VOID *context) { vm_session_control_print_status(((vm_session *)context)->control); }
static C_VOID vm_session_console_debug(C_VOID *context)
{
    vm_session *machine = (vm_session *)context;
    if (machine == STD_NULL) return;
    if (vm_session_control_is_running(machine->control)) {
        vm_session_control_request_pause(machine->control, VM_SESSION_PAUSE_EXPLICIT);
        if (!vm_session_control_wait_for_pause(machine->control, 2000u)) return;
    }
    core_product_debug_context_initialize(machine->debugger_context);
    machine->debugger_context->wait_scope = machine->wait_scope;
    static const core_product_debug_input_provider input_provider = {
        vm_session_debug_flush_console_input, STD_NULL
    };
    core_product_debug_main(machine->debugger_context,
        vm_session_debug_target(machine), &input_provider);
}
static C_VOID vm_session_console_record_start(C_VOID *context, const C_CHAR *path) { vm_machine_debug_record_start(((vm_session *)context)->debug, path); }
static C_VOID vm_session_console_record_stop(C_VOID *context) { vm_machine_debug_record_stop(((vm_session *)context)->debug); }
static C_VOID vm_session_console_set_boot_hdd(C_VOID *context, C_INT enabled) { vm_profile_default_bios_set_boot_hdd(((vm_session *)context)->default_bios, enabled); }
static C_VOID vm_session_console_set_memory(C_VOID *context, STD_SIZE_T bytes) { vm_session *machine = (vm_session *)context; if (machine != STD_NULL) core_machine_memory_allocate_for(vm_composition_machine_access_memory(machine->core_access), bytes); }
static C_VOID vm_session_console_create_fdd(C_VOID *context) { vm_machine_fdd_create_for(((vm_session *)context)->fdd); }
static C_INT vm_session_console_insert_fdd(C_VOID *context, const C_CHAR *path) { return vm_machine_fdd_insert_for(((vm_session *)context)->fdd, path); }
static C_INT vm_session_console_remove_fdd(C_VOID *context, const C_CHAR *path) { return vm_machine_fdd_remove_for(((vm_session *)context)->fdd, path); }
static C_VOID vm_session_console_create_hdd(C_VOID *context, uint16_t cylinders) { vm_machine_hdd_create(((vm_session *)context)->hdd, cylinders); }
static C_INT vm_session_console_insert_hdd(C_VOID *context, const C_CHAR *path) { return vm_machine_hdd_insert(((vm_session *)context)->hdd, path); }
static C_INT vm_session_console_remove_hdd(C_VOID *context, const C_CHAR *path) { return vm_machine_hdd_remove(((vm_session *)context)->hdd, path); }
static C_VOID vm_session_console_start(C_VOID *context) { vm_session_start((vm_session *)context); }
static C_VOID vm_session_console_reset(C_VOID *context) { vm_session_reset((vm_session *)context); }
static C_VOID vm_session_console_stop(C_VOID *context) { vm_session_stop((vm_session *)context); }
static C_VOID vm_session_console_resume(C_VOID *context) { vm_session_resume((vm_session *)context); }

static const nxvm_product_console_target vmCompositionConsoleTargetTemplate = {
    vm_session_console_initialize, vm_session_console_finalize,
    vm_session_console_is_running, vm_session_console_print_machine,
    vm_session_console_get_window_display, vm_session_console_set_window_display,
    vm_session_console_print_bios, vm_session_console_print_status,
    vm_session_console_debug, vm_session_console_record_start,
    vm_session_console_record_stop, vm_session_console_set_boot_hdd,
    vm_session_console_set_memory, vm_session_console_create_fdd,
    vm_session_console_insert_fdd, vm_session_console_remove_fdd,
    vm_session_console_create_hdd, vm_session_console_insert_hdd,
    vm_session_console_remove_hdd, vm_session_console_start,
    vm_session_console_reset, vm_session_console_stop,
    vm_session_console_resume, STD_NULL
};

C_VOID vm_session_console_target_initialize(
    nxvm_product_console_target *target,
    vm_session *machine)
{
    if (target == STD_NULL) return;
    *target = vmCompositionConsoleTargetTemplate;
    target->context = machine;
}

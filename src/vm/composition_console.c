#include "type.h"

#include "vm/composition_console.h"


#include "vm/composition_machine.h"

#include "vm/composition_debug.h"

#include "vm/composition.h"

#include "vm/composition_control.h"

#include "core/machine/memory.h"

#include "vm/machine/fdd.h"

#include "vm/machine/hdd.h"

#include "vm/machine/debug.h"

#include "vm/profile/default_profile/firmware/bios.h"

#include "vm/platform/platform.h"

#include "core/product/debug/debug.h"

static C_VOID vm_composition_console_initialize(C_VOID *context)
{
    vm_composition_live_machine *machine = (vm_composition_live_machine *)context;
    vm_composition_initialize(machine);
#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
    vm_machine_fdd_insert_for(machine->fdd, "d:/fd.img");
    vm_machine_hdd_insert(machine->hdd, "d:/hd.img");
#else
    vm_machine_fdd_insert_for(machine->fdd, "/Users/xha/fd.img");
    vm_machine_hdd_insert(machine->hdd, "/Users/xha/hd.img");
#endif
}

static C_VOID vm_composition_console_finalize(C_VOID *context) { vm_composition_finalize((vm_composition_live_machine *)context); }
static C_INT vm_composition_console_is_running(C_VOID *context) { return vm_composition_control_is_running(((vm_composition_live_machine *)context)->control); }
static C_VOID vm_composition_console_print_machine(C_VOID *context) { vm_composition_print_machine((vm_composition_live_machine *)context); }
static C_INT vm_composition_console_get_window_display(C_VOID *context) { return vm_platform_run_context_get_window_display(((vm_composition_live_machine *)context)->platform_run_context); }
static C_VOID vm_composition_console_set_window_display(C_VOID *context, C_INT enabled) { vm_platform_run_context_set_window_display(((vm_composition_live_machine *)context)->platform_run_context, enabled); }
static C_VOID vm_composition_console_print_bios(C_VOID *context) { vm_profile_default_bios_print(((vm_composition_live_machine *)context)->default_bios); }
static C_VOID vm_composition_console_print_status(C_VOID *context) { vm_composition_control_print_status(((vm_composition_live_machine *)context)->control); }
static C_VOID vm_composition_console_debug(C_VOID *context)
{
    vm_composition_live_machine *machine = (vm_composition_live_machine *)context;
    core_product_wait_scope previous;
    if (machine == STD_NULL) return;
    if (vm_composition_control_is_running(machine->control)) {
        vm_composition_control_request_pause(machine->control, VM_COMPOSITION_PAUSE_EXPLICIT);
        if (!vm_composition_control_wait_for_pause(machine->control, 2000u)) return;
    }
    previous = core_product_wait_scope_enter(machine->wait_scope);
    core_product_debug_context_initialize(machine->debugger_context);
    core_product_debug_main(machine->debugger_context, vm_composition_debug_target(machine));
    core_product_wait_scope_leave(previous);
}
static C_VOID vm_composition_console_record_start(C_VOID *context, const C_CHAR *path) { vm_machine_debug_record_start(((vm_composition_live_machine *)context)->debug, path); }
static C_VOID vm_composition_console_record_stop(C_VOID *context) { vm_machine_debug_record_stop(((vm_composition_live_machine *)context)->debug); }
static C_VOID vm_composition_console_set_boot_hdd(C_VOID *context, C_INT enabled) { vm_profile_default_bios_set_boot_hdd(((vm_composition_live_machine *)context)->default_bios, enabled); }
static C_VOID vm_composition_console_set_memory(C_VOID *context, STD_SIZE_T bytes) { vm_composition_live_machine *machine = (vm_composition_live_machine *)context; if (machine != STD_NULL) core_machine_memory_allocate_for(machine->ram, bytes); }
static C_VOID vm_composition_console_create_fdd(C_VOID *context) { vm_machine_fdd_create_for(((vm_composition_live_machine *)context)->fdd); }
static C_INT vm_composition_console_insert_fdd(C_VOID *context, const C_CHAR *path) { return vm_machine_fdd_insert_for(((vm_composition_live_machine *)context)->fdd, path); }
static C_INT vm_composition_console_remove_fdd(C_VOID *context, const C_CHAR *path) { return vm_machine_fdd_remove_for(((vm_composition_live_machine *)context)->fdd, path); }
static C_VOID vm_composition_console_create_hdd(C_VOID *context, uint16_t cylinders) { vm_machine_hdd_create(((vm_composition_live_machine *)context)->hdd, cylinders); }
static C_INT vm_composition_console_insert_hdd(C_VOID *context, const C_CHAR *path) { return vm_machine_hdd_insert(((vm_composition_live_machine *)context)->hdd, path); }
static C_INT vm_composition_console_remove_hdd(C_VOID *context, const C_CHAR *path) { return vm_machine_hdd_remove(((vm_composition_live_machine *)context)->hdd, path); }
static C_VOID vm_composition_console_start(C_VOID *context) { vm_composition_start((vm_composition_live_machine *)context); }
static C_VOID vm_composition_console_reset(C_VOID *context) { vm_composition_reset((vm_composition_live_machine *)context); }
static C_VOID vm_composition_console_stop(C_VOID *context) { vm_composition_stop((vm_composition_live_machine *)context); }
static C_VOID vm_composition_console_resume(C_VOID *context) { vm_composition_resume((vm_composition_live_machine *)context); }

static const nxvm_product_console_target vmCompositionConsoleTargetTemplate = {
    vm_composition_console_initialize, vm_composition_console_finalize,
    vm_composition_console_is_running, vm_composition_console_print_machine,
    vm_composition_console_get_window_display, vm_composition_console_set_window_display,
    vm_composition_console_print_bios, vm_composition_console_print_status,
    vm_composition_console_debug, vm_composition_console_record_start,
    vm_composition_console_record_stop, vm_composition_console_set_boot_hdd,
    vm_composition_console_set_memory, vm_composition_console_create_fdd,
    vm_composition_console_insert_fdd, vm_composition_console_remove_fdd,
    vm_composition_console_create_hdd, vm_composition_console_insert_hdd,
    vm_composition_console_remove_hdd, vm_composition_console_start,
    vm_composition_console_reset, vm_composition_console_stop,
    vm_composition_console_resume, STD_NULL
};

C_VOID vm_composition_console_target_initialize(
    nxvm_product_console_target *target,
    vm_composition_live_machine *machine)
{
    if (target == STD_NULL) return;
    *target = vmCompositionConsoleTargetTemplate;
    target->context = machine;
}

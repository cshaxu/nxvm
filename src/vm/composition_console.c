#include "vm/composition_console.h"

#include "vm/composition_machine.h"
#include "vm/composition.h"
#include "vm/composition_control.h"
#include "core/machine/memory.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/machine/debug.h"
#include "vm/profile/default_profile/firmware/bios.h"
#include "vm/platform/platform.h"
#include "core/product/debug/debug.h"

static void vm_composition_console_initialize(void *context)
{
    vm_composition_live_machine *machine = (vm_composition_live_machine *)context;
    machineInit(machine);
#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
    vm_machine_fdd_insert_for(machine->fdd, "d:/fd.img");
    vm_machine_hdd_insert(machine->hdd, "d:/hd.img");
#else
    vm_machine_fdd_insert_for(machine->fdd, "/Users/xha/fd.img");
    vm_machine_hdd_insert(machine->hdd, "/Users/xha/hd.img");
#endif
}

static void vm_composition_console_finalize(void *context) { machineFinal((vm_composition_live_machine *)context); }
static int vm_composition_console_is_running(void *context) { return vm_composition_control_is_running(((vm_composition_live_machine *)context)->control); }
static void vm_composition_console_print_machine(void *context) { devicePrintMachine((vm_composition_live_machine *)context); }
static int vm_composition_console_get_window_display(void *context) { (void)context; return platform.flagMode; }
static void vm_composition_console_set_window_display(void *context, int enabled) { (void)context; platform.flagMode = enabled; }
static void vm_composition_console_print_bios(void *context) { (void)context; vm_profile_default_bios_print(); }
static void vm_composition_console_print_status(void *context) { vm_composition_control_print_status(((vm_composition_live_machine *)context)->control); }
static void vm_composition_console_debug(void *context)
{
    (void)context;
    vm_composition_live_machine *machine = (vm_composition_live_machine *)context;
    if (vm_composition_control_is_running(machine->control)) {
        vm_composition_control_request_pause(machine->control, VM_COMPOSITION_PAUSE_EXPLICIT);
        if (!vm_composition_control_wait_for_pause(machine->control, 2000u)) return;
    }
    debugMain();
}
static void vm_composition_console_record_start(void *context, const char *path) { vm_machine_debug_record_start(((vm_composition_live_machine *)context)->debug, path); }
static void vm_composition_console_record_stop(void *context) { vm_machine_debug_record_stop(((vm_composition_live_machine *)context)->debug); }
static void vm_composition_console_set_boot_hdd(void *context, int enabled) { (void)context; vm_profile_default_bios_set_boot_hdd(enabled); }
static void vm_composition_console_set_memory(void *context, size_t bytes) { (void)context; core_machine_memory_allocate(bytes); }
static void vm_composition_console_create_fdd(void *context) { vm_machine_fdd_create_for(((vm_composition_live_machine *)context)->fdd); }
static int vm_composition_console_insert_fdd(void *context, const char *path) { return vm_machine_fdd_insert_for(((vm_composition_live_machine *)context)->fdd, path); }
static int vm_composition_console_remove_fdd(void *context, const char *path) { return vm_machine_fdd_remove_for(((vm_composition_live_machine *)context)->fdd, path); }
static void vm_composition_console_create_hdd(void *context, uint16_t cylinders) { vm_machine_hdd_create(((vm_composition_live_machine *)context)->hdd, cylinders); }
static int vm_composition_console_insert_hdd(void *context, const char *path) { return vm_machine_hdd_insert(((vm_composition_live_machine *)context)->hdd, path); }
static int vm_composition_console_remove_hdd(void *context, const char *path) { return vm_machine_hdd_remove(((vm_composition_live_machine *)context)->hdd, path); }
static void vm_composition_console_start(void *context) { machineStart((vm_composition_live_machine *)context); }
static void vm_composition_console_reset(void *context) { machineReset((vm_composition_live_machine *)context); }
static void vm_composition_console_stop(void *context) { machineStop((vm_composition_live_machine *)context); }
static void vm_composition_console_resume(void *context) { machineResume((vm_composition_live_machine *)context); }

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
    vm_composition_console_resume, NULL
};

void vm_composition_console_target_initialize(
    nxvm_product_console_target *target,
    vm_composition_live_machine *machine)
{
    if (target == NULL) return;
    *target = vmCompositionConsoleTargetTemplate;
    target->context = machine;
}

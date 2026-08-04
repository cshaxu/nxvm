#include "type.h"

#include "vm/composition/session/provider.h"

#include "vm/composition/session/control.h"
#include "vm/composition/session/debug_target.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/machine_info.h"
#include "core/machine/memory.h"
#include "core/product/debug/debug.h"
#include "vm/machine/debug.h"
#include "vm/machine/fdd.h"
#include "vm/machine/hdd.h"
#include "vm/platform/platform.h"
#include "vm/profile/default_profile/firmware/bios.h"

static vm_session *vm_session_provider_selected(C_VOID *context)
{
    C_VOID *session = STD_NULL;

    if (core_product_session_manager_borrow_selected(
            (core_product_session_manager *)context, &session) != NTVDM64_STATUS_OK) {
        return STD_NULL;
    }
    return (vm_session *)session;
}

static C_VOID vm_session_machine_initialize(C_VOID *context) { (C_VOID)context; }
static C_VOID vm_session_machine_finalize(C_VOID *context) { (C_VOID)context; }
static C_INT vm_session_machine_is_running(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL && vm_session_control_is_running(s->control); }
static C_VOID vm_session_machine_print(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_print_machine(s); }
static C_INT vm_session_machine_get_window(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL && vm_platform_run_context_get_window_display(s->platform_run_context); }
static C_VOID vm_session_machine_set_window(C_VOID *context, C_INT value) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_platform_run_context_set_window_display(s->platform_run_context, value); }
static C_VOID vm_session_machine_print_bios(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_profile_default_bios_print(s->default_bios); }
static C_VOID vm_session_machine_print_status(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_control_print_status(s->control); }
static C_VOID vm_session_machine_debug(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); static const core_product_debug_input_provider input = { vm_session_debug_flush_console_input, STD_NULL }; if (s == STD_NULL) return; if (vm_session_control_is_running(s->control)) { vm_session_control_request_pause(s->control, VM_SESSION_PAUSE_EXPLICIT); if (!vm_session_control_wait_for_pause(s->control, 2000u)) return; } core_product_debug_context_initialize(s->debugger_context); s->debugger_context->wait_scope = s->wait_scope; core_product_debug_main(s->debugger_context, vm_session_debug_target(s), &input); }
static C_VOID vm_session_machine_record_start(C_VOID *context, const C_CHAR *path) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_machine_debug_record_start(s->debug, path); }
static C_VOID vm_session_machine_record_stop(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_machine_debug_record_stop(s->debug); }
static C_VOID vm_session_machine_boot_hdd(C_VOID *context, C_INT value) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_profile_default_bios_set_boot_hdd(s->default_bios, value); }
static C_VOID vm_session_machine_memory(C_VOID *context, STD_SIZE_T bytes) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) (C_VOID)vm_session_reconfigure_memory(s, bytes); }
static C_VOID vm_session_machine_create_fdd(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_machine_fdd_create_for(s->fdd); }
static C_INT vm_session_machine_insert_fdd(C_VOID *context, const C_CHAR *path) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL ? vm_session_insert_fdd(s, path) : -1; }
static C_INT vm_session_machine_remove_fdd(C_VOID *context, const C_CHAR *path) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL ? vm_machine_fdd_remove_for(s->fdd, path) : -1; }
static C_VOID vm_session_machine_create_hdd(C_VOID *context, uint16_t c) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_machine_hdd_create(s->hdd, c); }
static C_INT vm_session_machine_insert_hdd(C_VOID *context, const C_CHAR *path) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL ? vm_session_insert_hdd(s, path) : -1; }
static C_INT vm_session_machine_remove_hdd(C_VOID *context, const C_CHAR *path) { vm_session *s = vm_session_provider_selected(context); return s != STD_NULL ? vm_machine_hdd_remove(s->hdd, path) : -1; }
static C_VOID vm_session_machine_start(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_start(s); }
static C_VOID vm_session_machine_reset(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_reset(s); }
static C_VOID vm_session_machine_stop(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_stop(s); }
static C_VOID vm_session_machine_resume(C_VOID *context) { vm_session *s = vm_session_provider_selected(context); if (s != STD_NULL) vm_session_resume(s); }

static const vm_product_console_machine_provider vmSessionMachineProviderTemplate = {
    vm_session_machine_initialize, vm_session_machine_finalize,
    vm_session_machine_is_running, vm_session_machine_print,
    vm_session_machine_get_window, vm_session_machine_set_window,
    vm_session_machine_print_bios, vm_session_machine_print_status,
    vm_session_machine_debug, vm_session_machine_record_start,
    vm_session_machine_record_stop, vm_session_machine_boot_hdd,
    vm_session_machine_memory, vm_session_machine_create_fdd,
    vm_session_machine_insert_fdd, vm_session_machine_remove_fdd,
    vm_session_machine_create_hdd, vm_session_machine_insert_hdd,
    vm_session_machine_remove_hdd, vm_session_machine_start,
    vm_session_machine_reset, vm_session_machine_stop,
    vm_session_machine_resume, STD_NULL
};

C_VOID vm_session_machine_provider_initialize(
    vm_product_console_machine_provider *machine_provider,
    core_product_session_manager *manager)
{
    if (machine_provider == STD_NULL) return;
    *machine_provider = vmSessionMachineProviderTemplate;
    machine_provider->context = manager;
}

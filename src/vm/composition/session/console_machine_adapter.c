#include "type.h"

#include "vm/composition/session/provider.h"
#include "vm/composition/session/selected_session.h"

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

static C_INT vm_session_machine_is_running(C_VOID *context)
{
    vm_session *session = vm_session_borrow_selected(context);

    return session != STD_NULL && vm_session_control_is_running(&session->control);
}

static C_VOID vm_session_machine_print(C_VOID *context)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) vm_session_print_machine(session);
}

static C_INT vm_session_machine_get_window(C_VOID *context)
{
    vm_session *session = vm_session_borrow_selected(context);

    return session != STD_NULL && vm_platform_run_context_get_window_display(
        &session->platform_run_context);
}

static C_VOID vm_session_machine_set_window(C_VOID *context, C_INT value)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) {
        vm_platform_run_context_set_window_display(&session->platform_run_context,
            value);
    }
}

static C_VOID vm_session_machine_print_bios(C_VOID *context)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) vm_profile_default_bios_print(&session->default_bios);
}

static C_VOID vm_session_machine_print_status(C_VOID *context)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) vm_session_control_print_status(&session->control);
}

static C_VOID vm_session_machine_debug(C_VOID *context)
{
    static const core_product_debug_input_provider input = {
        vm_session_debug_flush_console_input,
        STD_NULL
    };
    vm_session *session = vm_session_borrow_selected(context);

    if (session == STD_NULL) return;
    if (vm_session_control_is_running(&session->control)) {
        vm_session_control_request_pause(&session->control,
            VM_SESSION_PAUSE_EXPLICIT);
        if (!vm_session_control_wait_for_pause(&session->control, 2000u)) return;
    }
    core_product_debug_context_initialize(&session->debugger_context);
    session->debugger_context.wait_scope = &session->wait_scope;
    core_product_debug_main(&session->debugger_context,
        vm_session_debug_target(session), &input);
}

static C_VOID vm_session_machine_record_start(C_VOID *context,
    const C_CHAR *path)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) vm_machine_debug_record_start(&session->debug, path);
}

static C_VOID vm_session_machine_record_stop(C_VOID *context)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) vm_machine_debug_record_stop(&session->debug);
}

static C_VOID vm_session_machine_boot_hdd(C_VOID *context, C_INT value)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) {
        vm_profile_default_bios_set_boot_hdd(&session->default_bios, value);
    }
}

static C_VOID vm_session_machine_memory(C_VOID *context, STD_SIZE_T bytes)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) (C_VOID)vm_session_reconfigure_memory(session, bytes);
}

static C_VOID vm_session_machine_create_fdd(C_VOID *context)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) vm_machine_fdd_create_for(&session->fdd);
}

static C_INT vm_session_machine_insert_fdd(C_VOID *context, const C_CHAR *path)
{
    vm_session *session = vm_session_borrow_selected(context);

    return session != STD_NULL ? vm_session_insert_fdd(session, path) : -1;
}

static C_INT vm_session_machine_remove_fdd(C_VOID *context, const C_CHAR *path)
{
    vm_session *session = vm_session_borrow_selected(context);

    return session != STD_NULL ? vm_machine_fdd_remove_for(&session->fdd, path) : -1;
}

static C_VOID vm_session_machine_create_hdd(C_VOID *context, uint16_t cylinders)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) vm_machine_hdd_create(&session->hdd, cylinders);
}

static C_INT vm_session_machine_insert_hdd(C_VOID *context, const C_CHAR *path)
{
    vm_session *session = vm_session_borrow_selected(context);

    return session != STD_NULL ? vm_session_insert_hdd(session, path) : -1;
}

static C_INT vm_session_machine_remove_hdd(C_VOID *context, const C_CHAR *path)
{
    vm_session *session = vm_session_borrow_selected(context);

    return session != STD_NULL ? vm_machine_hdd_remove(&session->hdd, path) : -1;
}

static C_VOID vm_session_machine_start(C_VOID *context)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) vm_session_start(session);
}

static C_VOID vm_session_machine_reset(C_VOID *context)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) vm_session_reset(session);
}

static C_VOID vm_session_machine_stop(C_VOID *context)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) vm_session_stop(session);
}

static C_VOID vm_session_machine_resume(C_VOID *context)
{
    vm_session *session = vm_session_borrow_selected(context);

    if (session != STD_NULL) vm_session_resume(session);
}

static const vm_product_console_machine_provider vmSessionMachineProviderTemplate = {
    vm_session_machine_is_running,
    vm_session_machine_print,
    vm_session_machine_get_window,
    vm_session_machine_set_window,
    vm_session_machine_print_bios,
    vm_session_machine_print_status,
    vm_session_machine_debug,
    vm_session_machine_record_start,
    vm_session_machine_record_stop,
    vm_session_machine_boot_hdd,
    vm_session_machine_memory,
    vm_session_machine_create_fdd,
    vm_session_machine_insert_fdd,
    vm_session_machine_remove_fdd,
    vm_session_machine_create_hdd,
    vm_session_machine_insert_hdd,
    vm_session_machine_remove_hdd,
    vm_session_machine_start,
    vm_session_machine_reset,
    vm_session_machine_stop,
    vm_session_machine_resume,
    STD_NULL
};

C_VOID vm_session_machine_provider_initialize(
    vm_product_console_machine_provider *machine_provider,
    core_product_session_manager *manager)
{
    if (machine_provider == STD_NULL) return;
    *machine_provider = vmSessionMachineProviderTemplate;
    machine_provider->context = manager;
}

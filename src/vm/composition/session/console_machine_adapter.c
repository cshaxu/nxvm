#include "type.h"

#include "vm/composition/session/session.h"

#include "vm/composition/session/provider.h"

#include "vm/composition/session/control.h"
#include "vm/composition/session/debug_target.h"
#include "vm/composition/session/fault.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/machine_info.h"
#include "core/product/debug/debug.h"
#include "core/product/session/session_interface.h"
#include "vm/machine/debug.h"
#include "vm/machine/fdd.h"
#include "vm/platform/platform.h"
#include "vm/profile/default_profile/firmware/bios.h"

static vm_session *vm_session_machine_borrow_selected(C_VOID *context)
{
    C_VOID *session = STD_NULL;
    core_product_session_manager *manager =
        (core_product_session_manager *)context;

    if (manager == STD_NULL || core_product_session_manager_borrow_selected(
            manager, &session) != TYPE_STATUS_OK) {
        return STD_NULL;
    }
    return (vm_session *)session;
}

static C_INT vm_session_machine_is_running(C_VOID *context)
{
    vm_session *session = vm_session_machine_borrow_selected(context);

    return session != STD_NULL && vm_session_control_is_running(&session->control);
}

static C_VOID vm_session_machine_print(C_VOID *context)
{
    vm_session *session = vm_session_machine_borrow_selected(context);

    if (session != STD_NULL) vm_session_print_machine(session);
}

static vm_product_console_display_mode vm_session_machine_get_display_mode(C_VOID *context)
{
    vm_session *session = vm_session_machine_borrow_selected(context);
    C_INT mode;

    if (session == STD_NULL) return VM_PRODUCT_CONSOLE_DISPLAY_CONSOLE;
    mode = vm_platform_run_context_get_display_mode(&session->platform_run_context);
    if (mode == VM_PLATFORM_DISPLAY_WINDOW) return VM_PRODUCT_CONSOLE_DISPLAY_WINDOW;
    if (mode == VM_PLATFORM_DISPLAY_AUTO) return VM_PRODUCT_CONSOLE_DISPLAY_AUTO;
    return VM_PRODUCT_CONSOLE_DISPLAY_CONSOLE;
}

static C_VOID vm_session_machine_set_display_mode(C_VOID *context,
    vm_product_console_display_mode mode)
{
    vm_session *session = vm_session_machine_borrow_selected(context);
    vm_platform_display_mode platformMode;

    if (session != STD_NULL) {
        platformMode = mode == VM_PRODUCT_CONSOLE_DISPLAY_WINDOW ?
            VM_PLATFORM_DISPLAY_WINDOW : mode == VM_PRODUCT_CONSOLE_DISPLAY_AUTO ?
            VM_PLATFORM_DISPLAY_AUTO : VM_PLATFORM_DISPLAY_CONSOLE;
        vm_platform_run_context_set_display_mode(&session->platform_run_context,
            platformMode);
    }
}

static C_VOID vm_session_machine_print_bios(C_VOID *context)
{
    vm_session *session = vm_session_machine_borrow_selected(context);

    if (session != STD_NULL) vm_profile_default_bios_print(&session->default_bios);
}

static C_VOID vm_session_machine_print_status(C_VOID *context)
{
    vm_session *session = vm_session_machine_borrow_selected(context);

    if (session != STD_NULL) {
        vm_session_control_print_status(&session->control);
        vm_session_fault_print(session);
    }
}

static C_VOID vm_session_machine_debug(C_VOID *context)
{
    static const core_product_debug_input_provider input = {
        vm_session_debug_flush_console_input,
        STD_NULL
    };
    vm_session *session = vm_session_machine_borrow_selected(context);

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
    vm_session *session = vm_session_machine_borrow_selected(context);

    if (session != STD_NULL) vm_machine_debug_record_start(&session->debug, path);
}

static C_VOID vm_session_machine_record_stop(C_VOID *context)
{
    vm_session *session = vm_session_machine_borrow_selected(context);

    if (session != STD_NULL) vm_machine_debug_record_stop(&session->debug);
}

static C_INT vm_session_machine_insert_fdd(C_VOID *context, const C_CHAR *path)
{
    vm_session *session = vm_session_machine_borrow_selected(context);

    return session != STD_NULL ? vm_session_insert_fdd(session, path) : -1;
}

static C_INT vm_session_machine_remove_fdd(C_VOID *context, const C_CHAR *path)
{
    vm_session *session = vm_session_machine_borrow_selected(context);

    return session != STD_NULL ? vm_machine_fdd_remove_for(&session->fdd, path) : -1;
}

static type_status vm_session_machine_start(C_VOID *context)
{
    vm_session *session = vm_session_machine_borrow_selected(context);

    return session == STD_NULL ? TYPE_STATUS_INVALID_STATE :
        vm_session_start(session);
}

static C_VOID vm_session_machine_reset(C_VOID *context)
{
    vm_session *session = vm_session_machine_borrow_selected(context);

    if (session != STD_NULL) vm_session_reset(session);
}

static C_VOID vm_session_machine_stop(C_VOID *context)
{
    vm_session *session = vm_session_machine_borrow_selected(context);

    if (session != STD_NULL) vm_session_stop(session);
}

static type_status vm_session_machine_resume(C_VOID *context)
{
    vm_session *session = vm_session_machine_borrow_selected(context);

    return session == STD_NULL ? TYPE_STATUS_INVALID_STATE :
        vm_session_resume(session);
}

static const vm_product_console_machine_provider vmSessionMachineProviderTemplate = {
    vm_session_machine_is_running,
    vm_session_machine_print,
    vm_session_machine_get_display_mode,
    vm_session_machine_set_display_mode,
    vm_session_machine_print_bios,
    vm_session_machine_print_status,
    vm_session_machine_debug,
    vm_session_machine_record_start,
    vm_session_machine_record_stop,
    vm_session_machine_insert_fdd,
    vm_session_machine_remove_fdd,
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

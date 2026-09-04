#include "type.h"

#include "vm/composition/session/session_private.h"

#include "vm/composition/session/console_machine_adapter.h"

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

typedef type_status (*vm_session_machine_selected_operation)(
    vm_session *session, C_VOID *context);

typedef struct vm_session_machine_selected_call {
    vm_session_machine_selected_operation operation;
    C_VOID *context;
} vm_session_machine_selected_call;

static type_status vm_session_machine_apply_selected(C_VOID *context,
    C_VOID *opaque)
{
    vm_session_machine_selected_call *call =
        (vm_session_machine_selected_call *)context;

    if (call == STD_NULL || call->operation == STD_NULL || opaque == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return call->operation((vm_session *)opaque, call->context);
}

static type_status vm_session_machine_apply(C_VOID *context,
    vm_session_machine_selected_operation operation, C_VOID *operation_context)
{
    vm_session_machine_selected_call call = {operation, operation_context};
    core_product_session_manager *manager = (core_product_session_manager *)context;

    if (manager == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    return core_product_session_manager_apply_selected(manager,
        vm_session_machine_apply_selected, &call);
}

static type_status vm_session_machine_read_running(vm_session *session,
    C_VOID *context)
{
    *(C_INT *)context = vm_session_control_is_running(&session->control);
    return TYPE_STATUS_OK;
}

static C_INT vm_session_machine_is_running(C_VOID *context)
{
    C_INT running = 0;

    (C_VOID)vm_session_machine_apply(context, vm_session_machine_read_running,
        &running);
    return running;
}

static type_status vm_session_machine_print_selected(vm_session *session,
    C_VOID *context)
{
    (C_VOID)context;
    vm_session_print_machine(session);
    return TYPE_STATUS_OK;
}

static C_VOID vm_session_machine_print(C_VOID *context)
{
    (C_VOID)vm_session_machine_apply(context, vm_session_machine_print_selected,
        STD_NULL);
}

static type_status vm_session_machine_read_display_mode(vm_session *session,
    C_VOID *context)
{
    vm_session_display_mode *mode =
        (vm_session_display_mode *)context;
    C_INT platform_mode = vm_platform_run_context_get_display_mode(
        session->platform_run_context);

    *mode = platform_mode == VM_PLATFORM_DISPLAY_WINDOW ?
        VM_SESSION_DISPLAY_WINDOW : VM_SESSION_DISPLAY_CONSOLE;
    return TYPE_STATUS_OK;
}

static vm_session_display_mode vm_session_machine_get_display_mode(C_VOID *context)
{
    vm_session_display_mode mode = VM_SESSION_DISPLAY_CONSOLE;

    (C_VOID)vm_session_machine_apply(context, vm_session_machine_read_display_mode,
        &mode);
    return mode;
}

static type_status vm_session_machine_write_display_mode(vm_session *session,
    C_VOID *context)
{
    vm_session_display_mode mode = *(vm_session_display_mode *)context;
    vm_platform_display_mode platform_mode = mode == VM_SESSION_DISPLAY_WINDOW ?
        VM_PLATFORM_DISPLAY_WINDOW : VM_PLATFORM_DISPLAY_CONSOLE;

    vm_platform_run_context_set_display_mode(session->platform_run_context,
        platform_mode);
    return TYPE_STATUS_OK;
}

static C_VOID vm_session_machine_set_display_mode(C_VOID *context,
    vm_session_display_mode mode)
{
    (C_VOID)vm_session_machine_apply(context, vm_session_machine_write_display_mode,
        &mode);
}

static type_status vm_session_machine_print_bios_selected(vm_session *session,
    C_VOID *context)
{
    (C_VOID)context;
    if (session == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    STD_PRINTF("BIOS: %s\n", session->firmware_kind ==
        VM_SESSION_FIRMWARE_EXTERNAL_PC_AT_ROM && session->pc_at_rom_external ?
        "external ROM mapped at F0000h" : "profile ROM mapped");
    return TYPE_STATUS_OK;
}

static C_VOID vm_session_machine_print_bios(C_VOID *context)
{
    (C_VOID)vm_session_machine_apply(context,
        vm_session_machine_print_bios_selected, STD_NULL);
}

static type_status vm_session_machine_print_status_selected(vm_session *session,
    C_VOID *context)
{
    (C_VOID)context;
    vm_session_control_print_status(&session->control);
    vm_session_fault_print(session);
    return TYPE_STATUS_OK;
}

static C_VOID vm_session_machine_print_status(C_VOID *context)
{
    (C_VOID)vm_session_machine_apply(context,
        vm_session_machine_print_status_selected, STD_NULL);
}

static type_status vm_session_machine_read_speed(vm_session *session,
    C_VOID *context)
{
    vm_session_speed speed;
    vm_product_console_speed *out_speed = (vm_product_console_speed *)context;

    if (out_speed == STD_NULL || vm_session_get_speed(session, &speed) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_speed = speed == VM_SESSION_SPEED_TURBO ? VM_PRODUCT_CONSOLE_SPEED_TURBO :
        VM_PRODUCT_CONSOLE_SPEED_STANDARD;
    return TYPE_STATUS_OK;
}

static type_status vm_session_machine_get_speed(C_VOID *context,
    vm_product_console_speed *out_speed)
{
    return out_speed == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        vm_session_machine_apply(context, vm_session_machine_read_speed, out_speed);
}

static type_status vm_session_machine_write_speed(vm_session *session,
    C_VOID *context)
{
    vm_product_console_speed speed = *(vm_product_console_speed *)context;

    return vm_session_set_speed(session, speed == VM_PRODUCT_CONSOLE_SPEED_TURBO ?
        VM_SESSION_SPEED_TURBO : VM_SESSION_SPEED_STANDARD);
}

static type_status vm_session_machine_set_speed(C_VOID *context,
    vm_product_console_speed speed)
{
    if (speed != VM_PRODUCT_CONSOLE_SPEED_STANDARD &&
        speed != VM_PRODUCT_CONSOLE_SPEED_TURBO) return TYPE_STATUS_INVALID_ARGUMENT;
    return vm_session_machine_apply(context, vm_session_machine_write_speed, &speed);
}

static type_status vm_session_machine_debug_selected(vm_session *session,
    C_VOID *context)
{
    const core_product_debug_input_provider *input =
        (const core_product_debug_input_provider *)context;

    if (vm_session_control_is_running(&session->control)) {
        vm_session_control_request_pause(&session->control,
            VM_SESSION_PAUSE_EXPLICIT);
        if (!vm_session_control_wait_for_pause(&session->control, 2000u)) {
            return TYPE_STATUS_INVALID_STATE;
        }
    }
    core_product_debugger_run(session->debugger, vm_session_debug_target(session),
        input, &session->wait_scope);
    return TYPE_STATUS_OK;
}

static C_VOID vm_session_machine_debug(C_VOID *context)
{
    static const core_product_debug_input_provider input = {
        vm_session_debug_flush_console_input,
        STD_NULL
    };
    (C_VOID)vm_session_machine_apply(context, vm_session_machine_debug_selected,
        (C_VOID *)&input);
}

static type_status vm_session_machine_record_start_selected(vm_session *session,
    C_VOID *context)
{
    vm_machine_debug_record_start(&session->debug, (const C_CHAR *)context);
    return TYPE_STATUS_OK;
}

static C_VOID vm_session_machine_record_start(C_VOID *context,
    const C_CHAR *path)
{
    (C_VOID)vm_session_machine_apply(context,
        vm_session_machine_record_start_selected, (C_VOID *)path);
}

static type_status vm_session_machine_record_stop_selected(vm_session *session,
    C_VOID *context)
{
    (C_VOID)context;
    vm_machine_debug_record_stop(&session->debug);
    return TYPE_STATUS_OK;
}

static C_VOID vm_session_machine_record_stop(C_VOID *context)
{
    (C_VOID)vm_session_machine_apply(context,
        vm_session_machine_record_stop_selected, STD_NULL);
}

typedef struct vm_session_machine_fdd_call {
    const C_CHAR *path;
    C_INT result;
} vm_session_machine_fdd_call;

static type_status vm_session_machine_insert_fdd_selected(vm_session *session,
    C_VOID *context)
{
    vm_session_machine_fdd_call *call = (vm_session_machine_fdd_call *)context;

    call->result = vm_session_insert_fdd(session, call->path);
    return TYPE_STATUS_OK;
}

static C_INT vm_session_machine_insert_fdd(C_VOID *context, const C_CHAR *path)
{
    vm_session_machine_fdd_call call = {path, -1};

    (C_VOID)vm_session_machine_apply(context,
        vm_session_machine_insert_fdd_selected, &call);
    return call.result;
}

static type_status vm_session_machine_remove_fdd_selected(vm_session *session,
    C_VOID *context)
{
    vm_session_machine_fdd_call *call = (vm_session_machine_fdd_call *)context;

    call->result = vm_session_remove_fdd(session, call->path);
    return TYPE_STATUS_OK;
}

static C_INT vm_session_machine_remove_fdd(C_VOID *context, const C_CHAR *path)
{
    vm_session_machine_fdd_call call = {path, -1};

    (C_VOID)vm_session_machine_apply(context,
        vm_session_machine_remove_fdd_selected, &call);
    return call.result;
}

static type_status vm_session_machine_start_selected(vm_session *session,
    C_VOID *context)
{
    (C_VOID)context;
    return vm_session_start(session);
}

static type_status vm_session_machine_start(C_VOID *context)
{
    return vm_session_machine_apply(context, vm_session_machine_start_selected,
        STD_NULL);
}

static type_status vm_session_machine_reset_selected(vm_session *session,
    C_VOID *context)
{
    (C_VOID)context;
    vm_session_reset(session);
    return TYPE_STATUS_OK;
}

static C_VOID vm_session_machine_reset(C_VOID *context)
{
    (C_VOID)vm_session_machine_apply(context, vm_session_machine_reset_selected,
        STD_NULL);
}

static type_status vm_session_machine_stop_selected(vm_session *session,
    C_VOID *context)
{
    (C_VOID)context;
    vm_session_stop(session);
    return TYPE_STATUS_OK;
}

static C_VOID vm_session_machine_stop(C_VOID *context)
{
    (C_VOID)vm_session_machine_apply(context, vm_session_machine_stop_selected,
        STD_NULL);
}

static type_status vm_session_machine_resume_selected(vm_session *session,
    C_VOID *context)
{
    (C_VOID)context;
    return vm_session_resume(session);
}

static type_status vm_session_machine_resume(C_VOID *context)
{
    return vm_session_machine_apply(context, vm_session_machine_resume_selected,
        STD_NULL);
}

static const vm_session_machine_provider vmSessionMachineProviderTemplate = {
    vm_session_machine_is_running,
    vm_session_machine_print,
    vm_session_machine_get_display_mode,
    vm_session_machine_set_display_mode,
    vm_session_machine_print_bios,
    vm_session_machine_print_status,
    vm_session_machine_get_speed,
    vm_session_machine_set_speed,
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

C_VOID vm_composition_console_machine_provider_initialize(
    vm_session_machine_provider *machine_provider,
    core_product_session_manager *manager)
{
    if (machine_provider == STD_NULL) return;
    *machine_provider = vmSessionMachineProviderTemplate;
    machine_provider->context = manager;
}

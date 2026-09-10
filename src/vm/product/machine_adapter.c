#include "type.h"

#include "vm/composition/session/session_interface.h"

#include "vm/product/machine_adapter.h"

#include "vm/product/session_factory.h"

#include "vm/composition/session/lifecycle.h"

#include "vm/product/session_catalog.h"

typedef type_status (*vm_session_machine_selected_operation)(
    vm_session *session, C_VOID *context);

static type_status vm_session_machine_apply(C_VOID *context,
    vm_session_machine_selected_operation operation, C_VOID *operation_context)
{
    vm_session **slot = (vm_session **)context;

    if (slot == STD_NULL || *slot == STD_NULL || operation == STD_NULL)
        return TYPE_STATUS_INVALID_STATE;
    return operation(*slot, operation_context);
}

static type_status vm_session_machine_read_running(vm_session *session,
    C_VOID *context)
{
    *(C_INT *)context = vm_session_is_running(session);
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

static C_VOID vm_session_machine_set_lifecycle_reporter(C_VOID *context,
    vm_product_console_lifecycle_reporter reporter, C_VOID *reporter_context)
{
    vm_session **slot = (vm_session **)context;

    if (slot != STD_NULL && *slot != STD_NULL) {
        vm_session_set_lifecycle_reporter(*slot,
            (vm_session_lifecycle_reporter)reporter, reporter_context);
    }
}

static C_VOID vm_session_machine_set_display_reporter(C_VOID *context,
    vm_product_console_display_reporter reporter, C_VOID *reporter_context)
{
    vm_session **slot = (vm_session **)context;

    if (slot != STD_NULL && *slot != STD_NULL) {
        vm_session_set_display_reporter(*slot,
            (vm_session_display_reporter)reporter, reporter_context);
    }
}

static type_status vm_session_machine_print_bios_selected(vm_session *session,
    C_VOID *context)
{
    (C_VOID)context;
    vm_session_print_bios(session);
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
    vm_session_print_status(session);
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
    (C_VOID)context;

    return vm_session_run_debugger(session);
}

static C_VOID vm_session_machine_debug(C_VOID *context)
{
    (C_VOID)vm_session_machine_apply(context, vm_session_machine_debug_selected,
        STD_NULL);
}

static type_status vm_session_machine_record_start_selected(vm_session *session,
    C_VOID *context)
{
    return vm_session_record_start(session, (const C_CHAR *)context);
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
    return vm_session_record_stop(session);
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

static type_status vm_session_machine_open_profile(C_VOID *context,
    const vm_product_session_request *request)
{
    vm_session **slot = (vm_session **)context;

    if (slot == STD_NULL || *slot != STD_NULL) return TYPE_STATUS_INVALID_STATE;
    return vm_session_create_from_request(request, slot);
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

static type_status vm_session_machine_stop(C_VOID *context)
{
    return vm_session_machine_apply(context, vm_session_machine_stop_selected,
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

static type_status vm_session_machine_request_pause_selected(vm_session *session,
    C_VOID *context)
{
    (C_VOID)context;
    return vm_session_request_pause(session);
}

static type_status vm_session_machine_request_pause(C_VOID *context)
{
    return vm_session_machine_apply(context,
        vm_session_machine_request_pause_selected, STD_NULL);
}

typedef struct vm_session_machine_input_call {
    const core_machine_guest_input_event *event;
    type_status status;
} vm_session_machine_input_call;

static type_status vm_session_machine_submit_host_input_selected(vm_session *session,
    C_VOID *context)
{
    vm_session_machine_input_call *call = context;

    call->status = vm_session_submit_host_input(session, call->event);
    return call->status;
}

static type_status vm_session_machine_submit_host_input(C_VOID *context,
    const core_machine_guest_input_event *event)
{
    vm_session_machine_input_call call = {event, TYPE_STATUS_INVALID_ARGUMENT};

    if (event == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    return vm_session_machine_apply(context,
        vm_session_machine_submit_host_input_selected, &call);
}

static const vm_session_machine_provider vmSessionMachineProviderTemplate = {
    vm_session_machine_is_running,
    vm_session_machine_print,
    vm_session_machine_set_lifecycle_reporter,
    vm_session_machine_set_display_reporter,
    vm_session_machine_print_bios,
    vm_session_machine_print_status,
    vm_session_machine_get_speed,
    vm_session_machine_set_speed,
    vm_session_machine_debug,
    vm_session_machine_record_start,
    vm_session_machine_record_stop,
    vm_session_machine_insert_fdd,
    vm_session_machine_remove_fdd,
    vm_session_machine_open_profile,
    vm_session_machine_start,
    vm_session_machine_reset,
    vm_session_machine_stop,
    vm_session_machine_resume,
    vm_session_machine_request_pause,
    vm_session_machine_submit_host_input,
    STD_NULL
};

C_VOID vm_product_machine_provider_initialize(
    vm_session_machine_provider *machine_provider,
    vm_session **session_slot)
{
    if (machine_provider == STD_NULL) return;
    *machine_provider = vmSessionMachineProviderTemplate;
    machine_provider->context = session_slot;
}

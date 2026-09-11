#include "type.h"

#include "vm/machine/runtime/machine_interface.h"

#include "vm/product/machine_adapter.h"

#include "vm/product/session_factory.h"

#include "vm/machine/runtime/lifecycle.h"

#include "vm/product/session_catalog.h"

typedef type_status (*vm_product_machine_selected_operation)(
    vm_machine *session, C_VOID *context);

static C_VOID vm_product_machine_result_sink(C_VOID *context,
    const vm_machine_result *result);

static vm_machine **vm_product_machine_slot(C_VOID *context)
{
    vm_product_machine_provider *provider = context;

    return provider == STD_NULL ? STD_NULL :
        (vm_machine **)provider->machine_slot;
}

static type_status vm_product_machine_apply(C_VOID *context,
    vm_product_machine_selected_operation operation, C_VOID *operation_context)
{
    vm_machine **slot = vm_product_machine_slot(context);

    if (slot == STD_NULL || *slot == STD_NULL || operation == STD_NULL)
        return TYPE_STATUS_INVALID_STATE;
    return operation(*slot, operation_context);
}

static type_status vm_product_machine_read_running(vm_machine *session,
    C_VOID *context)
{
    *(C_INT *)context = vm_machine_is_running(session);
    return TYPE_STATUS_OK;
}

static C_INT vm_product_machine_is_running(C_VOID *context)
{
    C_INT running = 0;

    (C_VOID)vm_product_machine_apply(context, vm_product_machine_read_running,
        &running);
    return running;
}

static type_status vm_product_machine_print_selected(vm_machine *session,
    C_VOID *context)
{
    (C_VOID)context;
    vm_machine_print_machine(session);
    return TYPE_STATUS_OK;
}

static C_VOID vm_product_machine_print(C_VOID *context)
{
    (C_VOID)vm_product_machine_apply(context, vm_product_machine_print_selected,
        STD_NULL);
}

static C_VOID vm_product_machine_result_sink(C_VOID *context,
    const vm_machine_result *result)
{
    vm_product_machine_provider *provider = context;
    core_machine_guest_display_frame frame = {0};
    vm_machine_lifecycle lifecycle;

    if (provider == STD_NULL || result == STD_NULL) return;
    switch (result->kind) {
    case VM_MACHINE_RESULT_RUNNING: lifecycle = VM_MACHINE_RUNNING; break;
    case VM_MACHINE_RESULT_PAUSED: lifecycle = VM_MACHINE_PAUSED; break;
    case VM_MACHINE_RESULT_RESET: lifecycle = VM_MACHINE_RESET; break;
    case VM_MACHINE_RESULT_STOPPED: lifecycle = VM_MACHINE_STOPPED; break;
    case VM_MACHINE_RESULT_DISPLAY:
        if (provider->display_reporter == STD_NULL) return;
        frame.kind = result->value.display.graphics ?
            CORE_MACHINE_GUEST_DISPLAY_KIND_INDEXED_PIXELS :
            CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT;
        frame.columns = result->value.display.columns;
        frame.rows = result->value.display.rows;
        frame.cursor_top = result->value.display.cursor_top;
        frame.cursor_bottom = result->value.display.cursor_bottom;
        frame.cursor_x = result->value.display.cursor_x;
        frame.cursor_y = result->value.display.cursor_y;
        frame.cursor_visible = result->value.display.cursor_visible;
        frame.buffer_changed = result->value.display.buffer_changed;
        frame.cursor_changed = result->value.display.cursor_changed;
        frame.text_glyphs_present = result->value.display.glyphs_present;
        frame.pixel_width = result->value.display.pixel_width;
        frame.pixel_height = result->value.display.pixel_height;
        frame.generation = result->value.display.generation;
        STD_MEMCPY(frame.characters, result->value.display.characters,
            sizeof(frame.characters));
        STD_MEMCPY(frame.attributes, result->value.display.attributes,
            sizeof(frame.attributes));
        STD_MEMCPY(frame.text_glyphs, result->value.display.glyphs,
            sizeof(frame.text_glyphs));
        STD_MEMCPY(frame.pixels, result->value.display.pixels,
            sizeof(frame.pixels));
        STD_MEMCPY(frame.palette_rgb, result->value.display.palette_rgb,
            sizeof(frame.palette_rgb));
        provider->display_reporter(provider->display_reporter_context, &frame);
        return;
    default:
        return;
    }
    if (provider->lifecycle_reporter != STD_NULL)
        provider->lifecycle_reporter(provider->lifecycle_reporter_context,
            lifecycle);
}

static C_VOID vm_product_machine_set_lifecycle_reporter(C_VOID *context,
    vm_product_console_lifecycle_reporter reporter, C_VOID *reporter_context)
{
    vm_product_machine_provider *provider = context;
    vm_machine **slot = vm_product_machine_slot(context);

    if (provider == STD_NULL) return;
    provider->lifecycle_reporter = reporter;
    provider->lifecycle_reporter_context = reporter_context;
    if (slot != STD_NULL && *slot != STD_NULL)
        vm_machine_set_result_sink(*slot, reporter != STD_NULL ||
            provider->display_reporter != STD_NULL ?
            vm_product_machine_result_sink : STD_NULL, provider);
}

static C_VOID vm_product_machine_set_display_reporter(C_VOID *context,
    vm_product_console_display_reporter reporter, C_VOID *reporter_context)
{
    vm_product_machine_provider *provider = context;
    vm_machine **slot = vm_product_machine_slot(context);

    if (provider == STD_NULL) return;
    provider->display_reporter = reporter;
    provider->display_reporter_context = reporter_context;
    if (slot != STD_NULL && *slot != STD_NULL)
        vm_machine_set_result_sink(*slot, reporter != STD_NULL ||
            provider->lifecycle_reporter != STD_NULL ?
            vm_product_machine_result_sink : STD_NULL, provider);
}

static type_status vm_product_machine_print_bios_selected(vm_machine *session,
    C_VOID *context)
{
    (C_VOID)context;
    vm_machine_print_bios(session);
    return TYPE_STATUS_OK;
}

static C_VOID vm_product_machine_print_bios(C_VOID *context)
{
    (C_VOID)vm_product_machine_apply(context,
        vm_product_machine_print_bios_selected, STD_NULL);
}

static type_status vm_product_machine_print_status_selected(vm_machine *session,
    C_VOID *context)
{
    (C_VOID)context;
    vm_machine_print_status(session);
    return TYPE_STATUS_OK;
}

static C_VOID vm_product_machine_print_status(C_VOID *context)
{
    (C_VOID)vm_product_machine_apply(context,
        vm_product_machine_print_status_selected, STD_NULL);
}

static type_status vm_product_machine_read_speed(vm_machine *session,
    C_VOID *context)
{
    vm_machine_speed speed;
    vm_product_console_speed *out_speed = (vm_product_console_speed *)context;

    if (out_speed == STD_NULL || vm_machine_get_speed(session, &speed) != TYPE_STATUS_OK) {
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_speed = speed == VM_MACHINE_SPEED_TURBO ? VM_PRODUCT_CONSOLE_SPEED_TURBO :
        VM_PRODUCT_CONSOLE_SPEED_STANDARD;
    return TYPE_STATUS_OK;
}

static type_status vm_product_machine_get_speed(C_VOID *context,
    vm_product_console_speed *out_speed)
{
    return out_speed == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        vm_product_machine_apply(context, vm_product_machine_read_speed, out_speed);
}

static type_status vm_product_machine_write_speed(vm_machine *session,
    C_VOID *context)
{
    vm_product_console_speed speed = *(vm_product_console_speed *)context;

    return vm_machine_set_speed(session, speed == VM_PRODUCT_CONSOLE_SPEED_TURBO ?
        VM_MACHINE_SPEED_TURBO : VM_MACHINE_SPEED_STANDARD);
}

static type_status vm_product_machine_set_speed(C_VOID *context,
    vm_product_console_speed speed)
{
    if (speed != VM_PRODUCT_CONSOLE_SPEED_STANDARD &&
        speed != VM_PRODUCT_CONSOLE_SPEED_TURBO) return TYPE_STATUS_INVALID_ARGUMENT;
    return vm_product_machine_apply(context, vm_product_machine_write_speed, &speed);
}

static type_status vm_product_machine_debug_selected(vm_machine *session,
    C_VOID *context)
{
    (C_VOID)context;

    return vm_machine_run_debugger(session);
}

static C_VOID vm_product_machine_debug(C_VOID *context)
{
    (C_VOID)vm_product_machine_apply(context, vm_product_machine_debug_selected,
        STD_NULL);
}

static type_status vm_product_machine_record_start_selected(vm_machine *session,
    C_VOID *context)
{
    return vm_machine_record_start(session, (const C_CHAR *)context);
}

static C_VOID vm_product_machine_record_start(C_VOID *context,
    const C_CHAR *path)
{
    (C_VOID)vm_product_machine_apply(context,
        vm_product_machine_record_start_selected, (C_VOID *)path);
}

static type_status vm_product_machine_record_stop_selected(vm_machine *session,
    C_VOID *context)
{
    (C_VOID)context;
    return vm_machine_record_stop(session);
}

static C_VOID vm_product_machine_record_stop(C_VOID *context)
{
    (C_VOID)vm_product_machine_apply(context,
        vm_product_machine_record_stop_selected, STD_NULL);
}

typedef struct vm_product_machine_fdd_call {
    const C_CHAR *path;
    C_INT result;
} vm_product_machine_fdd_call;

static type_status vm_product_machine_insert_fdd_selected(vm_machine *session,
    C_VOID *context)
{
    vm_product_machine_fdd_call *call = (vm_product_machine_fdd_call *)context;

    call->result = vm_machine_insert_fdd(session, call->path);
    return TYPE_STATUS_OK;
}

static C_INT vm_product_machine_insert_fdd(C_VOID *context, const C_CHAR *path)
{
    vm_product_machine_fdd_call call = {path, -1};

    (C_VOID)vm_product_machine_apply(context,
        vm_product_machine_insert_fdd_selected, &call);
    return call.result;
}

static type_status vm_product_machine_remove_fdd_selected(vm_machine *session,
    C_VOID *context)
{
    vm_product_machine_fdd_call *call = (vm_product_machine_fdd_call *)context;

    call->result = vm_machine_remove_fdd(session, call->path);
    return TYPE_STATUS_OK;
}

static C_INT vm_product_machine_remove_fdd(C_VOID *context, const C_CHAR *path)
{
    vm_product_machine_fdd_call call = {path, -1};

    (C_VOID)vm_product_machine_apply(context,
        vm_product_machine_remove_fdd_selected, &call);
    return call.result;
}

static type_status vm_product_machine_start_selected(vm_machine *session,
    C_VOID *context)
{
    (C_VOID)context;
    return vm_machine_start(session);
}

static type_status vm_product_machine_open_profile(C_VOID *context,
    const vm_product_session_request *request)
{
    vm_product_machine_provider *provider = context;
    vm_machine **slot = vm_product_machine_slot(context);

    if (slot == STD_NULL || *slot != STD_NULL) return TYPE_STATUS_INVALID_STATE;
    if (vm_machine_create_from_request(request, slot) != TYPE_STATUS_OK)
        return TYPE_STATUS_INVALID_STATE;
    if (provider->lifecycle_reporter != STD_NULL ||
        provider->display_reporter != STD_NULL)
        vm_machine_set_result_sink(*slot, vm_product_machine_result_sink,
            provider);
    return TYPE_STATUS_OK;
}

static type_status vm_product_machine_start(C_VOID *context)
{
    return vm_product_machine_apply(context, vm_product_machine_start_selected,
        STD_NULL);
}

static type_status vm_product_machine_reset_selected(vm_machine *session,
    C_VOID *context)
{
    (C_VOID)context;
    vm_machine_reset(session);
    return TYPE_STATUS_OK;
}

static C_VOID vm_product_machine_reset(C_VOID *context)
{
    (C_VOID)vm_product_machine_apply(context, vm_product_machine_reset_selected,
        STD_NULL);
}

static type_status vm_product_machine_stop_selected(vm_machine *session,
    C_VOID *context)
{
    (C_VOID)context;
    vm_machine_stop(session);
    return TYPE_STATUS_OK;
}

static type_status vm_product_machine_stop(C_VOID *context)
{
    return vm_product_machine_apply(context, vm_product_machine_stop_selected,
        STD_NULL);
}

static type_status vm_product_machine_resume_selected(vm_machine *session,
    C_VOID *context)
{
    (C_VOID)context;
    return vm_machine_resume(session);
}

static type_status vm_product_machine_resume(C_VOID *context)
{
    return vm_product_machine_apply(context, vm_product_machine_resume_selected,
        STD_NULL);
}

static type_status vm_product_machine_request_pause_selected(vm_machine *session,
    C_VOID *context)
{
    (C_VOID)context;
    return vm_machine_request_pause(session);
}

static type_status vm_product_machine_request_pause(C_VOID *context)
{
    return vm_product_machine_apply(context,
        vm_product_machine_request_pause_selected, STD_NULL);
}

typedef struct vm_product_machine_input_call {
    const core_machine_guest_input_event *event;
    type_status status;
} vm_product_machine_input_call;

static type_status vm_product_machine_submit_host_input_selected(vm_machine *session,
    C_VOID *context)
{
    vm_product_machine_input_call *call = context;

    call->status = vm_machine_submit_host_input(session, call->event);
    return call->status;
}

static type_status vm_product_machine_submit_host_input(C_VOID *context,
    const core_machine_guest_input_event *event)
{
    vm_product_machine_input_call call = {event, TYPE_STATUS_INVALID_ARGUMENT};

    if (event == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    return vm_product_machine_apply(context,
        vm_product_machine_submit_host_input_selected, &call);
}

static const vm_product_machine_provider vmSessionMachineProviderTemplate = {
    vm_product_machine_is_running,
    vm_product_machine_print,
    vm_product_machine_set_lifecycle_reporter,
    vm_product_machine_set_display_reporter,
    vm_product_machine_print_bios,
    vm_product_machine_print_status,
    vm_product_machine_get_speed,
    vm_product_machine_set_speed,
    vm_product_machine_debug,
    vm_product_machine_record_start,
    vm_product_machine_record_stop,
    vm_product_machine_insert_fdd,
    vm_product_machine_remove_fdd,
    vm_product_machine_open_profile,
    vm_product_machine_start,
    vm_product_machine_reset,
    vm_product_machine_stop,
    vm_product_machine_resume,
    vm_product_machine_request_pause,
    vm_product_machine_submit_host_input,
    STD_NULL
};

C_VOID vm_product_machine_provider_initialize(
    vm_product_machine_provider *machine_provider,
    vm_machine **session_slot)
{
    if (machine_provider == STD_NULL) return;
    *machine_provider = vmSessionMachineProviderTemplate;
    machine_provider->machine_slot = session_slot;
    machine_provider->context = machine_provider;
}

#include "type.h"

#include "core/machine/guest_display_frame.h"
#include "vm/platform/platform_internal.h"
#include "vm/platform/ux_binding.h"
#include "vm/platform/ux_frame.h"

#include <string.h>

C_INT vm_platform_run_context_handle_ux_input(C_VOID *opaque,
    const ux_input_event *event)
{
    vm_platform_run_context *context = opaque;
    vm_platform_run_handle *handle = context == STD_NULL ? STD_NULL : context->run_handle;
    ux_input_event key = { 0 };
    if (context == STD_NULL || event == STD_NULL || handle == STD_NULL) return 0;
    if (event->type == UX_EVENT_SOURCE_RETIRED) {
        type_unsigned_32 index;

        for (index = 0u; index < 512u; ++index) {
            if (!handle->ux_pressed_keys[index] ||
                handle->ux_key_sources[index] != event->source_identity) continue;
            key.type = UX_EVENT_KEY;
            key.data.key.scan_code = (type_unsigned_16)index;
            key.data.key.pressed = LIB_FALSE;
            if (!vm_platform_ux_event_submit(context, &key)) return 0;
            handle->ux_pressed_keys[index] = TYPE_FALSE;
            handle->ux_key_sources[index] = 0u;
        }
        return 1;
    }
    if (event->type == UX_EVENT_HOTKEY) {
        if (strcmp(event->data.hotkey.identifier, "pause") == 0) {
            vm_platform_run_handle_report(handle, VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED);
            return 1;
        }
        if (strcmp(event->data.hotkey.identifier, "release-mouse") == 0) {
            return vm_platform_run_context_release_mouse(context) == TYPE_STATUS_OK;
        }
        if (strcmp(event->data.hotkey.identifier, "cad") == 0 ||
            strcmp(event->data.hotkey.identifier, "alt-enter") == 0) {
            C_INT cad = strcmp(event->data.hotkey.identifier, "cad") == 0;
            type_unsigned_16 scans[] = { 0x38u, 0x1cu, 0x153u };
            type_unsigned_16 keys[] = { 0x12u, 0x0du, 0x2eu };
            type_unsigned_32 count = cad ? 3u : 2u;
            type_unsigned_32 index;
            if (cad) {
                scans[0] = 0x1du;
                keys[0] = 0x11u;
                scans[1] = 0x38u;
                keys[1] = 0x12u;
            }
            for (index = 0u; index < count; ++index) {
                key.type = UX_EVENT_KEY; key.data.key.scan_code = scans[index];
                key.data.key.key = keys[index]; key.data.key.pressed = LIB_TRUE;
                if (!vm_platform_ux_event_submit(context, &key)) return 0;
            }
            for (index = count; index-- != 0u;) {
                key.data.key.scan_code = scans[index]; key.data.key.key = keys[index];
                key.data.key.pressed = LIB_FALSE;
                if (!vm_platform_ux_event_submit(context, &key)) return 0;
            }
            return 1;
        }
        return 0;
    }
    if (event->type == UX_EVENT_WINDOW_CLOSE) {
        vm_platform_run_handle_report(handle,
            VM_PLATFORM_RUN_EVENT_WINDOW_CLOSE_REQUESTED);
        return 1;
    }
    if (!vm_platform_ux_event_submit(context, event)) return 0;
    if (event->type == UX_EVENT_KEY && event->data.key.scan_code < 512u) {
        handle->ux_pressed_keys[event->data.key.scan_code] = event->data.key.pressed != 0u;
        handle->ux_key_sources[event->data.key.scan_code] =
            event->data.key.pressed ? event->source_identity : 0u;
    }
    return 1;
}

static void vm_platform_failure(void *opaque, lib_u64 source, lib_status status)
{
    vm_platform_run_context *context = opaque;
    (void)source; (void)status;
    if (context != STD_NULL && context->run_handle != STD_NULL)
        vm_platform_run_handle_report(context->run_handle, VM_PLATFORM_RUN_EVENT_STARTUP_FAILED);
}

static void vm_platform_destroy_leaf(vm_platform_run_context *context)
{
    if (context == STD_NULL) return;
    if (context->console != STD_NULL && context->console_binding.release != STD_NULL)
        (C_VOID)context->console_binding.release(context->console_binding.context,
            ux_console_get_console(context->console));
    if (context->console != STD_NULL)
        vm_platform_run_handle_note_console_released(context->run_handle);
    if (context->console != STD_NULL) ux_console_destroy(context->console);
    context->console = STD_NULL;
    if (context->window != STD_NULL) ux_window_destroy(context->window);
    context->window = STD_NULL;
    context->window_active = TYPE_FALSE;
}

static type_status vm_platform_create_leaf(vm_platform_run_context *context, C_INT window)
{
    ux_component_options options = { 0 };
    lib_status status;
    options.input_context = context;
    options.input_sink = vm_platform_run_context_handle_ux_input;
    options.failure_context = context; options.failure_sink = vm_platform_failure;
    options.hotkeys = context->hotkeys;
    if (window) {
        ux_window_options window_options = { 0 };
        window_options.component = options; window_options.initial_title = "NXVM (Running)";
        window_options.initial_frozen = LIB_FALSE;
        status = ux_window_create(&context->window, &window_options);
        context->window_active = status == LIB_STATUS_OK;
    } else {
        /* A composition or integration caller may run a headless session.
         * Console ownership belongs to the product; absent its binding, no
         * native Console leaf exists and execution remains valid. */
        if (context->console_binding.claim == STD_NULL) return TYPE_STATUS_OK;
        status = ux_console_create(&context->console, &options);
        if (status == LIB_STATUS_OK) status = (lib_status)context->console_binding.claim(
            context->console_binding.context, ux_console_get_console(context->console));
        if (status != LIB_STATUS_OK) vm_platform_destroy_leaf(context);
    }
    return (type_status)status;
}

static type_status vm_platform_select_leaf(vm_platform_run_context *context, C_INT window)
{
    if (context == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if ((window && context->window != STD_NULL) || (!window && context->console != STD_NULL)) return TYPE_STATUS_OK;
    vm_platform_destroy_leaf(context);
    return vm_platform_create_leaf(context, window);
}

type_status vm_platform_run_context_create(const vm_platform_execution *execution,
    const vm_platform_host_input_sink *input_sink,
    const core_machine_guest_presentation_mailbox *presentation,
    const core_utils_wait_scope *wait_scope, vm_platform_run_context **out_context)
{
    vm_platform_run_context *context;
    if (out_context == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_context = STD_NULL;
    context = STD_CALLOC(1u, sizeof(*context));
    if (context == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    context->execution = execution;
    if (input_sink != STD_NULL) context->input_sink = *input_sink;
    context->presentation = presentation; context->wait_scope = wait_scope;
    context->core_frame = STD_MALLOC(sizeof(*context->core_frame));
    context->ux_frame = STD_MALLOC(sizeof(*context->ux_frame));
    if (context->core_frame == STD_NULL || context->ux_frame == STD_NULL) {
        vm_platform_run_context_destroy(context); return TYPE_STATUS_NO_MEMORY;
    }
    ux_hotkey_registry_initialize(&context->hotkeys);
    (void)ux_hotkey_registry_register(&context->hotkeys, 'P', UX_HOTKEY_MODIFIER_CONTROL | UX_HOTKEY_MODIFIER_ALT, "pause");
    (void)ux_hotkey_registry_register(&context->hotkeys, 'D', UX_HOTKEY_MODIFIER_CONTROL | UX_HOTKEY_MODIFIER_ALT, "cad");
    (void)ux_hotkey_registry_register(&context->hotkeys, 'F', UX_HOTKEY_MODIFIER_CONTROL | UX_HOTKEY_MODIFIER_ALT, "alt-enter");
    (void)ux_hotkey_registry_register(&context->hotkeys, 'M', UX_HOTKEY_MODIFIER_CONTROL | UX_HOTKEY_MODIFIER_ALT, "release-mouse");
    context->display_mode = VM_PLATFORM_DISPLAY_CONSOLE;
    *out_context = context; return TYPE_STATUS_OK;
}

C_VOID vm_platform_run_context_destroy(vm_platform_run_context *context)
{ if (context != STD_NULL) vm_platform_destroy_leaf(context); if (context != STD_NULL) STD_FREE(context->ux_frame); if (context != STD_NULL) STD_FREE(context->core_frame); STD_FREE(context); }

type_status vm_platform_run_context_set_console_binding(
    vm_platform_run_context *context,
    const vm_platform_console_binding *binding)
{
    if (context == STD_NULL || binding == STD_NULL ||
        binding->claim == STD_NULL || binding->release == STD_NULL ||
        context->console != STD_NULL) return TYPE_STATUS_INVALID_STATE;
    context->console_binding = *binding;
    return TYPE_STATUS_OK;
}

type_status vm_platform_run_context_publish_ux_frame(vm_platform_run_context *context)
{
    type_status status;
    C_INT window;
    if (context == STD_NULL || context->presentation == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    status = core_machine_guest_presentation_mailbox_capture(context->presentation, context->core_frame);
    if (status != TYPE_STATUS_OK || (status = vm_platform_ux_frame_from_core(context->core_frame, context->ux_frame)) != TYPE_STATUS_OK) return status;
    /* Unit-level Core/VM display tests may publish copied frames without a
     * product run. Native leaves exist only for an active product execution. */
    if (context->run_handle == STD_NULL) return TYPE_STATUS_OK;
    window = context->display_mode == VM_PLATFORM_DISPLAY_WINDOW || context->ux_frame->graphics != 0u;
    if ((status = vm_platform_select_leaf(context, window)) != TYPE_STATUS_OK) return status;
    if (!window && context->console == STD_NULL) return TYPE_STATUS_OK;
    return (type_status)(window ? ux_window_publish_frame(context->window, context->ux_frame) : ux_console_publish_frame(context->console, context->ux_frame));
}

type_status vm_platform_host_input_sink_submit(const vm_platform_host_input_sink *sink, const core_machine_guest_input_event *event)
{ return sink == STD_NULL || sink->submit == STD_NULL ? TYPE_STATUS_INVALID_STATE : sink->submit(sink->context, event); }
C_INT vm_platform_run_context_get_window_display(const vm_platform_run_context *context)
{ return context != STD_NULL && context->window_active; }
C_INT vm_platform_run_context_get_display_mode(const vm_platform_run_context *context)
{ return context == STD_NULL ? VM_PLATFORM_DISPLAY_CONSOLE : context->display_mode; }
type_status vm_platform_run_context_set_display_mode(vm_platform_run_context *context, vm_platform_display_mode mode)
{ if (context == STD_NULL || mode > VM_PLATFORM_DISPLAY_WINDOW) return TYPE_STATUS_INVALID_ARGUMENT; context->display_mode = mode; return context->run_handle == STD_NULL ? TYPE_STATUS_OK : vm_platform_select_leaf(context, mode == VM_PLATFORM_DISPLAY_WINDOW); }
type_status vm_platform_run_context_set_window_display(vm_platform_run_context *context, C_INT enabled)
{ return vm_platform_run_context_set_display_mode(context, enabled ? VM_PLATFORM_DISPLAY_WINDOW : VM_PLATFORM_DISPLAY_CONSOLE); }
type_status vm_platform_run_context_set_window_title(vm_platform_run_context *context, const C_CHAR *title)
{ return context == STD_NULL || title == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT : context->window == STD_NULL ? TYPE_STATUS_OK : (type_status)ux_window_set_title(context->window, title); }
type_status vm_platform_run_context_set_mouse_capturable(vm_platform_run_context *context, C_INT capturable)
{ return context == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT : context->window == STD_NULL ? TYPE_STATUS_OK : (type_status)(capturable ? ux_window_unfreeze(context->window) : ux_window_freeze(context->window)); }
type_status vm_platform_run_context_release_mouse(vm_platform_run_context *context)
{ return context == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT : context->window == STD_NULL ? TYPE_STATUS_OK : (type_status)ux_window_release_mouse(context->window); }
type_status vm_platform_run_context_close_window(vm_platform_run_context *context)
{ if (context == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT; if (context->window != STD_NULL) vm_platform_destroy_leaf(context); return TYPE_STATUS_OK; }

type_status vm_platform_run_handle_create(vm_platform_run_handle **out_handle) { vm_platform_run_handle *handle; if (out_handle == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT; *out_handle = STD_CALLOC(1u, sizeof(**out_handle)); if (*out_handle == STD_NULL) return TYPE_STATUS_NO_MEMORY; handle = *out_handle; vm_platform_run_handle_initialize(handle); return TYPE_STATUS_OK; }
C_VOID vm_platform_run_handle_initialize(vm_platform_run_handle *handle) { if (handle != STD_NULL) { STD_MEMSET(handle, 0, sizeof(*handle)); STD_ATOMIC_INIT(&handle->last_event, VM_PLATFORM_RUN_EVENT_NONE); STD_ATOMIC_INIT(&handle->stop_reported, TYPE_FALSE); STD_ATOMIC_INIT(&handle->pause_reported, TYPE_FALSE); STD_ATOMIC_INIT(&handle->window_close_reported, TYPE_FALSE); } }
C_VOID vm_platform_run_handle_destroy(vm_platform_run_handle *handle) { STD_FREE(handle); }
C_INT vm_platform_run_handle_is_active(const vm_platform_run_handle *handle) { return handle != STD_NULL && handle->active; }
C_INT vm_platform_run_handle_is_window_display(const vm_platform_run_handle *handle) { return handle != STD_NULL && handle->context != STD_NULL && handle->context->window_active; }
C_INT vm_platform_run_handle_is_console_display(const vm_platform_run_handle *handle) { return handle != STD_NULL && handle->context != STD_NULL && handle->context->console != STD_NULL; }
C_VOID vm_platform_run_handle_report(vm_platform_run_handle *handle, vm_platform_run_event event) { if (handle == STD_NULL) return; STD_ATOMIC_STORE(&handle->last_event,event); if (event == VM_PLATFORM_RUN_EVENT_STOP_REQUESTED || event == VM_PLATFORM_RUN_EVENT_STARTUP_FAILED) STD_ATOMIC_STORE(&handle->stop_reported,TYPE_TRUE); else if(event == VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED) STD_ATOMIC_STORE(&handle->pause_reported,TYPE_TRUE); else if (event == VM_PLATFORM_RUN_EVENT_WINDOW_CLOSE_REQUESTED) STD_ATOMIC_STORE(&handle->window_close_reported, TYPE_TRUE); }
vm_platform_run_event vm_platform_run_handle_get_last_event(const vm_platform_run_handle *handle) { return handle == STD_NULL ? VM_PLATFORM_RUN_EVENT_NONE : (vm_platform_run_event)STD_ATOMIC_LOAD(&handle->last_event); }
C_INT vm_platform_run_handle_take_stop_report(vm_platform_run_handle *handle) { return handle != STD_NULL && STD_ATOMIC_EXCHANGE(&handle->stop_reported,TYPE_FALSE); }
C_INT vm_platform_run_handle_take_pause_report(vm_platform_run_handle *handle) { return handle != STD_NULL && STD_ATOMIC_EXCHANGE(&handle->pause_reported,TYPE_FALSE); }
C_INT vm_platform_run_handle_take_window_close_report(vm_platform_run_handle *handle) { return handle != STD_NULL && STD_ATOMIC_EXCHANGE(&handle->window_close_reported, TYPE_FALSE); }
C_VOID vm_platform_run_handle_request_presenter_stop(vm_platform_run_handle *handle) { if (handle != STD_NULL && handle->context != STD_NULL) vm_platform_destroy_leaf(handle->context); }

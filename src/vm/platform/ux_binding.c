#include "vm/platform/ux_binding.h"

#include "core/platform/input_interface.h"
#ifdef _WIN32
#include "core/platform/win32/keyboard.h"
#include "lib/ux/win32/input.h"
#endif
#include "vm/platform/platform_internal.h"

#include <limits.h>
#include <stdio.h>

static type_signed_16 vm_platform_ux_mouse_delta(type_signed_32 value)
{
    return value < INT16_MIN ? INT16_MIN : value > INT16_MAX ? INT16_MAX :
        (type_signed_16)value;
}

#ifdef _WIN32
static type_status vm_platform_ux_submit_core(C_VOID *opaque,
    const core_platform_input_event *event)
{
    vm_platform_run_handle *handle = opaque;
    vm_platform_run_context *context = handle == STD_NULL ? STD_NULL :
        (vm_platform_run_context *)handle->context;

    return context == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        vm_platform_host_input_sink_submit(&context->input_sink, event);
}
#endif

static ux_run_state vm_platform_ux_state(C_VOID *opaque)
{
    const vm_platform_run_handle *handle = opaque;
    const vm_platform_run_context *context = handle == STD_NULL ? STD_NULL :
        handle->context;
    vm_platform_execution_lifecycle state = context == STD_NULL ?
        VM_PLATFORM_EXECUTION_STOPPED :
        vm_platform_execution_get_lifecycle_for(context->execution);
    return state == VM_PLATFORM_EXECUTION_RUNNING ? UX_RUN_RUNNING :
        state == VM_PLATFORM_EXECUTION_PAUSED ? UX_RUN_PAUSED : UX_RUN_STOPPED;
}

static C_INT vm_platform_ux_input(C_VOID *opaque, const ux_event *event)
{
    vm_platform_run_handle *handle = opaque;
    vm_platform_run_context *context = handle == STD_NULL ? STD_NULL :
        (vm_platform_run_context *)handle->context;
    core_platform_input_event input;

    if (context == STD_NULL || event == STD_NULL) return TYPE_FALSE;
    STD_MEMSET(&input, 0, sizeof(input));
    if (event->type == UX_EVENT_KEY) {
        input.kind = CORE_PLATFORM_INPUT_KEY;
        input.data.key.scan_code = event->data.key.scan_code;
        input.data.key.virtual_key = event->data.key.virtual_key;
        input.data.key.pressed = event->data.key.pressed;
    } else if (event->type == UX_EVENT_MOUSE) {
        input.kind = CORE_PLATFORM_INPUT_RELATIVE_MOUSE;
        input.data.relative_mouse.delta_x = vm_platform_ux_mouse_delta(
            event->data.mouse.delta_x);
        input.data.relative_mouse.delta_y = vm_platform_ux_mouse_delta(
            event->data.mouse.delta_y);
        input.data.relative_mouse.buttons = (event->data.mouse.left_down ? 1u : 0u) |
            (event->data.mouse.right_down ? 2u : 0u);
    }
#ifdef _WIN32
    else if (event->type == UX_EVENT_TEXT) {
        return core_platform_win32_keyboard_submit_character(handle,
            vm_platform_ux_submit_core,
            event->data.text.scalar) == TYPE_STATUS_OK;
    }
#else
    else if (event->type == UX_EVENT_TEXT && event->data.text.scalar <= 0xffffu) {
        input.kind = CORE_PLATFORM_INPUT_KEY;
        input.data.key.virtual_key = (type_unsigned_16)event->data.text.scalar;
        input.data.key.pressed = TYPE_TRUE;
    }
#endif
    else return TYPE_FALSE;
    return vm_platform_host_input_sink_submit(&context->input_sink, &input) ==
        TYPE_STATUS_OK;
}

static ux_run_result vm_platform_ux_action(C_VOID *opaque, ux_action action,
    ux_event_sink input_sink)
{
    vm_platform_run_handle *handle = opaque;
    if (handle == STD_NULL) return UX_RUN_ERROR_RESULT;
#ifdef _WIN32
    /* Win32 has already delivered the host chord's Ctrl/Alt makes through the
       ordinary input route before it consumes the action key.  Always clear
       those modifiers through the shared normalizer helpers: pause/release
       must not leave a guest modifier down, and the two injected chords must
       start from a neutral guest state. */
    if (action == UX_ACTION_PAUSE_TOGGLE || action == UX_ACTION_RELEASE_MOUSE) {
        if (!ux_win32_keyboard_release_ctrl_alt(handle, input_sink))
            return UX_RUN_ERROR_RESULT;
    }
#endif
    if (action == UX_ACTION_PAUSE_TOGGLE) {
        vm_platform_run_handle_report(handle, VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED);
        return UX_RUN_CONTINUE;
    }
    if (action == UX_ACTION_RELEASE_MOUSE) {
        vm_platform_run_handle_report(handle, VM_PLATFORM_RUN_EVENT_MOUSE_RELEASE_REQUESTED);
    } else if (action == UX_ACTION_SEND_CTRL_ALT_DEL ||
        action == UX_ACTION_SEND_ALT_ENTER) {
#ifdef _WIN32
        if (!(action == UX_ACTION_SEND_CTRL_ALT_DEL ?
                ux_win32_keyboard_submit_ctrl_alt_del(handle, input_sink) :
                ux_win32_keyboard_submit_alt_enter(handle, input_sink))) {
            return UX_RUN_ERROR_RESULT;
        }
#else
        ux_event event = { 0 };
        type_unsigned_16 scans[] = { 0x1du, 0x38u, 0x153u };
        type_unsigned_16 virtual_keys[] = { 0x11u, 0x12u, 0x2eu };
        type_unsigned_32 count = action == UX_ACTION_SEND_CTRL_ALT_DEL ? 3u : 2u;

        if (action == UX_ACTION_SEND_ALT_ENTER) {
            scans[0u] = 0x38u;
            scans[1u] = 0x1cu;
            virtual_keys[0u] = 0x12u;
            virtual_keys[1u] = 0x0du;
        }
        type_unsigned_32 index;

        event.type = UX_EVENT_KEY;
        for (index = 0u; index < count; ++index) {
            event.data.key.scan_code = scans[index];
            event.data.key.virtual_key = virtual_keys[index];
            event.data.key.pressed = TYPE_TRUE;
            if (!input_sink(handle, &event)) return UX_RUN_ERROR_RESULT;
        }
        for (index = count; index != 0u; --index) {
            event.data.key.scan_code = scans[index - 1u];
            event.data.key.virtual_key = virtual_keys[index - 1u];
            event.data.key.pressed = TYPE_FALSE;
            if (!input_sink(handle, &event)) return UX_RUN_ERROR_RESULT;
        }
#endif
    }
    return UX_RUN_CONTINUE;
}

static C_VOID vm_platform_ux_title(C_VOID *opaque, C_CHAR *buffer,
    type_unsigned_32 buffer_size)
{
    const ux_run_state state = vm_platform_ux_state(opaque);
    const C_CHAR *suffix = state == UX_RUN_RUNNING ? "Running" :
        state == UX_RUN_PAUSED ? "Paused" : state == UX_RUN_ERROR ? "Error" :
        "Stopped";

    if (buffer == STD_NULL || buffer_size == 0u) return;
    (C_VOID)snprintf(buffer, buffer_size, "NXVM (%s)", suffix);
}

static ux_run_result vm_platform_ux_close(C_VOID *opaque, ux_event_sink input_sink)
{
    return vm_platform_ux_action(opaque, UX_ACTION_PAUSE_TOGGLE, input_sink);
}

type_status vm_platform_ux_binding_initialize(const vm_platform_run_context *context,
    vm_platform_run_handle *handle, ux_binding *out_binding)
{
    if (context == STD_NULL || handle == STD_NULL || out_binding == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMSET(out_binding, 0, sizeof(*out_binding));
    /* The binding is the one product-side association of a live session with
       a UX loop; both callbacks dereference this private, VM-owned link. */
    handle->context = context;
    out_binding->context = handle;
    out_binding->mailbox = context->ux_mailbox;
    out_binding->router = (ux_router *)&context->ux_router;
    out_binding->actions = &context->ux_actions;
    out_binding->input_sink = vm_platform_ux_input;
    out_binding->get_state = vm_platform_ux_state;
    out_binding->handle_action = vm_platform_ux_action;
    out_binding->handle_close = vm_platform_ux_close;
    out_binding->get_title = vm_platform_ux_title;
    return TYPE_STATUS_OK;
}

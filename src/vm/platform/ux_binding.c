#include "vm/platform/ux_binding.h"

#include "core/machine/guest_input_interface.h"
#include "vm/platform/platform_internal.h"

#include <limits.h>
#include <stdio.h>

static type_signed_16 vm_platform_ux_mouse_delta(type_signed_32 value)
{
    return value < INT16_MIN ? INT16_MIN : value > INT16_MAX ? INT16_MAX :
        (type_signed_16)value;
}

static C_INT vm_platform_ux_map_key_identity(type_unsigned_32 identity,
    type_unsigned_16 *scan_code, type_unsigned_16 *virtual_key)
{
    type_unsigned_16 scan = 0u;
    type_unsigned_16 key = 0u;

    switch (identity) {
    case UX_KEY_ENTER: scan = 0x1cu; key = 0x0du; break;
    case UX_KEY_BACKSPACE: scan = 0x0eu; key = 0x08u; break;
    case UX_KEY_F1: scan = 0x3bu; break;
    case UX_KEY_F2: scan = 0x3cu; break;
    case UX_KEY_F3: scan = 0x3du; break;
    case UX_KEY_F4: scan = 0x3eu; break;
    case UX_KEY_F5: scan = 0x3fu; break;
    case UX_KEY_F6: scan = 0x40u; break;
    case UX_KEY_F7: scan = 0x41u; break;
    case UX_KEY_F8: scan = 0x42u; break;
    case UX_KEY_F9: scan = 0x43u; break;
    case UX_KEY_F10: scan = 0x44u; break;
    case UX_KEY_F11: scan = 0x57u; break;
    case UX_KEY_F12: scan = 0x58u; break;
    case UX_KEY_UP: scan = 0x48u; break;
    case UX_KEY_DOWN: scan = 0x50u; break;
    case UX_KEY_LEFT: scan = 0x4bu; break;
    case UX_KEY_RIGHT: scan = 0x4du; break;
    case UX_KEY_HOME: scan = 0x47u; break;
    case UX_KEY_END: scan = 0x4fu; break;
    case UX_KEY_PAGE_UP: scan = 0x49u; break;
    case UX_KEY_PAGE_DOWN: scan = 0x51u; break;
    case UX_KEY_INSERT: scan = 0x52u; break;
    case UX_KEY_DELETE: scan = 0x53u; break;
    default: return TYPE_FALSE;
    }
    *scan_code = scan;
    *virtual_key = key;
    return TYPE_TRUE;
}

static C_INT vm_platform_ux_action_key(vm_platform_run_handle *handle,
    ux_event_sink input_sink, type_unsigned_16 scan_code,
    type_unsigned_16 virtual_key, C_INT pressed);

static ux_run_state vm_platform_ux_state(C_VOID *opaque)
{
    const vm_platform_run_handle *handle = opaque;
    const vm_platform_run_context *context = handle == STD_NULL ? STD_NULL :
        handle->context;
    lib_session_lifecycle state = context == STD_NULL ? LIB_SESSION_STOPPED :
        lib_session_executor_get_lifecycle(context->execution);
    return state == LIB_SESSION_RUNNING ? UX_RUN_RUNNING :
        state == LIB_SESSION_PAUSED ? UX_RUN_PAUSED : UX_RUN_STOPPED;
}

C_INT vm_platform_ux_event_submit(const vm_platform_run_context *context,
    const ux_event *event)
{
    core_machine_guest_input_event input;

    if (context == STD_NULL || event == STD_NULL) return TYPE_FALSE;
    STD_MEMSET(&input, 0, sizeof(input));
    if (event->type == UX_EVENT_KEY) {
        type_unsigned_16 scan_code = event->data.key.scan_code;
        type_unsigned_16 virtual_key = (type_unsigned_16)event->data.key.virtual_key;

        if (scan_code == 0u && !vm_platform_ux_map_key_identity(
                event->data.key.virtual_key, &scan_code, &virtual_key))
            return TYPE_FALSE;
        input.kind = CORE_MACHINE_GUEST_INPUT_KEY;
        input.data.key.scan_code = scan_code;
        input.data.key.virtual_key = virtual_key;
        input.data.key.pressed = event->data.key.pressed;
    } else if (event->type == UX_EVENT_MOUSE) {
        input.kind = CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE;
        input.data.relative_mouse.delta_x = vm_platform_ux_mouse_delta(
            event->data.mouse.delta_x);
        input.data.relative_mouse.delta_y = vm_platform_ux_mouse_delta(
            event->data.mouse.delta_y);
        input.data.relative_mouse.buttons =
            (event->data.mouse.buttons & UX_MOUSE_BUTTON_LEFT ? 1u : 0u) |
            (event->data.mouse.buttons & UX_MOUSE_BUTTON_RIGHT ? 2u : 0u);
    }
#ifdef _WIN32
    else if (event->type == UX_EVENT_TEXT) return TYPE_FALSE;
#else
    else if (event->type == UX_EVENT_TEXT && event->data.text.scalar <= 0xffffu) {
        input.kind = CORE_MACHINE_GUEST_INPUT_KEY;
        input.data.key.virtual_key = (type_unsigned_16)event->data.text.scalar;
        input.data.key.pressed = TYPE_TRUE;
    }
#endif
    else return TYPE_FALSE;
    if (vm_platform_host_input_sink_submit(&context->input_sink, &input) !=
        TYPE_STATUS_OK) return TYPE_FALSE;
    return TYPE_TRUE;
}

static C_INT vm_platform_ux_input(C_VOID *opaque, const ux_event *event)
{
    vm_platform_run_handle *handle = opaque;
    const vm_platform_run_context *context = handle == STD_NULL ? STD_NULL :
        handle->context;

    if (!vm_platform_ux_event_submit(context, event)) return TYPE_FALSE;
    if (event->type == UX_EVENT_KEY && event->data.key.scan_code < 512u)
        handle->ux_pressed_keys[event->data.key.scan_code] =
            event->data.key.pressed != 0u;
    return TYPE_TRUE;
}

static C_INT vm_platform_ux_action_key(vm_platform_run_handle *handle,
    ux_event_sink input_sink, type_unsigned_16 scan_code,
    type_unsigned_16 virtual_key, C_INT pressed)
{
    ux_event event = { 0 };

    event.type = UX_EVENT_KEY;
    event.data.key.scan_code = scan_code;
    event.data.key.virtual_key = virtual_key;
    event.data.key.pressed = pressed;
    return input_sink(handle, &event);
}

static C_INT vm_platform_ux_release_inputs(C_VOID *opaque,
    ux_event_sink input_sink)
{
    vm_platform_run_handle *handle = opaque;
    ux_event mouse = { 0 };
    type_unsigned_32 scan_code;

    if (handle == STD_NULL || input_sink == STD_NULL) return TYPE_FALSE;
    mouse.type = UX_EVENT_MOUSE;
    mouse.data.mouse.relative = TYPE_TRUE;
    if (!input_sink(handle, &mouse)) return TYPE_FALSE;
    for (scan_code = 0u; scan_code < 512u; ++scan_code) {
        if (handle->ux_pressed_keys[scan_code] &&
            !vm_platform_ux_action_key(handle, input_sink,
                (type_unsigned_16)scan_code, 0u, TYPE_FALSE))
            return TYPE_FALSE;
    }
    return TYPE_TRUE;
}

static ux_run_result vm_platform_ux_action(C_VOID *opaque, ux_action action,
    ux_event_sink input_sink)
{
    vm_platform_run_handle *handle = opaque;
    if (handle == STD_NULL || input_sink == STD_NULL) return UX_RUN_ERROR_RESULT;
    if (action == VM_PLATFORM_UX_ACTION_PAUSE_TOGGLE) {
        vm_platform_run_handle_report(handle, VM_PLATFORM_RUN_EVENT_PAUSE_REQUESTED);
        return UX_RUN_CONTINUE;
    }
    if (action == VM_PLATFORM_UX_ACTION_RELEASE_MOUSE) {
        vm_platform_run_handle_report(handle, VM_PLATFORM_RUN_EVENT_MOUSE_RELEASE_REQUESTED);
    } else if (action == VM_PLATFORM_UX_ACTION_SEND_CTRL_ALT_DEL ||
        action == VM_PLATFORM_UX_ACTION_SEND_ALT_ENTER) {
        if (action == VM_PLATFORM_UX_ACTION_SEND_CTRL_ALT_DEL) {
            if (!vm_platform_ux_action_key(handle, input_sink, 0x1du, 0x11u, TYPE_TRUE) ||
                !vm_platform_ux_action_key(handle, input_sink, 0x38u, 0x12u, TYPE_TRUE) ||
                !vm_platform_ux_action_key(handle, input_sink, 0x153u, 0x2eu, TYPE_TRUE) ||
                !vm_platform_ux_action_key(handle, input_sink, 0x153u, 0x2eu, TYPE_FALSE) ||
                !vm_platform_ux_action_key(handle, input_sink, 0x38u, 0x12u, TYPE_FALSE) ||
                !vm_platform_ux_action_key(handle, input_sink, 0x1du, 0x11u, TYPE_FALSE))
                return UX_RUN_ERROR_RESULT;
        } else if (!vm_platform_ux_action_key(handle, input_sink, 0x38u, 0x12u,
                TYPE_TRUE) || !vm_platform_ux_action_key(handle, input_sink, 0x1cu,
                0x0du, TYPE_TRUE) || !vm_platform_ux_action_key(handle, input_sink,
                0x1cu, 0x0du, TYPE_FALSE) || !vm_platform_ux_action_key(handle,
                input_sink, 0x38u, 0x12u, TYPE_FALSE)) return UX_RUN_ERROR_RESULT;
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
    return vm_platform_ux_action(opaque, VM_PLATFORM_UX_ACTION_PAUSE_TOGGLE,
        input_sink);
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
    out_binding->release_inputs = vm_platform_ux_release_inputs;
    out_binding->get_state = vm_platform_ux_state;
    out_binding->handle_action = vm_platform_ux_action;
    out_binding->handle_close = vm_platform_ux_close;
    out_binding->get_title = vm_platform_ux_title;
    return TYPE_STATUS_OK;
}

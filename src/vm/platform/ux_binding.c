#include "vm/platform/ux_binding.h"

#include "core/machine/guest_input_interface.h"
#include "vm/platform/platform_internal.h"

#include <limits.h>

static C_INT vm_platform_ux_key_scan(ux_key key, type_unsigned_16 *scan)
{
    static const struct { ux_key key; type_unsigned_16 scan; } map[] = {
        { UX_KEY_ENTER, 0x1cu }, { UX_KEY_BACKSPACE, 0x0eu },
        { UX_KEY_F1, 0x3bu }, { UX_KEY_F2, 0x3cu }, { UX_KEY_F3, 0x3du },
        { UX_KEY_F4, 0x3eu }, { UX_KEY_F5, 0x3fu }, { UX_KEY_F6, 0x40u },
        { UX_KEY_F7, 0x41u }, { UX_KEY_F8, 0x42u }, { UX_KEY_F9, 0x43u },
        { UX_KEY_F10, 0x44u }, { UX_KEY_F11, 0x57u }, { UX_KEY_F12, 0x58u },
        { UX_KEY_UP, 0x48u }, { UX_KEY_DOWN, 0x50u }, { UX_KEY_LEFT, 0x4bu },
        { UX_KEY_RIGHT, 0x4du }, { UX_KEY_HOME, 0x47u }, { UX_KEY_END, 0x4fu },
        { UX_KEY_PAGE_UP, 0x49u }, { UX_KEY_PAGE_DOWN, 0x51u },
        { UX_KEY_INSERT, 0x52u }, { UX_KEY_DELETE, 0x53u }
    };
    STD_SIZE_T index;
    if (scan == STD_NULL) return TYPE_FALSE;
    for (index = 0u; index < sizeof(map) / sizeof(map[0]); ++index)
        if (map[index].key == key) { *scan = map[index].scan; return TYPE_TRUE; }
    return TYPE_FALSE;
}

static type_signed_16 vm_platform_ux_mouse_delta(type_signed_32 value)
{ return value < INT16_MIN ? INT16_MIN : value > INT16_MAX ? INT16_MAX : (type_signed_16)value; }

C_INT vm_platform_ux_event_submit(const vm_platform_run_context *context,
    const ux_input_event *event)
{
    core_machine_guest_input_event input = { 0 };
    type_unsigned_16 scan = 0u;
    if (context == STD_NULL || event == STD_NULL) return TYPE_FALSE;
    if (event->type == UX_EVENT_KEY) {
        scan = event->data.key.scan_code;
        if (scan == 0u && !vm_platform_ux_key_scan(event->data.key.key, &scan)) return TYPE_FALSE;
        input.kind = CORE_MACHINE_GUEST_INPUT_KEY;
        input.data.key.scan_code = scan;
        input.data.key.virtual_key = (type_unsigned_16)event->data.key.key;
        input.data.key.pressed = event->data.key.pressed != 0u;
    } else if (event->type == UX_EVENT_TEXT && event->data.text.scalar <= 0xffffu) {
        input.kind = CORE_MACHINE_GUEST_INPUT_KEY;
        input.data.key.virtual_key = (type_unsigned_16)event->data.text.scalar;
        input.data.key.pressed = TYPE_TRUE;
    } else if (event->type == UX_EVENT_MOUSE) {
        input.kind = CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE;
        input.data.relative_mouse.delta_x = vm_platform_ux_mouse_delta(event->data.mouse.delta_x);
        input.data.relative_mouse.delta_y = vm_platform_ux_mouse_delta(event->data.mouse.delta_y);
        input.data.relative_mouse.buttons = (event->data.mouse.buttons & UX_MOUSE_BUTTON_LEFT ? 1u : 0u) |
            (event->data.mouse.buttons & UX_MOUSE_BUTTON_RIGHT ? 2u : 0u);
    } else return TYPE_FALSE;
    return vm_platform_host_input_sink_submit(&context->input_sink, &input) == TYPE_STATUS_OK;
}

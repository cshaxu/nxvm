#include "lib/base/base.h"
#include "lib/ux/linux/input.h"

lib_bool ux_linux_key_to_event(ux_linux_key key, ux_event *out_event)
{
    lib_u16 scan_code;
    lib_u16 virtual_key = 0u;

    if (out_event == LIB_NULL) return LIB_FALSE;
    switch (key) {
    case UX_LINUX_KEY_ENTER: scan_code = 0x1cu; virtual_key = 0x0du; break;
    case UX_LINUX_KEY_BACKSPACE: scan_code = 0x0eu; virtual_key = 0x08u; break;
    case UX_LINUX_KEY_F1: scan_code = 0x3bu; break;
    case UX_LINUX_KEY_F2: scan_code = 0x3cu; break;
    case UX_LINUX_KEY_F3: scan_code = 0x3du; break;
    case UX_LINUX_KEY_F4: scan_code = 0x3eu; break;
    case UX_LINUX_KEY_F5: scan_code = 0x3fu; break;
    case UX_LINUX_KEY_F6: scan_code = 0x40u; break;
    case UX_LINUX_KEY_F7: scan_code = 0x41u; break;
    case UX_LINUX_KEY_F8: scan_code = 0x42u; break;
    case UX_LINUX_KEY_F9: scan_code = 0x43u; break;
    case UX_LINUX_KEY_F10: scan_code = 0x44u; break;
    case UX_LINUX_KEY_F11: scan_code = 0x57u; break;
    case UX_LINUX_KEY_F12: scan_code = 0x58u; break;
    case UX_LINUX_KEY_UP: scan_code = 0x48u; break;
    case UX_LINUX_KEY_DOWN: scan_code = 0x50u; break;
    case UX_LINUX_KEY_LEFT: scan_code = 0x4bu; break;
    case UX_LINUX_KEY_RIGHT: scan_code = 0x4du; break;
    case UX_LINUX_KEY_HOME: scan_code = 0x47u; break;
    case UX_LINUX_KEY_END: scan_code = 0x4fu; break;
    case UX_LINUX_KEY_PAGE_UP: scan_code = 0x49u; break;
    case UX_LINUX_KEY_PAGE_DOWN: scan_code = 0x51u; break;
    case UX_LINUX_KEY_INSERT: scan_code = 0x52u; break;
    case UX_LINUX_KEY_DELETE: scan_code = 0x53u; break;
    default: return LIB_FALSE;
    }
    memset(out_event, 0, sizeof(*out_event));
    out_event->type = UX_EVENT_KEY;
    out_event->data.key.scan_code = scan_code;
    out_event->data.key.virtual_key = virtual_key;
    out_event->data.key.pressed = LIB_TRUE;
    return LIB_TRUE;
}

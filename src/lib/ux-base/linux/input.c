#include "lib/base/base_interface.h"
#include "lib/ux-base/linux/input.h"

#include <string.h>

lib_bool ux_linux_key_to_event(ux_linux_key key, ux_event *out_event)
{
    lib_u32 key_identity;

    if (out_event == LIB_NULL) return LIB_FALSE;
    switch (key) {
    case UX_LINUX_KEY_ENTER: key_identity = UX_KEY_ENTER; break;
    case UX_LINUX_KEY_BACKSPACE: key_identity = UX_KEY_BACKSPACE; break;
    case UX_LINUX_KEY_F1: key_identity = UX_KEY_F1; break;
    case UX_LINUX_KEY_F2: key_identity = UX_KEY_F2; break;
    case UX_LINUX_KEY_F3: key_identity = UX_KEY_F3; break;
    case UX_LINUX_KEY_F4: key_identity = UX_KEY_F4; break;
    case UX_LINUX_KEY_F5: key_identity = UX_KEY_F5; break;
    case UX_LINUX_KEY_F6: key_identity = UX_KEY_F6; break;
    case UX_LINUX_KEY_F7: key_identity = UX_KEY_F7; break;
    case UX_LINUX_KEY_F8: key_identity = UX_KEY_F8; break;
    case UX_LINUX_KEY_F9: key_identity = UX_KEY_F9; break;
    case UX_LINUX_KEY_F10: key_identity = UX_KEY_F10; break;
    case UX_LINUX_KEY_F11: key_identity = UX_KEY_F11; break;
    case UX_LINUX_KEY_F12: key_identity = UX_KEY_F12; break;
    case UX_LINUX_KEY_UP: key_identity = UX_KEY_UP; break;
    case UX_LINUX_KEY_DOWN: key_identity = UX_KEY_DOWN; break;
    case UX_LINUX_KEY_LEFT: key_identity = UX_KEY_LEFT; break;
    case UX_LINUX_KEY_RIGHT: key_identity = UX_KEY_RIGHT; break;
    case UX_LINUX_KEY_HOME: key_identity = UX_KEY_HOME; break;
    case UX_LINUX_KEY_END: key_identity = UX_KEY_END; break;
    case UX_LINUX_KEY_PAGE_UP: key_identity = UX_KEY_PAGE_UP; break;
    case UX_LINUX_KEY_PAGE_DOWN: key_identity = UX_KEY_PAGE_DOWN; break;
    case UX_LINUX_KEY_INSERT: key_identity = UX_KEY_INSERT; break;
    case UX_LINUX_KEY_DELETE: key_identity = UX_KEY_DELETE; break;
    default: return LIB_FALSE;
    }
    memset(out_event, 0, sizeof(*out_event));
    out_event->type = UX_EVENT_KEY;
    out_event->data.key.key = key_identity;
    out_event->data.key.pressed = LIB_TRUE;
    return LIB_TRUE;
}

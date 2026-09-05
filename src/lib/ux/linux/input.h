#ifndef UX_LINUX_INPUT_H
#define UX_LINUX_INPUT_H

#include "lib/ux/event.h"

typedef enum ux_linux_key {
    UX_LINUX_KEY_ENTER,
    UX_LINUX_KEY_BACKSPACE,
    UX_LINUX_KEY_F1,
    UX_LINUX_KEY_F2,
    UX_LINUX_KEY_F3,
    UX_LINUX_KEY_F4,
    UX_LINUX_KEY_F5,
    UX_LINUX_KEY_F6,
    UX_LINUX_KEY_F7,
    UX_LINUX_KEY_F8,
    UX_LINUX_KEY_F9,
    UX_LINUX_KEY_F10,
    UX_LINUX_KEY_F11,
    UX_LINUX_KEY_F12,
    UX_LINUX_KEY_UP,
    UX_LINUX_KEY_DOWN,
    UX_LINUX_KEY_LEFT,
    UX_LINUX_KEY_RIGHT,
    UX_LINUX_KEY_HOME,
    UX_LINUX_KEY_END,
    UX_LINUX_KEY_PAGE_UP,
    UX_LINUX_KEY_PAGE_DOWN,
    UX_LINUX_KEY_INSERT,
    UX_LINUX_KEY_DELETE
} ux_linux_key;

/* Translate a semantic terminal key into one guest Set-1 make transition.
 * Printable text is intentionally not represented here: it remains Unicode
 * text at the native terminal boundary. */
type_bool ux_linux_key_to_event(ux_linux_key key, ux_event *out_event);

#endif

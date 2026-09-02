#ifndef CORE_PLATFORM_WIN32_KEYBOARD_H
#define CORE_PLATFORM_WIN32_KEYBOARD_H

#include "type.h"
#include "core/platform/input_interface.h"

typedef type_status (*core_platform_win32_keyboard_submit)(C_VOID *context,
    const core_platform_input_event *event);

/* Converts one Windows Unicode scalar to the physical key sequence selected by
 * the active host layout.  It never owns guest state or text input. */
type_status core_platform_win32_keyboard_submit_character(C_VOID *context,
    core_platform_win32_keyboard_submit submit, type_unsigned_32 scalar);

#endif

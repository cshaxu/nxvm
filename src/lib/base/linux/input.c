#include "lib/types/types_interface.h"
#include "lib/base/linux/input.h"
#include "lib/base/input.h"

static lib_u32 base_linux_key_identity(base_linux_key key)
{
    lib_u32 key_identity;

    switch (key) {
    case BASE_LINUX_KEY_ENTER: key_identity = LIB_KEY_ENTER; break;
    case BASE_LINUX_KEY_BACKSPACE: key_identity = LIB_KEY_BACKSPACE; break;
    case BASE_LINUX_KEY_F1: key_identity = LIB_KEY_F1; break;
    case BASE_LINUX_KEY_F2: key_identity = LIB_KEY_F2; break;
    case BASE_LINUX_KEY_F3: key_identity = LIB_KEY_F3; break;
    case BASE_LINUX_KEY_F4: key_identity = LIB_KEY_F4; break;
    case BASE_LINUX_KEY_F5: key_identity = LIB_KEY_F5; break;
    case BASE_LINUX_KEY_F6: key_identity = LIB_KEY_F6; break;
    case BASE_LINUX_KEY_F7: key_identity = LIB_KEY_F7; break;
    case BASE_LINUX_KEY_F8: key_identity = LIB_KEY_F8; break;
    case BASE_LINUX_KEY_F9: key_identity = LIB_KEY_F9; break;
    case BASE_LINUX_KEY_F10: key_identity = LIB_KEY_F10; break;
    case BASE_LINUX_KEY_F11: key_identity = LIB_KEY_F11; break;
    case BASE_LINUX_KEY_F12: key_identity = LIB_KEY_F12; break;
    case BASE_LINUX_KEY_UP: key_identity = LIB_KEY_UP; break;
    case BASE_LINUX_KEY_DOWN: key_identity = LIB_KEY_DOWN; break;
    case BASE_LINUX_KEY_LEFT: key_identity = LIB_KEY_LEFT; break;
    case BASE_LINUX_KEY_RIGHT: key_identity = LIB_KEY_RIGHT; break;
    case BASE_LINUX_KEY_HOME: key_identity = LIB_KEY_HOME; break;
    case BASE_LINUX_KEY_END: key_identity = LIB_KEY_END; break;
    case BASE_LINUX_KEY_PAGE_UP: key_identity = LIB_KEY_PAGE_UP; break;
    case BASE_LINUX_KEY_PAGE_DOWN: key_identity = LIB_KEY_PAGE_DOWN; break;
    case BASE_LINUX_KEY_INSERT: key_identity = LIB_KEY_INSERT; break;
    case BASE_LINUX_KEY_DELETE: key_identity = LIB_KEY_DELETE; break;
    default: return 0u;
    }
    return key_identity;
}

lib_bool base_keyboard_platform_transition(lib_u16 scan, lib_u16 raw_key,
    lib_u16 *out_scan, lib_u32 *out_key)
{
    (void)scan;
    *out_scan = 0u;
    *out_key = base_linux_key_identity((base_linux_key)raw_key);
    return *out_key != 0u;
}

lib_bool base_keyboard_platform_map_scalar(lib_u32 scalar,
    lib_u16 *out_raw_key, lib_u8 *out_modifiers)
{
    /* Terminal text has no portable physical-key layout: use TEXT delivery. */
    (void)scalar; (void)out_raw_key; (void)out_modifiers;
    return LIB_FALSE;
}

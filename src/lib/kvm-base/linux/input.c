#include "lib/types/types_interface.h"
#include "lib/kvm-base/linux/input.h"
#include "lib/kvm-base/input.h"

static lib_u32 kvm_linkvm_key_identity(kvm_linkvm_key key)
{
    lib_u32 key_identity;

    switch (key) {
    case KVM_LINKVM_KEY_ENTER: key_identity = KVM_KEY_ENTER; break;
    case KVM_LINKVM_KEY_BACKSPACE: key_identity = KVM_KEY_BACKSPACE; break;
    case KVM_LINKVM_KEY_F1: key_identity = KVM_KEY_F1; break;
    case KVM_LINKVM_KEY_F2: key_identity = KVM_KEY_F2; break;
    case KVM_LINKVM_KEY_F3: key_identity = KVM_KEY_F3; break;
    case KVM_LINKVM_KEY_F4: key_identity = KVM_KEY_F4; break;
    case KVM_LINKVM_KEY_F5: key_identity = KVM_KEY_F5; break;
    case KVM_LINKVM_KEY_F6: key_identity = KVM_KEY_F6; break;
    case KVM_LINKVM_KEY_F7: key_identity = KVM_KEY_F7; break;
    case KVM_LINKVM_KEY_F8: key_identity = KVM_KEY_F8; break;
    case KVM_LINKVM_KEY_F9: key_identity = KVM_KEY_F9; break;
    case KVM_LINKVM_KEY_F10: key_identity = KVM_KEY_F10; break;
    case KVM_LINKVM_KEY_F11: key_identity = KVM_KEY_F11; break;
    case KVM_LINKVM_KEY_F12: key_identity = KVM_KEY_F12; break;
    case KVM_LINKVM_KEY_UP: key_identity = KVM_KEY_UP; break;
    case KVM_LINKVM_KEY_DOWN: key_identity = KVM_KEY_DOWN; break;
    case KVM_LINKVM_KEY_LEFT: key_identity = KVM_KEY_LEFT; break;
    case KVM_LINKVM_KEY_RIGHT: key_identity = KVM_KEY_RIGHT; break;
    case KVM_LINKVM_KEY_HOME: key_identity = KVM_KEY_HOME; break;
    case KVM_LINKVM_KEY_END: key_identity = KVM_KEY_END; break;
    case KVM_LINKVM_KEY_PAGE_UP: key_identity = KVM_KEY_PAGE_UP; break;
    case KVM_LINKVM_KEY_PAGE_DOWN: key_identity = KVM_KEY_PAGE_DOWN; break;
    case KVM_LINKVM_KEY_INSERT: key_identity = KVM_KEY_INSERT; break;
    case KVM_LINKVM_KEY_DELETE: key_identity = KVM_KEY_DELETE; break;
    default: return 0u;
    }
    return key_identity;
}

lib_bool kvm_keyboard_platform_transition(lib_u16 scan, lib_u16 raw_key,
    lib_u16 *out_scan, lib_u32 *out_key)
{
    (void)scan;
    *out_scan = 0u;
    *out_key = kvm_linkvm_key_identity((kvm_linkvm_key)raw_key);
    return *out_key != 0u;
}

lib_bool kvm_keyboard_platform_map_scalar(lib_u32 scalar,
    lib_u16 *out_raw_key, lib_u8 *out_modifiers)
{
    /* Terminal text has no portable physical-key layout: use TEXT delivery. */
    (void)scalar; (void)out_raw_key; (void)out_modifiers;
    return LIB_FALSE;
}

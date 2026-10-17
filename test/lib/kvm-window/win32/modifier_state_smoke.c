#include "lib/types/test.h"
#include "lib/types/types_interface.h"
#include "lib/types/win32/input.h"
static lib_u32 pressed;
static lib_win32_key_state fake_key_state(lib_i32 key)
{
    lib_u32 bit = key == LIB_WIN32_KEY_CONTROL ? 1u : key == LIB_WIN32_KEY_ALT ? 2u : key == LIB_WIN32_KEY_SHIFT ? 4u : 0u;
    return (lib_win32_key_state)((pressed & bit) != 0u ? 0x8000u : 0u);
}

#undef lib_win32_get_key_state
#define lib_win32_get_key_state fake_key_state
#include "lib/kvm-window/win32/input.c"
#define CHECK(expression) do { if (!(expression)) return __LINE__; } while (0)

int main(void)
{
    for (pressed = LIB_FALSE; pressed != 8u; ++pressed) {
        lib_u8 expected = 0u;
        if ((pressed & 1u) != 0u) expected |= KVM_HOTKEY_MODIFIER_CONTROL;
        if ((pressed & 2u) != 0u) expected |= KVM_HOTKEY_MODIFIER_ALT;
        if ((pressed & 4u) != 0u) expected |= KVM_HOTKEY_MODIFIER_SHIFT;
        CHECK(kvm_window_modifiers_from_key_state() == expected);
    }
    return 0;
}

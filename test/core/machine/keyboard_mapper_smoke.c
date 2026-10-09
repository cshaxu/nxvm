#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "core/machine/keyboard_mapper_interface.h"
#include "core/board-base/machine_board_interface.h"

static lib_i32 vm_keyboard_native_set2_expect(kvm_key key, lib_u16 scan,
    lib_u32 flags, lib_bool pressed, const lib_u8 *expected, lib_u8 count)
{
    kvm_input_event event = {0};
    vm_profile_default_keyboard_sequence sequence;
    lib_u8 index;

    event.type = KVM_EVENT_KEY;
    event.data.key.key = key;
    event.data.key.scan_code = scan;
    event.data.key.flags = flags;
    event.data.key.pressed = pressed;
    if (vm_profile_default_keyboard_map_kvm_event(&event, &sequence) !=
            LIB_STATUS_OK || sequence.count != count) {
        return 0;
    }
    for (index = 0u; index < count; ++index) {
        if (sequence.bytes[index] != expected[index]) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    static const lib_u8 function_set1[] = { 0x3bu, 0x3cu, 0x3du,
        0x3eu, 0x3fu, 0x40u, 0x41u, 0x42u, 0x43u, 0x44u, 0x57u, 0x58u };
    static const lib_u8 function_set2[] = { 0x05u, 0x06u, 0x04u,
        0x0cu, 0x03u, 0x0bu, 0x83u, 0x0au, 0x01u, 0x09u, 0x78u, 0x07u };
    static const lib_u8 a_make[] = { 0x1cu };
    static const lib_u8 a_break[] = { 0xf0u, 0x1cu };
    static const lib_u8 up_make[] = { 0xe0u, 0x75u };
    static const lib_u8 up_break[] = { 0xe0u, 0xf0u, 0x75u };
    static const lib_u8 right_make[] = { 0xe0u, 0x74u };
    static const lib_u8 left_control_make[] = { 0x14u };
    static const lib_u8 right_control_make[] = { 0xe0u, 0x14u };
    static const lib_u8 left_alt_break[] = { 0xf0u, 0x11u };
    static const lib_u8 right_alt_break[] = { 0xe0u, 0xf0u, 0x11u };
    static const lib_u8 pause_make[] = {
        0xe1u, 0x14u, 0x77u, 0xe1u, 0xf0u, 0x14u, 0xf0u, 0x77u
    };
    lib_u8 index;
    vm_profile_default_keyboard_sequence set1;

    {
        kvm_input_event event = { .type = KVM_EVENT_KEY };
        event.data.key.key = KVM_KEY_UP;
        event.data.key.scan_code = 0x48u;
        event.data.key.flags = KVM_KEY_FLAG_EXTENDED;
        event.data.key.pressed = LIB_TRUE;
        if (vm_profile_default_keyboard_map_kvm_event_for_scan_set(&event,
                CORE_MACHINE_KEYBOARD_SCAN_SET_1, &set1) != LIB_STATUS_OK ||
            set1.count != 2u || set1.bytes[0u] != 0xe0u || set1.bytes[1u] != 0x48u)
            return 1;
        event.data.key.pressed = LIB_FALSE;
        if (vm_profile_default_keyboard_map_kvm_event_for_scan_set(&event,
                CORE_MACHINE_KEYBOARD_SCAN_SET_1, &set1) != LIB_STATUS_OK ||
            set1.count != 2u || set1.bytes[1u] != 0xc8u) return 1;
    }

    if (!vm_keyboard_native_set2_expect('A', 0x1eu, 0u, LIB_TRUE, a_make, sizeof(a_make)) ||
        !vm_keyboard_native_set2_expect('A', 0x1eu, 0u, LIB_FALSE, a_break, sizeof(a_break)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_UP, 0x48u, KVM_KEY_FLAG_EXTENDED, LIB_TRUE, up_make,
            sizeof(up_make)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_UP, 0x48u, KVM_KEY_FLAG_EXTENDED, LIB_FALSE, up_break,
            sizeof(up_break)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_RIGHT, 0u, KVM_KEY_FLAG_EXTENDED, LIB_TRUE, right_make,
            sizeof(right_make)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_CONTROL, 0x1du, 0u, LIB_TRUE,
            left_control_make, sizeof(left_control_make)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_CONTROL, 0x1du, KVM_KEY_FLAG_EXTENDED, LIB_TRUE,
            right_control_make, sizeof(right_control_make)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_ALT, 0x38u, 0u, LIB_FALSE,
            left_alt_break, sizeof(left_alt_break)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_ALT, 0x38u, KVM_KEY_FLAG_EXTENDED, LIB_FALSE,
            right_alt_break, sizeof(right_alt_break)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_PAUSE, 0u, 0u, LIB_TRUE, pause_make,
            sizeof(pause_make)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_PAUSE, 0u, 0u, LIB_FALSE, LIB_NULL, 0u)) {
        return 1;
    }
    for (index = 0u; index < sizeof(function_set1); ++index) {
        if (!vm_keyboard_native_set2_expect(KVM_KEY_F1 + index,
                function_set1[index], 0u, LIB_TRUE,
                &function_set2[index], 1u)) return 1;
    }
    lib_c_printf("HOST-SET1-TO-NATIVE-SET2:OK\n");
    return 0;
}

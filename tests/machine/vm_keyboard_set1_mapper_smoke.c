#include "type.h"

#include "vm/profile/default_profile/keyboard_mapper.h"

static C_INT vm_keyboard_set1_expect(type_unsigned_16 scan, type_unsigned_16 key,
    C_INT pressed, const type_unsigned_8 *expected, type_unsigned_8 count)
{
    vm_profile_default_keyboard_sequence sequence;
    type_unsigned_8 index;

    if (vm_profile_default_keyboard_map_host_key(scan, key, pressed,
            &sequence) != TYPE_STATUS_OK || sequence.count != count) {
        return 0;
    }
    for (index = 0u; index < count; ++index) {
        if (sequence.bytes[index] != expected[index]) return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 a_make[] = { 0x1eu };
    static const type_unsigned_8 a_break[] = { 0x9eu };
    static const type_unsigned_8 up_make[] = { 0xe0u, 0x48u };
    static const type_unsigned_8 up_break[] = { 0xe0u, 0xc8u };
    static const type_unsigned_8 pause_make[] = {
        0xe1u, 0x1du, 0x45u, 0xe1u, 0x9du, 0xc5u
    };

    if (!vm_keyboard_set1_expect(0x1eu, 'A', 1, a_make, sizeof(a_make)) ||
        !vm_keyboard_set1_expect(0x1eu, 'A', 0, a_break, sizeof(a_break)) ||
        !vm_keyboard_set1_expect(0x0148u, 0x26u, 1, up_make,
            sizeof(up_make)) ||
        !vm_keyboard_set1_expect(0x0148u, 0x26u, 0, up_break,
            sizeof(up_break)) ||
        !vm_keyboard_set1_expect(0u, 0x13u, 1, pause_make,
            sizeof(pause_make)) ||
        !vm_keyboard_set1_expect(0u, 0x13u, 0, STD_NULL, 0u)) {
        return 1;
    }
    STD_PRINTF("M5:T226:S3:SET1-MAPPER:OK\n");
    return 0;
}

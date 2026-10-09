#include "lib/types/types_interface.h"
#include "lib/types/test.h"

#include "core/chips/pit825x/pit825x_interface.h"

static void x86_pit_write_counter(x86_pit *pit, lib_u16 control,
    lib_u16 count)
{
    x86_pit_write_register(pit, 3u, control);
    x86_pit_write_register(pit, ((control >> 6) & 0x03u),
        count & 0xffu);
    x86_pit_write_register(pit, ((control >> 6) & 0x03u),
        count >> 8);
}

static lib_u8 x86_pit_read_byte(x86_pit *pit, lib_u16 port_id)
{
    lib_u8 value = 0u;
    lib_test_assert(x86_pit_read_counter(pit, (lib_u8)port_id, &value) == LIB_STATUS_OK);
    return value;
}

lib_i32 main(void)
{
    x86_pit *pit = LIB_NULL;
    lib_u8 status_before;
    lib_u8 status_after;
    lib_i32 failed = 0;

    if (x86_pit_create(X86_PIT_PERSONALITY_8254, &pit) != LIB_STATUS_OK) return 1;
    x86_pit_reset(pit);
    x86_pit_set_output(pit, 0u, LIB_NULL, LIB_NULL);

    x86_pit_write_register(pit, 3u, 0x0036u);
    x86_pit_write_register(pit, 3u, 0x00ecu);
    status_before = x86_pit_read_byte(pit, 0u);
    failed |= status_before != 0xf6u;
    x86_pit_write_register(pit, 0u, 0x0034u);
    x86_pit_write_register(pit, 0u, 0x0012u);
    x86_pit_write_register(pit, 3u, 0x00ecu);
    status_after = x86_pit_read_byte(pit, 0u);
    failed |= status_after != 0xf6u;
    x86_pit_write_register(pit, 3u, 0x0000u);
    failed |= x86_pit_read_byte(pit, 0u) == 0x34u;
    failed |= x86_pit_read_byte(pit, 0u) == 0x12u;
    x86_pit_advance(pit, 1u);
    x86_pit_write_register(pit, 3u, 0x00ecu);
    status_after = x86_pit_read_byte(pit, 0u);
    failed |= status_after != 0xb6u;

    x86_pit_write_counter(pit, 0x0074u, 0x5678u);
    x86_pit_advance(pit, 1u);
    x86_pit_write_register(pit, 3u, 0x00d8u);
    failed |= x86_pit_read_byte(pit, 0u) != 0x32u;
    failed |= x86_pit_read_byte(pit, 0u) != 0x12u;
    failed |= x86_pit_read_byte(pit, 1u) != 0x78u;
    failed |= x86_pit_read_byte(pit, 1u) != 0x56u;

    x86_pit_write_register(pit, 3u, 0x00ccu);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_read_byte(pit, 0u) != 0xb6u;
    failed |= x86_pit_read_byte(pit, 0u) != 0x32u;
    failed |= x86_pit_read_byte(pit, 0u) != 0x12u;

    x86_pit_write_register(pit, 3u, 0x00dcu);
    x86_pit_advance(pit, 1u);
    x86_pit_write_register(pit, 3u, 0x00dcu);
    failed |= x86_pit_read_byte(pit, 0u) != 0x30u;
    failed |= x86_pit_read_byte(pit, 0u) != 0x12u;
    failed |= x86_pit_read_byte(pit, 0u) != 0x2eu;

    x86_pit_write_register(pit, 3u, 0x0000u);
    x86_pit_advance(pit, 1u);
    x86_pit_write_register(pit, 3u, 0x0000u);
    failed |= x86_pit_read_byte(pit, 0u) != 0x2eu;
    failed |= x86_pit_read_byte(pit, 0u) != 0x12u;
    failed |= x86_pit_read_byte(pit, 0u) != 0x2cu;

    /* The one-byte LSB/MSB forms remain independent on counters 1 and 2. */
    x86_pit_write_register(pit, 3u, 0x0050u);
    x86_pit_write_register(pit, 1u, 0x0034u);
    x86_pit_advance(pit, 1u);
    x86_pit_write_register(pit, 3u, 0u);
    failed |= x86_pit_read_byte(pit, 1u) != 0x34u;
    x86_pit_write_register(pit, 3u, 0x00a0u);
    x86_pit_write_register(pit, 2u, 0x0012u);
    x86_pit_advance(pit, 1u);
    x86_pit_write_register(pit, 3u, 0x0080u);
    failed |= x86_pit_read_byte(pit, 2u) != 0x12u;

    x86_pit_destroy(pit);
    if (failed) return 1;
    return 0;
}

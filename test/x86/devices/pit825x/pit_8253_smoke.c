#include "lib/types/test.h"
#include "x86/devices/pit825x/pit825x_interface.h"

lib_i32 main(void)
{
    x86_pit *pit = LIB_NULL;
    lib_u8 value = 0u;

    lib_test_assert(x86_pit_create(X86_PIT_PERSONALITY_8253, &pit) == LIB_STATUS_OK);
    x86_pit_reset(pit);
    lib_test_assert(x86_pit_write_register(pit, 3u, 0x34u) == LIB_STATUS_OK);
    lib_test_assert(x86_pit_write_register(pit, 0u, 3u) == LIB_STATUS_OK);
    lib_test_assert(x86_pit_write_register(pit, 0u, 0u) == LIB_STATUS_OK);
    x86_pit_advance(pit, 1u);
    /* 8253 ignores the 8254 read-back encoding: neither status nor count latches. */
    lib_test_assert(x86_pit_write_register(pit, 3u, 0xecu) == LIB_STATUS_OK);
    lib_test_assert(x86_pit_read_counter(pit, 0u, &value) == LIB_STATUS_OK);
    lib_test_assert(value == 3u);
    x86_pit_advance(pit, 1u);
    lib_test_assert(x86_pit_read_counter(pit, 0u, &value) == LIB_STATUS_OK);
    lib_test_assert(value == 0u);
    lib_test_assert(x86_pit_read_counter(pit, 0u, &value) == LIB_STATUS_OK);
    lib_test_assert(value == 2u);
    x86_pit_destroy(pit);
    return 0;
}

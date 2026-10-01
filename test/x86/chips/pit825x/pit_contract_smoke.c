#include "lib/types/test.h"
#include "x86/chips/pit825x/pit825x_interface.h"

typedef struct {
    lib_u32 edges;
    lib_bool level;
} output_probe;

static void output(void *context, lib_bool level)
{
    output_probe *probe = context;
    ++probe->edges;
    probe->level = level;
}

lib_i32 main(void)
{
    x86_pit *first = LIB_NULL;
    x86_pit *second = LIB_NULL;
    output_probe probe = {0};
    lib_u64 deadline = 0u;
    lib_u8 value = 0xa5u;

    lib_test_assert(x86_pit_create(X86_PIT_PERSONALITY_8254, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_pit_create((x86_pit_personality)2, &first) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(first == LIB_NULL);
    lib_test_assert(x86_pit_create(X86_PIT_PERSONALITY_8254, &first) == LIB_STATUS_OK);
    lib_test_assert(x86_pit_create(X86_PIT_PERSONALITY_8254, &second) == LIB_STATUS_OK);
    x86_pit_reset(first);
    x86_pit_reset(second);
    lib_test_assert(x86_pit_read_counter(first, 0u, &value) == LIB_STATUS_OK && value == 0xa5u);
    lib_test_assert(x86_pit_read_counter(first, 3u, &value) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_pit_read_counter(first, 0u, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_pit_write_register(first, 4u, 0u) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(x86_pit_ticks_until_output(first, 0u, &deadline) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(x86_pit_ticks_until_output(first, 3u, &deadline) == LIB_STATUS_INVALID_ARGUMENT);
    x86_pit_set_output(first, 0u, output, &probe);
    lib_test_assert(probe.edges == 0u);
    lib_test_assert(x86_pit_write_register(first, 3u, 0x10u) == LIB_STATUS_OK);
    lib_test_assert(x86_pit_write_register(first, 0u, 2u) == LIB_STATUS_OK);
    x86_pit_advance(first, 3u);
    lib_test_assert(probe.level == LIB_TRUE && probe.edges == 1u);
    lib_test_assert(x86_pit_get_output(second, 0u) == LIB_FALSE);
    x86_pit_reset(first);
    lib_test_assert(probe.level == LIB_FALSE && probe.edges == 2u);
    lib_test_assert(x86_pit_write_register(first, 3u, 0x10u) == LIB_STATUS_OK);
    lib_test_assert(x86_pit_write_register(first, 0u, 2u) == LIB_STATUS_OK);
    x86_pit_advance(first, 3u);
    lib_test_assert(probe.level == LIB_TRUE && probe.edges == 3u);
    x86_pit_destroy(first);
    lib_test_assert(probe.level == LIB_FALSE && probe.edges == 4u);
    x86_pit_destroy(second);
    x86_pit_destroy(LIB_NULL);
    return 0;
}

#include "lib/types/types_interface.h"
#include "lib/types/test.h"

#include "core/chips/pit825x/pit825x_interface.h"

typedef struct {
    lib_u8 level[16];
    lib_u32 count;
    lib_bool last_level;
} x86_pit_waveform_probe;

static void x86_pit_waveform_output(void *owner,
    lib_bool asserted)
{
    x86_pit_waveform_probe *probe =
        (x86_pit_waveform_probe *)owner;
    if (probe->count < 16u) probe->level[probe->count] = asserted;
    ++probe->count;
    probe->last_level = asserted;
}

static void x86_pit_waveform_write(x86_pit *pit, lib_u8 control,
    lib_u16 count)
{
    x86_pit_write_register(pit, 3u, control);
    x86_pit_write_register(pit, 0u, count & 0xffu);
    x86_pit_write_register(pit, 0u, count >> 8);
    x86_pit_advance(pit, 1u);
}

static lib_u8 x86_pit_waveform_read(x86_pit *pit, lib_u8 counter)
{
    lib_u8 value = 0u;
    lib_test_assert(x86_pit_read_counter(pit, counter, &value) == LIB_STATUS_OK);
    return value;
}

static lib_i32 x86_pit_waveform_expect(lib_u8 actual,
    lib_u8 expected)
{
    return actual == expected ? 0 : 1;
}

static lib_i32 x86_pit_waveform_expect_deadline(const x86_pit *pit,
    lib_u64 expected)
{
    lib_u64 actual = 0u;

    return x86_pit_ticks_until_output(pit, 0u, &actual) !=
        LIB_STATUS_OK || actual != expected;
}

static lib_i32 x86_pit_waveform_deadline_cases(x86_pit *pit)
{
    lib_i32 failed = 0;

    x86_pit_reset(pit);
    x86_pit_waveform_write(pit, 0x30u, 3u);
    failed |= x86_pit_waveform_expect_deadline(pit, 3u);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect_deadline(pit, 2u);
    x86_pit_set_gate(pit, 0u, LIB_FALSE);
    failed |= x86_pit_ticks_until_output(pit, 0u, &(lib_u64) {0u}) !=
        LIB_STATUS_INVALID_STATE;

    x86_pit_reset(pit);
    x86_pit_set_gate(pit, 0u, LIB_FALSE);
    x86_pit_waveform_write(pit, 0x32u, 3u);
    x86_pit_set_gate(pit, 0u, LIB_TRUE);
    failed |= x86_pit_waveform_expect_deadline(pit, 1u);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect_deadline(pit, 3u);

    x86_pit_reset(pit);
    x86_pit_waveform_write(pit, 0x34u, 3u);
    failed |= x86_pit_waveform_expect_deadline(pit, 2u);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect_deadline(pit, 1u);

    x86_pit_reset(pit);
    x86_pit_waveform_write(pit, 0x36u, 4u);
    failed |= x86_pit_waveform_expect_deadline(pit, 2u);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect_deadline(pit, 1u);

    x86_pit_reset(pit);
    x86_pit_waveform_write(pit, 0x38u, 3u);
    failed |= x86_pit_waveform_expect_deadline(pit, 3u);

    x86_pit_reset(pit);
    x86_pit_set_gate(pit, 0u, LIB_FALSE);
    x86_pit_waveform_write(pit, 0x3au, 3u);
    x86_pit_set_gate(pit, 0u, LIB_TRUE);
    failed |= x86_pit_waveform_expect_deadline(pit, 1u);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect_deadline(pit, 3u);
    return failed;
}

lib_i32 main(void)
{
    x86_pit *pit = LIB_NULL;
    x86_pit_waveform_probe probe;
    lib_i32 failed = 0;

    lib_memory_set(&probe, 0, sizeof(probe));
    if (x86_pit_create(X86_PIT_PERSONALITY_8254, &pit) != LIB_STATUS_OK) return 1;
    x86_pit_reset(pit);
    x86_pit_set_output(pit, 0u, x86_pit_waveform_output,
        &probe);

    /* The output consumer observes OUT; it is not the counter's GATE source. */
    x86_pit_set_gate(pit, 0u, LIB_FALSE);
    x86_pit_set_output(pit, 0u, x86_pit_waveform_output,
        &probe);
    x86_pit_waveform_write(pit, 0x30u, 3u);
    x86_pit_advance(pit, 4u);
    failed |= x86_pit_get_output(pit, 0u) != LIB_FALSE;
    x86_pit_set_gate(pit, 0u, LIB_TRUE);

    /* Mode 0: a low GATE pauses the terminal-count transition. */
    x86_pit_waveform_write(pit, 0x30u, 3u);
    x86_pit_advance(pit, 1u);
    x86_pit_set_gate(pit, 0u, LIB_FALSE);
    x86_pit_advance(pit, 4u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_set_gate(pit, 0u, LIB_TRUE);
    x86_pit_advance(pit, 2u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_waveform_write(pit, 0x30u, 3u);
    x86_pit_advance(pit, 1u);
    x86_pit_write_register(pit, 0u, 2u);
    x86_pit_advance(pit, 3u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_write_register(pit, 0u, 0u);
    x86_pit_advance(pit, 3u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);

    /* An LSB-only mode-0 count is complete at its single write, so a rewrite
     * must also restart OUT low before its new terminal transition. */
    x86_pit_reset(pit);
    x86_pit_write_register(pit, 3u, 0x10u);
    x86_pit_write_register(pit, 0u, 2u);
    x86_pit_advance(pit, 3u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_write_register(pit, 0u, 2u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_advance(pit, 3u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);

    /* Mode 1 only starts on a rising GATE and supports retrigger. */
    x86_pit_set_gate(pit, 0u, LIB_FALSE);
    x86_pit_waveform_write(pit, 0x32u, 3u);
    x86_pit_advance(pit, 3u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_set_gate(pit, 0u, LIB_TRUE);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);

    /* Mode 2 rewrites take effect at the current period boundary. */
    x86_pit_waveform_write(pit, 0x34u, 3u);
    x86_pit_advance(pit, 1u);
    x86_pit_write_register(pit, 0u, 2u);
    x86_pit_write_register(pit, 0u, 0u);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_advance(pit, 2u);
    x86_pit_set_gate(pit, 0u, LIB_FALSE);
    x86_pit_set_gate(pit, 0u, LIB_TRUE);
    x86_pit_advance(pit, 3u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);

    /* Mode 2 produces one low tick, then reloads high. */
    x86_pit_waveform_write(pit, 0x34u, 3u);
    x86_pit_advance(pit, 3u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    failed |= x86_pit_waveform_expect(probe.last_level, LIB_TRUE);
    x86_pit_advance(pit, 2u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    failed |= x86_pit_waveform_expect(probe.last_level, LIB_FALSE);

    /* Encodings 6 and 7 are the documented aliases of modes 2 and 3. */
    x86_pit_waveform_write(pit, 0x3cu, 2u);
    x86_pit_advance(pit, 2u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_waveform_write(pit, 0x3eu, 4u);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);

    /* Mode 3 has ceil(N/2) high ticks and floor(N/2) low ticks. */
    x86_pit_waveform_write(pit, 0x36u, 5u);
    x86_pit_advance(pit, 2u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_advance(pit, 2u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);

    /* Odd mode 3 starts CE at N-1 and decrements it by two. */
    x86_pit_waveform_write(pit, 0x36u, 5u);
    x86_pit_write_register(pit, 3u, 0x0000u);
    failed |= x86_pit_waveform_read(pit, 0u) != 4u;
    failed |= x86_pit_waveform_read(pit, 0u) != 0u;
    x86_pit_advance(pit, 1u);
    x86_pit_write_register(pit, 3u, 0x0000u);
    failed |= x86_pit_waveform_read(pit, 0u) != 2u;
    failed |= x86_pit_waveform_read(pit, 0u) != 0u;

    /* Mode 3 rewrites take effect only at the current half-cycle boundary. */
    x86_pit_waveform_write(pit, 0x36u, 4u);
    x86_pit_advance(pit, 1u);
    x86_pit_write_register(pit, 0u, 2u);
    x86_pit_write_register(pit, 0u, 0u);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);

    /* Modes 4 and 5 retain a single low strobe. */
    x86_pit_waveform_write(pit, 0x38u, 3u);
    x86_pit_advance(pit, 1u);
    x86_pit_set_gate(pit, 0u, LIB_FALSE);
    x86_pit_advance(pit, 3u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_set_gate(pit, 0u, LIB_TRUE);
    x86_pit_advance(pit, 2u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);

    /* Modes 2/3 sample a rising GATE and reload on its next CLK. */
    x86_pit_set_gate(pit, 0u, LIB_FALSE);
    x86_pit_waveform_write(pit, 0x34u, 3u);
    x86_pit_set_gate(pit, 0u, LIB_TRUE);
    x86_pit_advance(pit, 3u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_set_gate(pit, 0u, LIB_FALSE);
    x86_pit_waveform_write(pit, 0x36u, 4u);
    x86_pit_set_gate(pit, 0u, LIB_TRUE);
    x86_pit_advance(pit, 2u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);

    /* The mode-3 count-one edge case stays high without counter underflow. */
    x86_pit_waveform_write(pit, 0x36u, 1u);
    x86_pit_advance(pit, 3u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_set_gate(pit, 0u, LIB_FALSE);
    x86_pit_waveform_write(pit, 0x3au, 2u);
    x86_pit_set_gate(pit, 0u, LIB_TRUE);
    x86_pit_advance(pit, 3u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);

    /* Zero loads are 65536 binary ticks and 10000 packed-BCD ticks. */
    x86_pit_waveform_write(pit, 0x30u, 0u);
    x86_pit_advance(pit, 65535u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);
    x86_pit_waveform_write(pit, 0x31u, 0u);
    x86_pit_advance(pit, 9999u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_FALSE);
    x86_pit_advance(pit, 1u);
    failed |= x86_pit_waveform_expect(
        x86_pit_get_output(pit, 0u), LIB_TRUE);

    /* Mode 2 contributes an OUT low/high pair to its output consumer. */
    failed |= probe.count < 2u;
    failed |= x86_pit_waveform_deadline_cases(pit);
    x86_pit_destroy(pit);
    if (failed) return 1;
    return 0;
}

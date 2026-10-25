#include "lib/types/test.h"
#include "x86-nxvm/chips/rtc146818/rtc146818_interface.h"

typedef struct output_probe {
    lib_bool level;
    lib_u32 edges;
} output_probe;

static void output(void *context, lib_bool asserted)
{
    output_probe *probe = context;
    probe->level = asserted;
    ++probe->edges;
}

static void check_month_inputs(void)
{
    const x86_rtc_config config = {1u, 0u, 0u, LIB_FALSE};
    lib_u32 binary, month;
    for (binary = 0u; binary < 2u; ++binary) {
        for (month = 0u; month < 256u; ++month) {
            x86_rtc *rtc = LIB_NULL;
            lib_u64 ticks = 0u;
            lib_test_assert(x86_rtc_create(&config, LIB_NULL, LIB_NULL,
                &rtc) == LIB_STATUS_OK);
            x86_rtc_write_register(rtc, X86_RTC_REG_B,
                X86_RTC_REG_B_24H | X86_RTC_REG_B_AIE |
                (binary ? X86_RTC_REG_B_DM : 0u));
            x86_rtc_write_register(rtc, X86_RTC_MONTH, (lib_u8)month);
            x86_rtc_write_register(rtc, X86_RTC_HOUR, binary ? 23u : 0x23u);
            x86_rtc_write_register(rtc, X86_RTC_MINUTE, binary ? 59u : 0x59u);
            x86_rtc_write_register(rtc, X86_RTC_SECOND, binary ? 59u : 0x59u);
            /* Default alarms match midnight. Preview must not mutate state. */
            lib_test_assert(x86_rtc_ticks_until_irq(rtc, &ticks) == LIB_STATUS_OK);
            lib_test_assert(ticks == 1u);
            lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_SECOND) ==
                (binary ? 59u : 0x59u));
            x86_rtc_advance(rtc, ticks);
            lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_SECOND) == 0u);
            lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_REG_C) &
                X86_RTC_REG_C_AF);
            x86_rtc_destroy(rtc);
        }
    }
}

static void check_divider_rates(void)
{
    static const lib_u32 rates[2][16] = {
        {0u, 32768u, 16384u, 8192u, 4096u, 2048u, 1024u, 512u,
            256u, 128u, 64u, 32u, 16u, 8u, 4u, 2u},
        {0u, 256u, 128u, 8192u, 4096u, 2048u, 1024u, 512u,
            256u, 128u, 64u, 32u, 16u, 8u, 4u, 2u}
    };
    const x86_rtc_config config = {65536u, 0u, 0u, LIB_FALSE};
    for (lib_u8 divider = 0u; divider < 8u; ++divider) {
        for (lib_u8 rate = 0u; rate < 16u; ++rate) {
            x86_rtc *rtc = LIB_NULL;
            output_probe probe = {LIB_FALSE, 0u};
            lib_u64 ticks = 0u;
            lib_test_assert(x86_rtc_create(&config, output, &probe, &rtc) == LIB_STATUS_OK);
            x86_rtc_write_register(rtc, X86_RTC_REG_A, 0x60u);
            x86_rtc_write_register(rtc, X86_RTC_REG_A, (lib_u8)(divider * 16u + rate));
            x86_rtc_write_register(rtc, X86_RTC_REG_B,
                X86_RTC_REG_B_SET | X86_RTC_REG_B_PIE | X86_RTC_REG_B_SQWE);
            if (divider >= 3u || rate == 0u) {
                /* Test-mode encodings are inactive, not silicon-qualified. */
                lib_test_assert(x86_rtc_ticks_until_irq(rtc, &ticks) == LIB_STATUS_INVALID_STATE);
                x86_rtc_advance(rtc, config.ticks_per_second);
                lib_test_assert(!probe.level && !x86_rtc_get_square_wave(rtc));
                lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_REG_C) == 0u);
            } else {
                lib_u64 period = config.ticks_per_second / rates[divider == 2u][rate];
                lib_test_assert(x86_rtc_ticks_until_irq(rtc, &ticks) == LIB_STATUS_OK && ticks == period);
                x86_rtc_advance(rtc, period / 2u - 1u);
                lib_test_assert(!probe.level && !x86_rtc_get_square_wave(rtc));
                x86_rtc_advance(rtc, 1u);
                lib_test_assert(!probe.level && x86_rtc_get_square_wave(rtc));
                x86_rtc_write_register(rtc, X86_RTC_REG_B,
                    X86_RTC_REG_B_SET | X86_RTC_REG_B_PIE);
                lib_test_assert(!x86_rtc_get_square_wave(rtc));
                x86_rtc_write_register(rtc, X86_RTC_REG_B,
                    X86_RTC_REG_B_SET | X86_RTC_REG_B_PIE | X86_RTC_REG_B_SQWE);
                lib_test_assert(x86_rtc_get_square_wave(rtc));
                lib_test_assert(x86_rtc_ticks_until_irq(rtc, &ticks) == LIB_STATUS_OK && ticks == period / 2u);
                x86_rtc_advance(rtc, ticks);
                lib_test_assert(probe.level && !x86_rtc_get_square_wave(rtc));
                lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_REG_C) ==
                    (X86_RTC_REG_C_PF | X86_RTC_REG_C_IRQF));
                lib_test_assert(!probe.level);
            }
            x86_rtc_destroy(rtc);
        }
    }
}

static void check_update_bases(void)
{
    for (lib_u8 divider = 0u; divider < 3u; ++divider) {
        const x86_rtc_config config = {1000000u, 0u, 0u, LIB_FALSE};
        x86_rtc *rtc = LIB_NULL;
        lib_u64 ticks = 0u;
        lib_u32 duration = divider == 2u ? 1984u : 248u;
        lib_test_assert(x86_rtc_create(&config, LIB_NULL, LIB_NULL, &rtc) == LIB_STATUS_OK);
        for (lib_u8 reset = 0x60u; reset <= 0x70u; reset += 0x10u) {
            x86_rtc_write_register(rtc, X86_RTC_REG_A, reset);
            x86_rtc_write_register(rtc, X86_RTC_REG_A, (lib_u8)(divider * 16u));
            x86_rtc_write_register(rtc, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_UIE);
            (void)x86_rtc_read_register(rtc, X86_RTC_REG_C);
            lib_test_assert(x86_rtc_ticks_until_irq(rtc, &ticks) == LIB_STATUS_OK && ticks == 500000u + duration);
            x86_rtc_advance(rtc, 500000u - 245u);
            lib_test_assert((x86_rtc_read_register(rtc, X86_RTC_REG_A) & X86_RTC_REG_A_UIP) == 0u);
            x86_rtc_advance(rtc, 1u);
            lib_test_assert((x86_rtc_read_register(rtc, X86_RTC_REG_A) & X86_RTC_REG_A_UIP) != 0u);
            x86_rtc_advance(rtc, 244u + duration - 1u);
            lib_test_assert((x86_rtc_read_register(rtc, X86_RTC_REG_C) & X86_RTC_REG_C_UF) == 0u);
            x86_rtc_advance(rtc, 1u);
            lib_test_assert((x86_rtc_read_register(rtc, X86_RTC_REG_C) & X86_RTC_REG_C_UF) != 0u);
        }
        x86_rtc_destroy(rtc);
    }
}

static void check_set_phase(void)
{
    const x86_rtc_config config = {7u, 1u, 1u, LIB_FALSE};
    x86_rtc *rtc = LIB_NULL;
    lib_u64 ticks = 0u;
    lib_test_assert(x86_rtc_create(&config, LIB_NULL, LIB_NULL, &rtc) == LIB_STATUS_OK);
    x86_rtc_advance(rtc, 5u);
    lib_test_assert((x86_rtc_read_register(rtc, X86_RTC_REG_A) & X86_RTC_REG_A_UIP) != 0u);
    x86_rtc_write_register(rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_SET | X86_RTC_REG_B_UIE | X86_RTC_REG_B_24H);
    lib_test_assert((x86_rtc_read_register(rtc, X86_RTC_REG_B) & X86_RTC_REG_B_UIE) == 0u);
    lib_test_assert((x86_rtc_read_register(rtc, X86_RTC_REG_A) & X86_RTC_REG_A_UIP) == 0u);
    x86_rtc_advance(rtc, LIB_UINT64_MAX);
    lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_SECOND) == 0u);
    lib_test_assert((x86_rtc_read_register(rtc, X86_RTC_REG_C) &
        (X86_RTC_REG_C_UF | X86_RTC_REG_C_AF)) == 0u);
    x86_rtc_write_register(rtc, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_UIE);
    lib_test_assert(x86_rtc_ticks_until_irq(rtc, &ticks) == LIB_STATUS_OK &&
        ticks == 7u - (5u + LIB_UINT64_MAX % 7u) % 7u);
    x86_rtc_advance(rtc, ticks);
    lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_SECOND) == 1u);
    x86_rtc_destroy(rtc);
}

static void check_phase_continuity(void)
{
    const x86_rtc_config config = {65536u, 8u, 65u, LIB_FALSE};
    for (lib_u8 divider = 0u; divider < 3u; ++divider) {
        for (lib_u8 rate = 1u; rate < 16u; ++rate) {
            x86_rtc *whole = LIB_NULL, *split = LIB_NULL;
            lib_u64 whole_ticks = 0u, split_ticks = 0u;
            lib_test_assert(x86_rtc_create(&config, LIB_NULL, LIB_NULL, &whole) == LIB_STATUS_OK);
            lib_test_assert(x86_rtc_create(&config, LIB_NULL, LIB_NULL, &split) == LIB_STATUS_OK);
            x86_rtc_write_register(whole, X86_RTC_REG_A, (lib_u8)(divider * 16u + rate));
            x86_rtc_write_register(split, X86_RTC_REG_A, (lib_u8)(divider * 16u + rate));
            x86_rtc_write_register(whole, X86_RTC_REG_B, X86_RTC_REG_B_24H |
                X86_RTC_REG_B_PIE | X86_RTC_REG_B_SQWE);
            x86_rtc_write_register(split, X86_RTC_REG_B, X86_RTC_REG_B_24H |
                X86_RTC_REG_B_PIE | X86_RTC_REG_B_SQWE);
            x86_rtc_advance(whole, 500123u);
            x86_rtc_advance(split, 17u);
            x86_rtc_advance(split, 65536u);
            x86_rtc_advance(split, 434570u);
            lib_test_assert(x86_rtc_read_register(whole, X86_RTC_SECOND) ==
                x86_rtc_read_register(split, X86_RTC_SECOND));
            lib_test_assert(x86_rtc_read_register(whole, X86_RTC_REG_C) ==
                x86_rtc_read_register(split, X86_RTC_REG_C));
            lib_test_assert(x86_rtc_get_square_wave(whole) == x86_rtc_get_square_wave(split));
            lib_test_assert(x86_rtc_ticks_until_irq(whole, &whole_ticks) == LIB_STATUS_OK);
            lib_test_assert(x86_rtc_ticks_until_irq(split, &split_ticks) == LIB_STATUS_OK);
            lib_test_assert(whole_ticks == split_ticks);
            x86_rtc_destroy(whole);
            x86_rtc_destroy(split);
        }
    }
    {
        x86_rtc *rtc = LIB_NULL;
        lib_test_assert(x86_rtc_create(&config, LIB_NULL, LIB_NULL, &rtc) == LIB_STATUS_OK);
        x86_rtc_write_register(rtc, X86_RTC_REG_A, 0x20u);
        x86_rtc_write_register(rtc, X86_RTC_REG_B, X86_RTC_REG_B_SET | X86_RTC_REG_B_SQWE);
        x86_rtc_advance(rtc, 16384u);
        lib_test_assert(!x86_rtc_get_square_wave(rtc));
        x86_rtc_write_register(rtc, X86_RTC_REG_A, 0x2fu);
        lib_test_assert(x86_rtc_get_square_wave(rtc));
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_REG_C) == 0u);
        x86_rtc_advance(rtc, 16384u);
        lib_test_assert(!x86_rtc_get_square_wave(rtc));
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_REG_C) == X86_RTC_REG_C_PF);
        x86_rtc_destroy(rtc);
    }
}

static void check_flags_and_enables(void)
{
    static const lib_u8 flags[] = {0u, X86_RTC_REG_C_PF, X86_RTC_REG_C_UF,
        X86_RTC_REG_C_UF | X86_RTC_REG_C_PF,
        X86_RTC_REG_C_UF | X86_RTC_REG_C_AF,
        X86_RTC_REG_C_UF | X86_RTC_REG_C_AF | X86_RTC_REG_C_PF};
    const x86_rtc_config config = {32768u, 8u, 65u, LIB_FALSE};
    /* AF occurs during an update; AF without UF is not independently reachable. */
    for (lib_u8 scenario = 0u; scenario < 48u; ++scenario) {
        lib_u8 expected = flags[scenario / 8u];
        lib_u8 enable = (lib_u8)((scenario % 8u) << 4u);
        lib_bool asserted = (expected & enable) != 0u;
        x86_rtc *rtc = LIB_NULL;
        output_probe probe = {LIB_FALSE, 0u};
        lib_test_assert(x86_rtc_create(&config, output, &probe, &rtc) == LIB_STATUS_OK);
        x86_rtc_write_register(rtc, X86_RTC_REG_A,
            (expected & X86_RTC_REG_C_PF) != 0u ? 0x2fu : 0x20u);
        x86_rtc_write_register(rtc, X86_RTC_HOUR_ALARM,
            (expected & X86_RTC_REG_C_AF) != 0u ? 0xc0u : 0x23u);
        x86_rtc_write_register(rtc, X86_RTC_MINUTE_ALARM, 0xc0u);
        x86_rtc_write_register(rtc, X86_RTC_SECOND_ALARM, 0xc0u);
        x86_rtc_advance(rtc, (expected & X86_RTC_REG_C_UF) != 0u ?
            32768u : (expected & X86_RTC_REG_C_PF) != 0u ? 16384u : 0u);
        lib_test_assert(!probe.level && probe.edges == 0u);
        x86_rtc_write_register(rtc, X86_RTC_REG_C, 0u);
        x86_rtc_write_register(rtc, X86_RTC_REG_D, 0u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_REG_D) == X86_RTC_REG_D_VRT);
        x86_rtc_write_register(rtc, X86_RTC_REG_B, X86_RTC_REG_B_24H | enable);
        lib_test_assert(probe.level == asserted);
        /* Disabling delivery preserves flags; re-enabling republishes their level. */
        x86_rtc_write_register(rtc, X86_RTC_REG_B, X86_RTC_REG_B_24H);
        lib_test_assert(!probe.level);
        x86_rtc_write_register(rtc, X86_RTC_REG_B, X86_RTC_REG_B_24H | enable);
        lib_test_assert(probe.level == asserted);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_REG_C) ==
            (expected | (asserted ? X86_RTC_REG_C_IRQF : 0u)));
        lib_test_assert(!probe.level && probe.edges == (asserted ? 4u : 0u));
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_REG_C) == 0u);
        x86_rtc_destroy(rtc);
    }
}

static void check_ram_reset(void)
{
    for (lib_u8 extended = LIB_FALSE; extended <= LIB_TRUE; ++extended) {
        const x86_rtc_config config = {32768u, 8u, 65u, extended};
        const lib_u8 count = extended ? X86_RTC_REGISTER_COUNT :
            X86_RTC_DEFAULT_REGISTER_COUNT;
        x86_rtc *rtc = LIB_NULL;

        lib_test_assert(x86_rtc_create(&config, LIB_NULL, LIB_NULL, &rtc) == LIB_STATUS_OK);
        for (lib_u8 index = 0x0eu; index < count; ++index)
            x86_rtc_write_register(rtc, index, (lib_u8)(index ^ 0xa5u));
        x86_rtc_write_register(rtc, count, 0x5au);
        lib_test_assert(x86_rtc_read_register(rtc, count) == 0u);
        x86_rtc_reset(rtc);
        for (lib_u8 index = 0x0eu; index < count; ++index)
            lib_test_assert(x86_rtc_read_register(rtc, index) == (lib_u8)(index ^ 0xa5u));
        x86_rtc_destroy(rtc);
    }
}

static void check_zero_elapsed(void)
{
    const x86_rtc_config config = {32768u, 8u, 65u, LIB_FALSE};
    for (lib_u8 scenario = 0u; scenario < 4u; ++scenario) {
        x86_rtc *rtc = LIB_NULL;
        output_probe probe = {LIB_FALSE, 0u};
        lib_u64 before, after;
        lib_test_assert(x86_rtc_create(&config, output, &probe, &rtc) == LIB_STATUS_OK);
        x86_rtc_write_register(rtc, X86_RTC_REG_B, X86_RTC_REG_B_24H |
            X86_RTC_REG_B_PIE | X86_RTC_REG_B_SQWE |
            (scenario >= 2u ? X86_RTC_REG_B_SET : 0u));
        x86_rtc_advance(rtc, scenario & 1u ? 32u : 16u);
        const lib_u32 edges = probe.edges;
        const lib_bool irq = probe.level, square = x86_rtc_get_square_wave(rtc);
        lib_test_assert(x86_rtc_ticks_until_irq(rtc, &before) == LIB_STATUS_OK);
        x86_rtc_advance(rtc, 0u);
        lib_test_assert(x86_rtc_ticks_until_irq(rtc, &after) == LIB_STATUS_OK && before == after);
        lib_test_assert(probe.edges == edges && probe.level == irq);
        lib_test_assert(x86_rtc_get_square_wave(rtc) == square);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_SECOND) == 0u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_REG_C) ==
            (scenario & 1u ? X86_RTC_REG_C_PF | X86_RTC_REG_C_IRQF : 0u));
        x86_rtc_destroy(rtc);
    }
}

lib_i32 main(void)
{
    const x86_rtc_config config = {32768u, 8u, 65u, LIB_FALSE};
    const x86_rtc_config invalid = {0u, 0u, 0u, LIB_FALSE};
    const x86_rtc_config invalid_extended = {32768u, 8u, 65u, 2u};
    x86_rtc *rtc = LIB_NULL;
    output_probe probe = {LIB_FALSE, 0u};
    lib_u64 ticks = 123u;
    lib_test_assert(x86_rtc_create(&invalid, LIB_NULL, LIB_NULL, &rtc) ==
        LIB_STATUS_INVALID_ARGUMENT && rtc == LIB_NULL);
    lib_test_assert(x86_rtc_create(&invalid_extended, LIB_NULL, LIB_NULL, &rtc) ==
        LIB_STATUS_INVALID_ARGUMENT && rtc == LIB_NULL);
    lib_test_assert(x86_rtc_create(&config, output, &probe, &rtc) == LIB_STATUS_OK);
    lib_test_assert(x86_rtc_ticks_until_irq(rtc, &ticks) == LIB_STATUS_INVALID_STATE);
    lib_test_assert(ticks == 123u);
    x86_rtc_write_register(rtc, 64u, 255u);
    lib_test_assert(x86_rtc_read_register(rtc, 64u) == 0u);
    x86_rtc_write_register(rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_UIE);
    x86_rtc_advance(rtc, 32768u);
    lib_test_assert(probe.level && probe.edges == 1u);
    x86_rtc_advance(rtc, 32768u);
    lib_test_assert(probe.edges == 1u);
    x86_rtc_reset(rtc);
    lib_test_assert(!probe.level && probe.edges == 2u);
    x86_rtc_write_register(rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_UIE);
    x86_rtc_advance(rtc, 32768u);
    x86_rtc_destroy(rtc);
    lib_test_assert(!probe.level && probe.edges == 4u);
    x86_rtc_destroy(LIB_NULL);
    check_month_inputs();
    check_divider_rates();
    check_update_bases();
    check_set_phase();
    check_phase_continuity();
    check_flags_and_enables();
    check_ram_reset();
    check_zero_elapsed();
    return 0;
}

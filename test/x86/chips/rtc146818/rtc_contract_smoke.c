#include "lib/types/test.h"
#include "x86/chips/rtc146818/rtc146818_interface.h"

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
    const x86_rtc_config config = {1u, 0u, 0u};
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

lib_i32 main(void)
{
    const x86_rtc_config config = {32768u, 8u, 65u};
    const x86_rtc_config invalid = {0u, 0u, 0u};
    x86_rtc *rtc = LIB_NULL;
    output_probe probe = {LIB_FALSE, 0u};
    lib_u64 ticks = 123u;
    lib_test_assert(x86_rtc_create(&invalid, LIB_NULL, LIB_NULL, &rtc) ==
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
    return 0;
}

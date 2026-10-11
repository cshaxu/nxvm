#include "lib/types/test.h"
#include "core/chips/rtc146818/rtc146818_interface.h"

typedef struct rtc_fixture {
    x86_rtc *rtc;
    lib_bool irq;
} rtc_fixture;

static void rtc_output(void *context, lib_bool asserted)
{
    rtc_fixture *fixture = context;
    fixture->irq = asserted;
}

static void rtc_initialize(rtc_fixture *fixture)
{
    const x86_rtc_config config = {4u, 0u, 0u};
    fixture->irq = LIB_FALSE;
    lib_test_assert(x86_rtc_create(&config, rtc_output, fixture,
        &fixture->rtc) == LIB_STATUS_OK);
}

static lib_i32 rtc_cmos_test_calendar_and_reset(void)
{
    rtc_fixture fixture;
    lib_i32 failed = 0;

    rtc_initialize(&fixture);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_SET);
    x86_rtc_write_register(fixture.rtc, X86_RTC_HOUR, 0x92u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_SECOND, 0x59u);
    x86_rtc_advance(fixture.rtc, 4u);
    failed |= x86_rtc_read_register(fixture.rtc, X86_RTC_HOUR) != 0x92u ||
        x86_rtc_read_register(fixture.rtc, X86_RTC_SECOND) != 0x59u;
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B, 0u);
    x86_rtc_advance(fixture.rtc, 4u);
    failed |= x86_rtc_read_register(fixture.rtc, X86_RTC_HOUR) != 0x92u ||
        x86_rtc_read_register(fixture.rtc, X86_RTC_MINUTE) != 0x01u ||
        x86_rtc_read_register(fixture.rtc, X86_RTC_SECOND) != 0x00u;
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_SET | X86_RTC_REG_B_DM);
    x86_rtc_write_register(fixture.rtc, X86_RTC_HOUR, 0x8cu);
    failed |= x86_rtc_read_register(fixture.rtc, X86_RTC_HOUR) != 0x8cu;
    x86_rtc_write_register(fixture.rtc, 0x14u, 0x5au);
    x86_rtc_reset(fixture.rtc);
    failed |= x86_rtc_read_register(fixture.rtc, 0x14u) != 0x5au ||
        x86_rtc_read_register(fixture.rtc, X86_RTC_MINUTE) != 0x01u ||
        x86_rtc_read_register(fixture.rtc, X86_RTC_SECOND) != 0x00u ||
        fixture.irq;
    x86_rtc_destroy(fixture.rtc);
    return failed;
}


static lib_i32 rtc_cmos_test_phase_and_divider(void)
{
    rtc_fixture fixture;
    x86_rtc_config config = {32768u, 8u, 65u};
    lib_i32 failed = 0;

    fixture.irq = LIB_FALSE;
    lib_test_assert(x86_rtc_create(&config, rtc_output, &fixture,
        &fixture.rtc) == LIB_STATUS_OK);
    x86_rtc_advance(fixture.rtc, 32768u - 73u);
    failed |= (x86_rtc_read_register(fixture.rtc, X86_RTC_REG_A) &
        X86_RTC_REG_A_UIP) == 0u;
    x86_rtc_reset(fixture.rtc);
    failed |= (x86_rtc_read_register(fixture.rtc, X86_RTC_REG_A) &
        X86_RTC_REG_A_UIP) == 0u;
    x86_rtc_advance(fixture.rtc, 32768u);
    failed |=
        x86_rtc_read_register(fixture.rtc, X86_RTC_SECOND) != 1u ||
        (x86_rtc_read_register(fixture.rtc, X86_RTC_REG_C) & X86_RTC_REG_C_UF) == 0u;
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_A, 0x66u);
    x86_rtc_advance(fixture.rtc, 32768u);
    failed |= x86_rtc_read_register(fixture.rtc, X86_RTC_SECOND) != 1u;
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_A, 0x26u);
    x86_rtc_advance(fixture.rtc, 16384u + 64u);
    failed |= x86_rtc_read_register(fixture.rtc, X86_RTC_SECOND) != 1u;
    x86_rtc_advance(fixture.rtc, 1u);
    failed |= x86_rtc_read_register(fixture.rtc, X86_RTC_SECOND) != 2u;
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_SQWE);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_A, 0x6fu);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_A, 0x2fu);
    x86_rtc_advance(fixture.rtc, 8192u);
    failed |= !x86_rtc_get_square_wave(fixture.rtc);
    x86_rtc_reset(fixture.rtc);
    failed |= x86_rtc_get_square_wave(fixture.rtc) ||
        (x86_rtc_read_register(fixture.rtc, X86_RTC_REG_B) & X86_RTC_REG_B_SQWE) != 0u;
    x86_rtc_write_register(fixture.rtc, 0x32u, 0x5au);
    failed |= x86_rtc_read_register(fixture.rtc, 0x32u) != 0x5au;
    x86_rtc_destroy(fixture.rtc);
    return failed;
}


static lib_i32 rtc_cmos_test_alarm_deadline(void)
{
    rtc_fixture fixture;
    lib_u64 ticks = 0u;
    lib_i32 failed = 0;

    rtc_initialize(&fixture);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_AIE);
    x86_rtc_write_register(fixture.rtc, X86_RTC_SECOND_ALARM, 0x02u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_MINUTE_ALARM, 0x00u);
    x86_rtc_write_register(fixture.rtc, X86_RTC_HOUR_ALARM, 0x00u);
    failed |= x86_rtc_ticks_until_irq(fixture.rtc, &ticks) !=
        LIB_STATUS_OK || ticks != 8u;
    x86_rtc_advance(fixture.rtc, ticks);
    failed |= !fixture.irq;
    failed |= (x86_rtc_read_register(fixture.rtc, X86_RTC_REG_C) &
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_AF)) !=
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_AF) ||
        fixture.irq;
    x86_rtc_destroy(fixture.rtc);
    return failed;
}

static void rtc_cmos_test_daylight(void)
{
    const x86_rtc_config config = {32768u, 8u, 65u};
    /* Five independent binary axes, without nested execution loops. */
    for (lib_u8 scenario = 0u; scenario < 32u; ++scenario) {
        lib_bool binary = (scenario & 1u) != 0u;
        lib_bool clock24 = (scenario & 2u) != 0u;
        lib_bool autumn = (scenario & 4u) != 0u;
        lib_bool last = (scenario & 8u) != 0u;
        lib_bool enabled = (scenario & 16u) != 0u;
        x86_rtc *rtc = LIB_NULL;
        lib_u64 ticks = 0u;
        lib_u8 mode = (binary ? X86_RTC_REG_B_DM : 0u) |
            (clock24 ? X86_RTC_REG_B_24H : 0u) | (enabled ? 1u : 0u);
        lib_u8 day = (lib_u8)(23u + autumn + last);
        lib_u8 hour = enabled && last ? (autumn ? 1u : 3u) : 2u;
        lib_test_assert(x86_rtc_create(&config, LIB_NULL, LIB_NULL, &rtc) == LIB_STATUS_OK);
        x86_rtc_write_register(rtc, X86_RTC_REG_B, mode | X86_RTC_REG_B_SET);
        x86_rtc_write_register(rtc, X86_RTC_MONTH, autumn ? (binary ? 10u : 0x10u) : 4u);
        x86_rtc_write_register(rtc, X86_RTC_DAY_MONTH,
            binary ? day : (lib_u8)(((day / 10u) << 4u) | day % 10u));
        x86_rtc_write_register(rtc, X86_RTC_DAY_WEEK, 1u);
        x86_rtc_write_register(rtc, X86_RTC_HOUR, 1u);
        x86_rtc_write_register(rtc, X86_RTC_MINUTE, binary ? 59u : 0x59u);
        /* Initialize at least two seconds before rollover. */
        x86_rtc_write_register(rtc, X86_RTC_SECOND, binary ? 58u : 0x58u);
        x86_rtc_write_register(rtc, X86_RTC_HOUR_ALARM, hour);
        x86_rtc_write_register(rtc, X86_RTC_MINUTE_ALARM, 0u);
        x86_rtc_write_register(rtc, X86_RTC_SECOND_ALARM, 0u);
        x86_rtc_write_register(rtc, X86_RTC_REG_B, mode | X86_RTC_REG_B_AIE);
        lib_test_assert(x86_rtc_ticks_until_irq(rtc, &ticks) == LIB_STATUS_OK && ticks == 65536u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_SECOND) == (binary ? 58u : 0x58u));
        x86_rtc_advance(rtc, ticks);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_HOUR) == hour);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_MINUTE) == 0u);
        lib_test_assert((x86_rtc_read_register(rtc, X86_RTC_REG_C) & X86_RTC_REG_C_AF) != 0u);
        x86_rtc_reset(rtc);
        x86_rtc_advance(rtc, 32768u * 3600u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_HOUR) == hour + 1u);
        x86_rtc_destroy(rtc);
    }
}


static lib_u8 calendar_value(lib_u8 value, lib_bool binary)
{
    return binary ? value : (lib_u8)(((value / 10u) << 4u) | (value % 10u));
}

static void rtc_cmos_test_alarm_wildcards(void)
{
    const x86_rtc_config config = {4u, 1u, 1u};
    static const lib_u8 alarm_registers[] = {
        X86_RTC_SECOND_ALARM, X86_RTC_MINUTE_ALARM, X86_RTC_HOUR_ALARM
    };
    for (lib_u32 scenario = 0u; scenario < 2048u; ++scenario) {
        lib_u8 low = (lib_u8)(scenario % 64u);
        lib_u8 mask = (lib_u8)(scenario / 64u % 8u);
        lib_bool binary = (scenario / 512u & 1u) != 0u;
        lib_bool clock24 = scenario / 1024u != 0u;
        lib_u8 mode = X86_RTC_REG_B_AIE |
            (binary ? X86_RTC_REG_B_DM : 0u) |
            (clock24 ? X86_RTC_REG_B_24H : 0u);
        lib_u8 alarm[3] = {
            calendar_value(13u, binary), calendar_value(34u, binary),
            clock24 ? calendar_value(23u, binary) :
                (lib_u8)(calendar_value(11u, binary) | 0x80u)
        };
        rtc_fixture fixture = {0};
        lib_u64 ticks = 0u;

        lib_test_assert(x86_rtc_create(&config, rtc_output, &fixture,
            &fixture.rtc) == LIB_STATUS_OK);
        x86_rtc_write_register(fixture.rtc, X86_RTC_REG_A, 0x20u);
        x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B, mode | X86_RTC_REG_B_SET);
        x86_rtc_write_register(fixture.rtc, X86_RTC_SECOND, calendar_value(12u, binary));
        x86_rtc_write_register(fixture.rtc, X86_RTC_MINUTE, calendar_value(34u, binary));
        x86_rtc_write_register(fixture.rtc, X86_RTC_HOUR, alarm[2]);
        for (lib_u8 field = 0u; field < 3u; ++field) {
            if ((mask & (1u << field)) != 0u) alarm[field] = (lib_u8)(0xc0u | low);
            x86_rtc_write_register(fixture.rtc, alarm_registers[field], alarm[field]);
        }
        x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B, mode);
        lib_test_assert(x86_rtc_ticks_until_irq(fixture.rtc, &ticks) == LIB_STATUS_OK && ticks == 4u);
        x86_rtc_advance(fixture.rtc, 3u);
        lib_test_assert(!fixture.irq);
        x86_rtc_advance(fixture.rtc, 1u);
        lib_test_assert(fixture.irq);
        lib_test_assert(x86_rtc_read_register(fixture.rtc, X86_RTC_REG_C) ==
            (X86_RTC_REG_C_AF | X86_RTC_REG_C_UF | X86_RTC_REG_C_IRQF));
        lib_test_assert(!fixture.irq);
        /* Every non-wildcard field must participate in the comparison. */
        for (lib_u8 field = 0u; field < 3u; ++field) {
            if ((mask & (1u << field)) != 0u) continue;
            x86_rtc_write_register(fixture.rtc, X86_RTC_SECOND, calendar_value(12u, binary));
            x86_rtc_write_register(fixture.rtc, alarm_registers[field], 0u);
            x86_rtc_advance(fixture.rtc, 4u);
            lib_test_assert(!fixture.irq);
            lib_test_assert(x86_rtc_read_register(fixture.rtc, X86_RTC_REG_C) == X86_RTC_REG_C_UF);
            x86_rtc_write_register(fixture.rtc, alarm_registers[field], alarm[field]);
        }
        x86_rtc_destroy(fixture.rtc);
    }
}

static void rtc_cmos_test_month_rollover(void)
{
    static const lib_u8 days[] = {31u, 28u, 31u, 30u, 31u, 30u,
        31u, 31u, 30u, 31u, 30u, 31u};
    const x86_rtc_config config = {4u, 1u, 1u};
    for (lib_u32 scenario = 0u; scenario < 4800u; ++scenario) {
        lib_u8 month = (lib_u8)(scenario % 12u + 1u);
        lib_u8 year = (lib_u8)(scenario / 12u % 100u);
        lib_bool binary = (scenario / 1200u & 1u) != 0u;
        lib_bool clock24 = (scenario / 2400u) != 0u;
        lib_u8 day = (lib_u8)(days[month - 1u] +
            (month == 2u && year % 4u == 0u ? 1u : 0u));
        lib_u8 mode = (binary ? X86_RTC_REG_B_DM : 0u) |
            (clock24 ? X86_RTC_REG_B_24H : 0u);
        x86_rtc *rtc = LIB_NULL;
        lib_test_assert(x86_rtc_create(&config, LIB_NULL, LIB_NULL, &rtc) == LIB_STATUS_OK);
        x86_rtc_write_register(rtc, X86_RTC_REG_A, 0x20u);
        x86_rtc_write_register(rtc, X86_RTC_REG_B, mode | X86_RTC_REG_B_SET);
        x86_rtc_write_register(rtc, X86_RTC_YEAR, calendar_value(year, binary));
        x86_rtc_write_register(rtc, X86_RTC_MONTH, calendar_value(month, binary));
        x86_rtc_write_register(rtc, X86_RTC_DAY_MONTH, calendar_value(day, binary));
        x86_rtc_write_register(rtc, X86_RTC_DAY_WEEK, 7u);
        x86_rtc_write_register(rtc, X86_RTC_HOUR,
            clock24 ? calendar_value(23u, binary) : calendar_value(11u, binary) | 0x80u);
        x86_rtc_write_register(rtc, X86_RTC_MINUTE, calendar_value(59u, binary));
        x86_rtc_write_register(rtc, X86_RTC_SECOND, calendar_value(58u, binary));
        x86_rtc_write_register(rtc, X86_RTC_REG_B, mode);
        x86_rtc_advance(rtc, 8u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_SECOND) == 0u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_MINUTE) == 0u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_HOUR) ==
            (clock24 ? 0u : calendar_value(12u, binary)));
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_DAY_WEEK) == 1u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_DAY_MONTH) == 1u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_MONTH) ==
            calendar_value(month == 12u ? 1u : (lib_u8)(month + 1u), binary));
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_YEAR) ==
            calendar_value(month == 12u ? (lib_u8)((year + 1u) % 100u) : year, binary));
        x86_rtc_destroy(rtc);
    }
}

static void rtc_cmos_test_hour_rollover(void)
{
    const x86_rtc_config config = {4u, 1u, 1u};
    for (lib_u8 scenario = 0u; scenario < 96u; ++scenario) {
        lib_u8 hour = (lib_u8)(scenario % 24u);
        lib_u8 next = (lib_u8)((hour + 1u) % 24u);
        lib_bool binary = (scenario / 24u & 1u) != 0u;
        lib_bool clock24 = scenario / 48u != 0u;
        lib_u8 mode = (binary ? X86_RTC_REG_B_DM : 0u) |
            (clock24 ? X86_RTC_REG_B_24H : 0u);
        lib_u8 encoded = clock24 ? calendar_value(hour, binary) :
            calendar_value(hour % 12u == 0u ? 12u : hour % 12u, binary) |
            (hour >= 12u ? 0x80u : 0u);
        lib_u8 expected = clock24 ? calendar_value(next, binary) :
            calendar_value(next % 12u == 0u ? 12u : next % 12u, binary) |
            (next >= 12u ? 0x80u : 0u);
        x86_rtc *rtc = LIB_NULL;
        lib_test_assert(x86_rtc_create(&config, LIB_NULL, LIB_NULL, &rtc) == LIB_STATUS_OK);
        x86_rtc_write_register(rtc, X86_RTC_REG_A, 0x20u);
        x86_rtc_write_register(rtc, X86_RTC_REG_B, mode | X86_RTC_REG_B_SET);
        x86_rtc_write_register(rtc, X86_RTC_HOUR, encoded);
        x86_rtc_write_register(rtc, X86_RTC_MINUTE, calendar_value(59u, binary));
        x86_rtc_write_register(rtc, X86_RTC_SECOND, calendar_value(59u, binary));
        x86_rtc_write_register(rtc, X86_RTC_REG_B, mode);
        x86_rtc_advance(rtc, 4u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_HOUR) == expected);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_MINUTE) == 0u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_SECOND) == 0u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_DAY_MONTH) == (hour == 23u ? 2u : 1u));
        x86_rtc_destroy(rtc);
    }
}

static void rtc_cmos_test_daylight_alarm_horizon(void)
{
    const x86_rtc_config config = {4u, 1u, 1u};
    for (lib_u8 scenario = 0u; scenario < 4u; ++scenario) {
        lib_bool binary = (scenario & 1u) != 0u;
        lib_bool clock24 = (scenario & 2u) != 0u;
        lib_u8 midnight = clock24 ? 0u : calendar_value(12u, binary);
        lib_u8 mode = 1u | X86_RTC_REG_B_AIE |
            (binary ? X86_RTC_REG_B_DM : 0u) | (clock24 ? X86_RTC_REG_B_24H : 0u);
        x86_rtc *rtc = LIB_NULL;
        lib_u64 ticks = 0u;
        lib_test_assert(x86_rtc_create(&config, LIB_NULL, LIB_NULL, &rtc) == LIB_STATUS_OK);
        x86_rtc_write_register(rtc, X86_RTC_REG_A, 0x20u);
        x86_rtc_write_register(rtc, X86_RTC_REG_B, mode | X86_RTC_REG_B_SET);
        x86_rtc_write_register(rtc, X86_RTC_MONTH, calendar_value(10u, binary));
        x86_rtc_write_register(rtc, X86_RTC_DAY_MONTH, calendar_value(25u, binary));
        x86_rtc_write_register(rtc, X86_RTC_DAY_WEEK, 1u);
        x86_rtc_write_register(rtc, X86_RTC_HOUR, midnight);
        x86_rtc_write_register(rtc, X86_RTC_SECOND, 1u);
        x86_rtc_write_register(rtc, X86_RTC_HOUR_ALARM, midnight);
        x86_rtc_write_register(rtc, X86_RTC_REG_B, mode);
        /* The next midnight crosses the one-hour autumn repeat: 25 hours less one second. */
        lib_test_assert(x86_rtc_ticks_until_irq(rtc, &ticks) == LIB_STATUS_OK && ticks == 359996u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_SECOND) == 1u);
        x86_rtc_advance(rtc, ticks - 1u);
        lib_test_assert((x86_rtc_read_register(rtc, X86_RTC_REG_C) & X86_RTC_REG_C_AF) == 0u);
        x86_rtc_advance(rtc, 1u);
        lib_test_assert((x86_rtc_read_register(rtc, X86_RTC_REG_C) & X86_RTC_REG_C_AF) != 0u);
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_DAY_MONTH) == calendar_value(26u, binary));
        lib_test_assert(x86_rtc_read_register(rtc, X86_RTC_HOUR) == midnight);
        x86_rtc_destroy(rtc);
    }
}

lib_i32 main(void)
{
    lib_test_assert(rtc_cmos_test_calendar_and_reset() == 0);
    lib_test_assert(rtc_cmos_test_phase_and_divider() == 0);
    lib_test_assert(rtc_cmos_test_alarm_deadline() == 0);
    rtc_cmos_test_daylight();
    rtc_cmos_test_alarm_wildcards();
    rtc_cmos_test_month_rollover();
    rtc_cmos_test_hour_rollover();
    rtc_cmos_test_daylight_alarm_horizon();
    return 0;
}

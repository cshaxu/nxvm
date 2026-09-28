#include "lib/types/test.h"
#include "x86/devices/rtc146818/rtc146818_interface.h"

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

static lib_i32 rtc_cmos_s3_test_calendar_and_reset(void)
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


static lib_i32 rtc_cmos_s3_test_phase_and_divider(void)
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
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_A, 0x06u);
    x86_rtc_advance(fixture.rtc, 32768u);
    failed |= x86_rtc_read_register(fixture.rtc, X86_RTC_SECOND) != 1u;
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_A, 0x26u);
    x86_rtc_advance(fixture.rtc, 16383u);
    failed |= x86_rtc_read_register(fixture.rtc, X86_RTC_SECOND) != 1u;
    x86_rtc_advance(fixture.rtc, 1u);
    failed |= x86_rtc_read_register(fixture.rtc, X86_RTC_SECOND) != 2u;
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_B,
        X86_RTC_REG_B_24H | X86_RTC_REG_B_SQWE);
    x86_rtc_write_register(fixture.rtc, X86_RTC_REG_A, 0x2fu);
    x86_rtc_advance(fixture.rtc, 16384u);
    failed |= !x86_rtc_get_square_wave(fixture.rtc);
    x86_rtc_reset(fixture.rtc);
    failed |= x86_rtc_get_square_wave(fixture.rtc) ||
        (x86_rtc_read_register(fixture.rtc, X86_RTC_REG_B) & X86_RTC_REG_B_SQWE) != 0u;
    x86_rtc_write_register(fixture.rtc, 0x32u, 0x5au);
    failed |= x86_rtc_read_register(fixture.rtc, 0x32u) != 0x5au;
    x86_rtc_destroy(fixture.rtc);
    return failed;
}


static lib_i32 rtc_cmos_s3_test_alarm_deadline(void)
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


lib_i32 main(void)
{
    lib_test_assert(rtc_cmos_s3_test_calendar_and_reset() == 0);
    lib_test_assert(rtc_cmos_s3_test_phase_and_divider() == 0);
    lib_test_assert(rtc_cmos_s3_test_alarm_deadline() == 0);
    return 0;
}

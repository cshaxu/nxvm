/* Copyright 2012-2026 Neko. */
#include "x86/chips/rtc146818/rtc.h"

static lib_u8 rtc_encode(const x86_rtc *rtc, lib_u8 value)
{
    return (rtc->registers[X86_RTC_REG_B] & X86_RTC_REG_B_DM) != 0u ?
        value : (lib_u8)(((value / 10u) << 4u) | (value % 10u));
}

static lib_u8 rtc_decode(const x86_rtc *rtc, lib_u8 value)
{
    return (rtc->registers[X86_RTC_REG_B] & X86_RTC_REG_B_DM) != 0u ?
        value : (lib_u8)(((value >> 4u) * 10u) + (value & 0x0fu));
}

static lib_u8 rtc_hour_encode(const x86_rtc *rtc)
{
    lib_u8 hour = rtc->calendar.hour;

    if ((rtc->registers[X86_RTC_REG_B] & X86_RTC_REG_B_24H) != 0u) {
        return rtc_encode(rtc, hour);
    }
    if (hour >= 12u) hour = (lib_u8)(hour - 12u);
    if (hour == 0u) hour = 12u;
    return rtc_encode(rtc, hour) | (rtc->calendar.hour >= 12u ? 0x80u : 0u);
}

static lib_u8 rtc_hour_decode(const x86_rtc *rtc, lib_u8 value)
{
    lib_u8 hour;

    if ((rtc->registers[X86_RTC_REG_B] & X86_RTC_REG_B_24H) != 0u) {
        return rtc_decode(rtc, value);
    }
    hour = rtc_decode(rtc, value & 0x7fu);
    if (hour == 12u) hour = 0u;
    return (lib_u8)(hour + ((value & 0x80u) != 0u ? 12u : 0u));
}

static lib_bool rtc_divider_running(const x86_rtc *rtc)
{
    lib_u8 divider = rtc->registers[X86_RTC_REG_A] & 0x70u;

    return divider == 0x20u || divider == 0x50u || divider == 0x60u;
}

static lib_u32 rtc_divider_hz(const x86_rtc *rtc)
{
    switch (rtc->registers[X86_RTC_REG_A] & 0x70u) {
    case 0x20u: return 32768u;
    case 0x50u: return 1048576u;
    case 0x60u: return 4194304u;
    default: return 0u;
    }
}

static lib_u32 rtc_periodic_hz(const x86_rtc *rtc)
{
    lib_u8 rate = rtc->registers[X86_RTC_REG_A] & 0x0fu;
    lib_u32 base = rtc_divider_hz(rtc);

    if (base == 0u || rate == 0u) return 0u;
    if (rate == 1u) return base / 128u;
    if (rate == 2u) return base / 256u;
    return rate <= 15u ? base >> (rate - 1u) : 0u;
}

static void rtc_refresh_irq(x86_rtc *rtc)
{
    lib_u8 flags = rtc->registers[X86_RTC_REG_C];
    lib_u8 enable = rtc->registers[X86_RTC_REG_B];
    lib_bool was_asserted = (flags & X86_RTC_REG_C_IRQF) != 0u;

    if (((flags & X86_RTC_REG_C_PF) != 0u &&
            (enable & X86_RTC_REG_B_PIE) != 0u) ||
        ((flags & X86_RTC_REG_C_AF) != 0u &&
            (enable & X86_RTC_REG_B_AIE) != 0u) ||
        ((flags & X86_RTC_REG_C_UF) != 0u &&
            (enable & X86_RTC_REG_B_UIE) != 0u)) {
        rtc->registers[X86_RTC_REG_C] |= X86_RTC_REG_C_IRQF;
    } else {
        rtc->registers[X86_RTC_REG_C] &=
            (lib_u8)~X86_RTC_REG_C_IRQF;
    }
    if (rtc->output != LIB_NULL && was_asserted !=
            ((rtc->registers[X86_RTC_REG_C] & X86_RTC_REG_C_IRQF) != 0u)) {
        rtc->output(rtc->output_context, !was_asserted);
    }
}

static lib_u8 rtc_days_in_month(const x86_rtc *rtc)
{
    static const lib_u8 days[] = {31u, 28u, 31u, 30u, 31u, 30u,
        31u, 31u, 30u, 31u, 30u, 31u};
    lib_bool leap = (rtc->calendar.year & 3u) == 0u;

    /* Invalid programmed dates are not hardware-qualified. Keep their stored
     * value, but never use an out-of-range month as an array subscript. */
    if (rtc->calendar.month < 1u || rtc->calendar.month > 12u) return 31u;

    return rtc->calendar.month == 2u ? (lib_u8)(28u + leap) :
        days[rtc->calendar.month - 1u];
}

static void rtc_increment_second(x86_rtc *rtc)
{
    ++rtc->calendar.second;
    if (rtc->calendar.second < 60u) return;
    rtc->calendar.second = 0u;
    ++rtc->calendar.minute;
    if (rtc->calendar.minute < 60u) return;
    rtc->calendar.minute = 0u;
    ++rtc->calendar.hour;
    if (rtc->calendar.hour < 24u) return;
    rtc->calendar.hour = 0u;
    rtc->calendar.day_week = rtc->calendar.day_week == 7u ? 1u :
        (lib_u8)(rtc->calendar.day_week + 1u);
    ++rtc->calendar.day_month;
    if (rtc->calendar.day_month <= rtc_days_in_month(rtc)) return;
    rtc->calendar.day_month = 1u;
    ++rtc->calendar.month;
    if (rtc->calendar.month <= 12u) return;
    rtc->calendar.month = 1u;
    rtc->calendar.year = rtc->calendar.year == 99u ? 0u :
        (lib_u8)(rtc->calendar.year + 1u);
}

static lib_bool rtc_alarm_matches(const x86_rtc *rtc)
{
    lib_u8 second = rtc->registers[X86_RTC_SECOND_ALARM];
    lib_u8 minute = rtc->registers[X86_RTC_MINUTE_ALARM];
    lib_u8 hour = rtc->registers[X86_RTC_HOUR_ALARM];

    return ((second & 0xc0u) == 0xc0u || second == rtc_encode(rtc, rtc->calendar.second)) &&
        ((minute & 0xc0u) == 0xc0u || minute == rtc_encode(rtc, rtc->calendar.minute)) &&
        ((hour & 0xc0u) == 0xc0u || hour == rtc_hour_encode(rtc));
}

static lib_status rtc_ticks_until_alarm(const x86_rtc *rtc,
    lib_u64 *out_ticks)
{
    x86_rtc candidate;
    lib_u64 second;
    lib_u64 ticks;

    if (rtc == LIB_NULL || out_ticks == LIB_NULL || rtc->ticks_per_second == 0u ||
        rtc->calendar.second_ticks >= rtc->ticks_per_second) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    candidate = *rtc;
    for (second = 1u; second <= 86400u; ++second) {
        rtc_increment_second(&candidate);
        if (!rtc_alarm_matches(&candidate)) continue;
        ticks = rtc->ticks_per_second - rtc->calendar.second_ticks;
        if (second - 1u > (LIB_UINT64_MAX - ticks) / rtc->ticks_per_second) {
            return LIB_STATUS_INVALID_STATE;
        }
        *out_ticks = ticks + (second - 1u) * rtc->ticks_per_second;
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_INVALID_STATE;
}

static lib_bool rtc_uip_active(const x86_rtc *rtc)
{
    lib_u64 window = (lib_u64)rtc->uip_lead_ticks + rtc->update_ticks;

    return rtc_divider_running(rtc) &&
        (rtc->registers[X86_RTC_REG_B] & X86_RTC_REG_B_SET) == 0u &&
        window < rtc->ticks_per_second &&
        rtc->calendar.second_ticks >= rtc->ticks_per_second - window;
}

lib_u8 x86_rtc_read_register(x86_rtc *rtc, lib_u8 reg)
{
    if (rtc == LIB_NULL || reg >= X86_RTC_REGISTER_COUNT) return 0u;
    switch (reg) {
    case X86_RTC_SECOND: return rtc_encode(rtc, rtc->calendar.second);
    case X86_RTC_MINUTE: return rtc_encode(rtc, rtc->calendar.minute);
    case X86_RTC_HOUR: return rtc_hour_encode(rtc);
    case X86_RTC_DAY_WEEK: return rtc_encode(rtc, rtc->calendar.day_week);
    case X86_RTC_DAY_MONTH: return rtc_encode(rtc, rtc->calendar.day_month);
    case X86_RTC_MONTH: return rtc_encode(rtc, rtc->calendar.month);
    case X86_RTC_YEAR: return rtc_encode(rtc, rtc->calendar.year);
    case X86_RTC_REG_A: return rtc->registers[reg] |
        (rtc_uip_active(rtc) ? X86_RTC_REG_A_UIP : 0u);
    case X86_RTC_REG_C: {
        lib_u8 value = rtc->registers[reg];

        rtc->registers[reg] = 0u;
        if ((value & X86_RTC_REG_C_IRQF) != 0u && rtc->output != LIB_NULL) {
            rtc->output(rtc->output_context, LIB_FALSE);
        }
        return value;
    }
    default: return rtc->registers[reg];
    }
}

void x86_rtc_write_register(x86_rtc *rtc, lib_u8 reg,
    lib_u8 value)
{
    lib_bool was_running;

    if (rtc == LIB_NULL || reg >= X86_RTC_REGISTER_COUNT) return;
    was_running = rtc_divider_running(rtc);
    switch (reg) {
    case X86_RTC_SECOND: rtc->calendar.second = rtc_decode(rtc, value & 0x7fu); break;
    case X86_RTC_MINUTE: rtc->calendar.minute = rtc_decode(rtc, value); break;
    case X86_RTC_HOUR: rtc->calendar.hour = rtc_hour_decode(rtc, value); break;
    case X86_RTC_DAY_WEEK: rtc->calendar.day_week = rtc_decode(rtc, value); break;
    case X86_RTC_DAY_MONTH: rtc->calendar.day_month = rtc_decode(rtc, value); break;
    case X86_RTC_MONTH: rtc->calendar.month = rtc_decode(rtc, value); break;
    case X86_RTC_YEAR: rtc->calendar.year = rtc_decode(rtc, value); break;
    case X86_RTC_REG_A:
        rtc->registers[reg] = value & 0x7fu;
        if (!rtc_divider_running(rtc)) {
            rtc->calendar.second_ticks = 0u;
            rtc->calendar.periodic_ticks = 0u;
        } else if (!was_running) rtc->calendar.second_ticks = rtc->ticks_per_second / 2u;
        break;
    case X86_RTC_REG_C:
    case X86_RTC_REG_D: break;
    default: rtc->registers[reg] = value; break;
    }
    rtc_refresh_irq(rtc);
}

static lib_u32 rtc_phase_ticks(lib_u32 configured,
    lib_u32 ticks_per_second, lib_u32 microseconds)
{
    lib_u64 converted;

    if (configured != 0u) return configured;
    converted = ((lib_u64)ticks_per_second * microseconds) / 1000000u;
    return converted == 0u ? 1u : (lib_u32)converted;
}

lib_status x86_rtc_create(const x86_rtc_config *config,
    x86_rtc_output_provider output, void *context, x86_rtc **out_rtc)
{
    x86_rtc *rtc;
    if (out_rtc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_rtc = LIB_NULL;
    if (config == LIB_NULL || config->ticks_per_second == 0u) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    rtc = lib_allocate_zero(1u, sizeof(*rtc));
    if (rtc == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    rtc->ticks_per_second = config->ticks_per_second;
    rtc->uip_lead_ticks = rtc_phase_ticks(config->uip_lead_ticks,
        rtc->ticks_per_second, 244u);
    rtc->update_ticks = rtc_phase_ticks(config->update_ticks,
        rtc->ticks_per_second, 1984u);
    rtc->output = output;
    rtc->output_context = context;
    rtc->calendar.day_week = 6u;
    rtc->calendar.day_month = 1u;
    rtc->calendar.month = 1u;
    rtc->registers[X86_RTC_REG_A] = 0x26u;
    rtc->registers[X86_RTC_REG_B] = X86_RTC_REG_B_24H;
    rtc->registers[X86_RTC_REG_D] = X86_RTC_REG_D_VRT;
    *out_rtc = rtc;
    return LIB_STATUS_OK;
}

void x86_rtc_reset(x86_rtc *rtc)
{
    if (rtc == LIB_NULL) return;
    /* RESET clears delivery state but does not stop the clock/calendar phase. */
    rtc->registers[X86_RTC_REG_B] &= X86_RTC_REG_B_SET |
        X86_RTC_REG_B_DM | X86_RTC_REG_B_24H | 0x01u;
    (void)x86_rtc_read_register(rtc, X86_RTC_REG_C);
    rtc->registers[X86_RTC_REG_D] = X86_RTC_REG_D_VRT;
    rtc->square_wave = LIB_FALSE;
}

void x86_rtc_advance(x86_rtc *rtc, lib_u64 elapsed_ticks)
{
    lib_u32 periodic_hz;

    if (rtc == LIB_NULL || !rtc_divider_running(rtc)) return;
    periodic_hz = rtc_periodic_hz(rtc);
    if (periodic_hz != 0u) {
        lib_u64 seconds = elapsed_ticks / rtc->ticks_per_second;
        lib_u64 partial = elapsed_ticks % rtc->ticks_per_second;
        lib_u64 phase = rtc->calendar.periodic_ticks + partial * periodic_hz;
        lib_u64 edges = phase / rtc->ticks_per_second;

        rtc->calendar.periodic_ticks = phase % rtc->ticks_per_second;
        if (seconds != 0u || edges != 0u) {
            rtc->registers[X86_RTC_REG_C] |= X86_RTC_REG_C_PF;
            if ((rtc->registers[X86_RTC_REG_B] & X86_RTC_REG_B_SQWE) != 0u) {
                if (((seconds & 1u) != 0u && (periodic_hz & 1u) != 0u) ^
                    ((edges & 1u) != 0u)) {
                    rtc->square_wave = rtc->square_wave ? LIB_FALSE : LIB_TRUE;
                }
            }
        }
    }
    if ((rtc->registers[X86_RTC_REG_B] & X86_RTC_REG_B_SET) == 0u) {
        rtc->calendar.second_ticks += elapsed_ticks;
        while (rtc->calendar.second_ticks >= rtc->ticks_per_second) {
            rtc->calendar.second_ticks -= rtc->ticks_per_second;
            rtc_increment_second(rtc);
            rtc->registers[X86_RTC_REG_C] |= X86_RTC_REG_C_UF;
            if (rtc_alarm_matches(rtc)) rtc->registers[X86_RTC_REG_C] |=
                X86_RTC_REG_C_AF;
        }
    }
    rtc_refresh_irq(rtc);
}

void x86_rtc_destroy(x86_rtc *rtc)
{
    if (rtc == LIB_NULL) return;
    (void)x86_rtc_read_register(rtc, X86_RTC_REG_C);
    lib_release(rtc);
}

lib_status x86_rtc_ticks_until_irq(const x86_rtc *rtc,
    lib_u64 *out_ticks)
{
    lib_u32 periodic_hz;
    lib_u64 ticks = LIB_UINT64_MAX;
    lib_u64 update;
    lib_u8 enable;

    if (rtc == LIB_NULL || out_ticks == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!rtc_divider_running(rtc)) return LIB_STATUS_INVALID_STATE;
    enable = rtc->registers[X86_RTC_REG_B];
    periodic_hz = rtc_periodic_hz(rtc);
    if ((enable & X86_RTC_REG_B_PIE) != 0u && periodic_hz != 0u) {
        lib_u64 remaining = rtc->ticks_per_second -
            rtc->calendar.periodic_ticks;

        ticks = remaining / periodic_hz;
        if (remaining % periodic_hz != 0u) ++ticks;
    }
    if ((enable & X86_RTC_REG_B_UIE) != 0u &&
        (enable & X86_RTC_REG_B_SET) == 0u) {
        update = rtc->ticks_per_second - rtc->calendar.second_ticks;

        if (update < ticks) ticks = update;
    }
    if ((enable & X86_RTC_REG_B_AIE) != 0u &&
        (enable & X86_RTC_REG_B_SET) == 0u &&
        rtc_ticks_until_alarm(rtc, &update) == LIB_STATUS_OK && update < ticks) {
        ticks = update;
    }
    if (ticks == LIB_UINT64_MAX || ticks == 0u) return LIB_STATUS_INVALID_STATE;
    *out_ticks = ticks;
    return LIB_STATUS_OK;
}

lib_bool x86_rtc_get_square_wave(const x86_rtc *rtc)
{
    return rtc == LIB_NULL ? LIB_FALSE : rtc->square_wave;
}

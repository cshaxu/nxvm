/* Copyright 2012-2026 Neko. */

#include "type.h"

#include "core/machine/rtc.h"

static type_unsigned_8 rtc_encode(const core_machine_rtc *rtc, type_unsigned_8 value)
{
    return (rtc->registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_DM) != 0u ?
        value : (type_unsigned_8)(((value / 10u) << 4u) | (value % 10u));
}

static type_unsigned_8 rtc_decode(const core_machine_rtc *rtc, type_unsigned_8 value)
{
    return (rtc->registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_DM) != 0u ?
        value : (type_unsigned_8)(((value >> 4u) * 10u) + (value & 0x0fu));
}

static type_unsigned_8 rtc_hour_encode(const core_machine_rtc *rtc)
{
    type_unsigned_8 hour = rtc->calendar.hour;

    if ((rtc->registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_24H) != 0u) {
        return rtc_encode(rtc, hour);
    }
    if (hour >= 12u) hour = (type_unsigned_8)(hour - 12u);
    if (hour == 0u) hour = 12u;
    return rtc_encode(rtc, hour) | (rtc->calendar.hour >= 12u ? 0x80u : 0u);
}

static type_unsigned_8 rtc_hour_decode(const core_machine_rtc *rtc, type_unsigned_8 value)
{
    type_unsigned_8 hour;

    if ((rtc->registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_24H) != 0u) {
        return rtc_decode(rtc, value);
    }
    hour = rtc_decode(rtc, value & 0x7fu);
    if (hour == 12u) hour = 0u;
    return (type_unsigned_8)(hour + ((value & 0x80u) != 0u ? 12u : 0u));
}

static type_bool rtc_divider_running(const core_machine_rtc *rtc)
{
    type_unsigned_8 divider = rtc->registers[CORE_MACHINE_RTC_REG_A] & 0x70u;

    return divider == 0x20u || divider == 0x50u || divider == 0x60u;
}

static type_unsigned_32 rtc_divider_hz(const core_machine_rtc *rtc)
{
    switch (rtc->registers[CORE_MACHINE_RTC_REG_A] & 0x70u) {
    case 0x20u: return 32768u;
    case 0x50u: return 1048576u;
    case 0x60u: return 4194304u;
    default: return 0u;
    }
}

static type_unsigned_32 rtc_periodic_hz(const core_machine_rtc *rtc)
{
    type_unsigned_8 rate = rtc->registers[CORE_MACHINE_RTC_REG_A] & 0x0fu;
    type_unsigned_32 base = rtc_divider_hz(rtc);

    if (base == 0u || rate == 0u) return 0u;
    if (rate == 1u) return base / 128u;
    if (rate == 2u) return base / 256u;
    return rate <= 15u ? base >> (rate - 1u) : 0u;
}

static C_VOID rtc_refresh_irq(core_machine_rtc *rtc)
{
    type_unsigned_8 flags = rtc->registers[CORE_MACHINE_RTC_REG_C];
    type_unsigned_8 enable = rtc->registers[CORE_MACHINE_RTC_REG_B];

    if (((flags & CORE_MACHINE_RTC_REG_C_PF) != 0u &&
            (enable & CORE_MACHINE_RTC_REG_B_PIE) != 0u) ||
        ((flags & CORE_MACHINE_RTC_REG_C_AF) != 0u &&
            (enable & CORE_MACHINE_RTC_REG_B_AIE) != 0u) ||
        ((flags & CORE_MACHINE_RTC_REG_C_UF) != 0u &&
            (enable & CORE_MACHINE_RTC_REG_B_UIE) != 0u)) {
        rtc->registers[CORE_MACHINE_RTC_REG_C] |= CORE_MACHINE_RTC_REG_C_IRQF;
        core_machine_pic_irq_source_assert(&rtc->irq_source);
    } else {
        rtc->registers[CORE_MACHINE_RTC_REG_C] &=
            (type_unsigned_8)~CORE_MACHINE_RTC_REG_C_IRQF;
        core_machine_pic_irq_source_deassert(&rtc->irq_source);
    }
}

static type_unsigned_8 rtc_days_in_month(const core_machine_rtc *rtc)
{
    static const type_unsigned_8 days[] = {31u, 28u, 31u, 30u, 31u, 30u,
        31u, 31u, 30u, 31u, 30u, 31u};
    type_bool leap = (rtc->calendar.year & 3u) == 0u;

    return rtc->calendar.month == 2u ? (type_unsigned_8)(28u + leap) :
        days[rtc->calendar.month - 1u];
}

static C_VOID rtc_increment_second(core_machine_rtc *rtc)
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
        (type_unsigned_8)(rtc->calendar.day_week + 1u);
    ++rtc->calendar.day_month;
    if (rtc->calendar.day_month <= rtc_days_in_month(rtc)) return;
    rtc->calendar.day_month = 1u;
    ++rtc->calendar.month;
    if (rtc->calendar.month <= 12u) return;
    rtc->calendar.month = 1u;
    rtc->calendar.year = rtc->calendar.year == 99u ? 0u :
        (type_unsigned_8)(rtc->calendar.year + 1u);
}

static type_bool rtc_alarm_matches(const core_machine_rtc *rtc)
{
    type_unsigned_8 second = rtc->registers[CORE_MACHINE_RTC_SECOND_ALARM];
    type_unsigned_8 minute = rtc->registers[CORE_MACHINE_RTC_MINUTE_ALARM];
    type_unsigned_8 hour = rtc->registers[CORE_MACHINE_RTC_HOUR_ALARM];

    return ((second & 0xc0u) == 0xc0u || second == rtc_encode(rtc, rtc->calendar.second)) &&
        ((minute & 0xc0u) == 0xc0u || minute == rtc_encode(rtc, rtc->calendar.minute)) &&
        ((hour & 0xc0u) == 0xc0u || hour == rtc_hour_encode(rtc));
}

static type_status rtc_ticks_until_alarm(const core_machine_rtc *rtc,
    type_unsigned_64 *out_ticks)
{
    core_machine_rtc candidate;
    type_unsigned_64 second;
    type_unsigned_64 ticks;

    if (rtc == STD_NULL || out_ticks == STD_NULL || rtc->ticks_per_second == 0u ||
        rtc->calendar.second_ticks >= rtc->ticks_per_second) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    candidate = *rtc;
    for (second = 1u; second <= 86400u; ++second) {
        rtc_increment_second(&candidate);
        if (!rtc_alarm_matches(&candidate)) continue;
        ticks = rtc->ticks_per_second - rtc->calendar.second_ticks;
        if (second - 1u > (UINT64_MAX - ticks) / rtc->ticks_per_second) {
            return TYPE_STATUS_INVALID_STATE;
        }
        *out_ticks = ticks + (second - 1u) * rtc->ticks_per_second;
        return TYPE_STATUS_OK;
    }
    return TYPE_STATUS_INVALID_STATE;
}

static type_bool rtc_uip_active(const core_machine_rtc *rtc)
{
    type_unsigned_64 window = (type_unsigned_64)rtc->uip_lead_ticks + rtc->update_ticks;

    return rtc_divider_running(rtc) &&
        (rtc->registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_SET) == 0u &&
        window < rtc->ticks_per_second &&
        rtc->calendar.second_ticks >= rtc->ticks_per_second - window;
}

static type_unsigned_8 rtc_read_register(core_machine_rtc *rtc, type_unsigned_8 reg)
{
    switch (reg) {
    case CORE_MACHINE_RTC_SECOND: return rtc_encode(rtc, rtc->calendar.second);
    case CORE_MACHINE_RTC_MINUTE: return rtc_encode(rtc, rtc->calendar.minute);
    case CORE_MACHINE_RTC_HOUR: return rtc_hour_encode(rtc);
    case CORE_MACHINE_RTC_DAY_WEEK: return rtc_encode(rtc, rtc->calendar.day_week);
    case CORE_MACHINE_RTC_DAY_MONTH: return rtc_encode(rtc, rtc->calendar.day_month);
    case CORE_MACHINE_RTC_MONTH: return rtc_encode(rtc, rtc->calendar.month);
    case CORE_MACHINE_RTC_YEAR: return rtc_encode(rtc, rtc->calendar.year);
    case CORE_MACHINE_RTC_REG_A: return rtc->registers[reg] |
        (rtc_uip_active(rtc) ? CORE_MACHINE_RTC_REG_A_UIP : 0u);
    case CORE_MACHINE_RTC_REG_C: {
        type_unsigned_8 value = rtc->registers[reg];

        rtc->registers[reg] = 0u;
        core_machine_pic_irq_source_deassert(&rtc->irq_source);
        return value;
    }
    default: return rtc->registers[reg];
    }
}

static C_VOID rtc_write_register(core_machine_rtc *rtc, type_unsigned_8 reg,
    type_unsigned_8 value)
{
    type_bool was_running = rtc_divider_running(rtc);

    switch (reg) {
    case CORE_MACHINE_RTC_SECOND: rtc->calendar.second = rtc_decode(rtc, value & 0x7fu); break;
    case CORE_MACHINE_RTC_MINUTE: rtc->calendar.minute = rtc_decode(rtc, value); break;
    case CORE_MACHINE_RTC_HOUR: rtc->calendar.hour = rtc_hour_decode(rtc, value); break;
    case CORE_MACHINE_RTC_DAY_WEEK: rtc->calendar.day_week = rtc_decode(rtc, value); break;
    case CORE_MACHINE_RTC_DAY_MONTH: rtc->calendar.day_month = rtc_decode(rtc, value); break;
    case CORE_MACHINE_RTC_MONTH: rtc->calendar.month = rtc_decode(rtc, value); break;
    case CORE_MACHINE_RTC_YEAR: rtc->calendar.year = rtc_decode(rtc, value); break;
    case CORE_MACHINE_RTC_REG_A:
        rtc->registers[reg] = value & 0x7fu;
        if (!rtc_divider_running(rtc)) {
            rtc->calendar.second_ticks = 0u;
            rtc->calendar.periodic_ticks = 0u;
        } else if (!was_running) rtc->calendar.second_ticks = rtc->ticks_per_second / 2u;
        break;
    case CORE_MACHINE_RTC_REG_C:
    case CORE_MACHINE_RTC_REG_D: break;
    default: rtc->registers[reg] = value; break;
    }
}

static type_unsigned_32 rtc_phase_ticks(type_unsigned_32 configured,
    type_unsigned_32 ticks_per_second, type_unsigned_32 microseconds)
{
    type_unsigned_64 converted;

    if (configured != 0u) return configured;
    converted = ((type_unsigned_64)ticks_per_second * microseconds) / 1000000u;
    return converted == 0u ? 1u : (type_unsigned_32)converted;
}

C_VOID core_machine_rtc_initialize(core_machine_rtc *rtc, t_pic *pic_master,
    t_pic *pic_slave, const core_machine_rtc_config *config)
{
    if (rtc == STD_NULL || config == STD_NULL) return;
    STD_MEMSET(rtc, TYPE_ZERO_8, sizeof(*rtc));
    rtc->ticks_per_second = config->ticks_per_second == 0u ? 1u : config->ticks_per_second;
    rtc->uip_lead_ticks = rtc_phase_ticks(config->timing.uip_lead_ticks,
        rtc->ticks_per_second, 244u);
    rtc->update_ticks = rtc_phase_ticks(config->timing.update_ticks,
        rtc->ticks_per_second, 1984u);
    rtc->timing_provenance = config->timing.provenance;
    rtc->calendar.day_week = 6u;
    rtc->calendar.day_month = 1u;
    rtc->calendar.month = 1u;
    rtc->registers[CORE_MACHINE_RTC_REG_A] = 0x26u;
    rtc->registers[CORE_MACHINE_RTC_REG_B] = CORE_MACHINE_RTC_REG_B_24H;
    rtc->registers[CORE_MACHINE_RTC_REG_D] = CORE_MACHINE_RTC_REG_D_VRT;
    core_machine_pic_irq_source_bind(&rtc->irq_source, pic_master, pic_slave, config->irq);
}

C_VOID core_machine_rtc_reset(core_machine_rtc *rtc)
{
    if (rtc == STD_NULL) return;
    /* RESET clears delivery state but does not stop the clock/calendar phase. */
    rtc->registers[CORE_MACHINE_RTC_REG_B] &= CORE_MACHINE_RTC_REG_B_SET |
        CORE_MACHINE_RTC_REG_B_DM | CORE_MACHINE_RTC_REG_B_24H | 0x01u;
    rtc->registers[CORE_MACHINE_RTC_REG_C] = 0u;
    rtc->registers[CORE_MACHINE_RTC_REG_D] = CORE_MACHINE_RTC_REG_D_VRT;
    rtc->square_wave = TYPE_FALSE;
    core_machine_pic_irq_source_deassert(&rtc->irq_source);
}

C_VOID core_machine_rtc_advance(core_machine_rtc *rtc, type_unsigned_64 elapsed_ticks)
{
    type_unsigned_32 periodic_hz;

    if (rtc == STD_NULL || !rtc_divider_running(rtc)) return;
    periodic_hz = rtc_periodic_hz(rtc);
    if (periodic_hz != 0u) {
        type_unsigned_64 seconds = elapsed_ticks / rtc->ticks_per_second;
        type_unsigned_64 partial = elapsed_ticks % rtc->ticks_per_second;
        type_unsigned_64 phase = rtc->calendar.periodic_ticks + partial * periodic_hz;
        type_unsigned_64 edges = phase / rtc->ticks_per_second;

        rtc->calendar.periodic_ticks = phase % rtc->ticks_per_second;
        if (seconds != 0u || edges != 0u) {
            rtc->registers[CORE_MACHINE_RTC_REG_C] |= CORE_MACHINE_RTC_REG_C_PF;
            if ((rtc->registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_SQWE) != 0u) {
                if (((seconds & 1u) != 0u && (periodic_hz & 1u) != 0u) ^
                    ((edges & 1u) != 0u)) {
                    rtc->square_wave = rtc->square_wave ? TYPE_FALSE : TYPE_TRUE;
                }
            }
        }
    }
    if ((rtc->registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_SET) == 0u) {
        rtc->calendar.second_ticks += elapsed_ticks;
        while (rtc->calendar.second_ticks >= rtc->ticks_per_second) {
            rtc->calendar.second_ticks -= rtc->ticks_per_second;
            rtc_increment_second(rtc);
            rtc->registers[CORE_MACHINE_RTC_REG_C] |= CORE_MACHINE_RTC_REG_C_UF;
            if (rtc_alarm_matches(rtc)) rtc->registers[CORE_MACHINE_RTC_REG_C] |=
                CORE_MACHINE_RTC_REG_C_AF;
        }
    }
    rtc_refresh_irq(rtc);
}

C_VOID core_machine_rtc_finalize(core_machine_rtc *rtc)
{
    if (rtc != STD_NULL) core_machine_pic_irq_source_deassert(&rtc->irq_source);
}

type_status core_machine_rtc_ticks_until_irq(const core_machine_rtc *rtc,
    type_unsigned_64 *out_ticks)
{
    type_unsigned_32 periodic_hz;
    type_unsigned_64 ticks = UINT64_MAX;
    type_unsigned_64 update;
    type_unsigned_8 enable;

    if (rtc == STD_NULL || out_ticks == STD_NULL || !rtc_divider_running(rtc)) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    enable = rtc->registers[CORE_MACHINE_RTC_REG_B];
    periodic_hz = rtc_periodic_hz(rtc);
    if ((enable & CORE_MACHINE_RTC_REG_B_PIE) != 0u && periodic_hz != 0u) {
        type_unsigned_64 remaining = rtc->ticks_per_second -
            rtc->calendar.periodic_ticks;

        ticks = remaining / periodic_hz;
        if (remaining % periodic_hz != 0u) ++ticks;
    }
    if ((enable & CORE_MACHINE_RTC_REG_B_UIE) != 0u &&
        (enable & CORE_MACHINE_RTC_REG_B_SET) == 0u) {
        update = rtc->ticks_per_second - rtc->calendar.second_ticks;

        if (update < ticks) ticks = update;
    }
    if ((enable & CORE_MACHINE_RTC_REG_B_AIE) != 0u &&
        (enable & CORE_MACHINE_RTC_REG_B_SET) == 0u &&
        rtc_ticks_until_alarm(rtc, &update) == TYPE_STATUS_OK && update < ticks) {
        ticks = update;
    }
    if (ticks == UINT64_MAX || ticks == 0u) return TYPE_STATUS_INVALID_STATE;
    *out_ticks = ticks;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_rtc_select_register(core_machine_rtc *rtc, type_unsigned_8 index)
{
    if (rtc != STD_NULL) rtc->selected_register = index & 0x3fu;
}

type_unsigned_8 core_machine_rtc_read_selected(core_machine_rtc *rtc)
{
    return rtc == STD_NULL ? 0u : rtc_read_register(rtc, rtc->selected_register);
}

C_VOID core_machine_rtc_write_selected(core_machine_rtc *rtc, type_unsigned_8 value)
{
    if (rtc != STD_NULL) {
        rtc_write_register(rtc, rtc->selected_register, value);
        rtc_refresh_irq(rtc);
    }
}

C_VOID core_machine_rtc_write_nvram(core_machine_rtc *rtc, type_unsigned_8 index,
    type_unsigned_8 value)
{
    if (rtc == STD_NULL || index >= CORE_MACHINE_RTC_REGISTER_COUNT ||
        index <= CORE_MACHINE_RTC_REG_D) return;
    rtc->registers[index] = value;
}

type_bool core_machine_rtc_get_square_wave(const core_machine_rtc *rtc)
{
    return rtc == STD_NULL ? TYPE_FALSE : rtc->square_wave;
}

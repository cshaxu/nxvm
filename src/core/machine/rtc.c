/* Copyright 2012-2026 Neko. */

#include "type.h"

#include "core/machine/pic.h"
#include "core/machine/rtc.h"

static type_unsigned_8 core_machine_rtc_bcd(type_unsigned_8 value)
{ return (type_unsigned_8)(((value / 10u) << 4u) | (value % 10u)); }

static type_unsigned_8 core_machine_rtc_from_bcd(type_unsigned_8 value)
{ return (type_unsigned_8)(((value >> 4u) * 10u) + (value & 0x0fu)); }

static C_INT core_machine_rtc_binary_mode(const core_machine_rtc *cmos)
{ return (cmos->registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_DM) != 0u; }

static type_unsigned_8 core_machine_rtc_encode(const core_machine_rtc *cmos,
    type_unsigned_8 value)
{ return core_machine_rtc_binary_mode(cmos) ? value : core_machine_rtc_bcd(value); }

static type_unsigned_8 core_machine_rtc_decode(const core_machine_rtc *cmos,
    type_unsigned_8 value)
{ return core_machine_rtc_binary_mode(cmos) ? value : core_machine_rtc_from_bcd(value); }

static type_unsigned_8 core_machine_rtc_hour_encode(const core_machine_rtc *cmos)
{
    type_unsigned_8 hour = cmos->calendar.hour;
    if ((cmos->registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_24H) != 0u) {
        return core_machine_rtc_encode(cmos, hour);
    }
    return (hour >= 12u ? 0x80u : 0u) | core_machine_rtc_encode(cmos,
        (type_unsigned_8)(hour % 12u == 0u ? 12u : hour % 12u));
}

static type_unsigned_8 core_machine_rtc_hour_decode(const core_machine_rtc *cmos,
    type_unsigned_8 value)
{
    type_unsigned_8 hour;
    if ((cmos->registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_24H) != 0u) {
        return core_machine_rtc_decode(cmos, value);
    }
    hour = core_machine_rtc_decode(cmos, value & 0x7fu);
    if (hour == 12u) hour = 0u;
    return (type_unsigned_8)(hour + ((value & 0x80u) != 0u ? 12u : 0u));
}

static type_unsigned_8 core_machine_rtc_days_in_month(const core_machine_rtc *cmos)
{
    type_unsigned_8 leap = (cmos->calendar.year % 4u) == 0u;
    static const type_unsigned_8 days[] = { 31u, 28u, 31u, 30u, 31u, 30u,
        31u, 31u, 30u, 31u, 30u, 31u };
    return cmos->calendar.month == 2u ? (type_unsigned_8)(28u + leap) :
        days[cmos->calendar.month - 1u];
}

static C_VOID core_machine_rtc_raise_if_enabled(core_machine_rtc *cmos)
{
    type_unsigned_8 flags = cmos->registers[CORE_MACHINE_RTC_REG_C];
    type_unsigned_8 enable = cmos->registers[CORE_MACHINE_RTC_REG_B];
    if (((flags & CORE_MACHINE_RTC_REG_C_PF) != 0u && (enable & CORE_MACHINE_RTC_REG_B_PIE) != 0u) ||
        ((flags & CORE_MACHINE_RTC_REG_C_AF) != 0u && (enable & CORE_MACHINE_RTC_REG_B_AIE) != 0u) ||
        ((flags & CORE_MACHINE_RTC_REG_C_UF) != 0u && (enable & CORE_MACHINE_RTC_REG_B_UIE) != 0u)) {
        cmos->registers[CORE_MACHINE_RTC_REG_C] |= CORE_MACHINE_RTC_REG_C_IRQF;
        core_machine_pic_irq_source_assert(&cmos->irq_source);
    }
}

static C_VOID core_machine_rtc_increment_second(core_machine_rtc *cmos)
{
    cmos->calendar.second++;
    if (cmos->calendar.second < 60u) return;
    cmos->calendar.second = 0u;
    cmos->calendar.minute++;
    if (cmos->calendar.minute < 60u) return;
    cmos->calendar.minute = 0u;
    cmos->calendar.hour++;
    if (cmos->calendar.hour < 24u) return;
    cmos->calendar.hour = 0u;
    cmos->calendar.day_week = cmos->calendar.day_week == 7u ? 1u :
        (type_unsigned_8)(cmos->calendar.day_week + 1u);
    cmos->calendar.day_month++;
    if (cmos->calendar.day_month <= core_machine_rtc_days_in_month(cmos)) return;
    cmos->calendar.day_month = 1u;
    cmos->calendar.month++;
    if (cmos->calendar.month <= 12u) return;
    cmos->calendar.month = 1u;
    cmos->calendar.year++;
    if (cmos->calendar.year < 100u) return;
    cmos->calendar.year = 0u;
    cmos->calendar.century++;
}

static C_INT core_machine_rtc_alarm_matches(const core_machine_rtc *cmos)
{
    return cmos->registers[CORE_MACHINE_RTC_SECOND_ALARM] ==
        core_machine_rtc_encode(cmos, cmos->calendar.second) &&
        cmos->registers[CORE_MACHINE_RTC_MINUTE_ALARM] ==
        core_machine_rtc_encode(cmos, cmos->calendar.minute) &&
        cmos->registers[CORE_MACHINE_RTC_HOUR_ALARM] == core_machine_rtc_hour_encode(cmos);
}

static type_unsigned_32 core_machine_rtc_periodic_hz(const core_machine_rtc *cmos)
{
    type_unsigned_8 rate = cmos->registers[CORE_MACHINE_RTC_REG_A] & 0x0fu;
    return rate >= 3u && rate <= 15u ? 32768u >> (rate - 1u) : 0u;
}

static type_unsigned_8 core_machine_rtc_read_register(core_machine_rtc *cmos,
    type_unsigned_8 reg)
{
    switch (reg) {
    case CORE_MACHINE_RTC_SECOND: return core_machine_rtc_encode(cmos, cmos->calendar.second);
    case CORE_MACHINE_RTC_MINUTE: return core_machine_rtc_encode(cmos, cmos->calendar.minute);
    case CORE_MACHINE_RTC_HOUR: return core_machine_rtc_hour_encode(cmos);
    case CORE_MACHINE_RTC_DAY_WEEK: return core_machine_rtc_encode(cmos, cmos->calendar.day_week);
    case CORE_MACHINE_RTC_DAY_MONTH: return core_machine_rtc_encode(cmos, cmos->calendar.day_month);
    case CORE_MACHINE_RTC_MONTH: return core_machine_rtc_encode(cmos, cmos->calendar.month);
    case CORE_MACHINE_RTC_YEAR: return core_machine_rtc_encode(cmos, cmos->calendar.year);
    case CORE_MACHINE_RTC_CENTURY: return core_machine_rtc_encode(cmos, cmos->calendar.century);
    case CORE_MACHINE_RTC_REG_A:
        return cmos->registers[reg] | (cmos->calendar.second_ticks + 1u >=
            cmos->ticks_per_second ? CORE_MACHINE_RTC_REG_A_UIP : 0u);
    case CORE_MACHINE_RTC_REG_C: {
        type_unsigned_8 value = cmos->registers[reg];
        cmos->registers[reg] = 0u;
        core_machine_pic_irq_source_deassert(&cmos->irq_source);
        return value;
    }
    default: return cmos->registers[reg];
    }
}

static C_VOID core_machine_rtc_write_register(core_machine_rtc *cmos, type_unsigned_8 reg,
    type_unsigned_8 value)
{
    switch (reg) {
    case CORE_MACHINE_RTC_SECOND: cmos->calendar.second = core_machine_rtc_decode(cmos, value); break;
    case CORE_MACHINE_RTC_MINUTE: cmos->calendar.minute = core_machine_rtc_decode(cmos, value); break;
    case CORE_MACHINE_RTC_HOUR: cmos->calendar.hour = core_machine_rtc_hour_decode(cmos, value); break;
    case CORE_MACHINE_RTC_DAY_WEEK: cmos->calendar.day_week = core_machine_rtc_decode(cmos, value); break;
    case CORE_MACHINE_RTC_DAY_MONTH: cmos->calendar.day_month = core_machine_rtc_decode(cmos, value); break;
    case CORE_MACHINE_RTC_MONTH: cmos->calendar.month = core_machine_rtc_decode(cmos, value); break;
    case CORE_MACHINE_RTC_YEAR: cmos->calendar.year = core_machine_rtc_decode(cmos, value); break;
    case CORE_MACHINE_RTC_CENTURY: cmos->calendar.century = core_machine_rtc_decode(cmos, value); break;
    case CORE_MACHINE_RTC_REG_C:
    case CORE_MACHINE_RTC_REG_D: break;
    default: cmos->registers[reg] = value; break;
    }
}

C_VOID core_machine_rtc_initialize(core_machine_rtc *cmos, t_pic *pic_master,
    t_pic *pic_slave, const core_machine_rtc_config *config)
{
    if (cmos == STD_NULL || config == STD_NULL) return;
    STD_MEMSET(cmos, TYPE_ZERO_8, sizeof(*cmos));
    cmos->ticks_per_second = config->ticks_per_second == 0u ? 1u :
        config->ticks_per_second;
    core_machine_pic_irq_source_bind(&cmos->irq_source, pic_master,
        pic_slave, config->irq);
    core_machine_rtc_reset(cmos);
}

C_VOID core_machine_rtc_reset(core_machine_rtc *cmos)
{
    if (cmos == STD_NULL) return;
    STD_MEMSET(&cmos->calendar, TYPE_ZERO_8, sizeof(cmos->calendar));
    cmos->calendar.day_week = 6u; cmos->calendar.day_month = 1u; cmos->calendar.month = 1u;
    cmos->calendar.century = 20u;
    cmos->registers[CORE_MACHINE_RTC_REG_A] = 0x26u;
    cmos->registers[CORE_MACHINE_RTC_REG_B] = CORE_MACHINE_RTC_REG_B_24H;
    cmos->registers[CORE_MACHINE_RTC_REG_C] = 0u;
    cmos->registers[CORE_MACHINE_RTC_REG_D] = CORE_MACHINE_RTC_REG_D_VRT;
    core_machine_pic_irq_source_deassert(&cmos->irq_source);
}

C_VOID core_machine_rtc_advance(core_machine_rtc *cmos, type_unsigned_64 elapsed_ticks)
{
    type_unsigned_32 periodic_hz;
    if (cmos == STD_NULL || (cmos->registers[CORE_MACHINE_RTC_REG_B] & CORE_MACHINE_RTC_REG_B_SET) != 0u)
        return;
    cmos->calendar.second_ticks += elapsed_ticks;
    while (cmos->calendar.second_ticks >= cmos->ticks_per_second) {
        cmos->calendar.second_ticks -= cmos->ticks_per_second;
        core_machine_rtc_increment_second(cmos);
        cmos->registers[CORE_MACHINE_RTC_REG_C] |= CORE_MACHINE_RTC_REG_C_UF;
        if (core_machine_rtc_alarm_matches(cmos))
            cmos->registers[CORE_MACHINE_RTC_REG_C] |= CORE_MACHINE_RTC_REG_C_AF;
    }
    periodic_hz = core_machine_rtc_periodic_hz(cmos);
    if (periodic_hz != 0u) {
        cmos->calendar.periodic_ticks += elapsed_ticks * periodic_hz;
        while (cmos->calendar.periodic_ticks >= cmos->ticks_per_second) {
            cmos->calendar.periodic_ticks -= cmos->ticks_per_second;
            cmos->registers[CORE_MACHINE_RTC_REG_C] |= CORE_MACHINE_RTC_REG_C_PF;
        }
    }
    core_machine_rtc_raise_if_enabled(cmos);
}

C_VOID core_machine_rtc_finalize(core_machine_rtc *cmos)
{
    if (cmos != STD_NULL) core_machine_pic_irq_source_deassert(&cmos->irq_source);
}

C_VOID core_machine_rtc_select_register(core_machine_rtc *cmos, type_unsigned_8 index)
{
    if (cmos != STD_NULL) cmos->selected_register = index & 0x7fu;
}

type_unsigned_8 core_machine_rtc_read_selected(core_machine_rtc *cmos)
{
    return cmos == STD_NULL ? 0u : core_machine_rtc_read_register(cmos,
        cmos->selected_register);
}

C_VOID core_machine_rtc_write_selected(core_machine_rtc *cmos, type_unsigned_8 value)
{
    if (cmos != STD_NULL) {
        core_machine_rtc_write_register(cmos, cmos->selected_register, value);
    }
}

C_VOID core_machine_rtc_write_nvram(core_machine_rtc *cmos, type_unsigned_8 index,
    type_unsigned_8 value)
{
    if (cmos == STD_NULL || index >= CORE_MACHINE_RTC_REGISTER_COUNT ||
        index == CORE_MACHINE_RTC_REG_A || index == CORE_MACHINE_RTC_REG_B ||
        index == CORE_MACHINE_RTC_REG_C || index == CORE_MACHINE_RTC_REG_D) {
        return;
    }
    cmos->registers[index] = value;
}

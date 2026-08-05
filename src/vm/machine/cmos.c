/* Copyright 2012-2026 Neko. */

#include "type.h"

#include "core/product/utils.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"
#include "vm/machine/cmos.h"

static type_unsigned_8 vm_machine_cmos_bcd(type_unsigned_8 value)
{ return (type_unsigned_8)(((value / 10u) << 4u) | (value % 10u)); }

static type_unsigned_8 vm_machine_cmos_from_bcd(type_unsigned_8 value)
{ return (type_unsigned_8)(((value >> 4u) * 10u) + (value & 0x0fu)); }

static C_INT vm_machine_cmos_binary_mode(const t_cmos *cmos)
{ return (cmos->connect.reg[VCMOS_RTC_REG_B] & VCMOS_REG_B_DM) != 0u; }

static type_unsigned_8 vm_machine_cmos_encode(const t_cmos *cmos,
    type_unsigned_8 value)
{ return vm_machine_cmos_binary_mode(cmos) ? value : vm_machine_cmos_bcd(value); }

static type_unsigned_8 vm_machine_cmos_decode(const t_cmos *cmos,
    type_unsigned_8 value)
{ return vm_machine_cmos_binary_mode(cmos) ? value : vm_machine_cmos_from_bcd(value); }

static type_unsigned_8 vm_machine_cmos_hour_encode(const t_cmos *cmos)
{
    type_unsigned_8 hour = cmos->data.hour;
    if ((cmos->connect.reg[VCMOS_RTC_REG_B] & VCMOS_REG_B_24H) != 0u) {
        return vm_machine_cmos_encode(cmos, hour);
    }
    return (hour >= 12u ? 0x80u : 0u) | vm_machine_cmos_encode(cmos,
        (type_unsigned_8)(hour % 12u == 0u ? 12u : hour % 12u));
}

static type_unsigned_8 vm_machine_cmos_hour_decode(const t_cmos *cmos,
    type_unsigned_8 value)
{
    type_unsigned_8 hour;
    if ((cmos->connect.reg[VCMOS_RTC_REG_B] & VCMOS_REG_B_24H) != 0u) {
        return vm_machine_cmos_decode(cmos, value);
    }
    hour = vm_machine_cmos_decode(cmos, value & 0x7fu);
    if (hour == 12u) hour = 0u;
    return (type_unsigned_8)(hour + ((value & 0x80u) != 0u ? 12u : 0u));
}

static type_unsigned_8 vm_machine_cmos_days_in_month(const t_cmos *cmos)
{
    type_unsigned_8 leap = (cmos->data.year % 4u) == 0u;
    static const type_unsigned_8 days[] = { 31u, 28u, 31u, 30u, 31u, 30u,
        31u, 31u, 30u, 31u, 30u, 31u };
    return cmos->data.month == 2u ? (type_unsigned_8)(28u + leap) :
        days[cmos->data.month - 1u];
}

static C_VOID vm_machine_cmos_raise_if_enabled(t_cmos *cmos)
{
    type_unsigned_8 flags = cmos->connect.reg[VCMOS_RTC_REG_C];
    type_unsigned_8 enable = cmos->connect.reg[VCMOS_RTC_REG_B];
    if (((flags & VCMOS_REG_C_PF) != 0u && (enable & VCMOS_REG_B_PIE) != 0u) ||
        ((flags & VCMOS_REG_C_AF) != 0u && (enable & VCMOS_REG_B_AIE) != 0u) ||
        ((flags & VCMOS_REG_C_UF) != 0u && (enable & VCMOS_REG_B_UIE) != 0u)) {
        cmos->connect.reg[VCMOS_RTC_REG_C] |= VCMOS_REG_C_IRQF;
        core_machine_pic_irq_source_assert(&cmos->connect.irq_source);
    }
}

static C_VOID vm_machine_cmos_increment_second(t_cmos *cmos)
{
    cmos->data.second++;
    if (cmos->data.second < 60u) return;
    cmos->data.second = 0u;
    cmos->data.minute++;
    if (cmos->data.minute < 60u) return;
    cmos->data.minute = 0u;
    cmos->data.hour++;
    if (cmos->data.hour < 24u) return;
    cmos->data.hour = 0u;
    cmos->data.day_week = cmos->data.day_week == 7u ? 1u :
        (type_unsigned_8)(cmos->data.day_week + 1u);
    cmos->data.day_month++;
    if (cmos->data.day_month <= vm_machine_cmos_days_in_month(cmos)) return;
    cmos->data.day_month = 1u;
    cmos->data.month++;
    if (cmos->data.month <= 12u) return;
    cmos->data.month = 1u;
    cmos->data.year++;
    if (cmos->data.year < 100u) return;
    cmos->data.year = 0u;
    cmos->data.century++;
}

static C_INT vm_machine_cmos_alarm_matches(const t_cmos *cmos)
{
    return cmos->connect.reg[VCMOS_RTC_SECOND_ALARM] ==
        vm_machine_cmos_encode(cmos, cmos->data.second) &&
        cmos->connect.reg[VCMOS_RTC_MINUTE_ALARM] ==
        vm_machine_cmos_encode(cmos, cmos->data.minute) &&
        cmos->connect.reg[VCMOS_RTC_HOUR_ALARM] == vm_machine_cmos_hour_encode(cmos);
}

static uint32_t vm_machine_cmos_periodic_hz(const t_cmos *cmos)
{
    uint8_t rate = cmos->connect.reg[VCMOS_RTC_REG_A] & 0x0fu;
    return rate >= 3u && rate <= 15u ? 32768u >> (rate - 1u) : 0u;
}

static type_unsigned_8 vm_machine_cmos_read_register(t_cmos *cmos,
    type_unsigned_8 reg)
{
    switch (reg) {
    case VCMOS_RTC_SECOND: return vm_machine_cmos_encode(cmos, cmos->data.second);
    case VCMOS_RTC_MINUTE: return vm_machine_cmos_encode(cmos, cmos->data.minute);
    case VCMOS_RTC_HOUR: return vm_machine_cmos_hour_encode(cmos);
    case VCMOS_RTC_DAY_WEEK: return vm_machine_cmos_encode(cmos, cmos->data.day_week);
    case VCMOS_RTC_DAY_MONTH: return vm_machine_cmos_encode(cmos, cmos->data.day_month);
    case VCMOS_RTC_MONTH: return vm_machine_cmos_encode(cmos, cmos->data.month);
    case VCMOS_RTC_YEAR: return vm_machine_cmos_encode(cmos, cmos->data.year);
    case VCMOS_RTC_CENTURY: return vm_machine_cmos_encode(cmos, cmos->data.century);
    case VCMOS_RTC_REG_A:
        return cmos->connect.reg[reg] | (cmos->data.second_ticks + 1u >=
            cmos->connect.ticks_per_second ? VCMOS_REG_A_UIP : 0u);
    case VCMOS_RTC_REG_C: {
        type_unsigned_8 value = cmos->connect.reg[reg];
        cmos->connect.reg[reg] = 0u;
        core_machine_pic_irq_source_deassert(&cmos->connect.irq_source);
        return value;
    }
    default: return cmos->connect.reg[reg];
    }
}

static C_VOID vm_machine_cmos_write_register(t_cmos *cmos, type_unsigned_8 reg,
    type_unsigned_8 value)
{
    switch (reg) {
    case VCMOS_RTC_SECOND: cmos->data.second = vm_machine_cmos_decode(cmos, value); break;
    case VCMOS_RTC_MINUTE: cmos->data.minute = vm_machine_cmos_decode(cmos, value); break;
    case VCMOS_RTC_HOUR: cmos->data.hour = vm_machine_cmos_hour_decode(cmos, value); break;
    case VCMOS_RTC_DAY_WEEK: cmos->data.day_week = vm_machine_cmos_decode(cmos, value); break;
    case VCMOS_RTC_DAY_MONTH: cmos->data.day_month = vm_machine_cmos_decode(cmos, value); break;
    case VCMOS_RTC_MONTH: cmos->data.month = vm_machine_cmos_decode(cmos, value); break;
    case VCMOS_RTC_YEAR: cmos->data.year = vm_machine_cmos_decode(cmos, value); break;
    case VCMOS_RTC_CENTURY: cmos->data.century = vm_machine_cmos_decode(cmos, value); break;
    case VCMOS_RTC_REG_C:
    case VCMOS_RTC_REG_D: break;
    default: cmos->connect.reg[reg] = value; break;
    }
}

static C_VOID io_write_0070(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{
    t_cmos *cmos = owner;
    (C_VOID)port_id;
    cmos->data.regId = port->data.ioByte & 0x7fu;
    cmos->connect.cpu->data.flagMaskNMI = TYPE_GET_MSB_8(port->data.ioByte) ?
        TYPE_TRUE : TYPE_FALSE;
}

static C_VOID io_write_0071(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{
    (C_VOID)port_id;
    vm_machine_cmos_write_register((t_cmos *)owner,
        ((t_cmos *)owner)->data.regId, port->data.ioByte);
}

static C_VOID io_read_0071(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{
    (C_VOID)port_id;
    port->data.ioByte = vm_machine_cmos_read_register((t_cmos *)owner,
        ((t_cmos *)owner)->data.regId);
}

C_VOID vm_machine_cmos_initialize(t_cmos *cmos, t_cpu *cpu, t_pic *pic_master,
    t_pic *pic_slave, t_port *port, const vm_machine_cmos_config *config)
{
    if (cmos == STD_NULL || cpu == STD_NULL || port == STD_NULL || config == STD_NULL) return;
    STD_MEMSET(cmos, TYPE_ZERO_8, sizeof(*cmos));
    cmos->connect.cpu = cpu;
    cmos->connect.ticks_per_second = config->ticks_per_second == 0u ? 1u :
        config->ticks_per_second;
    core_machine_pic_irq_source_bind(&cmos->connect.irq_source, pic_master,
        pic_slave, config->irq);
    core_machine_port_add_read(port, config->data_port, io_read_0071, cmos);
    core_machine_port_add_write(port, config->index_port, io_write_0070, cmos);
    core_machine_port_add_write(port, config->data_port, io_write_0071, cmos);
    vm_machine_cmos_reset(cmos);
}

C_VOID vm_machine_cmos_reset(t_cmos *cmos)
{
    if (cmos == STD_NULL) return;
    STD_MEMSET(&cmos->data, TYPE_ZERO_8, sizeof(cmos->data));
    cmos->data.day_week = 6u; cmos->data.day_month = 1u; cmos->data.month = 1u;
    cmos->data.century = 20u;
    cmos->connect.reg[VCMOS_RTC_REG_A] = 0x26u;
    cmos->connect.reg[VCMOS_RTC_REG_B] = VCMOS_REG_B_24H;
    cmos->connect.reg[VCMOS_RTC_REG_C] = 0u;
    cmos->connect.reg[VCMOS_RTC_REG_D] = VCMOS_REG_D_VRT;
    core_machine_pic_irq_source_deassert(&cmos->connect.irq_source);
}

C_VOID vm_machine_cmos_apply_defaults(t_cmos *cmos,
    const vm_machine_cmos_defaults *defaults)
{
    if (cmos == STD_NULL || defaults == STD_NULL) return;
    cmos->connect.reg[VCMOS_TYPE_DISK_FLOPPY] = defaults->floppy_type;
    cmos->connect.reg[VCMOS_TYPE_DISK_FIXED] = defaults->fixed_disk_type;
    cmos->connect.reg[VCMOS_EQUIPMENT] = defaults->equipment;
    cmos->connect.reg[VCMOS_BASEMEM_LSB] = TYPE_MASK_UNSIGNED_8(defaults->base_memory_kib);
    cmos->connect.reg[VCMOS_BASEMEM_MSB] = TYPE_MASK_UNSIGNED_8(defaults->base_memory_kib >> 8);
}

C_VOID vm_machine_cmos_advance(t_cmos *cmos, uint64_t elapsed_ticks)
{
    uint32_t periodic_hz;
    if (cmos == STD_NULL || (cmos->connect.reg[VCMOS_RTC_REG_B] & VCMOS_REG_B_SET) != 0u)
        return;
    cmos->data.second_ticks += elapsed_ticks;
    while (cmos->data.second_ticks >= cmos->connect.ticks_per_second) {
        cmos->data.second_ticks -= cmos->connect.ticks_per_second;
        vm_machine_cmos_increment_second(cmos);
        cmos->connect.reg[VCMOS_RTC_REG_C] |= VCMOS_REG_C_UF;
        if (vm_machine_cmos_alarm_matches(cmos))
            cmos->connect.reg[VCMOS_RTC_REG_C] |= VCMOS_REG_C_AF;
    }
    periodic_hz = vm_machine_cmos_periodic_hz(cmos);
    if (periodic_hz != 0u) {
        cmos->data.periodic_ticks += elapsed_ticks * periodic_hz;
        while (cmos->data.periodic_ticks >= cmos->connect.ticks_per_second) {
            cmos->data.periodic_ticks -= cmos->connect.ticks_per_second;
            cmos->connect.reg[VCMOS_RTC_REG_C] |= VCMOS_REG_C_PF;
        }
    }
    vm_machine_cmos_raise_if_enabled(cmos);
}

C_VOID vm_machine_cmos_refresh(t_cmos *cmos) { (C_VOID)cmos; }
C_VOID vm_machine_cmos_finalize(t_cmos *cmos) { (C_VOID)cmos; }

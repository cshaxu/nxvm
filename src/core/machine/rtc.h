/* Copyright 2012-2026 Neko. */

#ifndef CORE_MACHINE_RTC_H
#define CORE_MACHINE_RTC_H

#include "type.h"
#include "core/machine/machine_interface.h"
#include "core/machine/pic.h"

/* MC146818-compatible RTC/CMOS registers occupy 00h--3Fh. */
#define CORE_MACHINE_RTC_REGISTER_COUNT 0x40u

#define CORE_MACHINE_RTC_SECOND       0x00u
#define CORE_MACHINE_RTC_SECOND_ALARM 0x01u
#define CORE_MACHINE_RTC_MINUTE       0x02u
#define CORE_MACHINE_RTC_MINUTE_ALARM 0x03u
#define CORE_MACHINE_RTC_HOUR         0x04u
#define CORE_MACHINE_RTC_HOUR_ALARM   0x05u
#define CORE_MACHINE_RTC_DAY_WEEK     0x06u
#define CORE_MACHINE_RTC_DAY_MONTH    0x07u
#define CORE_MACHINE_RTC_MONTH        0x08u
#define CORE_MACHINE_RTC_YEAR         0x09u
#define CORE_MACHINE_RTC_REG_A        0x0au
#define CORE_MACHINE_RTC_REG_B        0x0bu
#define CORE_MACHINE_RTC_REG_C        0x0cu
#define CORE_MACHINE_RTC_REG_D        0x0du

#define CORE_MACHINE_RTC_REG_A_UIP 0x80u
#define CORE_MACHINE_RTC_REG_B_SET 0x80u
#define CORE_MACHINE_RTC_REG_B_PIE 0x40u
#define CORE_MACHINE_RTC_REG_B_AIE 0x20u
#define CORE_MACHINE_RTC_REG_B_UIE 0x10u
#define CORE_MACHINE_RTC_REG_B_SQWE 0x08u
#define CORE_MACHINE_RTC_REG_B_DM  0x04u
#define CORE_MACHINE_RTC_REG_B_24H 0x02u
#define CORE_MACHINE_RTC_REG_C_IRQF 0x80u
#define CORE_MACHINE_RTC_REG_C_PF 0x40u
#define CORE_MACHINE_RTC_REG_C_AF 0x20u
#define CORE_MACHINE_RTC_REG_C_UF 0x10u
#define CORE_MACHINE_RTC_REG_D_VRT 0x80u

typedef struct core_machine_rtc_config {
    type_unsigned_8 irq;
    type_unsigned_32 ticks_per_second;
    core_machine_rtc_timing_plan timing;
} core_machine_rtc_config;

typedef struct core_machine_rtc_calendar {
    type_unsigned_8 second;
    type_unsigned_8 minute;
    type_unsigned_8 hour;
    type_unsigned_8 day_week;
    type_unsigned_8 day_month;
    type_unsigned_8 month;
    type_unsigned_8 year;
    type_unsigned_64 second_ticks;
    type_unsigned_64 periodic_ticks;
} core_machine_rtc_calendar;

typedef struct core_machine_rtc {
    type_unsigned_8 registers[CORE_MACHINE_RTC_REGISTER_COUNT];
    type_unsigned_8 selected_register;
    core_machine_rtc_calendar calendar;
    core_machine_pic_irq_source irq_source;
    type_unsigned_32 ticks_per_second;
    type_unsigned_32 uip_lead_ticks;
    type_unsigned_32 update_ticks;
    core_machine_rtc_timing_provenance timing_provenance;
    type_bool square_wave;
} core_machine_rtc;

C_VOID core_machine_rtc_initialize(core_machine_rtc *rtc, t_pic *pic_master,
    t_pic *pic_slave, const core_machine_rtc_config *config);
C_VOID core_machine_rtc_reset(core_machine_rtc *rtc);
C_VOID core_machine_rtc_finalize(core_machine_rtc *rtc);
C_VOID core_machine_rtc_advance(core_machine_rtc *rtc, type_unsigned_64 elapsed_ticks);
C_VOID core_machine_rtc_select_register(core_machine_rtc *rtc, type_unsigned_8 index);
type_unsigned_8 core_machine_rtc_read_selected(core_machine_rtc *rtc);
C_VOID core_machine_rtc_write_selected(core_machine_rtc *rtc, type_unsigned_8 value);
C_VOID core_machine_rtc_write_nvram(core_machine_rtc *rtc, type_unsigned_8 index,
    type_unsigned_8 value);
type_bool core_machine_rtc_get_square_wave(const core_machine_rtc *rtc);
type_status core_machine_rtc_ticks_until_irq(const core_machine_rtc *rtc,
    type_unsigned_64 *out_ticks);

#endif

/* Copyright 2012-2026 Neko. */

#ifndef CORE_MACHINE_RTC_H
#define CORE_MACHINE_RTC_H
#include "lib/types/types_interface.h"

#include "type.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/pic.h"

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
    lib_u8 irq;
    lib_u32 ticks_per_second;
    core_machine_rtc_timing_plan timing;
} core_machine_rtc_config;

typedef struct core_machine_rtc_calendar {
    lib_u8 second;
    lib_u8 minute;
    lib_u8 hour;
    lib_u8 day_week;
    lib_u8 day_month;
    lib_u8 month;
    lib_u8 year;
    lib_u64 second_ticks;
    lib_u64 periodic_ticks;
} core_machine_rtc_calendar;

typedef struct core_machine_rtc {
    lib_u8 registers[CORE_MACHINE_RTC_REGISTER_COUNT];
    lib_u8 selected_register;
    core_machine_rtc_calendar calendar;
    core_machine_pic_irq_source irq_source;
    lib_u32 ticks_per_second;
    lib_u32 uip_lead_ticks;
    lib_u32 update_ticks;
    core_machine_rtc_timing_provenance timing_provenance;
    type_bool square_wave;
} core_machine_rtc;

C_VOID core_machine_rtc_initialize(core_machine_rtc *rtc, t_pic *pic_master,
    t_pic *pic_slave, const core_machine_rtc_config *config);
C_VOID core_machine_rtc_reset(core_machine_rtc *rtc);
C_VOID core_machine_rtc_finalize(core_machine_rtc *rtc);
C_VOID core_machine_rtc_advance(core_machine_rtc *rtc, lib_u64 elapsed_ticks);
C_VOID core_machine_rtc_select_register(core_machine_rtc *rtc, lib_u8 index);
lib_u8 core_machine_rtc_read_selected(core_machine_rtc *rtc);
C_VOID core_machine_rtc_write_selected(core_machine_rtc *rtc, lib_u8 value);
C_VOID core_machine_rtc_write_nvram(core_machine_rtc *rtc, lib_u8 index,
    lib_u8 value);
type_bool core_machine_rtc_get_square_wave(const core_machine_rtc *rtc);
type_status core_machine_rtc_ticks_until_irq(const core_machine_rtc *rtc,
    lib_u64 *out_ticks);

#endif

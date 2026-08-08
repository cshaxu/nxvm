/* Copyright 2012-2026 Neko. */

#ifndef CORE_MACHINE_RTC_H
#define CORE_MACHINE_RTC_H

#include "type.h"
#include "core/machine/machine_interface.h"
#include "core/machine/pic.h"

#define CORE_MACHINE_RTC_REGISTER_COUNT 0x80u

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
#define CORE_MACHINE_RTC_CENTURY      0x32u

#define CORE_MACHINE_RTC_REG_A_UIP 0x80u
#define CORE_MACHINE_RTC_REG_B_SET 0x80u
#define CORE_MACHINE_RTC_REG_B_PIE 0x40u
#define CORE_MACHINE_RTC_REG_B_AIE 0x20u
#define CORE_MACHINE_RTC_REG_B_UIE 0x10u
#define CORE_MACHINE_RTC_REG_B_DM  0x04u
#define CORE_MACHINE_RTC_REG_B_24H 0x02u
#define CORE_MACHINE_RTC_REG_C_IRQF 0x80u
#define CORE_MACHINE_RTC_REG_C_PF 0x40u
#define CORE_MACHINE_RTC_REG_C_AF 0x20u
#define CORE_MACHINE_RTC_REG_C_UF 0x10u
#define CORE_MACHINE_RTC_REG_D_VRT 0x80u

typedef struct core_machine_rtc_config {
    uint8_t irq;
    uint32_t ticks_per_second;
} core_machine_rtc_config;

typedef struct core_machine_rtc_calendar {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day_week;
    uint8_t day_month;
    uint8_t month;
    uint8_t year;
    uint8_t century;
    uint64_t second_ticks;
    uint64_t periodic_ticks;
} core_machine_rtc_calendar;

typedef struct core_machine_rtc {
    uint8_t registers[CORE_MACHINE_RTC_REGISTER_COUNT];
    uint8_t selected_register;
    core_machine_rtc_calendar calendar;
    core_machine_pic_irq_source irq_source;
    uint32_t ticks_per_second;
} core_machine_rtc;

C_VOID core_machine_rtc_initialize(core_machine_rtc *rtc, t_pic *pic_master,
    t_pic *pic_slave, const core_machine_rtc_config *config);
C_VOID core_machine_rtc_reset(core_machine_rtc *rtc);
C_VOID core_machine_rtc_finalize(core_machine_rtc *rtc);
C_VOID core_machine_rtc_advance(core_machine_rtc *rtc, uint64_t elapsed_ticks);
C_VOID core_machine_rtc_select_register(core_machine_rtc *rtc, uint8_t index);
uint8_t core_machine_rtc_read_selected(core_machine_rtc *rtc);
C_VOID core_machine_rtc_write_selected(core_machine_rtc *rtc, uint8_t value);
C_VOID core_machine_rtc_write_nvram(core_machine_rtc *rtc, uint8_t index,
    uint8_t value);

#endif

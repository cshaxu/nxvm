/* Copyright 2012-2026 Neko. */
#ifndef X86_RTC146818_INTERFACE_H
#define X86_RTC146818_INTERFACE_H
#include "lib/types/types_interface.h"

/* MC146818-compatible RTC/CMOS registers occupy 00h--3Fh. */
#define X86_RTC_REGISTER_COUNT 0x40u

#define X86_RTC_SECOND       0x00u
#define X86_RTC_SECOND_ALARM 0x01u
#define X86_RTC_MINUTE       0x02u
#define X86_RTC_MINUTE_ALARM 0x03u
#define X86_RTC_HOUR         0x04u
#define X86_RTC_HOUR_ALARM   0x05u
#define X86_RTC_DAY_WEEK     0x06u
#define X86_RTC_DAY_MONTH    0x07u
#define X86_RTC_MONTH        0x08u
#define X86_RTC_YEAR         0x09u
#define X86_RTC_REG_A        0x0au
#define X86_RTC_REG_B        0x0bu
#define X86_RTC_REG_C        0x0cu
#define X86_RTC_REG_D        0x0du

#define X86_RTC_REG_A_UIP 0x80u
#define X86_RTC_REG_B_SET 0x80u
#define X86_RTC_REG_B_PIE 0x40u
#define X86_RTC_REG_B_AIE 0x20u
#define X86_RTC_REG_B_UIE 0x10u
#define X86_RTC_REG_B_SQWE 0x08u
#define X86_RTC_REG_B_DM  0x04u
#define X86_RTC_REG_B_24H 0x02u
#define X86_RTC_REG_C_IRQF 0x80u
#define X86_RTC_REG_C_PF 0x40u
#define X86_RTC_REG_C_AF 0x20u
#define X86_RTC_REG_C_UF 0x10u
#define X86_RTC_REG_D_VRT 0x80u

typedef struct x86_rtc x86_rtc;
typedef void (*x86_rtc_output_provider)(void *context, lib_bool asserted);
typedef struct x86_rtc_config {
    lib_u32 ticks_per_second;
    lib_u32 uip_lead_ticks;
    lib_u32 update_ticks;
} x86_rtc_config;

/* One execution owner, no concurrent calls. Config is copied; the output
 * context is borrowed until destroy. Output callbacks deliver logical IRQ
 * levels and must not recursively execute or destroy this RTC.
 * Durations use the configured ticks/second, never host time. Zero phase
 * inputs retain the existing 244/1984 microsecond conversion, rounded down
 * with a one-tick minimum; the caller owns its precision/provenance claim. */
lib_status x86_rtc_create(const x86_rtc_config *config,
    x86_rtc_output_provider output, void *context, x86_rtc **out_rtc);
void x86_rtc_destroy(x86_rtc *rtc);
/* RESET retains calendar, phase and RAM, clears enabled IRQ delivery. */
void x86_rtc_reset(x86_rtc *rtc);
/* Direct chip register selectors only; board index/NMI latches are external.
 * Reading C acknowledges flags/IRQ. Invalid reads return zero; invalid writes
 * are ignored. Guest-supplied malformed calendar data is not L3-qualified. */
lib_u8 x86_rtc_read_register(x86_rtc *rtc, lib_u8 index);
void x86_rtc_write_register(x86_rtc *rtc, lib_u8 index, lib_u8 value);
void x86_rtc_advance(x86_rtc *rtc, lib_u64 elapsed_ticks);
lib_bool x86_rtc_get_square_wave(const x86_rtc *rtc);
/* Observation only: OK returns a positive distance; INVALID_STATE means no
 * scheduled IRQ, including a stopped divider. Bad arguments are separate. */
lib_status x86_rtc_ticks_until_irq(const x86_rtc *rtc, lib_u64 *out_ticks);
#endif

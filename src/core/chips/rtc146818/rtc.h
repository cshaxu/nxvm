/* Copyright 2012-2026 Neko. */
#ifndef X86_RTC_H
#define X86_RTC_H
#include "core/chips/rtc146818/rtc146818_interface.h"

typedef struct x86_rtc_calendar {
    lib_u8 second;
    lib_u8 minute;
    lib_u8 hour;
    lib_u8 day_week;
    lib_u8 day_month;
    lib_u8 month;
    lib_u8 year;
    lib_u64 second_ticks;
    lib_u64 periodic_ticks;
} x86_rtc_calendar;

struct x86_rtc {
    lib_u8 registers[X86_RTC_REGISTER_COUNT];
    x86_rtc_calendar calendar;
    x86_rtc_output_provider output;
    void *output_context;
    lib_u32 ticks_per_second;
    lib_u32 uip_lead_ticks;
    lib_u32 update_ticks;
    lib_bool square_wave;
};

#endif

/* This file is a part of NXVM project. */

// CMOS; Real Time Clock: DS1302

#ifndef NXVM_VCMOS_H
#define NXVM_VCMOS_H

#include "vglobal.h"

#define VCMOS_DEBUG

#define CMOS_RTC_SECOND         0x00
#define CMOS_RTC_SECOND_ALARM   0x01
#define CMOS_RTC_MINUTE         0x02
#define CMOS_RTC_MINUTE_ALARM   0x03
#define CMOS_RTC_HOUR           0x04
#define CMOS_RTC_HOUR_ALARM     0x05
#define CMOS_RTC_DAY_WEEK       0x06
#define CMOS_RTC_DAY_MONTH      0x07
#define CMOS_RTC_MONTH          0x08
#define CMOS_RTC_YEAR           0x09
#define CMOS_RTC_REG_A          0x0a
#define CMOS_RTC_REG_B          0x0b
#define CMOS_RTC_REG_C          0x0c
#define CMOS_RTC_REG_D          0x0d
#define CMOS_STATUS_DIAG        0x0e
#define CMOS_STATUS_SHUTDOWN    0x0f
#define CMOS_TYPE_DISK_FLOPPY   0x10
#define CMOS_TYPE_DISK_FIXED    0x12
#define CMOS_EQUIPMENT          0x14
#define CMOS_BASEMEM_LSB        0x15
#define CMOS_BASEMEM_MSB        0x16
#define CMOS_EXTMEM_LSB         0x17
#define CMOS_EXTMEM_MSB         0x18
#define CMOS_DRIVE_C            0x19
#define CMOS_DRIVE_D            0x1a
#define CMOS_CHECKSUM_MSB       0x2e
#define CMOS_CHECKSUM_LSB       0x2f
#define CMOS_EXTMEM1MB_LSB      0x30
#define CMOS_EXTMEM1MB_MSB      0x31
#define CMOS_RTC_CENTURY        0x32
#define CMOS_FLAGS_INFO         0x33

typedef struct {
	t_nubit8 reg[0x80];                                    /* cmos registers */
	t_nubit8 rid;                            /* id of specified cmos register*/
} t_cmos;

extern t_cmos vcmos;

void CMOSInit();
void CMOSTerm();

#endif
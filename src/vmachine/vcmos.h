/* This file is a part of NXVM project. */

/* CMOS and Real Time Clock: DS1302 */

#ifndef NXVM_VCMOS_H
#define NXVM_VCMOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_CMOS "DS1302"

#define VCMOS_RTC_SECOND         0x00
#define VCMOS_RTC_SECOND_ALARM   0x01
#define VCMOS_RTC_MINUTE         0x02
#define VCMOS_RTC_MINUTE_ALARM   0x03
#define VCMOS_RTC_HOUR           0x04
#define VCMOS_RTC_HOUR_ALARM     0x05
#define VCMOS_RTC_DAY_WEEK       0x06
#define VCMOS_RTC_DAY_MONTH      0x07
#define VCMOS_RTC_MONTH          0x08
#define VCMOS_RTC_YEAR           0x09
#define VCMOS_RTC_REG_A          0x0a
#define VCMOS_RTC_REG_B          0x0b
#define VCMOS_RTC_REG_C          0x0c
#define VCMOS_RTC_REG_D          0x0d
#define VCMOS_STATUS_DIAG        0x0e
#define VCMOS_STATUS_SHUTDOWN    0x0f
#define VCMOS_TYPE_DISK_FLOPPY   0x10
#define VCMOS_TYPE_DISK_FIXED    0x12
#define VCMOS_EQUIPMENT          0x14
#define VCMOS_BASEMEM_LSB        0x15
#define VCMOS_BASEMEM_MSB        0x16
#define VCMOS_EXTMEM_LSB         0x17
#define VCMOS_EXTMEM_MSB         0x18
#define VCMOS_DRIVE_C            0x19
#define VCMOS_DRIVE_D            0x1a
#define VCMOS_CHECKSUM_MSB       0x2e
#define VCMOS_CHECKSUM_LSB       0x2f
#define VCMOS_EXTMEM1MB_LSB      0x30
#define VCMOS_EXTMEM1MB_MSB      0x31
#define VCMOS_RTC_CENTURY        0x32
#define VCMOS_FLAGS_INFO         0x33

typedef struct {
	t_nubit8 reg[0x80];                                    /* cmos registers */
	t_nubit8 rid;                            /* id of specified cmos register*/
} t_cmos;

extern t_cmos vcmos;

void vcmosInit();
void vcmosReset();
void vcmosRefresh();
void vcmosFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif

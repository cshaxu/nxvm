/* Copyright 2012-2014 Neko. */

/* VCMOS implements CMOS and Real Time Clock DS1302. */

#include "type.h"

#include "core/product/utils.h"


#include "core/machine/cpu.h"


#include "core/machine/port.h"

#include "vm/machine/cmos.h"

static C_VOID io_write_0070(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    t_cmos *cmos = (t_cmos *)owner;
    (C_VOID)port_id;
    cmos->data.regId = port->data.ioByte; /* select reg id */
    if (NTVDM64_TYPE_GET_MSB_8(cmos->data.regId)) {
        /* if MSB=1, disable NMI */
        cmos->connect.cpu->data.flagMaskNMI = NTVDM64_TYPE_TRUE;
    } else {
        cmos->connect.cpu->data.flagMaskNMI = NTVDM64_TYPE_FALSE;
    }
}
static C_VOID io_write_0071(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    t_cmos *cmos = (t_cmos *)owner;
    ntvdm64_type_native_unsigned i;
    ntvdm64_type_unsigned_16 checksum = NTVDM64_TYPE_ZERO_16;
    (C_VOID)port_id;
    cmos->connect.reg[cmos->data.regId] = port->data.ioByte;
    if ((cmos->data.regId >= VCMOS_TYPE_DISK_FLOPPY) && (cmos->data.regId < VCMOS_CHECKSUM_MSB)) {
        for (i = VCMOS_TYPE_DISK_FLOPPY; i < VCMOS_CHECKSUM_MSB; ++i) {
            checksum += cmos->connect.reg[i];
        }
    }
    cmos->connect.reg[VCMOS_CHECKSUM_LSB] = NTVDM64_TYPE_MASK_UNSIGNED_8(checksum);
    cmos->connect.reg[VCMOS_CHECKSUM_MSB] = NTVDM64_TYPE_MASK_UNSIGNED_8(checksum >> 8);
}
static C_VOID io_read_0071(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    t_cmos *cmos = (t_cmos *)owner;
    (C_VOID)port_id;
    port->data.ioByte = cmos->connect.reg[cmos->data.regId];
}

C_VOID vm_machine_cmos_initialize(t_cmos *cmos, t_cpu *cpu, t_port *port) {
    STD_MEMSET((C_VOID *)cmos, NTVDM64_TYPE_ZERO_8, sizeof(*cmos));
    cmos->connect.cpu = cpu;
    core_machine_port_add_read(port, 0x0071, io_read_0071, cmos);
    core_machine_port_add_write(port, 0x0070, io_write_0070, cmos);
    core_machine_port_add_write(port, 0x0071, io_write_0071, cmos);
}
C_VOID vm_machine_cmos_reset(t_cmos *cmos) {
    STD_MEMSET((C_VOID *)(&cmos->data), NTVDM64_TYPE_ZERO_8, sizeof(cmos->data));
}
C_VOID vm_machine_cmos_refresh(t_cmos *cmos) {
    time_t tCurr;
    struct tm *ptm;
    ntvdm64_type_unsigned_8 century, year, month, mday, wday, hour, min, sec;

    tCurr = STD_TIME(NULL);
    if (tCurr == cmos->connect.last_refresh) {
        return;
    } else {
        cmos->connect.last_refresh = tCurr;
    }
    ptm = STD_LOCALTIME(&tCurr);

    century = NTVDM64_TYPE_MASK_UNSIGNED_8(19 + ptm->tm_year / 100);
    year    = NTVDM64_TYPE_MASK_UNSIGNED_8(ptm->tm_year % 100);
    month   = NTVDM64_TYPE_MASK_UNSIGNED_8(ptm->tm_mon + 1);
    mday    = NTVDM64_TYPE_MASK_UNSIGNED_8(ptm->tm_mday);
    wday    = NTVDM64_TYPE_MASK_UNSIGNED_8(ptm->tm_wday + 1);
    hour    = NTVDM64_TYPE_MASK_UNSIGNED_8(ptm->tm_hour);
    min     = NTVDM64_TYPE_MASK_UNSIGNED_8(ptm->tm_min);
    sec     = NTVDM64_TYPE_MASK_UNSIGNED_8(ptm->tm_sec);

    cmos->connect.reg[VCMOS_RTC_SECOND]    = NTVDM64_TYPE_HEX_TO_BCD(sec);
    cmos->connect.reg[VCMOS_RTC_MINUTE]    = NTVDM64_TYPE_HEX_TO_BCD(min);
    cmos->connect.reg[VCMOS_RTC_HOUR]      = NTVDM64_TYPE_HEX_TO_BCD(hour);
    cmos->connect.reg[VCMOS_RTC_DAY_WEEK]  = NTVDM64_TYPE_HEX_TO_BCD(wday);
    cmos->connect.reg[VCMOS_RTC_DAY_MONTH] = NTVDM64_TYPE_HEX_TO_BCD(mday);
    cmos->connect.reg[VCMOS_RTC_MONTH]     = NTVDM64_TYPE_HEX_TO_BCD(month);
    cmos->connect.reg[VCMOS_RTC_YEAR]      = NTVDM64_TYPE_HEX_TO_BCD(year);
    cmos->connect.reg[VCMOS_RTC_CENTURY]   = NTVDM64_TYPE_HEX_TO_BCD(century);
}
C_VOID vm_machine_cmos_finalize(t_cmos *cmos) { (C_VOID)cmos; }

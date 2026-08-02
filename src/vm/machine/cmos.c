/* Copyright 2012-2014 Neko. */

/* VCMOS implements CMOS and Real Time Clock DS1302. */

#include "core/product/utils.h"

#include "core/machine/cpu.h"

#include "core/machine/port.h"
#include "vm/machine/cmos.h"

static void io_write_0070(t_port *port, t_nubit16 port_id, void *owner) {
    t_cmos *cmos = (t_cmos *)owner;
    (void)port_id;
    cmos->data.regId = port->data.ioByte; /* select reg id */
    if (GetMSB8(cmos->data.regId)) {
        /* if MSB=1, disable NMI */
        cmos->connect.cpu->data.flagMaskNMI = True;
    } else {
        cmos->connect.cpu->data.flagMaskNMI = False;
    }
}
static void io_write_0071(t_port *port, t_nubit16 port_id, void *owner) {
    t_cmos *cmos = (t_cmos *)owner;
    t_nubitcc i;
    t_nubit16 checksum = Zero16;
    (void)port_id;
    cmos->connect.reg[cmos->data.regId] = port->data.ioByte;
    if ((cmos->data.regId >= VCMOS_TYPE_DISK_FLOPPY) && (cmos->data.regId < VCMOS_CHECKSUM_MSB)) {
        for (i = VCMOS_TYPE_DISK_FLOPPY; i < VCMOS_CHECKSUM_MSB; ++i) {
            checksum += cmos->connect.reg[i];
        }
    }
    cmos->connect.reg[VCMOS_CHECKSUM_LSB] = GetMax8(checksum);
    cmos->connect.reg[VCMOS_CHECKSUM_MSB] = GetMax8(checksum >> 8);
}
static void io_read_0071(t_port *port, t_nubit16 port_id, void *owner) {
    t_cmos *cmos = (t_cmos *)owner;
    (void)port_id;
    port->data.ioByte = cmos->connect.reg[cmos->data.regId];
}

void vm_machine_cmos_initialize(t_cmos *cmos, t_cpu *cpu, t_port *port) {
    MEMSET((void *)cmos, Zero8, sizeof(*cmos));
    cmos->connect.cpu = cpu;
    core_machine_port_add_read(port, 0x0071, io_read_0071, cmos);
    core_machine_port_add_write(port, 0x0070, io_write_0070, cmos);
    core_machine_port_add_write(port, 0x0071, io_write_0071, cmos);
}
void vm_machine_cmos_reset(t_cmos *cmos) {
    MEMSET((void *)(&cmos->data), Zero8, sizeof(cmos->data));
}
void vm_machine_cmos_refresh(t_cmos *cmos) {
    time_t tCurr;
    struct tm *ptm;
    t_nubit8 century, year, month, mday, wday, hour, min, sec;

    tCurr = time(NULL);
    if (tCurr == cmos->connect.last_refresh) {
        return;
    } else {
        cmos->connect.last_refresh = tCurr;
    }
    ptm = LOCALTIME(&tCurr);

    century = GetMax8(19 + ptm->tm_year / 100);
    year    = GetMax8(ptm->tm_year % 100);
    month   = GetMax8(ptm->tm_mon + 1);
    mday    = GetMax8(ptm->tm_mday);
    wday    = GetMax8(ptm->tm_wday + 1);
    hour    = GetMax8(ptm->tm_hour);
    min     = GetMax8(ptm->tm_min);
    sec     = GetMax8(ptm->tm_sec);

    cmos->connect.reg[VCMOS_RTC_SECOND]    = Hex2BCD(sec);
    cmos->connect.reg[VCMOS_RTC_MINUTE]    = Hex2BCD(min);
    cmos->connect.reg[VCMOS_RTC_HOUR]      = Hex2BCD(hour);
    cmos->connect.reg[VCMOS_RTC_DAY_WEEK]  = Hex2BCD(wday);
    cmos->connect.reg[VCMOS_RTC_DAY_MONTH] = Hex2BCD(mday);
    cmos->connect.reg[VCMOS_RTC_MONTH]     = Hex2BCD(month);
    cmos->connect.reg[VCMOS_RTC_YEAR]      = Hex2BCD(year);
    cmos->connect.reg[VCMOS_RTC_CENTURY]   = Hex2BCD(century);
}
void vm_machine_cmos_finalize(t_cmos *cmos) { (void)cmos; }

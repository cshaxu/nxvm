/* Copyright 2012-2014 Neko. */

/* VCMOS implements CMOS and Real Time Clock DS1302. */

#include "type.h"

#include "core/product/utils.h"


#include "core/machine/cpu.h"


#include "core/machine/port.h"

#include "vm/machine/cmos.h"

static C_VOID io_write_0070(t_port *port, type_unsigned_16 port_id, C_VOID *owner) {
    t_cmos *cmos = (t_cmos *)owner;
    (C_VOID)port_id;
    cmos->data.regId = port->data.ioByte; /* select reg id */
    if (TYPE_GET_MSB_8(cmos->data.regId)) {
        /* if MSB=1, disable NMI */
        cmos->connect.cpu->data.flagMaskNMI = TYPE_TRUE;
    } else {
        cmos->connect.cpu->data.flagMaskNMI = TYPE_FALSE;
    }
}
static C_VOID io_write_0071(t_port *port, type_unsigned_16 port_id, C_VOID *owner) {
    t_cmos *cmos = (t_cmos *)owner;
    type_native_unsigned i;
    type_unsigned_16 checksum = TYPE_ZERO_16;
    (C_VOID)port_id;
    cmos->connect.reg[cmos->data.regId] = port->data.ioByte;
    if ((cmos->data.regId >= VCMOS_TYPE_DISK_FLOPPY) && (cmos->data.regId < VCMOS_CHECKSUM_MSB)) {
        for (i = VCMOS_TYPE_DISK_FLOPPY; i < VCMOS_CHECKSUM_MSB; ++i) {
            checksum += cmos->connect.reg[i];
        }
    }
    cmos->connect.reg[VCMOS_CHECKSUM_LSB] = TYPE_MASK_UNSIGNED_8(checksum);
    cmos->connect.reg[VCMOS_CHECKSUM_MSB] = TYPE_MASK_UNSIGNED_8(checksum >> 8);
}
static C_VOID io_read_0071(t_port *port, type_unsigned_16 port_id, C_VOID *owner) {
    t_cmos *cmos = (t_cmos *)owner;
    (C_VOID)port_id;
    port->data.ioByte = cmos->connect.reg[cmos->data.regId];
}

C_VOID vm_machine_cmos_initialize(t_cmos *cmos, t_cpu *cpu, t_port *port,
    const vm_machine_cmos_config *config) {
    if (cmos == STD_NULL || cpu == STD_NULL || port == STD_NULL ||
        config == STD_NULL) return;
    STD_MEMSET((C_VOID *)cmos, TYPE_ZERO_8, sizeof(*cmos));
    cmos->connect.cpu = cpu;
    core_machine_port_add_read(port, config->data_port, io_read_0071, cmos);
    core_machine_port_add_write(port, config->index_port, io_write_0070, cmos);
    core_machine_port_add_write(port, config->data_port, io_write_0071, cmos);
}
C_VOID vm_machine_cmos_reset(t_cmos *cmos) {
    STD_MEMSET((C_VOID *)(&cmos->data), TYPE_ZERO_8, sizeof(cmos->data));
}
C_VOID vm_machine_cmos_apply_defaults(t_cmos *cmos,
    const vm_machine_cmos_defaults *defaults)
{
    if (cmos == STD_NULL || defaults == STD_NULL) return;
    cmos->connect.reg[VCMOS_TYPE_DISK_FLOPPY] = defaults->floppy_type;
    cmos->connect.reg[VCMOS_TYPE_DISK_FIXED] = defaults->fixed_disk_type;
    cmos->connect.reg[VCMOS_EQUIPMENT] = defaults->equipment;
    cmos->connect.reg[VCMOS_BASEMEM_LSB] =
        TYPE_MASK_UNSIGNED_8(defaults->base_memory_kib);
    cmos->connect.reg[VCMOS_BASEMEM_MSB] =
        TYPE_MASK_UNSIGNED_8(defaults->base_memory_kib >> 8);
}
C_VOID vm_machine_cmos_refresh(t_cmos *cmos) {
    STD_TIME_T tCurr;
    struct tm *ptm;
    type_unsigned_8 century, year, month, mday, wday, hour, min, sec;

    tCurr = STD_TIME(STD_NULL);
    if (tCurr == cmos->connect.last_refresh) {
        return;
    } else {
        cmos->connect.last_refresh = tCurr;
    }
    ptm = STD_LOCALTIME(&tCurr);

    century = TYPE_MASK_UNSIGNED_8(19 + ptm->tm_year / 100);
    year    = TYPE_MASK_UNSIGNED_8(ptm->tm_year % 100);
    month   = TYPE_MASK_UNSIGNED_8(ptm->tm_mon + 1);
    mday    = TYPE_MASK_UNSIGNED_8(ptm->tm_mday);
    wday    = TYPE_MASK_UNSIGNED_8(ptm->tm_wday + 1);
    hour    = TYPE_MASK_UNSIGNED_8(ptm->tm_hour);
    min     = TYPE_MASK_UNSIGNED_8(ptm->tm_min);
    sec     = TYPE_MASK_UNSIGNED_8(ptm->tm_sec);

    cmos->connect.reg[VCMOS_RTC_SECOND]    = TYPE_HEX_TO_BCD(sec);
    cmos->connect.reg[VCMOS_RTC_MINUTE]    = TYPE_HEX_TO_BCD(min);
    cmos->connect.reg[VCMOS_RTC_HOUR]      = TYPE_HEX_TO_BCD(hour);
    cmos->connect.reg[VCMOS_RTC_DAY_WEEK]  = TYPE_HEX_TO_BCD(wday);
    cmos->connect.reg[VCMOS_RTC_DAY_MONTH] = TYPE_HEX_TO_BCD(mday);
    cmos->connect.reg[VCMOS_RTC_MONTH]     = TYPE_HEX_TO_BCD(month);
    cmos->connect.reg[VCMOS_RTC_YEAR]      = TYPE_HEX_TO_BCD(year);
    cmos->connect.reg[VCMOS_RTC_CENTURY]   = TYPE_HEX_TO_BCD(century);
}
C_VOID vm_machine_cmos_finalize(t_cmos *cmos) { (C_VOID)cmos; }

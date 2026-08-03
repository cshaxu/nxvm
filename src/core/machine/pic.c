/* Copyright 2012-2014 Neko. */

/*
 * VPIC implements programmable interrupt controller with
 * two Intel 8259A chips, one master and one slave.
 */

#include "type.h"

#include "core/machine/pit.h"
#include "core/machine/port.h"
#include "core/machine/pic.h"

/*
 * GetRegTopId: Internal function
 * Returns id of highest priority interrupt
 * Returns 0x08 if reg is null
 */
static ntvdm64_type_unsigned_8 GetRegTopId(t_pic *rpic, ntvdm64_type_unsigned_8 reg) {
    ntvdm64_type_unsigned_8 id = 0;
    if (reg == NTVDM64_TYPE_ZERO_8) {
        return 0x08;
    }
    reg = (reg << (VPIC_MAX_IRQ_COUNT - (rpic->data.irx))) | (reg>> (rpic->data.irx));
    while ((id < VPIC_MAX_IRQ_COUNT) && !NTVDM64_TYPE_MASK_UNSIGNED_1(reg >> id)) {
        id++;
    }
    return (id + rpic->data.irx) % VPIC_MAX_IRQ_COUNT;
}
/*
 * GetRegTopId: Internal function
 * Returns flag of higher priority interrupt
 */
static ntvdm64_type_bool HasINTR(t_pic *rpic) {
    ntvdm64_type_unsigned_8 reqId; /* top requested int id in master pic */
    ntvdm64_type_unsigned_8 svcId; /* top in service int id in master pic */
    reqId = VPIC_GetIntrTopId(rpic);
    svcId = VPIC_GetIsrTopId(rpic);
    if (reqId == 0x08) {
        /* no interrupt to pick up */
        return NTVDM64_TYPE_FALSE;
    }
    if (svcId == 0x08) {
        /* no interrupt in service */
        return NTVDM64_TYPE_TRUE;
    }
    /*
     * if irid and isid are on the same side of top priority int
     * request id, do regular comparison; otherwise order them.
     * for example, if irid < irx, that means irid's priority is lower
     * than irx, and we need to add VPIC_MAX_IRQ_COUNT to it to ensure
     * that it's priority is lower than irx in following comparison
     * (irid < isid) or (irid <= isid).
     */
    if (reqId < rpic->data.irx) {
        reqId += VPIC_MAX_IRQ_COUNT;
    }
    if (svcId < rpic->data.irx) {
        svcId += VPIC_MAX_IRQ_COUNT;
    }
    if (NTVDM64_TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_SFNM)) {
        return reqId <= svcId;
    } else {
        return reqId < svcId;
    }
}
/*
 * RespondINTR: Internal function
 * Adds INTR to IRR and removes it from ISR
 */
static void RespondINTR(t_pic *rpic, ntvdm64_type_unsigned_8 id) {
    NTVDM64_TYPE_SET_BIT(rpic->data.isr, VPIC_ISR_IRQ(id)); /* put int into ISR */
    NTVDM64_TYPE_CLEAR_BIT(rpic->data.irr, VPIC_IRR_IRQ(id)); /* remove int from  IRR */
    if (NTVDM64_TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_AEOI)) {
        /* Auto EOI Mode */
        NTVDM64_TYPE_CLEAR_BIT(rpic->data.isr, VPIC_ISR_IRQ(id));
        if (NTVDM64_TYPE_GET_BIT(rpic->data.ocw2, VPIC_OCW2_R)) {
            /* Rotate Mode */
            rpic->data.irx = (id + 1) % VPIC_MAX_IRQ_COUNT;
        }
    }
}

/*
 * io_read_00x0
 * PIC provide POLL, IRR, ISR based on OCW3
 * Reference: 16-32.PDF, Page 192
 * Reference: PC.PDF, Page 950
 */
static void io_read_00x0(t_pic *rpic, t_port *port) {
    if (NTVDM64_TYPE_GET_BIT(rpic->data.ocw3, VPIC_OCW3_P)) {
        /* P=1 (Poll Command) */
        if (VPIC_GetIntrTopId(rpic) == 0x08) {
            /* set all bits to 0 if there's no interrupt in queue */
            port->data.ioByte = NTVDM64_TYPE_ZERO_8;
        } else {
            /* set highest bit to 1 if there's an interrupt in queue */
            port->data.ioByte = VPIC_POLL_I | VPIC_GetIntrTopId(rpic);
        }
    } else {
        switch (rpic->data.ocw3 & (VPIC_OCW3_RR | VPIC_OCW3_RIS)) {
        case 0x02:
            /* RR=1, RIS=0, Read IRR */
            port->data.ioByte = rpic->data.irr;
            break;
        case 0x03:
            /* RR=1, RIS=1, Read ISR */
            port->data.ioByte = rpic->data.isr;
            break;
        default:
            /* RR=0, No Operation */
            break;
        }
    }
}
/*
 * io_write_00x0
 * PIC get ICW1, OCW2, OCW3
 * Reference: 16-32.PDF, Page 184
 * Reference: PC.PDF, Page 950
 */
static void io_write_00x0(t_pic *rpic, t_port *port) {
    ntvdm64_type_unsigned_8 id;
    if (NTVDM64_TYPE_GET_BIT(port->data.ioByte, VPIC_ICW1_I)) {
        /* ICW1 (D4=1) */
        rpic->data.icw1 = port->data.ioByte;
        rpic->data.status = ICW2;
        if (NTVDM64_TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_IC4)) {
            /* D0=1, IC4=1 */
        } else {
            /* D0=0, IC4=0 */
            rpic->data.icw4 = NTVDM64_TYPE_ZERO_8;
        }
        if (NTVDM64_TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_SNGL)) {
            /* D1=1, SNGL=1, ICW3=0 */
        } else {
            /* D1=0, SNGL=0, ICW3=1 */
        }
        if (NTVDM64_TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_LTIM)) {
            /* D3=1, LTIM=1, Level Triggered Mode */
        } else {
            /* D3=0, LTIM=0, Edge  Triggered Mode */
        }
    } else {
        /* OCWs (D4=0) */
        if (NTVDM64_TYPE_GET_BIT(port->data.ioByte, VPIC_OCW3_I)) {
            /* OCW3 (D3=1) */
            if (NTVDM64_TYPE_GET_BIT(port->data.ioByte, VPIC_OCW3_ESMM)) {
                /* ESMM=1: Enable Special Mask Mode */
                rpic->data.ocw3 = port->data.ioByte;
                if (NTVDM64_TYPE_GET_BIT(rpic->data.ocw3, VPIC_OCW3_SMM)) {
                    /* SMM=1: Set Special Mask Mode */
                } else {
                    /* SMM=0: Clear Sepcial Mask Mode */
                }
            } else {
                /* ESMM=0: Keep SMM */
                rpic->data.ocw3 = (rpic->data.ocw3 & VPIC_OCW3_SMM) | (port->data.ioByte & ~VPIC_OCW3_SMM);
            }
        } else {
            /* OCW2 (D3=0) */
            switch (port->data.ioByte & (VPIC_OCW2_EOI | VPIC_OCW2_SL | VPIC_OCW2_R)) {
            /* D7=R, D6=SL, D5=EOI(End Of Interrupt) */
            case 0x80:
                /* 100: Set (Rotate Priorities in Auto EOI Mode) */
                if (NTVDM64_TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_AEOI)) {
                    rpic->data.ocw2 = port->data.ioByte;
                }
                break;
            case 0x00:
                /* 000: Clear (Rotate Priorities in Auto EOI Mode) */
                if (NTVDM64_TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_AEOI)) {
                    rpic->data.ocw2 = port->data.ioByte;
                }
                /* Bug in easyVM (0x00 ?= 0x20) */
                break;
            case 0x20:
                /* 001: Non-specific EOI Command */
                /* Set bit of highest priority interrupt in ISR to 0,
                 IR0 > IR1 > IR2(IR8 > ... > IR15) > IR3 > ... > IR7 */
                rpic->data.ocw2 = port->data.ioByte;
                if (rpic->data.isr) {
                    id = VPIC_GetIsrTopId(rpic);
                    NTVDM64_TYPE_CLEAR_BIT(rpic->data.isr, VPIC_ISR_IRQ(id));
                }
                break;
            case 0x60:
                /* 011: Specific EOI Command */
                rpic->data.ocw2 = port->data.ioByte;
                if (rpic->data.isr) {
                    /* Get L2,L1,L0 */
                    id = rpic->data.ocw2 & VPIC_OCW2_L;
                    NTVDM64_TYPE_CLEAR_BIT(rpic->data.isr, VPIC_ISR_IRQ(id));
                }
                /* Bug in easyVM: "isr &= (1 << i)" */
                break;
            case 0xa0:
                /* 101: Rotate Priorities on Non-specific EOI */
                rpic->data.ocw2 = port->data.ioByte;
                if (rpic->data.isr) {
                    id = VPIC_GetIsrTopId(rpic);
                    NTVDM64_TYPE_CLEAR_BIT(rpic->data.isr, VPIC_ISR_IRQ(id));
                    rpic->data.irx = (id + 1) % VPIC_MAX_IRQ_COUNT;
                }
                break;
            case 0xe0:
                /* 111: Rotate Priority on Specific EOI Command */
                rpic->data.ocw2 = port->data.ioByte;
                if (rpic->data.isr) {
                    id = VPIC_GetIsrTopId(rpic);
                    NTVDM64_TYPE_CLEAR_BIT(rpic->data.isr, VPIC_ISR_IRQ(id));
                    rpic->data.irx = ((rpic->data.ocw2 & VPIC_OCW2_L) + 1) % VPIC_MAX_IRQ_COUNT;
                }
                break;
            case 0xc0:
                /* 110: Set Priority (does not reset current ISR bit) */
                rpic->data.ocw2 = port->data.ioByte;
                rpic->data.irx = (VPIC_GetOCW2_L(rpic->data.ocw2) + 1) % VPIC_MAX_IRQ_COUNT;
                break;
            case 0x40:
                /* 010: No Operation */
                break;
            default:
                break;
            }
        }
    }
}
/*
 * io_read_00x1
 * PIC provide IMR
 * Reference: 16-32.PDF, Page 184
 */
static void io_read_00x1(t_pic *rpic, t_port *port) {
    port->data.ioByte = rpic->data.imr;
}
/*
 * io_write_00x1
 * PIC get ICW2, ICW3, ICW4, OCW1 after ICW1
 */
static void io_write_00x1(t_pic *rpic, t_port *port) {
    switch (rpic->data.status) {
    case ICW2:
        rpic->data.icw2 = port->data.ioByte & VPIC_ICW2_VALID;
        if (!NTVDM64_TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_SNGL)) {
            /* ICW1.SNGL=0, ICW3=1 */
            rpic->data.status = ICW3;
        } else if (NTVDM64_TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_IC4)) {
            /* ICW1.SNGL=1, IC4=1 */
            rpic->data.status = ICW4;
        } else {
            /* ICW1.SNGL=1, IC4=0 */
            rpic->data.status = OCW1;
        }
        break;
    case ICW3:
        rpic->data.icw3 = port->data.ioByte;
        if (NTVDM64_TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_IC4)) {
            /* ICW1.IC4=1 */
            rpic->data.status = ICW4;
        } else {
            rpic->data.status = OCW1;
        }
        break;
    case ICW4:
        rpic->data.icw4 = port->data.ioByte & VPIC_ICW4_VALID;
        if (NTVDM64_TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_uPM)) {
            /* uPM=1, 16-bit 80x86 */
        } else {
            /* uPM=0, 8-bit 8080/8085 */
        }
        if (NTVDM64_TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_AEOI)) {
            /* AEOI=1, Automatic End of Interrupt */
        } else {
            /* AEOI=0, Non-automatic End of Interrupt */
        }
        if (NTVDM64_TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_BUF)) {
            /* BUF=1, Buffer */
            if (NTVDM64_TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_MS)) {
                /* M/S=1, Master 8259A */
            } else {
                /* M/S=0, Slave 8259A */
            }
        } else {
            /* BUF=0, Non-buffer */
        }
        if (NTVDM64_TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_SFNM)) {
            /* SFNM=1, Special Fully Nested Mode */
        } else {
            /* SFNM=0, Non-special Fully Nested Mode */
        }
        rpic->data.status = OCW1;
        break;
    case OCW1:
        rpic->data.ocw1 = port->data.ioByte;
        if (NTVDM64_TYPE_GET_BIT(rpic->data.ocw3, VPIC_OCW3_SMM)) {
            rpic->data.isr &= ~(rpic->data.imr);
        }
        break;
    default:
        break;
    }
}

/* The provider owner is the composition-owned PIC selected for this port. */
static void io_read_0020(t_port *port, ntvdm64_type_unsigned_16 port_id, void *owner) {
    (void)port_id;
    io_read_00x0((t_pic *)owner, port);
}
static void io_read_0021(t_port *port, ntvdm64_type_unsigned_16 port_id, void *owner) {
    (void)port_id;
    io_read_00x1((t_pic *)owner, port);
}
static void io_read_00A0(t_port *port, ntvdm64_type_unsigned_16 port_id, void *owner) {
    (void)port_id;
    io_read_00x0((t_pic *)owner, port);
}
static void io_read_00A1(t_port *port, ntvdm64_type_unsigned_16 port_id, void *owner) {
    (void)port_id;
    io_read_00x1((t_pic *)owner, port);
}
static void io_write_0020(t_port *port, ntvdm64_type_unsigned_16 port_id, void *owner) {
    (void)port_id;
    io_write_00x0((t_pic *)owner, port);
}
static void io_write_0021(t_port *port, ntvdm64_type_unsigned_16 port_id, void *owner) {
    (void)port_id;
    io_write_00x1((t_pic *)owner, port);
}
static void io_write_00A0(t_port *port, ntvdm64_type_unsigned_16 port_id, void *owner) {
    (void)port_id;
    io_write_00x0((t_pic *)owner, port);
}
static void io_write_00A1(t_port *port, ntvdm64_type_unsigned_16 port_id, void *owner) {
    (void)port_id;
    io_write_00x1((t_pic *)owner, port);
}

/*
 * vpicSetIRQ
 * Puts int request into IRR
 * Called by int request sender of devices, e.g. vpitIntTick
 */
void core_machine_pic_set_irq(t_pic *master, t_pic *slave, ntvdm64_type_unsigned_8 irq_id) {
    if (master == NULL) return;
    switch (irq_id) {
    case 0x00:
    case 0x01:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
        NTVDM64_TYPE_SET_BIT(master->data.irr, VPIC_IRR_IRQ(irq_id));
        break;
    case 0x08:
    case 0x09:
    case 0x0a:
    case 0x0b:
    case 0x0c:
    case 0x0d:
    case 0x0e:
    case 0x0f:
        if (slave == NULL) return;
        NTVDM64_TYPE_SET_BIT(master->data.irr, VPIC_IRR_IRQ(0x02));
        NTVDM64_TYPE_SET_BIT(slave->data.irr, VPIC_IRR_IRQ(irq_id - 0x08));
        break;
    case 0x02:
    default:
        break;
    }
}

void core_machine_pic_timer_output(void *owner) {
    core_machine_pic_set_irq((t_pic *)owner, NULL, 0x00);
}

ntvdm64_type_bool core_machine_pic_scan_interrupt(t_pic *master, t_pic *slave) {
    ntvdm64_type_bool flagINTR;
    if (master == NULL || slave == NULL) return NTVDM64_TYPE_FALSE;
    flagINTR = HasINTR(master);
    if (flagINTR && (VPIC_GetIntrTopId(master) == 2)) {
        /* check slave pic */
        flagINTR = HasINTR(slave);
    }
    return flagINTR;
}
ntvdm64_type_unsigned_8 core_machine_pic_get_interrupt(t_pic *master, t_pic *slave) {
    ntvdm64_type_unsigned_8 reqId1; /* top requested int id in master pic */
    ntvdm64_type_unsigned_8 reqId2; /* top requested int id in slave pic */
    if (master == NULL || slave == NULL) return 0;
    reqId1 = VPIC_GetIntrTopId(master);
    RespondINTR(master, reqId1);
    if (reqId1 == 0x02) {
        /* if IR2 has int request, then test slave pic */
        reqId2 = VPIC_GetIntrTopId(slave);
        RespondINTR(slave, reqId2);
        /* find the final int id based on slave ICW2 */
        return (reqId2 | slave->data.icw2);
    } else {
        /* find the final int id based on master ICW2 */
        return (reqId1 | master->data.icw2);
    }
}

void core_machine_pic_initialize(t_pic *master, t_pic *slave, t_port *port)
{
    if (master == NULL || slave == NULL || port == NULL) return;
    MEMSET((void *)master, NTVDM64_TYPE_ZERO_8, sizeof(*master));
    MEMSET((void *)slave, NTVDM64_TYPE_ZERO_8, sizeof(*slave));
    core_machine_port_add_read(port, 0x0020, io_read_0020, master);
    core_machine_port_add_read(port, 0x0021, io_read_0021, master);
    core_machine_port_add_read(port, 0x00a0, io_read_00A0, slave);
    core_machine_port_add_read(port, 0x00a1, io_read_00A1, slave);
    core_machine_port_add_write(port, 0x0020, io_write_0020, master);
    core_machine_port_add_write(port, 0x0021, io_write_0021, master);
    core_machine_port_add_write(port, 0x00a0, io_write_00A0, slave);
    core_machine_port_add_write(port, 0x00a1, io_write_00A1, slave);
}
void core_machine_pic_reset(t_pic *master, t_pic *slave) {
    if (master == NULL || slave == NULL) return;
    MEMSET((void *)(&master->data), NTVDM64_TYPE_ZERO_8, sizeof(t_pic_data));
    MEMSET((void *)(&slave->data), NTVDM64_TYPE_ZERO_8, sizeof(t_pic_data));
    master->data.status = slave->data.status = ICW1;
    master->data.ocw3 = slave->data.ocw3 = VPIC_OCW3_RR;
}
void core_machine_pic_refresh(t_pic *master, t_pic *slave) {
    if (master == NULL || slave == NULL) return;
    if (slave->data.irr & (~slave->data.imr)) {
        /* if slave pic has requested int, then
         * pass the request into IR2 of master pic */
        NTVDM64_TYPE_SET_BIT(master->data.irr, VPIC_IRR_IRQ(2));
    } else {
        /* remove IR2 from master pic */
        NTVDM64_TYPE_CLEAR_BIT(master->data.irr, VPIC_IRR_IRQ(2));
    }
}
void core_machine_pic_finalize(t_pic *master, t_pic *slave) {
    (void)master;
    (void)slave;
}

/*
Test Case for regular IBM PC use
Initialize (ICW1, ICW2, ICW3, ICW4 50%)
o20 11
o21 08
o21 04
o21 11
oa0 11
oa1 70
oa1 02
oa1 01
    PrintInfo
iff20
    Mask IRQ 5 and IRQ c by OCW1
o21 20
oa1 10
    SetIRQs
off20 1
off20 5
off20 a
off20 c
off20 d
    ScanINTR
        iff21
        result should be 01
    GetINTR
        iff22
        result should be 09
    EOI 0x20, PrintInfo, look at IRR, ISR(0), IMR
        o20 20
    ScanINTR
        iff21
        result should be 01
    GetINTR
        iff22
        result should be 72 now ISR1 is 4, ISR2 is 4
        SetIRQ
            off20 0
        ScanINTR
            iff21
            result should be 01
        GetINTR
            iff22
            result should be 08
        EOI
            o20 20, now ISR1 should be 4, ISR2 should be 4
        SetIRQ
            off20 8
        ScanINTR
            iff21
            result should be 01
        GetINTR
            iff22
            result should be 01, ISR2 should be 5
        EOI
            oa0 20, now ISR1 should be 4, ISR2 should be 4
        SetIRQ
            off20 4
        ScanINTR
            iff21
            result should be 00
    EOI
        oa0 20
        o20 20
    ScanINTR, PrintInfo, look at IRR, ISR, IMR
    GetINTR, PrintInfo, look at IRR, ISR, IMR
    EOI 0x20, PrintInfo, look at IRR, ISR, IMR (think about pic2)

    Test case for port commands
    Initialize
o20 11
o21 08
o21 04
o21 11
oa0 11
oa1 70
oa1 02
oa1 01
    SetIRQs
off20 1
off20 5
off20 a
off20 c
off20 d
    Test ESMM (OCW3 50%)
        o20 29    ocw3 = 0010 1001, see if D5 changes
        o20 4a    ocw3 = 0100 1010, see if D5 changes
        o20 6c    ocw3 = 0110 1100, see if D5 changes
        o20 49    ocw3 = 0100 1001, see if D5 changes
    Test SMM (OCW1)
        o20 49, disable SMM
        iff21
        iff22, get ISR 1
        o21 33
        iff20, print info
        o20 6c, enable SMM
        o21 33
        iff20, print info
    Test P/RR/RIS (OCW3 50%)
        o20 4c    ocw3 = 0100 1100, enable poll
        i20        see poll
        o20 4a    ocw3 = 0100 1010, disable poll, enable IRR
        i20        see irr
        o20 4b    ocw3 = 0100 1011, disable poll, enable ISR
        i20        see isr
    Test AEOI (ICW4 50%)
    not tested yet
    Test OCW2
    not tested yet
*/

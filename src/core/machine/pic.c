/* Copyright 2012-2014 Neko. */

/*
 * VPIC implements programmable interrupt controller with
 * two Intel 8259A chips, one master and one slave.
 */

#include "type.h"

#include "core/machine/pit.h"
#include "core/machine/port.h"
#include "core/machine/pic.h"

static C_VOID core_machine_pic_refresh_bound(t_pic *pic);

/*
 * GetRegTopId: Internal function
 * Returns id of highest priority interrupt
 * Returns 0x08 if reg is null
 */
static type_unsigned_8 GetRegTopId(t_pic *rpic, type_unsigned_8 reg) {
    type_unsigned_8 id = 0;
    if (reg == TYPE_ZERO_8) {
        return 0x08;
    }
    reg = (reg << (VPIC_MAX_IRQ_COUNT - (rpic->data.irx))) | (reg>> (rpic->data.irx));
    while ((id < VPIC_MAX_IRQ_COUNT) && !TYPE_MASK_UNSIGNED_1(reg >> id)) {
        id++;
    }
    return (id + rpic->data.irx) % VPIC_MAX_IRQ_COUNT;
}
/* The rank is relative to the current rotating priority base. */
static type_unsigned_8 core_machine_pic_priority_rank(const t_pic *pic,
    type_unsigned_8 id)
{
    return (type_unsigned_8)((id + VPIC_MAX_IRQ_COUNT - pic->data.irx) %
        VPIC_MAX_IRQ_COUNT);
}

static type_bool core_machine_pic_request_can_interrupt(const t_pic *pic,
    type_unsigned_8 request)
{
    type_unsigned_8 service;
    type_unsigned_8 effective_isr;
    type_unsigned_8 request_rank;
    type_unsigned_8 service_rank;

    effective_isr = pic->data.isr;
    if (TYPE_GET_BIT(pic->data.ocw3, VPIC_OCW3_SMM)) {
        effective_isr &= ~pic->data.imr;
    }
    service = GetRegTopId((t_pic *)pic, effective_isr);
    if (service == VPIC_MAX_IRQ_COUNT) return TYPE_TRUE;
    request_rank = core_machine_pic_priority_rank(pic, request);
    service_rank = core_machine_pic_priority_rank(pic, service);
    return request_rank < service_rank;
}

static type_unsigned_8 core_machine_pic_pending_requests(const t_pic *pic)
{
    return pic->data.irr | pic->data.cascade_irr;
}

static type_bool core_machine_pic_select_controller(const t_pic *pic,
    type_unsigned_8 *out_id)
{
    type_unsigned_8 offset;
    type_unsigned_8 id;

    if (pic == STD_NULL || out_id == STD_NULL) return TYPE_FALSE;
    for (offset = 0u; offset < VPIC_MAX_IRQ_COUNT; ++offset) {
        id = (type_unsigned_8)((pic->data.irx + offset) % VPIC_MAX_IRQ_COUNT);
        if (pic->data.unmask_remaining_ticks[id] == 0u &&
            TYPE_GET_BIT(core_machine_pic_pending_requests(pic) & ~pic->data.imr,
                VPIC_IRR_IRQ(id)) &&
            core_machine_pic_request_can_interrupt(pic, id)) {
            *out_id = id;
            return TYPE_TRUE;
        }
    }
    return TYPE_FALSE;
}

static type_bool core_machine_pic_cascade_line(const t_pic *master,
    const t_pic *slave, type_unsigned_8 *out_line)
{
    type_unsigned_8 line;

    if (master == STD_NULL || slave == STD_NULL || out_line == STD_NULL ||
        TYPE_GET_BIT(master->data.icw1, VPIC_ICW1_SNGL) ||
        TYPE_GET_BIT(slave->data.icw1, VPIC_ICW1_SNGL)) return TYPE_FALSE;
    line = slave->data.icw3 & 0x07u;
    if (!TYPE_GET_BIT(master->data.icw3, VPIC_ICW3_S(line))) return TYPE_FALSE;
    *out_line = line;
    return TYPE_TRUE;
}

static type_bool core_machine_pic_master_declares_slave(const t_pic *master,
    type_unsigned_8 line)
{
    return !TYPE_GET_BIT(master->data.icw1, VPIC_ICW1_SNGL) &&
        TYPE_GET_BIT(master->data.icw3, VPIC_ICW3_S(line));
}

static type_bool core_machine_pic_sfnm_cascade_can_interrupt(
    const t_pic *master, type_unsigned_8 master_id, const t_pic *slave)
{
    type_unsigned_8 cascade_line;
    type_unsigned_8 slave_id;

    return core_machine_pic_cascade_line(master, slave, &cascade_line) &&
        master_id == cascade_line &&
        TYPE_GET_BIT(master->data.icw4, VPIC_ICW4_SFNM) &&
        GetRegTopId((t_pic *)master, master->data.isr) == cascade_line &&
        core_machine_pic_select_controller(slave, &slave_id);
}

static type_bool core_machine_pic_select(t_pic *master, t_pic *slave,
    type_unsigned_8 *out_master_id, type_unsigned_8 *out_slave_id)
{
    type_unsigned_8 offset;
    type_unsigned_8 master_id;
    type_unsigned_8 slave_id;
    type_unsigned_8 cascade_line;

    if (master == STD_NULL || slave == STD_NULL || out_master_id == STD_NULL ||
        out_slave_id == STD_NULL) return TYPE_FALSE;
    for (offset = 0u; offset < VPIC_MAX_IRQ_COUNT; ++offset) {
        master_id = (type_unsigned_8)((master->data.irx + offset) %
            VPIC_MAX_IRQ_COUNT);
        if (master->data.unmask_remaining_ticks[master_id] != 0u ||
            !TYPE_GET_BIT(core_machine_pic_pending_requests(master) &
                ~master->data.imr,
                VPIC_IRR_IRQ(master_id))) {
            continue;
        }
        if (!core_machine_pic_request_can_interrupt(master, master_id) &&
            !core_machine_pic_sfnm_cascade_can_interrupt(master, master_id,
                slave)) {
            continue;
        }
        if (core_machine_pic_cascade_line(master, slave, &cascade_line) &&
            master_id == cascade_line) {
            if (!core_machine_pic_select_controller(slave, &slave_id)) continue;
            *out_slave_id = slave_id;
        } else if (core_machine_pic_master_declares_slave(master, master_id)) {
            continue;
        } else {
            *out_slave_id = VPIC_MAX_IRQ_COUNT;
        }
        *out_master_id = master_id;
        return TYPE_TRUE;
    }
    return TYPE_FALSE;
}
/*
 * RespondINTR: Internal function
 * Acknowledges the selected request by moving it to ISR.
 */
static C_VOID RespondINTR(t_pic *rpic, type_unsigned_8 id,
    type_bool cascade_request) {
    TYPE_SET_BIT(rpic->data.isr, VPIC_ISR_IRQ(id)); /* put C_INT into ISR */
    if (cascade_request) {
        TYPE_CLEAR_BIT(rpic->data.cascade_irr, VPIC_IRR_IRQ(id));
    } else {
        TYPE_CLEAR_BIT(rpic->data.irr, VPIC_IRR_IRQ(id));
    }
    if (TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_AEOI)) {
        /* Auto EOI Mode */
        TYPE_CLEAR_BIT(rpic->data.isr, VPIC_ISR_IRQ(id));
        if (TYPE_GET_BIT(rpic->data.ocw2, VPIC_OCW2_R)) {
            /* Rotate Mode */
            rpic->data.irx = (id + 1) % VPIC_MAX_IRQ_COUNT;
        }
    }
}

static C_VOID core_machine_pic_begin_initialization(t_pic *pic,
    type_unsigned_8 icw1)
{
    pic->data.irr = TYPE_ZERO_8;
    pic->data.imr = TYPE_ZERO_8;
    pic->data.isr = TYPE_ZERO_8;
    pic->data.icw1 = icw1;
    pic->data.icw2 = TYPE_ZERO_8;
    pic->data.icw3 = TYPE_ZERO_8;
    pic->data.icw4 = TYPE_ZERO_8;
    pic->data.ocw2 = TYPE_ZERO_8;
    pic->data.ocw3 = VPIC_OCW3_RR;
    pic->data.irx = TYPE_ZERO_8;
    pic->data.cascade_irr = TYPE_ZERO_8;
    pic->data.status = ICW2;
}

static type_unsigned_8 core_machine_pic_eoi_service(const t_pic *pic)
{
    type_unsigned_8 effective_isr = pic->data.isr;

    if (TYPE_GET_BIT(pic->data.ocw3, VPIC_OCW3_SMM)) {
        effective_isr &= ~pic->data.imr;
    }
    return GetRegTopId((t_pic *)pic, effective_isr);
}

static C_INT core_machine_pic_is_level(const t_pic *pic)
{
    return pic != STD_NULL && TYPE_GET_BIT(pic->data.icw1, VPIC_ICW1_LTIM);
}

static t_pic *core_machine_pic_irq_source_controller(
    core_machine_pic_irq_source *source, type_unsigned_8 *out_line)
{
    if (source == STD_NULL || out_line == STD_NULL) return STD_NULL;
    if (source->irq < 8u) {
        *out_line = source->irq;
        return source->master;
    }
    if (source->irq < 16u) {
        *out_line = (type_unsigned_8)(source->irq - 8u);
        return source->slave;
    }
    return STD_NULL;
}

/*
 * io_read_00x0
 * PIC provide POLL, IRR, ISR based on OCW3
 * Reference: 16-32.PDF, Page 192
 * Reference: PC.PDF, Page 950
 */
static C_VOID io_read_00x0(t_pic *rpic, t_port *port) {
    type_unsigned_8 id;

    if (TYPE_GET_BIT(rpic->data.ocw3, VPIC_OCW3_P)) {
        /* P=1 (Poll Command) */
        if (!core_machine_pic_select_controller(rpic, &id)) {
            /* set all bits to 0 if there's no interrupt in queue */
            port->data.ioByte = TYPE_ZERO_8;
        } else {
            /* A poll read acknowledges the selected controller request. */
            port->data.ioByte = VPIC_POLL_I | id;
            RespondINTR(rpic, id, TYPE_GET_BIT(rpic->data.cascade_irr,
                VPIC_IRR_IRQ(id)));
        }
        TYPE_CLEAR_BIT(rpic->data.ocw3, VPIC_OCW3_P);
    } else {
        switch (rpic->data.ocw3 & (VPIC_OCW3_RR | VPIC_OCW3_RIS)) {
        case 0x02:
            /* RR=1, RIS=0, Read IRR */
            port->data.ioByte = core_machine_pic_pending_requests(rpic);
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
static C_VOID io_write_00x0(t_pic *rpic, t_port *port) {
    type_unsigned_8 id;
    if (TYPE_GET_BIT(port->data.ioByte, VPIC_ICW1_I)) {
        /* ICW1 (D4=1) */
        core_machine_pic_begin_initialization(rpic, port->data.ioByte);
        if (TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_IC4)) {
            /* D0=1, IC4=1 */
        } else {
            /* D0=0, IC4=0 */
            rpic->data.icw4 = TYPE_ZERO_8;
        }
        if (TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_SNGL)) {
            /* D1=1, SNGL=1, ICW3=0 */
        } else {
            /* D1=0, SNGL=0, ICW3=1 */
        }
        if (TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_LTIM)) {
            /* D3=1, LTIM=1, Level Triggered Mode */
        } else {
            /* D3=0, LTIM=0, Edge  Triggered Mode */
        }
    } else {
        /* OCWs (D4=0) */
        if (TYPE_GET_BIT(port->data.ioByte, VPIC_OCW3_I)) {
            /* OCW3 (D3=1) */
            type_unsigned_8 old_ocw3 = rpic->data.ocw3;
            type_unsigned_8 ocw3 = port->data.ioByte;

            if (!TYPE_GET_BIT(ocw3, VPIC_OCW3_RR)) {
                ocw3 = (ocw3 & ~(VPIC_OCW3_RR | VPIC_OCW3_RIS)) |
                    (old_ocw3 & (VPIC_OCW3_RR | VPIC_OCW3_RIS));
            }
            if (TYPE_GET_BIT(port->data.ioByte, VPIC_OCW3_ESMM)) {
                /* ESMM=1: Enable Special Mask Mode */
                rpic->data.ocw3 = ocw3;
                if (TYPE_GET_BIT(rpic->data.ocw3, VPIC_OCW3_SMM)) {
                    /* SMM=1: Set Special Mask Mode */
                } else {
                    /* SMM=0: Clear Sepcial Mask Mode */
                }
            } else {
                /* ESMM=0: Keep SMM */
                rpic->data.ocw3 = (old_ocw3 & VPIC_OCW3_SMM) |
                    (ocw3 & ~VPIC_OCW3_SMM);
            }
        } else {
            /* OCW2 (D3=0) */
            switch (port->data.ioByte & (VPIC_OCW2_EOI | VPIC_OCW2_SL | VPIC_OCW2_R)) {
            /* D7=R, D6=SL, D5=EOI(End Of Interrupt) */
            case 0x80:
                /* 100: Set (Rotate Priorities in Auto EOI Mode) */
                if (TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_AEOI)) {
                    rpic->data.ocw2 = port->data.ioByte;
                }
                break;
            case 0x00:
                /* 000: Clear (Rotate Priorities in Auto EOI Mode) */
                if (TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_AEOI)) {
                    rpic->data.ocw2 = port->data.ioByte;
                }
                /* Bug in easyVM (0x00 ?= 0x20) */
                break;
            case 0x20:
                /* 001: Non-specific EOI Command */
                /* Set bit of highest priority interrupt in ISR to 0,
                 IR0 > IR1 > IR2(IR8 > ... > IR15) > IR3 > ... > IR7 */
                rpic->data.ocw2 = port->data.ioByte;
                id = core_machine_pic_eoi_service(rpic);
                if (id != VPIC_MAX_IRQ_COUNT) {
                    TYPE_CLEAR_BIT(rpic->data.isr, VPIC_ISR_IRQ(id));
                }
                break;
            case 0x60:
                /* 011: Specific EOI Command */
                rpic->data.ocw2 = port->data.ioByte;
                if (rpic->data.isr) {
                    /* Get L2,L1,L0 */
                    id = rpic->data.ocw2 & VPIC_OCW2_L;
                    TYPE_CLEAR_BIT(rpic->data.isr, VPIC_ISR_IRQ(id));
                }
                /* Bug in easyVM: "isr &= (1 << i)" */
                break;
            case 0xa0:
                /* 101: Rotate Priorities on Non-specific EOI */
                rpic->data.ocw2 = port->data.ioByte;
                id = core_machine_pic_eoi_service(rpic);
                if (id != VPIC_MAX_IRQ_COUNT) {
                    TYPE_CLEAR_BIT(rpic->data.isr, VPIC_ISR_IRQ(id));
                    rpic->data.irx = (id + 1) % VPIC_MAX_IRQ_COUNT;
                }
                break;
            case 0xe0:
                /* 111: Rotate Priority on Specific EOI Command */
                rpic->data.ocw2 = port->data.ioByte;
                if (rpic->data.isr) {
                    id = rpic->data.ocw2 & VPIC_OCW2_L;
                    TYPE_CLEAR_BIT(rpic->data.isr, VPIC_ISR_IRQ(id));
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
    core_machine_pic_refresh_bound(rpic);
}
/*
 * io_read_00x1
 * PIC provide IMR
 * Reference: 16-32.PDF, Page 184
 */
static C_VOID io_read_00x1(t_pic *rpic, t_port *port) {
    port->data.ioByte = rpic->data.imr;
}
/*
 * io_write_00x1
 * PIC get ICW2, ICW3, ICW4, OCW1 after ICW1
 */
static C_VOID io_write_00x1(t_pic *rpic, t_port *port) {
    type_unsigned_8 previous_imr;
    type_unsigned_8 released;
    type_unsigned_8 id;

    switch (rpic->data.status) {
    case ICW2:
        rpic->data.icw2 = port->data.ioByte & VPIC_ICW2_VALID;
        if (!TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_SNGL)) {
            /* ICW1.SNGL=0, ICW3=1 */
            rpic->data.status = ICW3;
        } else if (TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_IC4)) {
            /* ICW1.SNGL=1, IC4=1 */
            rpic->data.status = ICW4;
        } else {
            /* ICW1.SNGL=1, IC4=0 */
            rpic->data.status = OCW1;
        }
        break;
    case ICW3:
        rpic->data.icw3 = port->data.ioByte;
        if (TYPE_GET_BIT(rpic->data.icw1, VPIC_ICW1_IC4)) {
            /* ICW1.IC4=1 */
            rpic->data.status = ICW4;
        } else {
            rpic->data.status = OCW1;
        }
        break;
    case ICW4:
        rpic->data.icw4 = port->data.ioByte & VPIC_ICW4_VALID;
        if (TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_uPM)) {
            /* uPM=1, 16-bit 80x86 */
        } else {
            /* uPM=0, 8-bit 8080/8085 */
        }
        if (TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_AEOI)) {
            /* AEOI=1, Automatic End of Interrupt */
        } else {
            /* AEOI=0, Non-automatic End of Interrupt */
        }
        if (TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_BUF)) {
            /* BUF=1, Buffer */
            if (TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_MS)) {
                /* M/S=1, Master 8259A */
            } else {
                /* M/S=0, Slave 8259A */
            }
        } else {
            /* BUF=0, Non-buffer */
        }
        if (TYPE_GET_BIT(rpic->data.icw4, VPIC_ICW4_SFNM)) {
            /* SFNM=1, Special Fully Nested Mode */
        } else {
            /* SFNM=0, Non-special Fully Nested Mode */
        }
        rpic->data.status = OCW1;
        break;
    case OCW1:
        previous_imr = rpic->data.imr;
        rpic->data.imr = port->data.ioByte;
        released = previous_imr & ~rpic->data.imr &
            core_machine_pic_pending_requests(rpic);
        for (id = 0u; id < VPIC_MAX_IRQ_COUNT; ++id) {
            if (TYPE_GET_BIT(released, VPIC_IRR_IRQ(id))) {
                rpic->data.unmask_remaining_ticks[id] =
                    rpic->data.unmask_delivery_ticks[id];
            }
        }
        break;
    default:
        break;
    }
    core_machine_pic_refresh_bound(rpic);
}

C_VOID core_machine_pic_set_irq_timing(t_pic *master, t_pic *slave,
    const core_machine_pic_irq_timing *timing)
{
    type_unsigned_8 id;

    if (master == STD_NULL || slave == STD_NULL || timing == STD_NULL) return;
    for (id = 0u; id < VPIC_MAX_IRQ_COUNT; ++id) {
        master->data.unmask_delivery_ticks[id] = timing->unmask_delivery_ticks[id];
        slave->data.unmask_delivery_ticks[id] = timing->unmask_delivery_ticks[id + 8u];
    }
}

static C_VOID core_machine_pic_advance_one(t_pic *pic, type_unsigned_64 elapsed_ticks)
{
    type_unsigned_8 id;

    if (pic == STD_NULL || elapsed_ticks == 0u) return;
    for (id = 0u; id < VPIC_MAX_IRQ_COUNT; ++id) {
        if (elapsed_ticks >= pic->data.unmask_remaining_ticks[id]) {
            pic->data.unmask_remaining_ticks[id] = 0u;
        } else {
            pic->data.unmask_remaining_ticks[id] -= elapsed_ticks;
        }
    }
}

C_VOID core_machine_pic_advance(t_pic *master, t_pic *slave,
    type_unsigned_64 elapsed_ticks)
{
    core_machine_pic_advance_one(master, elapsed_ticks);
    core_machine_pic_advance_one(slave, elapsed_ticks);
}

static type_status core_machine_pic_ticks_until_one(const t_pic *pic,
    type_unsigned_64 *io_ticks)
{
    type_unsigned_8 id;
    type_unsigned_8 pending;

    if (pic == STD_NULL || io_ticks == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    pending = core_machine_pic_pending_requests(pic) & ~pic->data.imr;
    for (id = 0u; id < VPIC_MAX_IRQ_COUNT; ++id) {
        if (TYPE_GET_BIT(pending, VPIC_IRR_IRQ(id)) &&
            pic->data.unmask_remaining_ticks[id] != 0u &&
            pic->data.unmask_remaining_ticks[id] < *io_ticks) {
            *io_ticks = pic->data.unmask_remaining_ticks[id];
        }
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_pic_ticks_until_event(const t_pic *master, const t_pic *slave,
    type_unsigned_64 *out_ticks)
{
    type_unsigned_64 ticks = UINT64_MAX;

    if (master == STD_NULL || slave == STD_NULL || out_ticks == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    (C_VOID)core_machine_pic_ticks_until_one(master, &ticks);
    (C_VOID)core_machine_pic_ticks_until_one(slave, &ticks);
    if (ticks == UINT64_MAX) return TYPE_STATUS_INVALID_STATE;
    *out_ticks = ticks;
    return TYPE_STATUS_OK;
}

/* The provider owner is the composition-owned PIC selected for this port. */
static C_VOID io_read_0020(t_port *port, type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_read_00x0((t_pic *)owner, port);
}
static C_VOID io_read_0021(t_port *port, type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_read_00x1((t_pic *)owner, port);
}
static C_VOID io_read_00A0(t_port *port, type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_read_00x0((t_pic *)owner, port);
}
static C_VOID io_read_00A1(t_port *port, type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_read_00x1((t_pic *)owner, port);
}
static C_VOID io_write_0020(t_port *port, type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_write_00x0((t_pic *)owner, port);
}
static C_VOID io_write_0021(t_port *port, type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_write_00x1((t_pic *)owner, port);
}
static C_VOID io_write_00A0(t_port *port, type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_write_00x0((t_pic *)owner, port);
}
static C_VOID io_write_00A1(t_port *port, type_unsigned_16 port_id, C_VOID *owner) {
    (C_VOID)port_id;
    io_write_00x1((t_pic *)owner, port);
}

/*
 * vpicSetIRQ
 * Puts C_INT request into IRR
 * Called by C_INT request sender of devices, e.g. vpitIntTick
 */
C_VOID core_machine_pic_irq_source_bind(core_machine_pic_irq_source *source,
    t_pic *master, t_pic *slave, type_unsigned_8 irq_id)
{
    if (source == STD_NULL || master == STD_NULL || slave == STD_NULL ||
        irq_id >= 16u || irq_id == 2u) return;
    STD_MEMSET(source, TYPE_ZERO_8, sizeof(*source));
    source->master = master;
    source->slave = slave;
    source->irq = irq_id;
}

C_VOID core_machine_pic_irq_source_assert(core_machine_pic_irq_source *source)
{
    t_pic *controller;
    type_unsigned_8 line;

    if (source == STD_NULL || source->asserted) return;
    controller = core_machine_pic_irq_source_controller(source, &line);
    if (controller == STD_NULL) return;
    source->asserted = TYPE_TRUE;
    if (controller->data.asserted[line] != 0xffu) ++controller->data.asserted[line];
    TYPE_SET_BIT(controller->data.irr, VPIC_IRR_IRQ(line));
    core_machine_pic_refresh_bound(controller);
}

C_VOID core_machine_pic_irq_source_deassert(core_machine_pic_irq_source *source)
{
    t_pic *controller;
    type_unsigned_8 line;

    if (source == STD_NULL || !source->asserted) return;
    controller = core_machine_pic_irq_source_controller(source, &line);
    source->asserted = TYPE_FALSE;
    if (controller == STD_NULL || controller->data.asserted[line] == 0u) return;
    --controller->data.asserted[line];
    if (core_machine_pic_is_level(controller) && controller->data.asserted[line] == 0u) {
        TYPE_CLEAR_BIT(controller->data.irr, VPIC_IRR_IRQ(line));
    }
    core_machine_pic_refresh_bound(controller);
}

C_VOID core_machine_pic_timer_output(C_VOID *owner, type_bool asserted) {
    if (asserted) {
        core_machine_pic_irq_source_assert((core_machine_pic_irq_source *)owner);
    } else {
        core_machine_pic_irq_source_deassert((core_machine_pic_irq_source *)owner);
    }
}

type_bool core_machine_pic_scan_interrupt(t_pic *master, t_pic *slave) {
    type_unsigned_8 master_id;
    type_unsigned_8 slave_id;

    return core_machine_pic_select(master, slave, &master_id, &slave_id);
}
type_unsigned_8 core_machine_pic_peek_interrupt(t_pic *master, t_pic *slave) {
    type_unsigned_8 reqId1;
    type_unsigned_8 reqId2;

    if (!core_machine_pic_select(master, slave, &reqId1, &reqId2)) return 0;
    if (reqId2 != VPIC_MAX_IRQ_COUNT) {
        return (type_unsigned_8)(reqId2 | slave->data.icw2);
    }
    return (type_unsigned_8)(reqId1 | master->data.icw2);
}
type_unsigned_8 core_machine_pic_get_interrupt(t_pic *master, t_pic *slave) {
    type_unsigned_8 reqId1; /* top requested C_INT id in master pic */
    type_unsigned_8 reqId2; /* top requested C_INT id in slave pic */
    if (!core_machine_pic_select(master, slave, &reqId1, &reqId2)) {
        if (master != STD_NULL && master->data.status == OCW1) {
            return (type_unsigned_8)(master->data.icw2 | 7u);
        }
        return 0;
    }
    RespondINTR(master, reqId1, reqId2 != VPIC_MAX_IRQ_COUNT);
    if (reqId2 != VPIC_MAX_IRQ_COUNT) {
        /* The selected paired slave supplies the vector. */
        RespondINTR(slave, reqId2, TYPE_FALSE);
        core_machine_pic_refresh(master, slave);
        /* Find the final C_INT id based on the slave ICW2. */
        return (reqId2 | slave->data.icw2);
    } else {
        /* Find the final C_INT id based on the master ICW2. */
        return (reqId1 | master->data.icw2);
    }
}

C_VOID core_machine_pic_initialize(t_pic *master, t_pic *slave, t_port *port,
    core_machine_pic_topology topology)
{
    if (master == STD_NULL || slave == STD_NULL || port == STD_NULL ||
        (topology != CORE_MACHINE_PIC_TOPOLOGY_CASCADED &&
        topology != CORE_MACHINE_PIC_TOPOLOGY_SINGLE)) return;
    STD_MEMSET((C_VOID *)master, TYPE_ZERO_8, sizeof(*master));
    STD_MEMSET((C_VOID *)slave, TYPE_ZERO_8, sizeof(*slave));
    master->cascade_master = master;
    master->cascade_slave = slave;
    slave->cascade_master = master;
    slave->cascade_slave = slave;
    core_machine_port_add_read(port, 0x0020, io_read_0020, master);
    core_machine_port_add_read(port, 0x0021, io_read_0021, master);
    core_machine_port_add_write(port, 0x0020, io_write_0020, master);
    core_machine_port_add_write(port, 0x0021, io_write_0021, master);
    if (topology == CORE_MACHINE_PIC_TOPOLOGY_CASCADED) {
        core_machine_port_add_read(port, 0x00a0, io_read_00A0, slave);
        core_machine_port_add_read(port, 0x00a1, io_read_00A1, slave);
        core_machine_port_add_write(port, 0x00a0, io_write_00A0, slave);
        core_machine_port_add_write(port, 0x00a1, io_write_00A1, slave);
    }
}
C_VOID core_machine_pic_reset(t_pic *master, t_pic *slave) {
    type_unsigned_32 master_timing[VPIC_MAX_IRQ_COUNT];
    type_unsigned_32 slave_timing[VPIC_MAX_IRQ_COUNT];

    if (master == STD_NULL || slave == STD_NULL) return;
    STD_MEMCPY(master_timing, master->data.unmask_delivery_ticks,
        sizeof(master_timing));
    STD_MEMCPY(slave_timing, slave->data.unmask_delivery_ticks,
        sizeof(slave_timing));
    STD_MEMSET((C_VOID *)(&master->data), TYPE_ZERO_8, sizeof(t_pic_data));
    STD_MEMSET((C_VOID *)(&slave->data), TYPE_ZERO_8, sizeof(t_pic_data));
    STD_MEMCPY(master->data.unmask_delivery_ticks, master_timing,
        sizeof(master_timing));
    STD_MEMCPY(slave->data.unmask_delivery_ticks, slave_timing,
        sizeof(slave_timing));
    master->data.status = slave->data.status = ICW1;
    master->data.ocw3 = slave->data.ocw3 = VPIC_OCW3_RR;
}
C_VOID core_machine_pic_refresh(t_pic *master, t_pic *slave) {
    type_unsigned_8 id;
    type_unsigned_8 cascade_line;
    if (master == STD_NULL || slave == STD_NULL) return;
    if (core_machine_pic_is_level(master)) {
        for (id = 0u; id < VPIC_MAX_IRQ_COUNT; ++id) {
            if (master->data.asserted[id] != 0u) {
                TYPE_SET_BIT(master->data.irr, VPIC_IRR_IRQ(id));
            }
        }
    }
    if (core_machine_pic_is_level(slave)) {
        for (id = 0u; id < VPIC_MAX_IRQ_COUNT; ++id) {
            if (slave->data.asserted[id] != 0u) {
                TYPE_SET_BIT(slave->data.irr, VPIC_IRR_IRQ(id));
            }
        }
    }
    master->data.cascade_irr = TYPE_ZERO_8;
    if (core_machine_pic_cascade_line(master, slave, &cascade_line) &&
        core_machine_pic_select_controller(slave, &id)) {
        TYPE_SET_BIT(master->data.cascade_irr, VPIC_IRR_IRQ(cascade_line));
    }
}
static C_VOID core_machine_pic_refresh_bound(t_pic *pic)
{
    if (pic == STD_NULL) return;
    core_machine_pic_refresh(pic->cascade_master, pic->cascade_slave);
}
C_VOID core_machine_pic_finalize(t_pic *master, t_pic *slave) {
    (C_VOID)master;
    (C_VOID)slave;
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

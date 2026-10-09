/* Copyright 2012-2014 Neko. */
#include "core/chips/pic8259/pic.h"

/*
 * GetRegTopId: Internal function
 * Returns id of highest priority interrupt
 * Returns 0x08 if reg is null
 */
static lib_u8 GetRegTopId(const x86_pic *rpic, lib_u8 reg) {
    lib_u8 id = 0;
    if (reg == 0u) {
        return 0x08;
    }
    reg = (reg << (VPIC_MAX_IRQ_COUNT - (rpic->data.irx))) | (reg>> (rpic->data.irx));
    while ((id < VPIC_MAX_IRQ_COUNT) && !((reg >> id) & 1u)) {
        id++;
    }
    return (id + rpic->data.irx) % VPIC_MAX_IRQ_COUNT;
}
/* The rank is relative to the current rotating priority base. */
static lib_u8 pic_priority_rank(const x86_pic *pic,
    lib_u8 id)
{
    return (lib_u8)((id + VPIC_MAX_IRQ_COUNT - pic->data.irx) %
        VPIC_MAX_IRQ_COUNT);
}

static lib_bool pic_request_can_interrupt(const x86_pic *pic,
    lib_u8 request)
{
    lib_u8 service;
    lib_u8 effective_isr;
    lib_u8 request_rank;
    lib_u8 service_rank;

    effective_isr = pic->data.isr;
    if (PIC_BIT_IS_SET(pic->data.ocw3, VPIC_OCW3_SMM)) {
        effective_isr &= ~pic->data.imr;
    }
    service = GetRegTopId(pic, effective_isr);
    if (service == VPIC_MAX_IRQ_COUNT) return LIB_TRUE;
    request_rank = pic_priority_rank(pic, request);
    service_rank = pic_priority_rank(pic, service);
    return request_rank < service_rank;
}

static lib_u8 pic_pending_requests(const x86_pic *pic)
{
    return pic->data.irr | pic->data.cascade_irr;
}

lib_status x86_pic_capture_registers(const x86_pic *pic,
    x86_pic_register_state *out_state)
{
    if (pic == LIB_NULL || out_state == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_state = (x86_pic_register_state) {
        pic_pending_requests(pic), pic->data.imr, pic->data.isr
    };
    return LIB_STATUS_OK;
}

static lib_bool pic_select_controller(const x86_pic *pic,
    lib_u8 *out_id)
{
    lib_u8 offset;
    lib_u8 id;

    if (pic == LIB_NULL || out_id == LIB_NULL) return LIB_FALSE;
    for (offset = 0u; offset < VPIC_MAX_IRQ_COUNT; ++offset) {
        id = (lib_u8)((pic->data.irx + offset) % VPIC_MAX_IRQ_COUNT);
        if (pic->data.unmask_remaining_ticks[id] == 0u &&
            PIC_BIT_IS_SET(pic_pending_requests(pic) & ~pic->data.imr,
                VPIC_IRR_IRQ(id)) &&
            pic_request_can_interrupt(pic, id)) {
            *out_id = id;
            return LIB_TRUE;
        }
    }
    return LIB_FALSE;
}

/*
 * RespondINTR: Internal function
 * Acknowledges the selected request by moving it to ISR.
 */
static void RespondINTR(x86_pic *rpic, lib_u8 id,
    lib_u8 cascade_request) {
    PIC_BIT_SET(rpic->data.isr, VPIC_ISR_IRQ(id)); /* put lib_i32 into ISR */
    if (cascade_request) {
        PIC_BIT_CLEAR(rpic->data.cascade_irr, VPIC_IRR_IRQ(id));
    } else {
        PIC_BIT_CLEAR(rpic->data.irr, VPIC_IRR_IRQ(id));
    }
    if (PIC_BIT_IS_SET(rpic->data.icw4, VPIC_ICW4_AEOI)) {
        /* Auto EOI Mode */
        PIC_BIT_CLEAR(rpic->data.isr, VPIC_ISR_IRQ(id));
        if (PIC_BIT_IS_SET(rpic->data.ocw2, VPIC_OCW2_R)) {
            /* Rotate Mode */
            rpic->data.irx = (id + 1) % VPIC_MAX_IRQ_COUNT;
        }
    }
}

static void pic_begin_initialization(x86_pic *pic,
    lib_u8 icw1)
{
    pic->data.irr = 0u;
    pic->data.imr = 0u;
    pic->data.isr = 0u;
    pic->data.icw1 = icw1;
    pic->data.icw2 = 0u;
    pic->data.icw3 = 0u;
    pic->data.icw4 = 0u;
    pic->data.ocw2 = 0u;
    pic->data.ocw3 = VPIC_OCW3_RR;
    pic->data.irx = 0u;
    pic->data.cascade_irr = 0u;
    pic->data.status = ICW2;
}

static lib_u8 pic_eoi_service(const x86_pic *pic)
{
    lib_u8 effective_isr = pic->data.isr;

    if (PIC_BIT_IS_SET(pic->data.ocw3, VPIC_OCW3_SMM)) {
        effective_isr &= ~pic->data.imr;
    }
    return GetRegTopId(pic, effective_isr);
}

static lib_bool pic_is_level(const x86_pic *pic)
{
    return pic != LIB_NULL && PIC_BIT_IS_SET(pic->data.icw1, VPIC_ICW1_LTIM);
}

/*
 * io_read_00x0
 * PIC provide POLL, IRR, ISR based on OCW3
 * Reference: 16-32.PDF, Page 192
 * Reference: PC.PDF, Page 950
 */
static void io_read_00x0(x86_pic *rpic, lib_u8 *io_value) {
    lib_u8 id;

    if (PIC_BIT_IS_SET(rpic->data.ocw3, VPIC_OCW3_P)) {
        /* P=1 (Poll Command) */
        if (!pic_select_controller(rpic, &id)) {
            /* set all bits to 0 if there's no interrupt in queue */
            *io_value = 0u;
        } else {
            /* A poll read acknowledges the selected controller request. */
            *io_value = VPIC_POLL_I | id;
            RespondINTR(rpic, id, PIC_BIT_IS_SET(rpic->data.cascade_irr,
                VPIC_IRR_IRQ(id)));
        }
        PIC_BIT_CLEAR(rpic->data.ocw3, VPIC_OCW3_P);
    } else {
        switch (rpic->data.ocw3 & (VPIC_OCW3_RR | VPIC_OCW3_RIS)) {
        case 0x02:
            /* RR=1, RIS=0, Read IRR */
            *io_value = pic_pending_requests(rpic);
            break;
        case 0x03:
            /* RR=1, RIS=1, Read ISR */
            *io_value = rpic->data.isr;
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
static void io_write_00x0(x86_pic *rpic, lib_u8 *io_value) {
    lib_u8 id;
    if (PIC_BIT_IS_SET(*io_value, VPIC_ICW1_I)) {
        /* ICW1 (D4=1) */
        pic_begin_initialization(rpic, *io_value);
        if (PIC_BIT_IS_SET(rpic->data.icw1, VPIC_ICW1_IC4)) {
            /* D0=1, IC4=1 */
        } else {
            /* D0=0, IC4=0 */
            rpic->data.icw4 = 0u;
        }
        if (PIC_BIT_IS_SET(rpic->data.icw1, VPIC_ICW1_SNGL)) {
            /* D1=1, SNGL=1, ICW3=0 */
        } else {
            /* D1=0, SNGL=0, ICW3=1 */
        }
        if (PIC_BIT_IS_SET(rpic->data.icw1, VPIC_ICW1_LTIM)) {
            /* D3=1, LTIM=1, Level Triggered Mode */
        } else {
            /* D3=0, LTIM=0, Edge  Triggered Mode */
        }
    } else {
        /* OCWs (D4=0) */
        if (PIC_BIT_IS_SET(*io_value, VPIC_OCW3_I)) {
            /* OCW3 (D3=1) */
            lib_u8 old_ocw3 = rpic->data.ocw3;
            lib_u8 ocw3 = *io_value;

            if (!PIC_BIT_IS_SET(ocw3, VPIC_OCW3_RR)) {
                ocw3 = (ocw3 & ~(VPIC_OCW3_RR | VPIC_OCW3_RIS)) |
                    (old_ocw3 & (VPIC_OCW3_RR | VPIC_OCW3_RIS));
            }
            if (PIC_BIT_IS_SET(*io_value, VPIC_OCW3_ESMM)) {
                /* ESMM=1: Enable Special Mask Mode */
                rpic->data.ocw3 = ocw3;
                if (PIC_BIT_IS_SET(rpic->data.ocw3, VPIC_OCW3_SMM)) {
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
            switch (*io_value & (VPIC_OCW2_EOI | VPIC_OCW2_SL | VPIC_OCW2_R)) {
            /* D7=R, D6=SL, D5=EOI(End Of Interrupt) */
            case 0x80:
                /* 100: Set (Rotate Priorities in Auto EOI Mode) */
                if (PIC_BIT_IS_SET(rpic->data.icw4, VPIC_ICW4_AEOI)) {
                    rpic->data.ocw2 = *io_value;
                }
                break;
            case 0x00:
                /* 000: Clear (Rotate Priorities in Auto EOI Mode) */
                if (PIC_BIT_IS_SET(rpic->data.icw4, VPIC_ICW4_AEOI)) {
                    rpic->data.ocw2 = *io_value;
                }
                /* Bug in easyVM (0x00 ?= 0x20) */
                break;
            case 0x20:
                /* 001: Non-specific EOI Command */
                /* Set bit of highest priority interrupt in ISR to 0,
                 IR0 > IR1 > IR2(IR8 > ... > IR15) > IR3 > ... > IR7 */
                rpic->data.ocw2 = *io_value;
                id = pic_eoi_service(rpic);
                if (id != VPIC_MAX_IRQ_COUNT) {
                    PIC_BIT_CLEAR(rpic->data.isr, VPIC_ISR_IRQ(id));
                }
                break;
            case 0x60:
                /* 011: Specific EOI Command */
                rpic->data.ocw2 = *io_value;
                if (rpic->data.isr) {
                    /* Get L2,L1,L0 */
                    id = rpic->data.ocw2 & VPIC_OCW2_L;
                    PIC_BIT_CLEAR(rpic->data.isr, VPIC_ISR_IRQ(id));
                }
                /* Bug in easyVM: "isr &= (1 << i)" */
                break;
            case 0xa0:
                /* 101: Rotate Priorities on Non-specific EOI */
                rpic->data.ocw2 = *io_value;
                id = pic_eoi_service(rpic);
                if (id != VPIC_MAX_IRQ_COUNT) {
                    PIC_BIT_CLEAR(rpic->data.isr, VPIC_ISR_IRQ(id));
                    rpic->data.irx = (id + 1) % VPIC_MAX_IRQ_COUNT;
                }
                break;
            case 0xe0:
                /* 111: Rotate Priority on Specific EOI Command */
                rpic->data.ocw2 = *io_value;
                if (rpic->data.isr) {
                    id = rpic->data.ocw2 & VPIC_OCW2_L;
                    PIC_BIT_CLEAR(rpic->data.isr, VPIC_ISR_IRQ(id));
                    rpic->data.irx = ((rpic->data.ocw2 & VPIC_OCW2_L) + 1) % VPIC_MAX_IRQ_COUNT;
                }
                break;
            case 0xc0:
                /* 110: Set Priority (does not reset current ISR bit) */
                rpic->data.ocw2 = *io_value;
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
static void io_read_00x1(x86_pic *rpic, lib_u8 *io_value) {
    *io_value = rpic->data.imr;
}
/*
 * io_write_00x1
 * PIC get ICW2, ICW3, ICW4, OCW1 after ICW1
 */
static void io_write_00x1(x86_pic *rpic, lib_u8 *io_value) {
    lib_u8 previous_imr;
    lib_u8 released;
    lib_u8 id;

    switch (rpic->data.status) {
    case ICW2:
        rpic->data.icw2 = *io_value & VPIC_ICW2_VALID;
        if (!PIC_BIT_IS_SET(rpic->data.icw1, VPIC_ICW1_SNGL)) {
            /* ICW1.SNGL=0, ICW3=1 */
            rpic->data.status = ICW3;
        } else if (PIC_BIT_IS_SET(rpic->data.icw1, VPIC_ICW1_IC4)) {
            /* ICW1.SNGL=1, IC4=1 */
            rpic->data.status = ICW4;
        } else {
            /* ICW1.SNGL=1, IC4=0 */
            rpic->data.status = OCW1;
        }
        break;
    case ICW3:
        rpic->data.icw3 = *io_value;
        if (PIC_BIT_IS_SET(rpic->data.icw1, VPIC_ICW1_IC4)) {
            /* ICW1.IC4=1 */
            rpic->data.status = ICW4;
        } else {
            rpic->data.status = OCW1;
        }
        break;
    case ICW4:
        rpic->data.icw4 = *io_value & VPIC_ICW4_VALID;
        if (PIC_BIT_IS_SET(rpic->data.icw4, VPIC_ICW4_uPM)) {
            /* uPM=1, 16-bit 80x86 */
        } else {
            /* uPM=0, 8-bit 8080/8085 */
        }
        if (PIC_BIT_IS_SET(rpic->data.icw4, VPIC_ICW4_AEOI)) {
            /* AEOI=1, Automatic End of Interrupt */
        } else {
            /* AEOI=0, Non-automatic End of Interrupt */
        }
        if (PIC_BIT_IS_SET(rpic->data.icw4, VPIC_ICW4_BUF)) {
            /* BUF=1, Buffer */
            if (PIC_BIT_IS_SET(rpic->data.icw4, VPIC_ICW4_MS)) {
                /* M/S=1, Master 8259A */
            } else {
                /* M/S=0, Slave 8259A */
            }
        } else {
            /* BUF=0, Non-buffer */
        }
        if (PIC_BIT_IS_SET(rpic->data.icw4, VPIC_ICW4_SFNM)) {
            /* SFNM=1, Special Fully Nested Mode */
        } else {
            /* SFNM=0, Non-special Fully Nested Mode */
        }
        rpic->data.status = OCW1;
        break;
    case OCW1:
        previous_imr = rpic->data.imr;
        rpic->data.imr = *io_value;
        released = previous_imr & ~rpic->data.imr &
            pic_pending_requests(rpic);
        for (id = 0u; id < VPIC_MAX_IRQ_COUNT; ++id) {
            if (PIC_BIT_IS_SET(released, VPIC_IRR_IRQ(id))) {
                rpic->data.unmask_remaining_ticks[id] =
                    rpic->data.unmask_delivery_ticks[id];
            }
        }
        break;
    default:
        break;
    }
}

static void pic_advance_one(x86_pic *pic, lib_u64 elapsed_ticks)
{
    lib_u8 id;

    if (pic == LIB_NULL || elapsed_ticks == 0u) return;
    for (id = 0u; id < VPIC_MAX_IRQ_COUNT; ++id) {
        if (elapsed_ticks >= pic->data.unmask_remaining_ticks[id]) {
            pic->data.unmask_remaining_ticks[id] = 0u;
        } else {
            pic->data.unmask_remaining_ticks[id] -= elapsed_ticks;
        }
    }
}

static lib_status pic_ticks_until_one(const x86_pic *pic,
    lib_u64 *io_ticks)
{
    lib_u8 id;
    lib_u8 pending;

    if (pic == LIB_NULL || io_ticks == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    pending = pic_pending_requests(pic) & ~pic->data.imr;
    for (id = 0u; id < VPIC_MAX_IRQ_COUNT; ++id) {
        if (PIC_BIT_IS_SET(pending, VPIC_IRR_IRQ(id)) &&
            pic->data.unmask_remaining_ticks[id] != 0u &&
            pic->data.unmask_remaining_ticks[id] < *io_ticks) {
            *io_ticks = pic->data.unmask_remaining_ticks[id];
        }
    }
    return LIB_STATUS_OK;
}


lib_status x86_pic_create(lib_bool cascade_master, x86_pic **out_pic)
{
    x86_pic *pic;
    if (out_pic == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_pic = LIB_NULL;
    pic = lib_allocate_zero(1u, sizeof(*pic));
    if (pic == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    pic->cascade_master = cascade_master;
    x86_pic_reset(pic);
    *out_pic = pic;
    return LIB_STATUS_OK;
}

void x86_pic_destroy(x86_pic *pic)
{
    lib_release(pic);
}

void x86_pic_reset(x86_pic *pic)
{
    lib_u32 timing[VPIC_MAX_IRQ_COUNT];
    if (pic == LIB_NULL) return;
    lib_memory_copy(timing, pic->data.unmask_delivery_ticks, sizeof(timing));
    lib_memory_set(&pic->data, 0u, sizeof(pic->data));
    lib_memory_copy(pic->data.unmask_delivery_ticks, timing, sizeof(timing));
    pic->data.status = ICW1;
    pic->data.ocw3 = VPIC_OCW3_RR;
    pic->input_levels = 0u;
}

void x86_pic_read_register(x86_pic *pic, lib_u8 selector, lib_u8 *io_value)
{
    if (pic == LIB_NULL || io_value == LIB_NULL) return;
    if (selector == 0u) io_read_00x0(pic, io_value);
    else if (selector == 1u) io_read_00x1(pic, io_value);
}

void x86_pic_write_register(x86_pic *pic, lib_u8 selector, lib_u8 value)
{
    if (pic == LIB_NULL) return;
    if (selector == 0u) io_write_00x0(pic, &value);
    else if (selector == 1u) io_write_00x1(pic, &value);
}

void x86_pic_set_inputs(x86_pic *pic, lib_u8 levels,
    lib_u8 asserted_requests, lib_u8 cascade_lines)
{
    if (pic == LIB_NULL) return;
    if (pic_is_level(pic)) {
        pic->data.irr &= ~(pic->input_levels & ~levels);
        pic->data.irr |= levels;
    }
    pic->input_levels = levels;
    pic->data.irr |= asserted_requests;
    pic->data.cascade_irr = !pic->cascade_master ||
        PIC_BIT_IS_SET(pic->data.icw1, VPIC_ICW1_SNGL) ?
        0u : (lib_u8)(cascade_lines & pic->data.icw3);
}

lib_bool x86_pic_cascade_address(const x86_pic *pic, lib_u8 *out_address)
{
    if (pic == LIB_NULL || out_address == LIB_NULL ||
        PIC_BIT_IS_SET(pic->data.icw1, VPIC_ICW1_SNGL)) return LIB_FALSE;
    *out_address = pic->data.icw3 & 0x07u;
    return LIB_TRUE;
}

lib_status x86_pic_select(const x86_pic *pic, x86_pic_request *out_request)
{
    lib_u8 offset;
    lib_u8 id;
    lib_bool cascade;
    if (pic == LIB_NULL || out_request == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!pic->cascade_master) {
        if (!pic_select_controller(pic, &id)) return LIB_STATUS_INVALID_STATE;
        out_request->line = id;
        out_request->vector = (lib_u8)(id | pic->data.icw2);
        out_request->cascade = LIB_FALSE;
        return LIB_STATUS_OK;
    }
    for (offset = 0u; offset < VPIC_MAX_IRQ_COUNT; ++offset) {
        id = (lib_u8)((pic->data.irx + offset) % VPIC_MAX_IRQ_COUNT);
        if (pic->data.unmask_remaining_ticks[id] != 0u ||
            !PIC_BIT_IS_SET(pic_pending_requests(pic) & ~pic->data.imr,
                VPIC_IRR_IRQ(id))) continue;
        cascade = PIC_BIT_IS_SET(pic->data.cascade_irr, VPIC_IRR_IRQ(id));
        if (!pic_request_can_interrupt(pic, id) &&
            !(cascade && PIC_BIT_IS_SET(pic->data.icw4, VPIC_ICW4_SFNM) &&
                GetRegTopId(pic, pic->data.isr) == id)) continue;
        if (!cascade && !PIC_BIT_IS_SET(pic->data.icw1, VPIC_ICW1_SNGL) &&
            PIC_BIT_IS_SET(pic->data.icw3, VPIC_ICW3_S(id))) continue;
        out_request->line = id;
        out_request->vector = (lib_u8)(id | pic->data.icw2);
        out_request->cascade = cascade;
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_INVALID_STATE;
}

x86_pic_request x86_pic_acknowledge(x86_pic *pic)
{
    x86_pic_request request = { 7u, 0u, LIB_FALSE };
    if (pic == LIB_NULL) return request;
    if (x86_pic_select(pic, &request) == LIB_STATUS_OK) {
        RespondINTR(pic, request.line, request.cascade);
    } else if (pic->data.status == OCW1) {
        request.vector = (lib_u8)(pic->data.icw2 | 7u);
    }
    return request;
}

void x86_pic_set_irq_timing(x86_pic *pic, const lib_u32 ticks[8])
{
    if (pic == LIB_NULL || ticks == LIB_NULL) return;
    lib_memory_copy(pic->data.unmask_delivery_ticks, ticks,
        sizeof(pic->data.unmask_delivery_ticks));
}

void x86_pic_advance(x86_pic *pic, lib_u64 elapsed_ticks)
{
    pic_advance_one(pic, elapsed_ticks);
}

lib_status x86_pic_ticks_until_event(const x86_pic *pic, lib_u64 *out_ticks)
{
    lib_u64 ticks = LIB_UINT64_MAX;
    if (pic == LIB_NULL || out_ticks == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    (void)pic_ticks_until_one(pic, &ticks);
    if (ticks == LIB_UINT64_MAX) return LIB_STATUS_INVALID_STATE;
    *out_ticks = ticks;
    return LIB_STATUS_OK;
}

/* Copyright 2012-2026 Neko. */
#include "x86/devices/dma8237/dma.h"

void x86_dma_reset(x86_dma *rdma) {
    lib_memory_set((void *)(&rdma->data), 0u, sizeof(x86_dma_data));
    rdma->data.mask = VDMA_MASK_VALID;
}


void x86_dma_grant(x86_dma *dma, lib_u8 channel, lib_bool cascade)
{
    VDMA_SetISR(dma->data.isr, channel);
    DMA_BIT_SET(dma->data.acknowledged, VDMA_REQUEST_DRQ(channel));
    dma->data.phase = channel == 0u && DMA_BIT_IS_SET(dma->data.command,
        VDMA_COMMAND_M2M) ? VDMA_PHASE_S11 : VDMA_PHASE_S1;
    if (cascade && DMA_BIT_IS_SET(dma->data.command, VDMA_COMMAND_R)) {
        dma->data.drx = (channel + 1u) % VDMA_CHANNEL_COUNT;
    }
}

void x86_dma_release(x86_dma *dma)
{
    if (DMA_BIT_IS_SET(dma->data.isr, VDMA_ISR_IS)) {
        DMA_BIT_CLEAR(dma->data.acknowledged,
            VDMA_REQUEST_DRQ(VDMA_GetISR_ISR(dma->data.isr)));
    }
    dma->data.flagM2MWrite = LIB_FALSE;
    dma->data.phase = VDMA_PHASE_IDLE;
    dma->data.isr = 0u;
}


static void dma_read_address(x86_dma *dma, lib_u8 *io_value, lib_u8 channel)
{
    *io_value = !dma->data.flagMSB ?
        DMA_MASK_U8(dma->data.currAddr[channel]) :
        DMA_MASK_U8(dma->data.currAddr[channel] >> 8);
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_read_count(x86_dma *dma, lib_u8 *io_value, lib_u8 channel)
{
    *io_value = !dma->data.flagMSB ?
        DMA_MASK_U8(dma->data.currCount[channel]) :
        DMA_MASK_U8(dma->data.currCount[channel] >> 8);
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_write_address(x86_dma *dma, lib_u8 value, lib_u8 channel)
{
    if (!dma->data.flagMSB) {
        dma->data.baseAddr[channel] = DMA_MASK_U16(value);
    } else {
        dma->data.baseAddr[channel] |= DMA_MASK_U16(value << 8);
    }
    dma->data.currAddr[channel] = dma->data.baseAddr[channel];
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_write_count(x86_dma *dma, lib_u8 value, lib_u8 channel)
{
    if (!dma->data.flagMSB) {
        dma->data.baseCount[channel] = DMA_MASK_U16(value);
    } else {
        dma->data.baseCount[channel] |= DMA_MASK_U16(value << 8);
    }
    dma->data.currCount[channel] = dma->data.baseCount[channel];
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_write_request(x86_dma *dma, lib_u8 value)
{
    DMA_BIT_MAKE(dma->data.request,
        VDMA_REQUEST_DRQ(VDMA_GetREQSC_CS(value)),
        DMA_BIT_IS_SET(value, VDMA_REQSC_SR));
}

static void dma_write_mask(x86_dma *dma, lib_u8 value)
{
    DMA_BIT_MAKE(dma->data.mask,
        VDMA_MASK_DRQ(VDMA_GetMASKSC_CS(value)),
        DMA_BIT_IS_SET(value, VDMA_MASKSC_SM));
}


static lib_u8 GetRegTopId(const x86_dma *rdma, lib_u8 reg) {
    lib_u8 id = 0;
    if (reg == 0u) {
        return 0x08;
    }
    reg = (reg << (VDMA_CHANNEL_COUNT - (rdma->data.drx))) | (reg >> (rdma->data.drx));
    while ((id < VDMA_CHANNEL_COUNT) && !((reg >> id) & 1u)) {
        id++;
    }
    return (id + rdma->data.drx) % VDMA_CHANNEL_COUNT;
}

static lib_bool dma_software_request_is_valid(const x86_dma *dma,
    lib_u8 channel)
{
    return VDMA_GetREQUEST_DRQ(dma->data.request, channel) &&
        (VDMA_GetMODE_M(dma->data.mode[channel]) == 0x02u ||
            (channel == 0u && DMA_BIT_IS_SET(dma->data.command,
                VDMA_COMMAND_M2M)));
}

static lib_u8 dma_pending_requests(const x86_dma *dma)
{
    lib_u8 pending = VDMA_GetSTATUS_DRQS(dma->data.status) &
        (lib_u8)~dma->data.mask;
    lib_u8 channel;

    for (channel = 0u; channel < VDMA_CHANNEL_COUNT; ++channel) {
        if (dma_software_request_is_valid(dma, channel)) {
            DMA_BIT_SET(pending, VDMA_REQUEST_DRQ(channel));
        }
    }
    return pending;
}

static void IncreaseCurrAddr(x86_dma *rdma, lib_u8 id) {
    rdma->data.currAddr[id]++;
}
static void DecreaseCurrAddr(x86_dma *rdma, lib_u8 id) {
    rdma->data.currAddr[id]--;
}


lib_status x86_dma_create(x86_dma **out_dma)
{
    x86_dma *dma;
    if (out_dma == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_dma = LIB_NULL;
    dma = lib_allocate_zero(1u, sizeof(*dma));
    if (dma == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    x86_dma_reset(dma);
    *out_dma = dma;
    return LIB_STATUS_OK;
}

void x86_dma_destroy(x86_dma *dma)
{
    lib_release(dma);
}

void x86_dma_read_register(x86_dma *dma, lib_u8 selector, lib_u8 *io_value)
{
    if (selector < 8u) {
        if ((selector & 1u) == 0u) dma_read_address(dma, io_value, selector >> 1);
        else dma_read_count(dma, io_value, selector >> 1);
    } else if (selector == 8u) {
        *io_value = dma->data.status;
        DMA_BIT_CLEAR(dma->data.status, VDMA_STATUS_TCS);
    } else if (selector == 13u) {
        *io_value = dma->data.temp;
    }
}

void x86_dma_write_register(x86_dma *dma, lib_u8 selector, lib_u8 value)
{
    if (selector < 8u) {
        if ((selector & 1u) == 0u) dma_write_address(dma, value, selector >> 1);
        else dma_write_count(dma, value, selector >> 1);
        return;
    }
    switch (selector) {
    case 8u: dma->data.command = value; break;
    case 9u: dma_write_request(dma, value); break;
    case 10u: dma_write_mask(dma, value); break;
    case 11u: dma->data.mode[VDMA_GetMODE_CS(value)] = value; break;
    case 12u: dma->data.flagMSB = LIB_FALSE; break;
    case 13u: x86_dma_reset(dma); break;
    case 14u: dma->data.mask = 0u; break;
    case 15u: dma->data.mask = value & VDMA_MASKAC_VALID; break;
    default: break;
    }
}

void x86_dma_set_request(x86_dma *dma, lib_u8 channel, lib_bool asserted)
{
    if (channel < VDMA_CHANNEL_COUNT) {
        DMA_BIT_MAKE(dma->data.status, VDMA_STATUS_DRQ(channel), asserted);
    }
}

void x86_dma_terminate(x86_dma *dma)
{
    dma->data.flagEOP = LIB_TRUE;
}

x86_dma_signals x86_dma_get_signals(const x86_dma *dma)
{
    x86_dma_signals signals;
    signals.enabled = !DMA_BIT_IS_SET(dma->data.command, VDMA_COMMAND_CTRL);
    signals.requests = dma_pending_requests(dma);
    signals.active_channel = DMA_BIT_IS_SET(dma->data.isr, VDMA_ISR_IS) ?
        VDMA_GetISR_ISR(dma->data.isr) : VDMA_CHANNEL_COUNT;
    return signals;
}

lib_u8 x86_dma_select(const x86_dma *dma, lib_u8 requests)
{
    requests &= VDMA_MASK_VALID;
    return requests ? GetRegTopId(dma, requests) : VDMA_CHANNEL_COUNT;
}
static lib_bool Transmission(x86_dma *dma, const x86_dma_bus *bus,
    lib_u8 channel)
{
    lib_u8 kind = VDMA_GetMODE_TT(dma->data.mode[channel]);
    lib_u8 byte = 0u;

    if (kind == 3u || bus->cycle(bus->context, (x86_dma_cycle)kind, channel,
            dma->data.currAddr[channel], &byte) != LIB_STATUS_OK) {
        return LIB_FALSE;
    }
    dma->data.currCount[channel]--;
    if (DMA_BIT_IS_SET(dma->data.mode[channel], VDMA_MODE_AIDS)) {
        DecreaseCurrAddr(dma, channel);
    } else {
        IncreaseCurrAddr(dma, channel);
    }
    return LIB_TRUE;
}

static void dma_complete_transfer(x86_dma *dma, const x86_dma_bus *bus,
    lib_u8 channel, lib_bool memory_to_memory, lib_bool terminal_count)
{
    lib_u8 first = memory_to_memory ? 0u : channel;
    lib_u8 last = memory_to_memory ? 1u : channel;
    lib_u8 index;

    x86_dma_release(dma);
    for (index = first; index <= last; ++index) {
        DMA_BIT_CLEAR(dma->data.request, VDMA_REQUEST_DRQ(index));
        if (bus->terminal != LIB_NULL) {
            bus->terminal(bus->context, index);
        }
        if (DMA_BIT_IS_SET(dma->data.mode[index], VDMA_MODE_AI)) {
            dma->data.currAddr[index] = dma->data.baseAddr[index];
            dma->data.currCount[index] = dma->data.baseCount[index];
            DMA_BIT_CLEAR(dma->data.mask, VDMA_MASK_DRQ(index));
        } else {
            DMA_BIT_SET(dma->data.mask, VDMA_MASK_DRQ(index));
        }
    }
    if (terminal_count) {
        DMA_BIT_SET(dma->data.status,
            VDMA_STATUS_TC(memory_to_memory ? 1u : channel));
    }
}

static void Execute(x86_dma *rdma, const x86_dma_bus *bus, lib_u8 id) {
    lib_bool flagM2M = ((id == 0) &&
                      VDMA_GetREQUEST_DRQ(rdma->data.request, 0) &&
                      DMA_BIT_IS_SET(rdma->data.command, VDMA_COMMAND_M2M));
    lib_bool request_asserted = VDMA_GetSTATUS_DRQ(rdma->data.status, id);
    if (DMA_BIT_IS_SET(rdma->data.command, VDMA_COMMAND_R)) {
        rdma->data.drx = (id + 1) % VDMA_CHANNEL_COUNT;
    }
    if (flagM2M) {
        /* Memory-to-memory is two logical services: channel 0 reads the
         * temporary register first, then channel 1 writes it. */
        if (rdma->data.currCount[1] != 0xffff && !rdma->data.flagEOP) {
            lib_u8 byte = rdma->data.temp;

            if (!rdma->data.flagM2MWrite) {
                if (bus->cycle(bus->context, X86_DMA_MEMORY_READ, 0u,
                        rdma->data.currAddr[0u], &byte) != LIB_STATUS_OK) {
                    x86_dma_release(rdma);
                    return;
                }
                rdma->data.temp = byte;
                rdma->data.flagM2MWrite = LIB_TRUE;
                return;
            }
            if (bus->cycle(bus->context, X86_DMA_MEMORY_WRITE, 1u,
                    rdma->data.currAddr[1u], &byte) != LIB_STATUS_OK) {
                x86_dma_release(rdma);
                return;
            }
            rdma->data.flagM2MWrite = LIB_FALSE;
            rdma->data.currCount[1]--;
            if (DMA_BIT_IS_SET(rdma->data.mode[1u], VDMA_MODE_AIDS)) {
                DecreaseCurrAddr(rdma, 1u);
            } else {
                IncreaseCurrAddr(rdma, 1u);
            }
            if (!DMA_BIT_IS_SET(rdma->data.command, VDMA_COMMAND_C0AD)) {
                if (DMA_BIT_IS_SET(rdma->data.mode[0u], VDMA_MODE_AIDS)) {
                    DecreaseCurrAddr(rdma, 0u);
                } else {
                    IncreaseCurrAddr(rdma, 0u);
                }
            }
        }
        if (rdma->data.currCount[1] == 0xffffu) {
            rdma->data.flagEOP = LIB_TRUE;
        }
    } else {
        /* select mode and command */
        switch (VDMA_GetMODE_M(rdma->data.mode[id])) {
        case 0x00:
            /* demand */
            if (request_asserted && rdma->data.currCount[id] !=
                0xffffu && !rdma->data.flagEOP) {
                if (!Transmission(rdma, bus, id)) {
                    x86_dma_release(rdma);
                    return;
                }
            }
            if (!rdma->data.flagEOP && !request_asserted) x86_dma_release(rdma);
            break;
        case 0x01:
            /* single */
            if (!Transmission(rdma, bus, id)) {
                x86_dma_release(rdma);
                return;
            }
            if (!rdma->data.flagEOP) x86_dma_release(rdma);
            break;
        case 0x02:
            /* block */
            if (rdma->data.currCount[id] != 0xffffu &&
                !rdma->data.flagEOP) {
                if (!Transmission(rdma, bus, id)) {
                    x86_dma_release(rdma);
                    return;
                }
            }
            break;
        case 0x03:
            /* Cascade releases without a local transfer. The board services
             * the downstream controller while retaining the cascade grant. */
            x86_dma_release(rdma);
            return;
        default:
            break;
        }
        if (rdma->data.currCount[id] == 0xffffu) {
            rdma->data.flagEOP = LIB_TRUE;
        }
    }
    if (rdma->data.flagEOP) {
        dma_complete_transfer(rdma, bus, id, flagM2M,
            rdma->data.currCount[flagM2M ? 1u : id] == 0xffffu ||
            !DMA_BIT_IS_SET(rdma->data.mode[flagM2M ? 1u : id], VDMA_MODE_AI));
    }
    rdma->data.flagEOP = LIB_FALSE;
}

static lib_bool dma_address_high_changed(lib_u16 before,
    lib_u16 after)
{
    return (before & 0xff00u) != (after & 0xff00u);
}

void x86_dma_advance(x86_dma *dma, const x86_dma_bus *bus)
{
    lib_u8 channel;
    lib_u16 source_before;
    lib_u16 channel_before;

    if (!DMA_BIT_IS_SET(dma->data.isr, VDMA_ISR_IS)) return;
    channel = VDMA_GetISR_ISR(dma->data.isr);
    switch (dma->data.phase) {
    case VDMA_PHASE_S1:
        dma->data.phase = VDMA_PHASE_S2;
        break;
    case VDMA_PHASE_S2:
        dma->data.phase = DMA_BIT_IS_SET(dma->data.command, VDMA_COMMAND_TM) ?
            VDMA_PHASE_S4 : VDMA_PHASE_S3;
        break;
    case VDMA_PHASE_S3:
        dma->data.phase = VDMA_PHASE_S4;
        break;
    case VDMA_PHASE_S4:
        channel_before = dma->data.currAddr[channel];
        Execute(dma, bus, channel);
        if (DMA_BIT_IS_SET(dma->data.isr, VDMA_ISR_IS)) {
            dma->data.phase = dma_address_high_changed(channel_before,
                dma->data.currAddr[channel]) ? VDMA_PHASE_S1 : VDMA_PHASE_S2;
        }
        break;
    case VDMA_PHASE_S11:
        dma->data.phase = VDMA_PHASE_S12;
        break;
    case VDMA_PHASE_S12:
        dma->data.phase = VDMA_PHASE_S13;
        break;
    case VDMA_PHASE_S13:
        dma->data.phase = VDMA_PHASE_S14;
        break;
    case VDMA_PHASE_S14:
        Execute(dma, bus, channel);
        if (DMA_BIT_IS_SET(dma->data.isr, VDMA_ISR_IS) && dma->data.flagM2MWrite) {
            dma->data.phase = VDMA_PHASE_S21;
        }
        break;
    case VDMA_PHASE_S21:
        dma->data.phase = VDMA_PHASE_S22;
        break;
    case VDMA_PHASE_S22:
        dma->data.phase = VDMA_PHASE_S23;
        break;
    case VDMA_PHASE_S23:
        dma->data.phase = VDMA_PHASE_S24;
        break;
    case VDMA_PHASE_S24:
        source_before = dma->data.currAddr[0u];
        Execute(dma, bus, channel);
        if (DMA_BIT_IS_SET(dma->data.isr, VDMA_ISR_IS)) {
            dma->data.phase = dma_address_high_changed(source_before,
                dma->data.currAddr[0u]) ? VDMA_PHASE_S11 : VDMA_PHASE_S12;
        }
        break;
    default:
        x86_dma_release(dma);
        break;
    }
}

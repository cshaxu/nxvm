/* Copyright 2012-2014 Neko. */

/* VDMA implements two chips of Direct Memory Access Controller: Intel 8237A (Master+Slave). */

#include "type.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"
#include "core/machine/dma.h"

static C_VOID doReset(t_dma *rdma) {
    STD_MEMSET((C_VOID *)(&rdma->data), TYPE_ZERO_8, sizeof(t_dma_data));
    rdma->data.mask = VDMA_MASK_VALID;
}

static C_VOID dma_read_address(t_dma *dma, t_port *port, type_unsigned_8 channel)
{
    port->data.ioByte = !dma->data.flagMSB ?
        TYPE_MASK_UNSIGNED_8(dma->data.currAddr[channel]) :
        TYPE_MASK_UNSIGNED_8(dma->data.currAddr[channel] >> 8);
    dma->data.flagMSB = !dma->data.flagMSB;
}

static C_VOID dma_read_count(t_dma *dma, t_port *port, type_unsigned_8 channel)
{
    port->data.ioByte = !dma->data.flagMSB ?
        TYPE_MASK_UNSIGNED_8(dma->data.currCount[channel]) :
        TYPE_MASK_UNSIGNED_8(dma->data.currCount[channel] >> 8);
    dma->data.flagMSB = !dma->data.flagMSB;
}

static C_VOID dma_write_address(t_dma *dma, t_port *port, type_unsigned_8 channel)
{
    if (!dma->data.flagMSB) {
        dma->data.baseAddr[channel] = TYPE_MASK_UNSIGNED_16(port->data.ioByte);
    } else {
        dma->data.baseAddr[channel] |= TYPE_MASK_UNSIGNED_16(port->data.ioByte << 8);
    }
    dma->data.currAddr[channel] = dma->data.baseAddr[channel];
    dma->data.flagMSB = !dma->data.flagMSB;
}

static C_VOID dma_write_count(t_dma *dma, t_port *port, type_unsigned_8 channel)
{
    if (!dma->data.flagMSB) {
        dma->data.baseCount[channel] = TYPE_MASK_UNSIGNED_16(port->data.ioByte);
    } else {
        dma->data.baseCount[channel] |= TYPE_MASK_UNSIGNED_16(port->data.ioByte << 8);
    }
    dma->data.currCount[channel] = dma->data.baseCount[channel];
    dma->data.flagMSB = !dma->data.flagMSB;
}

static C_VOID dma_write_request(t_dma *dma, t_port *port)
{
    TYPE_MAKE_BIT(dma->data.request,
        VDMA_REQUEST_DRQ(VDMA_GetREQSC_CS(port->data.ioByte)),
        TYPE_GET_BIT(port->data.ioByte, VDMA_REQSC_SR));
}

static C_VOID dma_write_mask(t_dma *dma, t_port *port)
{
    TYPE_MAKE_BIT(dma->data.mask,
        VDMA_MASK_DRQ(VDMA_GetMASKSC_CS(port->data.ioByte)),
        TYPE_GET_BIT(port->data.ioByte, VDMA_MASKSC_SM));
}

static t_dma *dma_controller(t_dma *primary, type_unsigned_16 port_id)
{
    return port_id >= 0x00c0u ? primary->connect.peer : primary;
}

static type_unsigned_8 dma_page_channel(type_unsigned_16 port_id)
{
    switch (port_id) {
    case 0x0081: case 0x0089: return 2u;
    case 0x0082: case 0x008a: return 3u;
    case 0x0083: case 0x008b: return 1u;
    default: return 0u;
    }
}

static C_VOID dma_port_read(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{
    t_dma *primary = (t_dma *)owner;
    t_dma *dma;
    type_unsigned_8 channel;

    if (primary == STD_NULL) return;
    if (port_id <= 0x0007u) {
        channel = (type_unsigned_8)(port_id >> 1);
        if ((port_id & 1u) == 0u) dma_read_address(primary, port, channel);
        else dma_read_count(primary, port, channel);
        return;
    }
    if (port_id == 0x0008u) {
        port->data.ioByte = primary->data.status;
        TYPE_CLEAR_BIT(primary->data.status, VDMA_STATUS_TCS);
        return;
    }
    if (port_id == 0x000du) {
        port->data.ioByte = primary->data.temp;
        return;
    }
    if (port_id >= 0x0081u && port_id <= 0x008fu) {
        dma = dma_controller(primary, port_id);
        port->data.ioByte = dma->data.page[dma_page_channel(port_id)];
        return;
    }
    if (port_id >= 0x00c0u && port_id <= 0x00ceu &&
        (port_id & 1u) == 0u) {
        dma = primary->connect.peer;
        channel = (type_unsigned_8)((port_id - 0x00c0u) >> 1);
        if ((channel & 1u) == 0u) dma_read_address(dma, port, channel >> 1);
        else dma_read_count(dma, port, channel >> 1);
        return;
    }
    if (port_id == 0x00d0u) {
        port->data.ioByte = primary->connect.peer->data.status;
        TYPE_CLEAR_BIT(primary->connect.peer->data.status, VDMA_STATUS_TCS);
        return;
    }
    if (port_id == 0x00dau) {
        port->data.ioByte = primary->connect.peer->data.temp;
    }
}

static C_VOID dma_port_write(t_port *port, type_unsigned_16 port_id, C_VOID *owner)
{
    t_dma *primary = (t_dma *)owner;
    t_dma *dma;
    type_unsigned_8 channel;
    type_unsigned_16 local_port;

    if (primary == STD_NULL) return;
    if (port_id <= 0x0007u) {
        channel = (type_unsigned_8)(port_id >> 1);
        if ((port_id & 1u) == 0u) dma_write_address(primary, port, channel);
        else dma_write_count(primary, port, channel);
        return;
    }
    if (port_id >= 0x0081u && port_id <= 0x008fu) {
        dma = dma_controller(primary, port_id);
        dma->data.page[dma_page_channel(port_id)] = port->data.ioByte &
            (port_id < 0x0089u ? TYPE_MAX_UNSIGNED_8 : 0xfeu);
        return;
    }
    dma = port_id >= 0x00c0u ? primary->connect.peer : primary;
    local_port = port_id >= 0x00c0u ? port_id - 0x00c0u : port_id;
    if (port_id >= 0x00c0u && (local_port & 1u) == 0u &&
        local_port <= 0x000eu) {
        channel = (type_unsigned_8)(local_port >> 1);
        if ((channel & 1u) == 0u) dma_write_address(dma, port, channel >> 1);
        else dma_write_count(dma, port, channel >> 1);
        return;
    }
    if (port_id >= 0x00c0u && local_port >= 0x0010u) {
        /* The second 8237A occupies every other I/O address. Its D0h--DEh
         * control family maps to the first controller's 08h--0Fh functions. */
        local_port = (type_unsigned_16)(0x0008u +
            ((local_port - 0x0010u) >> 1));
    }
    switch (local_port) {
    case 0x0008: dma->data.command = port->data.ioByte; break;
    case 0x0009: dma_write_request(dma, port); break;
    case 0x000a: dma_write_mask(dma, port); break;
    case 0x000b:
        dma->data.mode[VDMA_GetMODE_CS(port->data.ioByte)] = port->data.ioByte;
        break;
    case 0x000c: dma->data.flagMSB = TYPE_FALSE; break;
    case 0x000d: doReset(dma); break;
    case 0x000e: dma->data.mask = TYPE_ZERO_8; break;
    case 0x000f: dma->data.mask = port->data.ioByte & VDMA_MASKAC_VALID; break;
    default: break;
    }
}

static type_unsigned_8 GetRegTopId(t_dma *rdma, type_unsigned_8 reg) {
    type_unsigned_8 id = 0;
    if (reg == TYPE_ZERO_8) {
        return 0x08;
    }
    reg = (reg << (VDMA_CHANNEL_COUNT - (rdma->data.drx))) | (reg >> (rdma->data.drx));
    while ((id < VDMA_CHANNEL_COUNT) && !TYPE_MASK_UNSIGNED_1(reg >> id)) {
        id++;
    }
    return (id + rdma->data.drx) % VDMA_CHANNEL_COUNT;
}
static C_VOID IncreaseCurrAddr(t_dma *rdma, type_unsigned_8 id) {
    rdma->data.currAddr[id]++;
    if (rdma->data.currAddr[id] == TYPE_ZERO_16) {
        rdma->data.page[id]++;
    }
}
static C_VOID DecreaseCurrAddr(t_dma *rdma, type_unsigned_8 id) {
    rdma->data.currAddr[id]--;
    if (rdma->data.currAddr[id] == TYPE_MAX_UNSIGNED_16) {
        rdma->data.page[id]--;
    }
}
static C_VOID Transmission(t_dma *rdma, t_latch *latch, t_ram *ram,
                         type_unsigned_8 id, type_bool flagWord) {
    switch (VDMA_GetMODE_TT(rdma->data.mode[id])) {
    case 0x00:
        /* verify */
        /* do nothing */
        rdma->data.currCount[id]--;
        if (TYPE_GET_BIT(rdma->data.mode[id], VDMA_MODE_AIDS)) {
            DecreaseCurrAddr(rdma, id);
        } else {
            IncreaseCurrAddr(rdma, id);
        }
        break;
    case 0x01:
        /* write */
        if (rdma->connect.read_provider[id] != STD_NULL) {
            rdma->connect.read_provider[id](rdma->connect.device_owner[id], latch);
        }
        if (!flagWord) {
            core_machine_memory_write_physical(ram,
                (rdma->data.page[id] << 16) + rdma->data.currAddr[id],
                (type_virtual_address)(&latch->data.byte), 1);
        } else {
            core_machine_memory_write_physical(ram,
                (rdma->data.page[id] << 16) + (rdma->data.currAddr[id] << 1),
                (type_virtual_address)(&latch->data.word), 2);
        }
        rdma->data.currCount[id]--;
        if (TYPE_GET_BIT(rdma->data.mode[id], VDMA_MODE_AIDS)) {
            DecreaseCurrAddr(rdma, id);
        } else {
            IncreaseCurrAddr(rdma, id);
        }
        break;
    case 0x02:
        /* read */
        if (!flagWord) {
            core_machine_memory_read_physical(ram,
                (rdma->data.page[id] << 16) + rdma->data.currAddr[id],
                (type_virtual_address)(&latch->data.byte), 1);
        } else {
            core_machine_memory_read_physical(ram,
                (rdma->data.page[id] << 16) + (rdma->data.currAddr[id] << 1),
                (type_virtual_address)(&latch->data.word), 2);
        }
        if (rdma->connect.write_provider[id] != STD_NULL) {
            rdma->connect.write_provider[id](rdma->connect.device_owner[id], latch);
        }
        rdma->data.currCount[id]--;
        if (TYPE_GET_BIT(rdma->data.mode[id], VDMA_MODE_AIDS)) {
            DecreaseCurrAddr(rdma, id);
        } else {
            IncreaseCurrAddr(rdma, id);
        }
        break;
    case 0x03:
        /* illegal */
        break;
    default:
        break;
    }
}
static C_VOID Execute(t_dma *rdma, t_latch *latch, t_ram *ram,
                    type_unsigned_8 id, type_bool flagWord) {
    type_bool flagM2M = ((id == 0) &&
                      VDMA_GetREQUEST_DRQ(rdma->data.request, 1) &&
                      TYPE_GET_BIT(rdma->data.command, VDMA_COMMAND_M2M));
    type_bool request_asserted = VDMA_GetSTATUS_DRQ(rdma->data.status, id);
    TYPE_CLEAR_BIT(rdma->data.status, VDMA_STATUS_DRQ(id));
    TYPE_CLEAR_BIT(rdma->data.request, VDMA_REQUEST_DRQ(id));
    if (TYPE_GET_BIT(rdma->data.command, VDMA_COMMAND_R)) {
        rdma->data.drx = (id + 1) % VDMA_CHANNEL_COUNT;
    }
    if (flagM2M) {
        /* memory-to-memory */
        if (rdma->data.currCount[1] != 0xffff && !rdma->data.flagEOP) {
            core_machine_memory_read_physical(ram,
                (rdma->data.page[0] << 16) + rdma->data.currAddr[0],
                (type_virtual_address)(&rdma->data.temp), 1);
            core_machine_memory_write_physical(ram,
                (rdma->data.page[1] << 16) + rdma->data.currAddr[1],
                (type_virtual_address)(&rdma->data.temp), 1);
            rdma->data.currCount[1]--;
            if (TYPE_GET_BIT(rdma->data.mode[id], VDMA_MODE_AIDS)) {
                DecreaseCurrAddr(rdma, 1);
                if (!TYPE_GET_BIT(rdma->data.command, VDMA_COMMAND_C0AD)) {
                    DecreaseCurrAddr(rdma, 0);
                }
            } else {
                IncreaseCurrAddr(rdma, 1);
                if (!TYPE_GET_BIT(rdma->data.command, VDMA_COMMAND_C0AD)) {
                    IncreaseCurrAddr(rdma, 0);
                }
            }
        }
        if (rdma->data.currCount[1] == TYPE_MAX_UNSIGNED_16) {
            TYPE_SET_BIT(rdma->data.status, VDMA_STATUS_TC(0));
            rdma->data.flagEOP = TYPE_TRUE;
        }
    } else {
        /* select mode and command */
        switch (VDMA_GetMODE_M(rdma->data.mode[id])) {
        case 0x00:
            /* demand */
            if (request_asserted && rdma->data.currCount[id] !=
                TYPE_MAX_UNSIGNED_16 && !rdma->data.flagEOP) {
                Transmission(rdma, latch, ram, id, flagWord);
            }
            if (!rdma->data.flagEOP) rdma->data.isr = TYPE_ZERO_8;
            break;
        case 0x01:
            /* single */
            Transmission(rdma, latch, ram, id, flagWord);
            if (!rdma->data.flagEOP) rdma->data.isr = TYPE_ZERO_8;
            break;
        case 0x02:
            /* block */
            if (rdma->data.currCount[id] != TYPE_MAX_UNSIGNED_16 &&
                !rdma->data.flagEOP) {
                Transmission(rdma, latch, ram, id, flagWord);
            }
            break;
        case 0x03:
            /* cascade */
            /* do nothing */
            rdma->data.flagEOP = TYPE_TRUE;
            break;
        default:
            break;
        }
        if (rdma->data.currCount[id] == TYPE_MAX_UNSIGNED_16) {
            TYPE_SET_BIT(rdma->data.status, VDMA_STATUS_TC(id)); /* set termination count */
            rdma->data.flagEOP = TYPE_TRUE;
        }
    }
    if (rdma->data.flagEOP) {
        rdma->data.isr = TYPE_ZERO_8;
        if (rdma->connect.close_provider[id] != STD_NULL) {
            rdma->connect.close_provider[id](rdma->connect.device_owner[id], latch);
        }
        if (TYPE_GET_BIT(rdma->data.mode[id], VDMA_MODE_AI)) {
            rdma->data.currAddr[id] = rdma->data.baseAddr[id];
            rdma->data.currCount[id] = rdma->data.baseCount[id];
            TYPE_CLEAR_BIT(rdma->data.mask, VDMA_MASK_DRQ(id));
        } else {
            TYPE_SET_BIT(rdma->data.mask, VDMA_MASK_DRQ(id));
        }
    }
    rdma->data.flagEOP = TYPE_FALSE;
}

static C_VOID core_machine_dma_set_drq(t_dma *primary, t_dma *secondary,
    type_unsigned_8 drq_id, type_bool asserted)
{
    if (primary == STD_NULL || secondary == STD_NULL) return;
    switch (drq_id) {
    case 0:
    case 1:
    case 2:
    case 3:
        TYPE_MAKE_BIT(primary->data.status, VDMA_STATUS_DRQ(drq_id), asserted);
        break;
    case 5:
    case 6:
    case 7:
        TYPE_MAKE_BIT(secondary->data.status, VDMA_STATUS_DRQ(drq_id - 4), asserted);
        break;
    case 4:
    default:
        break;
    }
    if (primary->data.status & VDMA_STATUS_DRQS) {
        TYPE_SET_BIT(secondary->data.status, VDMA_STATUS_DRQ(0));
    } else {
        TYPE_CLEAR_BIT(secondary->data.status, VDMA_STATUS_DRQ(0));
    }
}

type_status core_machine_dma_bind_channel(t_latch *latch, t_dma *primary,
    t_dma *secondary, uint8_t drq_id,
    const core_machine_dma_channel_provider *provider, C_VOID *owner,
    core_machine_dma_request_binding *out_binding)
{
    t_dma *dma;
    type_unsigned_8 channel;

    if (latch == STD_NULL || primary == STD_NULL || secondary == STD_NULL ||
        provider == STD_NULL || out_binding == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (drq_id <= 3u) {
        dma = primary;
        channel = drq_id;
    } else if (drq_id >= 5u && drq_id <= 7u) {
        dma = secondary;
        channel = drq_id - 4u;
    } else {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    if (dma->connect.latch != latch || dma->connect.peer == STD_NULL ||
        dma->connect.device_owner[channel] != STD_NULL) return TYPE_STATUS_INVALID_STATE;
    dma->connect.read_provider[channel] = provider->read_device;
    dma->connect.write_provider[channel] = provider->write_device;
    dma->connect.close_provider[channel] = provider->terminal_count;
    dma->connect.device_owner[channel] = owner;
    out_binding->core_owner = primary;
    out_binding->channel = drq_id;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_dma_request_assert(
    const core_machine_dma_request_binding *binding)
{
    t_dma *primary;

    if (binding == STD_NULL || binding->core_owner == STD_NULL) return;
    primary = (t_dma *)binding->core_owner;
    core_machine_dma_set_drq(primary, primary->connect.peer,
        binding->channel, TYPE_TRUE);
}

C_VOID core_machine_dma_request_deassert(
    const core_machine_dma_request_binding *binding)
{
    t_dma *primary;

    if (binding == STD_NULL || binding->core_owner == STD_NULL) return;
    primary = (t_dma *)binding->core_owner;
    core_machine_dma_set_drq(primary, primary->connect.peer,
        binding->channel, TYPE_FALSE);
}

C_VOID core_machine_dma_initialize(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_port *port)
{
    static const type_unsigned_16 primary_reads[] = {
        0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
        0x0008, 0x000d
    };
    static const type_unsigned_16 page_ports[] = {
        0x0081, 0x0082, 0x0083, 0x0087, 0x0089, 0x008a, 0x008b, 0x008f
    };
    static const type_unsigned_16 secondary_reads[] = {
        0x00c0, 0x00c2, 0x00c4, 0x00c6, 0x00c8, 0x00ca, 0x00cc, 0x00ce,
        0x00d0, 0x00da
    };
    type_native_unsigned index;

    if (latch == STD_NULL || primary == STD_NULL || secondary == STD_NULL ||
        port == STD_NULL) return;
    STD_MEMSET((C_VOID *)latch, TYPE_ZERO_8, sizeof(*latch));
    STD_MEMSET((C_VOID *)primary, TYPE_ZERO_8, sizeof(*primary));
    STD_MEMSET((C_VOID *)secondary, TYPE_ZERO_8, sizeof(*secondary));
    primary->connect.latch = latch;
    primary->connect.peer = secondary;
    secondary->connect.latch = latch;
    secondary->connect.peer = primary;
    for (index = 0; index < sizeof(primary_reads) / sizeof(primary_reads[0]);
         ++index) {
        core_machine_port_add_read(port, primary_reads[index], dma_port_read,
            primary);
    }
    for (index = 0; index < 0x10u; ++index) {
        core_machine_port_add_write(port, (type_unsigned_16)index, dma_port_write,
            primary);
    }
    for (index = 0; index < sizeof(page_ports) / sizeof(page_ports[0]);
         ++index) {
        core_machine_port_add_read(port, page_ports[index], dma_port_read,
            primary);
        core_machine_port_add_write(port, page_ports[index], dma_port_write,
            primary);
    }
    for (index = 0; index < sizeof(secondary_reads) /
         sizeof(secondary_reads[0]); ++index) {
        core_machine_port_add_read(port, secondary_reads[index], dma_port_read,
            primary);
    }
    for (index = 0; index <= 0x1eu; index += 2u) {
        core_machine_port_add_write(port, (type_unsigned_16)(0x00c0u + index),
            dma_port_write, primary);
    }
}

C_VOID core_machine_dma_reset(t_latch *latch, t_dma *primary,
    t_dma *secondary) {
    if (latch == STD_NULL || primary == STD_NULL || secondary == STD_NULL) return;
    STD_MEMSET((C_VOID *)(&latch->data), TYPE_ZERO_8, sizeof(t_latch_data));
    doReset(primary);
    doReset(secondary);
}

static C_VOID core_machine_dma_advance_one(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_ram *ram) {
    type_unsigned_8 id;
    type_unsigned_8 realDRQ1, realDRQ2;
    if (latch == STD_NULL || primary == STD_NULL || secondary == STD_NULL || ram == STD_NULL) return;
    if (TYPE_GET_BIT(secondary->data.command, VDMA_COMMAND_CTRL)) {
        return;
    }
    if (TYPE_GET_BIT(secondary->data.isr, VDMA_ISR_IS)) {
        if (VDMA_GetISR_ISR(secondary->data.isr)) {
            Execute(secondary, latch, ram, VDMA_GetISR_ISR(secondary->data.isr), TYPE_TRUE);
        } else if (TYPE_GET_BIT(primary->data.isr, VDMA_ISR_IS)) {
            Execute(primary, latch, ram, VDMA_GetISR_ISR(primary->data.isr), TYPE_FALSE);
        }
        if (!TYPE_GET_BIT(primary->data.isr, VDMA_ISR_IS)) {
            secondary->data.isr = TYPE_ZERO_8;
        }
        return;
    }
    if (TYPE_GET_BIT(primary->data.isr, VDMA_ISR_IS)) {
        Execute(primary, latch, ram, VDMA_GetISR_ISR(primary->data.isr), TYPE_FALSE);
        return;
    }
    if (!TYPE_GET_BIT(secondary->data.isr, VDMA_ISR_IS)) {
        realDRQ2 = secondary->data.request | (VDMA_GetSTATUS_DRQS(secondary->data.status) & ~secondary->data.mask);
        if (realDRQ2 == TYPE_ZERO_8) {
            return;
        }
        id = GetRegTopId(secondary, realDRQ2);
        if (id == 0) {
            if (TYPE_GET_BIT(primary->data.command, VDMA_COMMAND_CTRL)) {
                return;
            }
            realDRQ1 = primary->data.request | (VDMA_GetSTATUS_DRQS(primary->data.status) & ~primary->data.mask);
            if (realDRQ1 == TYPE_ZERO_8) {
                return;
            }
            id = GetRegTopId(primary, realDRQ1);
            VDMA_SetISR(secondary->data.isr, 0);
            VDMA_SetISR(primary->data.isr, id);
            Execute(primary, latch, ram, id, TYPE_FALSE);
            if (!TYPE_GET_BIT(primary->data.isr, VDMA_ISR_IS)) {
                secondary->data.isr = TYPE_ZERO_8;
            }
            if (!VDMA_GetSTATUS_DRQS(primary->data.status)) {
                TYPE_CLEAR_BIT(secondary->data.status, VDMA_STATUS_DRQ(0));
            }
            if (!primary->data.request) {
                TYPE_CLEAR_BIT(secondary->data.request, VDMA_REQUEST_DRQ(0));
            }
        } else {
            VDMA_SetISR(secondary->data.isr, id);
            Execute(secondary, latch, ram, id, TYPE_TRUE);
        }
    }
}

C_VOID core_machine_dma_advance(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_ram *ram, uint64_t elapsed_ticks)
{
    uint64_t tick;

    for (tick = 0u; tick < elapsed_ticks; ++tick) {
        core_machine_dma_advance_one(latch, primary, secondary, ram);
    }
}

C_VOID core_machine_dma_finalize(t_latch *latch, t_dma *primary,
    t_dma *secondary) {
    (C_VOID)latch;
    (C_VOID)primary;
    (C_VOID)secondary;
}

/*
debug
FOR FDC READ/WRITE
off00 0  reset dma
o0b 86   mode: block, increase, write, channel 2
         other options: c6(cascade)  46(single) 06(demand)
o0b 8a   mode: block, increase, read, channel 2
o04 00   write address to: channel 2 low
o04 00   write address to: channel 2 high
o05 00   write word count to: channel 2 low
o05 02   write word count to: channel 2 high
o0a 02   write mask: dma 1, channel 2
od4 00   write mask: dma 2, channel 0
off01 2  generate dreq; otherwise ASK DEVICE TO GENERATE DREQ
off02 0  refresh: exec
d0:0     show results

OTHER TEST CODES
off00 0
e6020 ab
o08 03
o0b 00
o03 5
o03 0
od4 0
o0a 0
o00 30
o00 60
o02 30
o02 80
o09 04
od2 04
off02 0
d6020
d8020
*/

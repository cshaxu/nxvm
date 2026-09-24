/* Copyright 2012-2014 Neko. */

/* VDMA implements two chips of Direct Memory Access Controller: Intel 8237A (Master+Slave). */
#include "lib/types/types_interface.h"
#include "app-nxvm/devices/device_support.h"


#include "app-nxvm/devices/memory.h"

#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/dma.h"

#include "app-nxvm/devices/transaction.h"

/* This only issues opaque binding nonces. It never selects a DMA instance. */
static lib_atomic_uptr core_machine_dma_next_request_token = 1u;

static lib_uptr core_machine_dma_request_token_allocate(void)
{
    lib_uptr expected = lib_atomic_uptr_load_explicit(&core_machine_dma_next_request_token, LIB_MEMORY_ORDER_ACQUIRE);

    while (expected != UINTPTR_MAX) {
        if (lib_atomic_uptr_compare_exchange_strong_explicit(
                &core_machine_dma_next_request_token, &expected, expected + 1u, LIB_MEMORY_ORDER_ACQ_REL, LIB_MEMORY_ORDER_ACQUIRE)) {
            return (lib_uptr)expected;
        }
    }
    return 0u;
}

static void core_machine_dma_controller_reset(t_dma *rdma) {
    lib_memory_set((void *)(&rdma->data), 0u, sizeof(t_dma_data));
    rdma->data.mask = VDMA_MASK_VALID;
}

static void dma_service_begin(t_dma *dma, lib_u8 channel)
{
    VDMA_SetISR(dma->data.isr, channel);
    CORE_MACHINE_BIT_SET(dma->data.acknowledged, VDMA_REQUEST_DRQ(channel));
    dma->data.phase = channel == 0u && CORE_MACHINE_BIT_IS_SET(dma->data.command,
        VDMA_COMMAND_M2M) ? VDMA_PHASE_S11 : VDMA_PHASE_S1;
}

static void dma_service_end(t_dma *dma)
{
    if (CORE_MACHINE_BIT_IS_SET(dma->data.isr, VDMA_ISR_IS)) {
        CORE_MACHINE_BIT_CLEAR(dma->data.acknowledged,
            VDMA_REQUEST_DRQ(VDMA_GetISR_ISR(dma->data.isr)));
    }
    dma->data.flagM2MWrite = LIB_FALSE;
    dma->data.phase = VDMA_PHASE_IDLE;
    dma->data.isr = 0u;
}

static void dma_read_address(t_dma *dma, t_port *port, lib_u8 channel)
{
    port->data.ioByte = !dma->data.flagMSB ?
        CORE_MACHINE_MASK_U8(dma->data.currAddr[channel]) :
        CORE_MACHINE_MASK_U8(dma->data.currAddr[channel] >> 8);
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_read_count(t_dma *dma, t_port *port, lib_u8 channel)
{
    port->data.ioByte = !dma->data.flagMSB ?
        CORE_MACHINE_MASK_U8(dma->data.currCount[channel]) :
        CORE_MACHINE_MASK_U8(dma->data.currCount[channel] >> 8);
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_write_address(t_dma *dma, t_port *port, lib_u8 channel)
{
    if (!dma->data.flagMSB) {
        dma->data.baseAddr[channel] = CORE_MACHINE_MASK_U16(port->data.ioByte);
    } else {
        dma->data.baseAddr[channel] |= CORE_MACHINE_MASK_U16(port->data.ioByte << 8);
    }
    dma->data.currAddr[channel] = dma->data.baseAddr[channel];
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_write_count(t_dma *dma, t_port *port, lib_u8 channel)
{
    if (!dma->data.flagMSB) {
        dma->data.baseCount[channel] = CORE_MACHINE_MASK_U16(port->data.ioByte);
    } else {
        dma->data.baseCount[channel] |= CORE_MACHINE_MASK_U16(port->data.ioByte << 8);
    }
    dma->data.currCount[channel] = dma->data.baseCount[channel];
    dma->data.flagMSB = !dma->data.flagMSB;
}

static void dma_write_request(t_dma *dma, t_port *port)
{
    CORE_MACHINE_BIT_MAKE(dma->data.request,
        VDMA_REQUEST_DRQ(VDMA_GetREQSC_CS(port->data.ioByte)),
        CORE_MACHINE_BIT_IS_SET(port->data.ioByte, VDMA_REQSC_SR));
}

static void dma_write_mask(t_dma *dma, t_port *port)
{
    CORE_MACHINE_BIT_MAKE(dma->data.mask,
        VDMA_MASK_DRQ(VDMA_GetMASKSC_CS(port->data.ioByte)),
        CORE_MACHINE_BIT_IS_SET(port->data.ioByte, VDMA_MASKSC_SM));
}

static t_dma *dma_controller(t_dma *primary, lib_u16 port_id)
{
    return ((port_id >= 0x0089u && port_id <= 0x008fu) ||
        port_id >= 0x00c0u) ? primary->connect.peer : primary;
}

static lib_u8 dma_page_channel(lib_u16 port_id)
{
    switch (port_id) {
    case 0x0081: case 0x0089: return 2u;
    case 0x0082: case 0x008a: return 3u;
    case 0x0083: case 0x008b: return 1u;
    default: return 0u;
    }
}

static lib_u8 dma_page_spare_index(lib_u16 port_id)
{
    switch (port_id) {
    case 0x0084: return 0u;
    case 0x0085: return 1u;
    case 0x0086: return 2u;
    case 0x0088: return 3u;
    case 0x008c: return 4u;
    case 0x008d: return 5u;
    case 0x008e: return 6u;
    default: return 7u;
    }
}

static void dma_port_read_byte(t_port *port, lib_u16 port_id,
    void *owner)
{
    t_dma *primary = (t_dma *)owner;
    t_dma *dma;
    lib_u8 channel;

    if (primary == LIB_NULL) return;
    if (port_id <= 0x0007u) {
        channel = (lib_u8)(port_id >> 1);
        if ((port_id & 1u) == 0u) dma_read_address(primary, port, channel);
        else dma_read_count(primary, port, channel);
        return;
    }
    if (port_id == 0x0008u) {
        port->data.ioByte = primary->data.status;
        CORE_MACHINE_BIT_CLEAR(primary->data.status, VDMA_STATUS_TCS);
        return;
    }
    if (port_id == 0x000du) {
        port->data.ioByte = primary->data.temp;
        return;
    }
    if (port_id == 0x0080u || port_id == 0x0084u || port_id == 0x0085u ||
        port_id == 0x0086u || port_id == 0x0088u ||
        (port_id >= 0x008cu && port_id <= 0x008eu)) {
        port->data.ioByte = primary->data.page_spare[dma_page_spare_index(port_id)];
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
        channel = (lib_u8)((port_id - 0x00c0u) >> 1);
        if ((channel & 1u) == 0u) dma_read_address(dma, port, channel >> 1);
        else dma_read_count(dma, port, channel >> 1);
        return;
    }
    if (port_id == 0x00d0u) {
        port->data.ioByte = primary->connect.peer->data.status;
        CORE_MACHINE_BIT_CLEAR(primary->connect.peer->data.status, VDMA_STATUS_TCS);
        return;
    }
    if (port_id == 0x00dau) {
        port->data.ioByte = primary->connect.peer->data.temp;
    }
}

static void dma_port_write_byte(t_port *port, lib_u16 port_id,
    void *owner)
{
    t_dma *primary = (t_dma *)owner;
    t_dma *dma;
    lib_u8 channel;
    lib_u16 local_port;

    if (primary == LIB_NULL) return;
    if (port_id <= 0x0007u) {
        channel = (lib_u8)(port_id >> 1);
        if ((port_id & 1u) == 0u) dma_write_address(primary, port, channel);
        else dma_write_count(primary, port, channel);
        return;
    }
    if (port_id == 0x0080u || port_id == 0x0084u || port_id == 0x0085u ||
        port_id == 0x0086u || port_id == 0x0088u ||
        (port_id >= 0x008cu && port_id <= 0x008eu)) {
        primary->data.page_spare[dma_page_spare_index(port_id)] = port->data.ioByte;
        return;
    }
    if (port_id >= 0x0081u && port_id <= 0x008fu) {
        dma = dma_controller(primary, port_id);
        /* The AT page-register latch is readable as written.  The secondary
         * controller's word address ignores page bit zero only when it forms
         * a physical address; discarding it here breaks board POST readback. */
        dma->data.page[dma_page_channel(port_id)] = port->data.ioByte;
        return;
    }
    dma = port_id >= 0x00c0u ? primary->connect.peer : primary;
    local_port = port_id >= 0x00c0u ? port_id - 0x00c0u : port_id;
    if (port_id >= 0x00c0u && (local_port & 1u) == 0u &&
        local_port <= 0x000eu) {
        channel = (lib_u8)(local_port >> 1);
        if ((channel & 1u) == 0u) dma_write_address(dma, port, channel >> 1);
        else dma_write_count(dma, port, channel >> 1);
        return;
    }
    if (port_id >= 0x00c0u && local_port >= 0x0010u) {
        /* The second 8237A occupies every other I/O address. Its D0h--DEh
         * control family maps to the first controller's 08h--0Fh functions. */
        local_port = (lib_u16)(0x0008u +
            ((local_port - 0x0010u) >> 1));
    }
    switch (local_port) {
    case 0x0008: dma->data.command = port->data.ioByte; break;
    case 0x0009: dma_write_request(dma, port); break;
    case 0x000a: dma_write_mask(dma, port); break;
    case 0x000b:
        dma->data.mode[VDMA_GetMODE_CS(port->data.ioByte)] = port->data.ioByte;
        break;
    case 0x000c: dma->data.flagMSB = LIB_FALSE; break;
    case 0x000d: core_machine_dma_controller_reset(dma); break;
    case 0x000e: dma->data.mask = 0u; break;
    case 0x000f: dma->data.mask = port->data.ioByte & VDMA_MASKAC_VALID; break;
    default: break;
    }
}

static lib_i32 dma_page_port_is_byte_lanes(const t_port *port,
    lib_u16 port_id)
{
    return port != LIB_NULL && port->data.access_bytes > 1u &&
        port_id >= 0x0080u && (lib_u32)port_id +
        port->data.access_bytes <= 0x0090u;
}

static void dma_port_read(t_port *port, lib_u16 port_id, void *owner)
{
    lib_u32 value = 0u;
    lib_u8 lane;

    if (!dma_page_port_is_byte_lanes(port, port_id)) {
        dma_port_read_byte(port, port_id, owner);
        return;
    }
    /* These page latches are individual eight-bit system-board endpoints.
     * A CPU word/dword transaction therefore reaches consecutive latches;
     * native-width endpoints (notably the HDC data port) retain their one
     * transaction route in the generic port owner. */
    for (lane = 0u; lane < port->data.access_bytes; ++lane) {
        port->data.ioDWord = 0u;
        dma_port_read_byte(port, (lib_u16)(port_id + lane), owner);
        value |= (lib_u32)port->data.ioByte << (lane * 8u);
    }
    port->data.ioDWord = value;
}

static void dma_port_write(t_port *port, lib_u16 port_id, void *owner)
{
    lib_u32 value;
    lib_u8 lane;

    if (!dma_page_port_is_byte_lanes(port, port_id)) {
        dma_port_write_byte(port, port_id, owner);
        return;
    }
    value = port->data.ioDWord;
    for (lane = 0u; lane < port->data.access_bytes; ++lane) {
        port->data.ioDWord = value >> (lane * 8u);
        dma_port_write_byte(port, (lib_u16)(port_id + lane), owner);
    }
    port->data.ioDWord = value;
}

static lib_u8 GetRegTopId(t_dma *rdma, lib_u8 reg) {
    lib_u8 id = 0;
    if (reg == 0u) {
        return 0x08;
    }
    reg = (reg << (VDMA_CHANNEL_COUNT - (rdma->data.drx))) | (reg >> (rdma->data.drx));
    while ((id < VDMA_CHANNEL_COUNT) && !CORE_MACHINE_MASK_U1(reg >> id)) {
        id++;
    }
    return (id + rdma->data.drx) % VDMA_CHANNEL_COUNT;
}

static lib_u8 dma_software_request_is_valid(const t_dma *dma,
    lib_u8 channel)
{
    return VDMA_GetREQUEST_DRQ(dma->data.request, channel) &&
        (VDMA_GetMODE_M(dma->data.mode[channel]) == 0x02u ||
            (channel == 0u && CORE_MACHINE_BIT_IS_SET(dma->data.command,
                VDMA_COMMAND_M2M)));
}

static lib_u8 dma_pending_requests(const t_dma *dma)
{
    lib_u8 pending = VDMA_GetSTATUS_DRQS(dma->data.status) &
        (lib_u8)~dma->data.mask;
    lib_u8 channel;

    for (channel = 0u; channel < VDMA_CHANNEL_COUNT; ++channel) {
        if (dma_software_request_is_valid(dma, channel)) {
            CORE_MACHINE_BIT_SET(pending, VDMA_REQUEST_DRQ(channel));
        }
    }
    return pending;
}

static void IncreaseCurrAddr(t_dma *rdma, lib_u8 id) {
    rdma->data.currAddr[id]++;
}
static void DecreaseCurrAddr(t_dma *rdma, lib_u8 id) {
    rdma->data.currAddr[id]--;
}

static lib_u32 dma_physical_address(const t_dma *dma,
    lib_u8 channel, lib_u8 word)
{
    lib_u8 page = dma->data.page[channel];
    lib_u32 address;

    if (word) page &= 0xfeu;
    address = (lib_u32)page << 16u;

    address += word ? (lib_u32)dma->data.currAddr[channel] << 1u :
        dma->data.currAddr[channel];
    return address;
}

static lib_u8 dma_memory_route_is_valid(t_ram *ram,
    lib_u32 physical, lib_uptr bytes,
    core_machine_memory_access access)
{
    core_machine_memory_route route;

    return core_machine_memory_query_physical(ram, physical, bytes, access,
        &route) == LIB_STATUS_OK;
}

static lib_u8 Transmission(t_dma *rdma, t_latch *latch, t_ram *ram,
    core_machine_transaction_state *transaction, lib_u8 id,
    lib_u8 flagWord)
{
    lib_u32 physical = dma_physical_address(rdma, id, flagWord);
    lib_uptr bytes = flagWord ? 2u : 1u;

    switch (VDMA_GetMODE_TT(rdma->data.mode[id])) {
    case 0x00:
        /* Verify consumes the peripheral byte without accessing memory.  The
         * device must still see the service cycle: an FDC can thereby report
         * an invalid sector instead of letting DMA silently count past it. */
        if (rdma->connect.read_provider[id] != LIB_NULL) {
            rdma->connect.read_provider[id](rdma->connect.device_owner[id], latch);
        }
        rdma->data.currCount[id]--;
        if (CORE_MACHINE_BIT_IS_SET(rdma->data.mode[id], VDMA_MODE_AIDS)) {
            DecreaseCurrAddr(rdma, id);
        } else {
            IncreaseCurrAddr(rdma, id);
        }
        return LIB_TRUE;
    case 0x01:
        /* write */
        if (!dma_memory_route_is_valid(ram, physical, bytes,
                CORE_MACHINE_MEMORY_ACCESS_WRITE)) {
            return LIB_FALSE;
        }
        if (transaction != LIB_NULL && core_machine_transaction_begin(
                transaction, CORE_MACHINE_TRANSACTION_OWNER_DMA,
                CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, physical,
                (lib_u32)bytes, id) != LIB_STATUS_OK) {
            return LIB_FALSE;
        }
        if (rdma->connect.read_provider[id] != LIB_NULL) {
            rdma->connect.read_provider[id](rdma->connect.device_owner[id], latch);
        }
        if ((!flagWord && core_machine_memory_write_physical(ram, physical,
                (lib_uptr)(&latch->data.byte), 1u) != LIB_STATUS_OK) ||
            (flagWord && core_machine_memory_write_physical(ram, physical,
                (lib_uptr)(&latch->data.word), 2u) != LIB_STATUS_OK)) {
            core_machine_transaction_cancel(transaction);
            return LIB_FALSE;
        }
        core_machine_transaction_commit(transaction);
        rdma->data.currCount[id]--;
        if (CORE_MACHINE_BIT_IS_SET(rdma->data.mode[id], VDMA_MODE_AIDS)) {
            DecreaseCurrAddr(rdma, id);
        } else {
            IncreaseCurrAddr(rdma, id);
        }
        return LIB_TRUE;
    case 0x02:
        /* read */
        if (!dma_memory_route_is_valid(ram, physical, bytes,
                CORE_MACHINE_MEMORY_ACCESS_READ)) {
            return LIB_FALSE;
        }
        if (transaction != LIB_NULL && core_machine_transaction_begin(
                transaction, CORE_MACHINE_TRANSACTION_OWNER_DMA,
                CORE_MACHINE_TRANSACTION_DMA_MEMORY_READ, physical,
                (lib_u32)bytes, id) != LIB_STATUS_OK) {
            return LIB_FALSE;
        }
        if ((!flagWord && core_machine_memory_read_physical(ram, physical,
                (lib_uptr)(&latch->data.byte), 1u) != LIB_STATUS_OK) ||
            (flagWord && core_machine_memory_read_physical(ram, physical,
                (lib_uptr)(&latch->data.word), 2u) != LIB_STATUS_OK)) {
            core_machine_transaction_cancel(transaction);
            return LIB_FALSE;
        }
        if (rdma->connect.write_provider[id] != LIB_NULL) {
            rdma->connect.write_provider[id](rdma->connect.device_owner[id], latch);
        }
        core_machine_transaction_commit(transaction);
        rdma->data.currCount[id]--;
        if (CORE_MACHINE_BIT_IS_SET(rdma->data.mode[id], VDMA_MODE_AIDS)) {
            DecreaseCurrAddr(rdma, id);
        } else {
            IncreaseCurrAddr(rdma, id);
        }
        return LIB_TRUE;
    case 0x03:
        /* illegal */
        return LIB_FALSE;
    default:
        return LIB_FALSE;
    }
}

static void dma_complete_transfer(t_dma *dma, t_latch *latch,
    lib_u8 channel, lib_u8 memory_to_memory, lib_u8 terminal_count)
{
    lib_u8 first = memory_to_memory ? 0u : channel;
    lib_u8 last = memory_to_memory ? 1u : channel;
    lib_u8 index;

    dma_service_end(dma);
    for (index = first; index <= last; ++index) {
        CORE_MACHINE_BIT_CLEAR(dma->data.request, VDMA_REQUEST_DRQ(index));
        if (dma->connect.close_provider[index] != LIB_NULL) {
            dma->connect.close_provider[index](dma->connect.device_owner[index],
                latch);
        }
        if (CORE_MACHINE_BIT_IS_SET(dma->data.mode[index], VDMA_MODE_AI)) {
            dma->data.currAddr[index] = dma->data.baseAddr[index];
            dma->data.currCount[index] = dma->data.baseCount[index];
            CORE_MACHINE_BIT_CLEAR(dma->data.mask, VDMA_MASK_DRQ(index));
        } else {
            CORE_MACHINE_BIT_SET(dma->data.mask, VDMA_MASK_DRQ(index));
        }
    }
    if (terminal_count) {
        CORE_MACHINE_BIT_SET(dma->data.status,
            VDMA_STATUS_TC(memory_to_memory ? 1u : channel));
    }
}

static void Execute(t_dma *rdma, t_latch *latch, t_ram *ram,
    core_machine_transaction_state *transaction, lib_u8 id,
    lib_u8 flagWord) {
    lib_u8 flagM2M = ((id == 0) &&
                      VDMA_GetREQUEST_DRQ(rdma->data.request, 0) &&
                      CORE_MACHINE_BIT_IS_SET(rdma->data.command, VDMA_COMMAND_M2M));
    lib_u8 request_asserted = VDMA_GetSTATUS_DRQ(rdma->data.status, id);
    if (CORE_MACHINE_BIT_IS_SET(rdma->data.command, VDMA_COMMAND_R)) {
        rdma->data.drx = (id + 1) % VDMA_CHANNEL_COUNT;
    }
    if (flagM2M) {
        /* Memory-to-memory is two logical services: channel 0 reads the
         * temporary register first, then channel 1 writes it. */
        if (rdma->data.currCount[1] != 0xffff && !rdma->data.flagEOP) {
            lib_u32 source = dma_physical_address(rdma, 0u, LIB_FALSE);
            lib_u32 destination = dma_physical_address(rdma, 1u,
                LIB_FALSE);

            if (!rdma->data.flagM2MWrite) {
                if (!dma_memory_route_is_valid(ram, source, 1u,
                        CORE_MACHINE_MEMORY_ACCESS_READ) ||
                    (transaction != LIB_NULL && core_machine_transaction_begin(
                        transaction, CORE_MACHINE_TRANSACTION_OWNER_DMA,
                        CORE_MACHINE_TRANSACTION_DMA_MEMORY_READ, source, 1u,
                        0u) != LIB_STATUS_OK) ||
                    core_machine_memory_read_physical(ram, source,
                        (lib_uptr)(&rdma->data.temp), 1u) != LIB_STATUS_OK) {
                    core_machine_transaction_cancel(transaction);
                    dma_service_end(rdma);
                    return;
                }
                core_machine_transaction_commit(transaction);
                rdma->data.flagM2MWrite = LIB_TRUE;
                return;
            }
            if (!dma_memory_route_is_valid(ram, destination, 1u,
                    CORE_MACHINE_MEMORY_ACCESS_WRITE) ||
                (transaction != LIB_NULL && core_machine_transaction_begin(
                    transaction, CORE_MACHINE_TRANSACTION_OWNER_DMA,
                    CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE, destination, 1u,
                    1u) != LIB_STATUS_OK) ||
                core_machine_memory_write_physical(ram, destination,
                    (lib_uptr)(&rdma->data.temp), 1u) != LIB_STATUS_OK) {
                core_machine_transaction_cancel(transaction);
                dma_service_end(rdma);
                return;
            }
            core_machine_transaction_commit(transaction);
            rdma->data.flagM2MWrite = LIB_FALSE;
            rdma->data.currCount[1]--;
            if (CORE_MACHINE_BIT_IS_SET(rdma->data.mode[1u], VDMA_MODE_AIDS)) {
                DecreaseCurrAddr(rdma, 1u);
            } else {
                IncreaseCurrAddr(rdma, 1u);
            }
            if (!CORE_MACHINE_BIT_IS_SET(rdma->data.command, VDMA_COMMAND_C0AD)) {
                if (CORE_MACHINE_BIT_IS_SET(rdma->data.mode[0u], VDMA_MODE_AIDS)) {
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
                if (!Transmission(rdma, latch, ram, transaction, id, flagWord)) {
                    dma_service_end(rdma);
                    return;
                }
            }
            if (!rdma->data.flagEOP && !request_asserted) dma_service_end(rdma);
            break;
        case 0x01:
            /* single */
            if (!Transmission(rdma, latch, ram, transaction, id, flagWord)) {
                dma_service_end(rdma);
                return;
            }
            if (!rdma->data.flagEOP) dma_service_end(rdma);
            break;
        case 0x02:
            /* block */
            if (rdma->data.currCount[id] != 0xffffu &&
                !rdma->data.flagEOP) {
                if (!Transmission(rdma, latch, ram, transaction, id, flagWord)) {
                    dma_service_end(rdma);
                    return;
                }
            }
            break;
        case 0x03:
            /* Cascade delegates the selected priority slot; it is neither a
             * transfer nor a terminal condition. The paired AT controllers
             * already delegate through core_machine_dma_advance_one(). */
            dma_service_end(rdma);
            return;
        default:
            break;
        }
        if (rdma->data.currCount[id] == 0xffffu) {
            rdma->data.flagEOP = LIB_TRUE;
        }
    }
    if (rdma->data.flagEOP) {
        dma_complete_transfer(rdma, latch, id, flagM2M,
            rdma->data.currCount[flagM2M ? 1u : id] == 0xffffu ||
            !CORE_MACHINE_BIT_IS_SET(rdma->data.mode[flagM2M ? 1u : id], VDMA_MODE_AI));
    }
    rdma->data.flagEOP = LIB_FALSE;
}

static lib_u8 dma_address_high_changed(lib_u16 before,
    lib_u16 after)
{
    return (before & 0xff00u) != (after & 0xff00u);
}

static void dma_service_advance(t_dma *dma, t_latch *latch, t_ram *ram,
    core_machine_transaction_state *transaction, lib_u8 channel,
    lib_u8 word)
{
    lib_u16 source_before;
    lib_u16 channel_before;

    switch (dma->data.phase) {
    case VDMA_PHASE_S1:
        dma->data.phase = VDMA_PHASE_S2;
        break;
    case VDMA_PHASE_S2:
        dma->data.phase = CORE_MACHINE_BIT_IS_SET(dma->data.command, VDMA_COMMAND_TM) ?
            VDMA_PHASE_S4 : VDMA_PHASE_S3;
        break;
    case VDMA_PHASE_S3:
        dma->data.phase = VDMA_PHASE_S4;
        break;
    case VDMA_PHASE_S4:
        channel_before = dma->data.currAddr[channel];
        Execute(dma, latch, ram, transaction, channel, word);
        if (CORE_MACHINE_BIT_IS_SET(dma->data.isr, VDMA_ISR_IS)) {
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
        Execute(dma, latch, ram, transaction, channel, LIB_FALSE);
        if (CORE_MACHINE_BIT_IS_SET(dma->data.isr, VDMA_ISR_IS) && dma->data.flagM2MWrite) {
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
        Execute(dma, latch, ram, transaction, channel, LIB_FALSE);
        if (CORE_MACHINE_BIT_IS_SET(dma->data.isr, VDMA_ISR_IS)) {
            dma->data.phase = dma_address_high_changed(source_before,
                dma->data.currAddr[0u]) ? VDMA_PHASE_S11 : VDMA_PHASE_S12;
        }
        break;
    default:
        dma_service_end(dma);
        break;
    }
}

static void core_machine_dma_set_drq(t_dma *primary, t_dma *secondary,
    lib_u8 drq_id, lib_u8 asserted)
{
    if (primary == LIB_NULL || secondary == LIB_NULL) return;
    switch (drq_id) {
    case 0:
    case 1:
    case 2:
    case 3:
        CORE_MACHINE_BIT_MAKE(primary->data.status, VDMA_STATUS_DRQ(drq_id), asserted);
        break;
    case 5:
    case 6:
    case 7:
        CORE_MACHINE_BIT_MAKE(secondary->data.status, VDMA_STATUS_DRQ(drq_id - 4), asserted);
        break;
    case 4:
    default:
        break;
    }
}

lib_status core_machine_dma_bind_channel(t_latch *latch, t_dma *primary,
    t_dma *secondary, lib_u8 drq_id,
    const core_machine_dma_channel_provider *provider, void *owner,
    core_machine_dma_request_binding *out_binding)
{
    t_dma *dma;
    lib_u8 channel;

    if (latch == LIB_NULL || primary == LIB_NULL || secondary == LIB_NULL ||
        provider == LIB_NULL || out_binding == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (drq_id <= 3u) {
        dma = primary;
        channel = drq_id;
    } else if (drq_id >= 5u && drq_id <= 7u) {
        dma = secondary;
        channel = drq_id - 4u;
    } else {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (dma->connect.latch != latch || dma->connect.peer == LIB_NULL ||
        dma->connect.device_owner[channel] != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    if (primary->connect.request_token == 0u) {
        primary->connect.request_token = core_machine_dma_request_token_allocate();
        if (primary->connect.request_token == 0u) return LIB_STATUS_INTERNAL_ERROR;
    }
    dma->connect.read_provider[channel] = provider->read_device;
    dma->connect.write_provider[channel] = provider->write_device;
    dma->connect.close_provider[channel] = provider->terminal_count;
    dma->connect.device_owner[channel] = owner;
    out_binding->core_token = primary->connect.request_token;
    out_binding->channel = drq_id;
    return LIB_STATUS_OK;
}

void core_machine_dma_request_assert(t_dma *primary, t_dma *secondary,
    const core_machine_dma_request_binding *binding)
{
    if (binding == LIB_NULL || primary == LIB_NULL || secondary == LIB_NULL ||
        binding->core_token == 0u ||
        binding->core_token != primary->connect.request_token ||
        primary->connect.peer != secondary) return;
    core_machine_dma_set_drq(primary, secondary,
        binding->channel, LIB_TRUE);
}

void core_machine_dma_request_deassert(t_dma *primary, t_dma *secondary,
    const core_machine_dma_request_binding *binding)
{
    if (binding == LIB_NULL || primary == LIB_NULL || secondary == LIB_NULL ||
        binding->core_token == 0u ||
        binding->core_token != primary->connect.request_token ||
        primary->connect.peer != secondary) return;
    core_machine_dma_set_drq(primary, secondary,
        binding->channel, LIB_FALSE);
}

void core_machine_dma_request_terminate(t_dma *primary, t_dma *secondary,
    const core_machine_dma_request_binding *binding)
{
    t_dma *dma;
    lib_u8 channel;

    if (binding == LIB_NULL || primary == LIB_NULL || secondary == LIB_NULL ||
        binding->core_token == 0u ||
        binding->core_token != primary->connect.request_token ||
        primary->connect.peer != secondary) return;
    if (binding->channel <= 3u) {
        dma = primary;
        channel = binding->channel;
    } else if (binding->channel >= 5u && binding->channel <= 7u) {
        dma = secondary;
        channel = binding->channel - 4u;
    } else {
        return;
    }
    if (CORE_MACHINE_BIT_IS_SET(dma->data.isr, VDMA_ISR_IS) &&
        VDMA_GetISR_ISR(dma->data.isr) == channel) {
        dma->data.flagEOP = LIB_TRUE;
    }
}

void core_machine_dma_initialize(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_port *port, lib_u8 controller_count)
{
    static const lib_u16 primary_reads[] = {
        0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
        0x0008, 0x000d
    };
    static const lib_u16 primary_page_ports[] = {
        0x0081, 0x0082, 0x0083
    };
    static const lib_u16 secondary_page_ports[] = {
        0x0087, 0x0089, 0x008a, 0x008b, 0x008f
    };
    static const lib_u16 spare_page_ports[] = {
        0x0080, 0x0084, 0x0085, 0x0086, 0x0088, 0x008c, 0x008d, 0x008e
    };
    static const lib_u16 secondary_reads[] = {
        0x00c0, 0x00c2, 0x00c4, 0x00c6, 0x00c8, 0x00ca, 0x00cc, 0x00ce,
        0x00d0, 0x00da
    };
    lib_uptr index;

    if (latch == LIB_NULL || primary == LIB_NULL || secondary == LIB_NULL ||
        port == LIB_NULL || (controller_count != 1u && controller_count != 2u)) return;
    lib_memory_set((void *)latch, 0u, sizeof(*latch));
    lib_memory_set((void *)primary, 0u, sizeof(*primary));
    lib_memory_set((void *)secondary, 0u, sizeof(*secondary));
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
        core_machine_port_add_write(port, (lib_u16)index, dma_port_write,
            primary);
    }
    for (index = 0; index < sizeof(primary_page_ports) /
            sizeof(primary_page_ports[0]);
         ++index) {
        core_machine_port_add_read(port, primary_page_ports[index], dma_port_read,
            primary);
        core_machine_port_add_write(port, primary_page_ports[index], dma_port_write,
            primary);
    }
    if (controller_count == 2u) {
        for (index = 0; index < sizeof(spare_page_ports) /
            sizeof(spare_page_ports[0]); ++index) {
            core_machine_port_add_read(port, spare_page_ports[index], dma_port_read,
                primary);
            core_machine_port_add_write(port, spare_page_ports[index], dma_port_write,
                primary);
        }
        for (index = 0; index < sizeof(secondary_page_ports) /
                sizeof(secondary_page_ports[0]); ++index) {
            core_machine_port_add_read(port, secondary_page_ports[index], dma_port_read,
                primary);
            core_machine_port_add_write(port, secondary_page_ports[index], dma_port_write,
                primary);
        }
        for (index = 0; index < sizeof(secondary_reads) /
             sizeof(secondary_reads[0]); ++index) {
            core_machine_port_add_read(port, secondary_reads[index], dma_port_read,
                primary);
        }
        for (index = 0; index <= 0x1eu; index += 2u) {
            core_machine_port_add_write(port, (lib_u16)(0x00c0u + index),
                dma_port_write, primary);
        }
    }
}

void core_machine_dma_reset(t_latch *latch, t_dma *primary,
    t_dma *secondary) {
    if (latch == LIB_NULL || primary == LIB_NULL || secondary == LIB_NULL) return;
    lib_memory_set((void *)(&latch->data), 0u, sizeof(t_latch_data));
    core_machine_dma_controller_reset(primary);
    core_machine_dma_controller_reset(secondary);
}

static void core_machine_dma_advance_one(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_ram *ram, core_machine_transaction_state *transaction) {
    lib_u8 id;
    lib_u8 realDRQ1, realDRQ2;
    if (latch == LIB_NULL || primary == LIB_NULL || secondary == LIB_NULL || ram == LIB_NULL) return;
    if (CORE_MACHINE_BIT_IS_SET(secondary->data.command, VDMA_COMMAND_CTRL)) {
        return;
    }
    if (CORE_MACHINE_BIT_IS_SET(secondary->data.isr, VDMA_ISR_IS)) {
        if (VDMA_GetISR_ISR(secondary->data.isr)) {
            dma_service_advance(secondary, latch, ram, transaction,
                VDMA_GetISR_ISR(secondary->data.isr), LIB_TRUE);
            return;
        } else if (!CORE_MACHINE_BIT_IS_SET(primary->data.command, VDMA_COMMAND_CTRL) &&
            CORE_MACHINE_BIT_IS_SET(primary->data.isr, VDMA_ISR_IS)) {
            dma_service_advance(primary, latch, ram, transaction,
                VDMA_GetISR_ISR(primary->data.isr), LIB_FALSE);
            if (!CORE_MACHINE_BIT_IS_SET(primary->data.isr, VDMA_ISR_IS)) {
                dma_service_end(secondary);
            }
        }
        return;
    }
    if (!CORE_MACHINE_BIT_IS_SET(primary->data.command, VDMA_COMMAND_CTRL) &&
        CORE_MACHINE_BIT_IS_SET(primary->data.isr, VDMA_ISR_IS)) {
        dma_service_advance(primary, latch, ram, transaction,
            VDMA_GetISR_ISR(primary->data.isr), LIB_FALSE);
        return;
    }
    if (!CORE_MACHINE_BIT_IS_SET(secondary->data.isr, VDMA_ISR_IS)) {
        realDRQ1 = CORE_MACHINE_BIT_IS_SET(primary->data.command, VDMA_COMMAND_CTRL) ?
            0u : dma_pending_requests(primary);
        realDRQ2 = dma_pending_requests(secondary) &
            (lib_u8)~VDMA_REQUEST_DRQ(0);
        if (realDRQ1 != 0u) {
            CORE_MACHINE_BIT_SET(realDRQ2, VDMA_REQUEST_DRQ(0));
        }
        if (realDRQ2 == 0u) {
            return;
        }
        id = GetRegTopId(secondary, realDRQ2);
        if (id == 0) {
            if (realDRQ1 == 0u) {
                return;
            }
            id = GetRegTopId(primary, realDRQ1);
            dma_service_begin(secondary, 0u);
            if (CORE_MACHINE_BIT_IS_SET(secondary->data.command, VDMA_COMMAND_R)) {
                secondary->data.drx = 1u;
            }
            dma_service_begin(primary, id);
            if (!primary->data.request) {
                CORE_MACHINE_BIT_CLEAR(secondary->data.request, VDMA_REQUEST_DRQ(0));
            }
        } else {
            dma_service_begin(secondary, id);
            Execute(secondary, latch, ram, transaction, id, LIB_TRUE);
        }
    }
}

lib_i32 core_machine_dma_has_pending_request(const t_dma *primary,
    const t_dma *secondary)
{
    lib_u8 primary_requests;
    lib_u8 secondary_requests;

    if (primary == LIB_NULL || secondary == LIB_NULL ||
        CORE_MACHINE_BIT_IS_SET(secondary->data.command, VDMA_COMMAND_CTRL)) return 0;
    if (CORE_MACHINE_BIT_IS_SET(secondary->data.isr, VDMA_ISR_IS)) return 1;
    if (!CORE_MACHINE_BIT_IS_SET(primary->data.command, VDMA_COMMAND_CTRL) &&
        CORE_MACHINE_BIT_IS_SET(primary->data.isr, VDMA_ISR_IS)) return 1;
    primary_requests = CORE_MACHINE_BIT_IS_SET(primary->data.command, VDMA_COMMAND_CTRL) ?
        0u : dma_pending_requests(primary);
    secondary_requests = dma_pending_requests(secondary) &
        (lib_u8)~VDMA_REQUEST_DRQ(0);
    if (primary_requests != 0u) {
        CORE_MACHINE_BIT_SET(secondary_requests, VDMA_REQUEST_DRQ(0));
    }
    return secondary_requests != 0u;
}

void core_machine_dma_advance(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_ram *ram, lib_u64 elapsed_ticks)
{
    lib_u64 tick;

    /* This non-transaction entry point is retained for focused controller
     * fixtures: one requested tick means one logical DMA primitive. Machine
     * execution always uses core_machine_dma_advance_transaction() below,
     * where each elapsed DMA tick advances exactly one Intel service phase. */
    for (tick = 0u; tick < elapsed_ticks; ++tick) {
        lib_u16 primary_address[VDMA_CHANNEL_COUNT];
        lib_u16 primary_count[VDMA_CHANNEL_COUNT];
        lib_u16 secondary_address[VDMA_CHANNEL_COUNT];
        lib_u16 secondary_count[VDMA_CHANNEL_COUNT];
        lib_u8 index;
        lib_u8 phase;
        lib_u8 m2m;
        lib_u8 m2m_write;

        if (primary == LIB_NULL || secondary == LIB_NULL) return;
        m2m = CORE_MACHINE_BIT_IS_SET(primary->data.command, VDMA_COMMAND_M2M) &&
            VDMA_GetREQUEST_DRQ(primary->data.request, 0u);
        m2m_write = primary->data.flagM2MWrite;
        for (index = 0u; index < VDMA_CHANNEL_COUNT; ++index) {
            primary_address[index] = primary->data.currAddr[index];
            primary_count[index] = primary->data.currCount[index];
            secondary_address[index] = secondary->data.currAddr[index];
            secondary_count[index] = secondary->data.currCount[index];
        }
        for (phase = 0u; phase < 16u; ++phase) {
            core_machine_dma_advance_one(latch, primary, secondary, ram, LIB_NULL);
            if (m2m ? primary->data.flagM2MWrite != m2m_write ||
                    (!CORE_MACHINE_BIT_IS_SET(primary->data.isr, VDMA_ISR_IS) &&
                        !CORE_MACHINE_BIT_IS_SET(secondary->data.isr, VDMA_ISR_IS)) :
                primary->data.currAddr[0u] != primary_address[0u] ||
                primary->data.currAddr[1u] != primary_address[1u] ||
                primary->data.currAddr[2u] != primary_address[2u] ||
                primary->data.currAddr[3u] != primary_address[3u] ||
                primary->data.currCount[0u] != primary_count[0u] ||
                primary->data.currCount[1u] != primary_count[1u] ||
                primary->data.currCount[2u] != primary_count[2u] ||
                primary->data.currCount[3u] != primary_count[3u] ||
                secondary->data.currAddr[0u] != secondary_address[0u] ||
                secondary->data.currAddr[1u] != secondary_address[1u] ||
                secondary->data.currAddr[2u] != secondary_address[2u] ||
                secondary->data.currAddr[3u] != secondary_address[3u] ||
                secondary->data.currCount[0u] != secondary_count[0u] ||
                secondary->data.currCount[1u] != secondary_count[1u] ||
                secondary->data.currCount[2u] != secondary_count[2u] ||
                secondary->data.currCount[3u] != secondary_count[3u] ||
                (!CORE_MACHINE_BIT_IS_SET(primary->data.isr, VDMA_ISR_IS) &&
                    !CORE_MACHINE_BIT_IS_SET(secondary->data.isr, VDMA_ISR_IS))) {
                break;
            }
        }
    }
}

void core_machine_dma_advance_transaction(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_ram *ram, core_machine_transaction_state *transaction,
    lib_u64 elapsed_ticks)
{
    lib_u64 tick;

    for (tick = 0u; tick < elapsed_ticks; ++tick) {
        core_machine_dma_advance_one(latch, primary, secondary, ram,
            transaction);
    }
}

void core_machine_dma_finalize(t_latch *latch, t_dma *primary,
    t_dma *secondary) {
    (void)latch;
    (void)primary;
    (void)secondary;
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

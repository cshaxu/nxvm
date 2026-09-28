/* Copyright 2012-2026 Neko. */
#include "app-nxvm/devices/dma_bus.h"
#include "app-nxvm/devices/memory.h"
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


static void core_machine_dma_controller_reset(t_dma *dma)
{
    lib_memory_set(&dma->data, 0u, sizeof(dma->data));
    if (dma->device != LIB_NULL) x86_dma_reset(dma->device);
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


static void dma_port_read_byte(t_port *port, lib_u16 port_id, void *owner)
{
    t_dma *primary = owner;
    t_dma *dma;
    if (port_id < 0x10u) {
        x86_dma_read_register(primary->device, (lib_u8)port_id, &port->data.ioByte);
    } else if (port_id >= 0xc0u && port_id <= 0xdeu && !(port_id & 1u)) {
        x86_dma_read_register(primary->connect.peer->device,
            (lib_u8)((port_id - 0xc0u) >> 1), &port->data.ioByte);
    } else if (port_id == 0x80u || port_id == 0x84u || port_id == 0x85u ||
        port_id == 0x86u || port_id == 0x88u ||
        (port_id >= 0x8cu && port_id <= 0x8eu)) {
        port->data.ioByte = primary->data.page_spare[dma_page_spare_index(port_id)];
    } else if (port_id >= 0x81u && port_id <= 0x8fu) {
        dma = dma_controller(primary, port_id);
        port->data.ioByte = dma->data.page[dma_page_channel(port_id)];
    }
}

static void dma_port_write_byte(t_port *port, lib_u16 port_id, void *owner)
{
    t_dma *primary = owner;
    t_dma *dma;
    lib_u8 selector;
    if (port_id == 0x80u || port_id == 0x84u || port_id == 0x85u ||
        port_id == 0x86u || port_id == 0x88u ||
        (port_id >= 0x8cu && port_id <= 0x8eu)) {
        primary->data.page_spare[dma_page_spare_index(port_id)] = port->data.ioByte;
        return;
    }
    if (port_id >= 0x81u && port_id <= 0x8fu) {
        dma = dma_controller(primary, port_id);
        dma->data.page[dma_page_channel(port_id)] = port->data.ioByte;
        return;
    }
    dma = port_id >= 0xc0u ? primary->connect.peer : primary;
    selector = (lib_u8)(port_id >= 0xc0u ? (port_id - 0xc0u) >> 1 : port_id);
    /* Preserve the existing board master-clear fanout, including page latches. */
    if (selector == 13u) core_machine_dma_controller_reset(dma);
    else x86_dma_write_register(dma->device, selector, port->data.ioByte);
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


typedef struct dma_cycle_context {
    t_dma *controller;
    t_latch *latch;
    t_ram *ram;
    core_machine_transaction_state *transaction;
    lib_bool word;
} dma_cycle_context;

static lib_status dma_bus_cycle(void *owner, x86_dma_cycle kind,
    lib_u8 channel, lib_u16 address, lib_u8 *byte)
{
    dma_cycle_context *context = owner;
    t_dma *dma = context->controller;
    lib_bool memory_only = kind == X86_DMA_MEMORY_READ || kind == X86_DMA_MEMORY_WRITE;
    lib_bool word = context->word && !memory_only;
    lib_bool write = kind == X86_DMA_DEVICE_TO_MEMORY || kind == X86_DMA_MEMORY_WRITE;
    lib_u8 page = dma->data.page[channel];
    lib_uptr bytes = word ? 2u : 1u;
    lib_uptr buffer = memory_only ? lib_pointer_to_uptr(byte) :
        (word ? lib_pointer_to_uptr(&context->latch->data.word) :
            lib_pointer_to_uptr(&context->latch->data.byte));
    lib_u32 physical;
    core_machine_memory_route route;
    lib_status status;
    if (kind == X86_DMA_VERIFY) {
        if (dma->connect.read_provider[channel] != LIB_NULL) {
            dma->connect.read_provider[channel](dma->connect.device_owner[channel],
                context->latch);
        }
        return LIB_STATUS_OK;
    }
    if (word) page &= 0xfeu;
    physical = ((lib_u32)page << 16) + (word ? (lib_u32)address << 1 : address);
    status = core_machine_memory_query_physical(context->ram, physical, bytes,
        write ? CORE_MACHINE_MEMORY_ACCESS_WRITE : CORE_MACHINE_MEMORY_ACCESS_READ,
        &route);
    if (status != LIB_STATUS_OK) return status;
    if (context->transaction != LIB_NULL) {
        status = core_machine_transaction_begin(context->transaction,
            CORE_MACHINE_TRANSACTION_OWNER_DMA,
            write ? CORE_MACHINE_TRANSACTION_DMA_MEMORY_WRITE :
                CORE_MACHINE_TRANSACTION_DMA_MEMORY_READ,
            physical, (lib_u32)bytes, channel);
        if (status != LIB_STATUS_OK) return status;
    }
    if (kind == X86_DMA_DEVICE_TO_MEMORY && dma->connect.read_provider[channel] != LIB_NULL) {
        dma->connect.read_provider[channel](dma->connect.device_owner[channel], context->latch);
    }
    status = write ? core_machine_memory_write_physical(context->ram, physical, buffer, bytes) :
        core_machine_memory_read_physical(context->ram, physical, buffer, bytes);
    if (status != LIB_STATUS_OK) {
        core_machine_transaction_cancel(context->transaction);
        return status;
    }
    if (kind == X86_DMA_MEMORY_TO_DEVICE && dma->connect.write_provider[channel] != LIB_NULL) {
        dma->connect.write_provider[channel](dma->connect.device_owner[channel], context->latch);
    }
    core_machine_transaction_commit(context->transaction);
    return LIB_STATUS_OK;
}

static void dma_bus_terminal(void *owner, lib_u8 channel)
{
    dma_cycle_context *context = owner;
    t_dma *dma = context->controller;
    if (dma->connect.close_provider[channel] != LIB_NULL) {
        dma->connect.close_provider[channel](dma->connect.device_owner[channel],
            context->latch);
    }
}

static void dma_bus_advance(t_dma *dma, t_latch *latch, t_ram *ram,
    core_machine_transaction_state *transaction, lib_bool word)
{
    dma_cycle_context context = { dma, latch, ram, transaction, word };
    x86_dma_bus bus = { dma_bus_cycle, dma_bus_terminal, &context };
    x86_dma_advance(dma->device, &bus);
}

static void core_machine_dma_set_drq(t_dma *primary, t_dma *secondary,
    lib_u8 channel, lib_bool asserted)
{
    t_dma *dma = channel < 4u ? primary : secondary;
    if (channel == 4u || channel > 7u || dma->device == LIB_NULL) return;
    x86_dma_set_request(dma->device, channel & 3u, asserted);
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
    if (dma->device == LIB_NULL || dma->connect.latch != latch || dma->connect.peer == LIB_NULL ||
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
    if (dma->device != LIB_NULL &&
        x86_dma_get_signals(dma->device).active_channel == channel) {
        x86_dma_terminate(dma->device);
    }
}


lib_status core_machine_dma_initialize(t_latch *latch, t_dma *primary,
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
    lib_status status;
    core_machine_port_provider_entry *checkpoint;

    if (latch == LIB_NULL || primary == LIB_NULL || secondary == LIB_NULL ||
        port == LIB_NULL || (controller_count != 1u && controller_count != 2u) || primary == secondary) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_memory_set((void *)latch, 0u, sizeof(*latch));
    lib_memory_set((void *)primary, 0u, sizeof(*primary));
    lib_memory_set((void *)secondary, 0u, sizeof(*secondary));
    primary->connect.latch = latch;
    primary->connect.peer = secondary;
    secondary->connect.latch = latch;
    secondary->connect.peer = primary;
    status = core_machine_port_registration_status(port);
    if (status != LIB_STATUS_OK) return status;
    status = x86_dma_create(&primary->device);
    if (status == LIB_STATUS_OK && controller_count == 2u) {
        status = x86_dma_create(&secondary->device);
    }
    if (status != LIB_STATUS_OK) {
        core_machine_dma_finalize(latch, primary, secondary);
        return status;
    }
    checkpoint = core_machine_port_registration_begin(port);
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
    status = core_machine_port_registration_status(port);
    if (status != LIB_STATUS_OK) {
        core_machine_port_rollback_registration(port, checkpoint);
        core_machine_dma_finalize(latch, primary, secondary);
    }
    return status;
}


void core_machine_dma_reset(t_latch *latch, t_dma *primary, t_dma *secondary)
{
    if (latch == LIB_NULL || primary == LIB_NULL || secondary == LIB_NULL) return;
    lib_memory_set(&latch->data, 0u, sizeof(latch->data));
    core_machine_dma_controller_reset(primary);
    core_machine_dma_controller_reset(secondary);
}

static void core_machine_dma_advance_one(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_ram *ram, core_machine_transaction_state *transaction)
{
    x86_dma_signals first = x86_dma_get_signals(primary->device);
    x86_dma_signals second;
    lib_u8 requests;
    lib_u8 channel;
    if (secondary->device == LIB_NULL) {
        if (!first.enabled) return;
        if (first.active_channel < 4u) dma_bus_advance(primary, latch, ram, transaction, LIB_FALSE);
        else if (first.requests) x86_dma_grant(primary->device,
            x86_dma_select(primary->device, first.requests), LIB_FALSE);
        return;
    }
    second = x86_dma_get_signals(secondary->device);
    if (!second.enabled) return;
    if (second.active_channel < 4u) {
        if (second.active_channel != 0u) {
            dma_bus_advance(secondary, latch, ram, transaction, LIB_TRUE);
        } else if (first.enabled && first.active_channel < 4u) {
            dma_bus_advance(primary, latch, ram, transaction, LIB_FALSE);
            if (x86_dma_get_signals(primary->device).active_channel == 4u) {
                x86_dma_release(secondary->device);
            }
        }
        return;
    }
    if (first.enabled && first.active_channel < 4u) {
        dma_bus_advance(primary, latch, ram, transaction, LIB_FALSE);
        return;
    }
    requests = second.requests & 0x0eu;
    if (first.enabled && first.requests) requests |= 1u;
    channel = x86_dma_select(secondary->device, requests);
    if (channel == 4u) return;
    x86_dma_grant(secondary->device, channel, channel == 0u);
    if (channel == 0u) {
        x86_dma_grant(primary->device,
            x86_dma_select(primary->device, first.requests), LIB_FALSE);
    }
}

lib_i32 core_machine_dma_has_pending_request(const t_dma *primary, const t_dma *secondary)
{
    x86_dma_signals first;
    x86_dma_signals second;
    if (primary == LIB_NULL || secondary == LIB_NULL || primary->device == LIB_NULL) return 0;
    first = x86_dma_get_signals(primary->device);
    if (secondary->device == LIB_NULL) {
        return first.enabled && (first.requests || first.active_channel < 4u);
    }
    second = x86_dma_get_signals(secondary->device);
    if (!second.enabled) return 0;
    return second.active_channel < 4u || (second.requests & 0x0eu) ||
        (first.enabled && (first.active_channel < 4u || first.requests));
}

void core_machine_dma_advance_transaction(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_ram *ram, core_machine_transaction_state *transaction,
    lib_u64 elapsed_ticks)
{
    lib_u64 tick;
    if (latch == LIB_NULL || primary == LIB_NULL || secondary == LIB_NULL ||
        primary->device == LIB_NULL || ram == LIB_NULL) return;
    for (tick = 0u; tick < elapsed_ticks; ++tick) {
        core_machine_dma_advance_one(latch, primary, secondary, ram, transaction);
    }
}

void core_machine_dma_finalize(t_latch *latch, t_dma *primary, t_dma *secondary)
{
    (void)latch;
    if (primary != LIB_NULL) {
        x86_dma_destroy(primary->device);
        primary->device = LIB_NULL;
    }
    if (secondary != LIB_NULL) {
        x86_dma_destroy(secondary->device);
        secondary->device = LIB_NULL;
    }
}

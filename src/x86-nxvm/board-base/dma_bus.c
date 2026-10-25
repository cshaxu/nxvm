/* Copyright 2012-2026 Neko. */
#include "x86-nxvm/board-base/dma_bus.h"
#include "x86-core/machine/memory_interface.h"

/* This only issues opaque binding nonces. It never selects a DMA instance. */
static lib_atomic_uptr x86_nxvm_board_dma_next_request_token = 1u;

static lib_uptr x86_nxvm_board_dma_request_token_allocate(void)
{
    lib_uptr expected = lib_atomic_uptr_load_explicit(&x86_nxvm_board_dma_next_request_token, LIB_MEMORY_ORDER_ACQUIRE);

    while (expected != LIB_UPTR_MAX) {
        if (lib_atomic_uptr_compare_exchange_strong_explicit(
                &x86_nxvm_board_dma_next_request_token, &expected, expected + 1u, LIB_MEMORY_ORDER_ACQ_REL, LIB_MEMORY_ORDER_ACQUIRE)) {
            return (lib_uptr)expected;
        }
    }
    return 0u;
}


static void x86_nxvm_board_dma_controller_reset(t_dma *dma)
{
    lib_memory_set(&dma->data, 0u, sizeof(dma->data));
    if (dma->device != LIB_NULL) x86_dma_reset(dma->device);
}

static void x86_nxvm_board_dma_update_cascade(t_dma *primary)
{
    x86_dma_signals signals;
    if (primary->connect.peer->device == LIB_NULL) return;
    signals = x86_dma_get_signals(primary->device);
    x86_dma_set_request(primary->connect.peer->device, 0u,
        signals.enabled && (signals.requests || signals.active_channel < 4u));
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


static lib_status dma_port_read(void *owner, lib_u16 port_id, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    t_dma *primary = owner;
    t_dma *dma;
    lib_u8 value;

    if (primary == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    value = (lib_u8)*out_value;
    if (primary->connect.page_ports_plain_scratch &&
        port_id >= 0x80u && port_id <= 0x8fu) {
        value = primary->data.page_scratch[port_id & 0x0fu];
    } else if (port_id < 0x10u) {
        x86_dma_read_register(primary->device, (lib_u8)port_id, &value);
    } else if (port_id >= 0xc0u && port_id <= 0xdeu && !(port_id & 1u)) {
        x86_dma_read_register(primary->connect.peer->device,
            (lib_u8)((port_id - 0xc0u) >> 1), &value);
    } else if (port_id == 0x80u || port_id == 0x84u || port_id == 0x85u ||
        port_id == 0x86u || port_id == 0x88u ||
        (port_id >= 0x8cu && port_id <= 0x8eu)) {
        value = primary->data.page_spare[dma_page_spare_index(port_id)];
    } else if (port_id >= 0x81u && port_id <= 0x8fu) {
        dma = dma_controller(primary, port_id);
        value = dma->data.page[dma_page_channel(port_id)];
    }
    *out_value = (*out_value & ~0xffu) | value;
    return LIB_STATUS_OK;
}

static lib_status dma_port_write(void *owner, lib_u16 port_id, lib_u32 value)
{
    t_dma *primary = owner;
    t_dma *dma;
    lib_u8 selector;
    if (primary->connect.page_ports_plain_scratch &&
        port_id >= 0x80u && port_id <= 0x8fu) {
        primary->data.page_scratch[port_id & 0x0fu] = (lib_u8)value;
        return LIB_STATUS_OK;
    }
    if (port_id == 0x80u || port_id == 0x84u || port_id == 0x85u ||
        port_id == 0x86u || port_id == 0x88u ||
        (port_id >= 0x8cu && port_id <= 0x8eu)) {
        primary->data.page_spare[dma_page_spare_index(port_id)] = (lib_u8)value;
        return LIB_STATUS_OK;
    }
    if (port_id >= 0x81u && port_id <= 0x8fu) {
        dma = dma_controller(primary, port_id);
        dma->data.page[dma_page_channel(port_id)] = (lib_u8)value;
        return LIB_STATUS_OK;
    }
    dma = port_id >= 0xc0u ? primary->connect.peer : primary;
    selector = (lib_u8)(port_id >= 0xc0u ? (port_id - 0xc0u) >> 1 : port_id);
    x86_dma_write_register(dma->device, selector, (lib_u8)value);
    x86_nxvm_board_dma_update_cascade(primary);
    return LIB_STATUS_OK;
}


typedef struct dma_cycle_context {
    t_dma *controller;
    t_latch *latch;
    x86_core_machine *machine;
    lib_bool word;
} dma_cycle_context;

static void dma_provider_call(x86_nxvm_board_dma_device_provider provider,
    void *owner, t_latch *latch)
{
    if (provider == LIB_NULL) return;
    t_latch value = *latch;
    provider(owner, &value);
    *latch = value;
}

static void dma_device_before_memory(void *owner, lib_u8 channel,
    lib_u16 *value)
{
    dma_cycle_context *context = owner;
    t_dma *dma = context->controller;
    dma_provider_call(dma->connect.read_provider[channel],
        dma->connect.device_owner[channel], context->latch);
    *value = context->word ? context->latch->data.word :
        context->latch->data.byte;
}

static void dma_device_after_memory(void *owner, lib_u8 channel,
    lib_u16 *value)
{
    dma_cycle_context *context = owner;
    t_dma *dma = context->controller;
    if (context->word) context->latch->data.word = *value;
    else context->latch->data.byte = (lib_u8)*value;
    dma_provider_call(dma->connect.write_provider[channel],
        dma->connect.device_owner[channel], context->latch);
}

static lib_status dma_bus_cycle(void *owner, x86_dma_cycle kind,
    lib_u8 channel, lib_u16 address, lib_u8 *byte)
{
    dma_cycle_context *context = owner;
    t_dma *dma = context->controller;
    lib_bool memory_only = kind == X86_DMA_MEMORY_READ || kind == X86_DMA_MEMORY_WRITE;
    lib_bool word = context->word && !memory_only;
    lib_bool write = kind == X86_DMA_DEVICE_TO_MEMORY || kind == X86_DMA_MEMORY_WRITE;
    lib_u8 page = dma->data.page[channel];
    lib_u8 bytes = word ? 2u : 1u;
    lib_u16 value = memory_only && write ? *byte : 0u;
    lib_u32 physical;
    lib_status status;
    if (kind == X86_DMA_VERIFY) {
        dma_provider_call(dma->connect.read_provider[channel],
            dma->connect.device_owner[channel], context->latch);
        return LIB_STATUS_OK;
    }
    if (word) page &= 0xfeu;
    physical = ((lib_u32)page << 16) + (word ? (lib_u32)address << 1 : address);
    status = x86_core_machine_dma_memory_cycle(context->machine, physical, bytes,
        channel, write ? X86_CORE_MACHINE_MEMORY_ACCESS_WRITE :
            X86_CORE_MACHINE_MEMORY_ACCESS_READ, &value,
        kind == X86_DMA_DEVICE_TO_MEMORY ? dma_device_before_memory : LIB_NULL,
        kind == X86_DMA_MEMORY_TO_DEVICE ? dma_device_after_memory : LIB_NULL,
        context);
    if (status == LIB_STATUS_OK && memory_only && !write) *byte = (lib_u8)value;
    return status;
}

static void dma_bus_terminal(void *owner, lib_u8 channel)
{
    dma_cycle_context *context = owner;
    t_dma *dma = context->controller;
    dma_provider_call(dma->connect.close_provider[channel],
        dma->connect.device_owner[channel], context->latch);
}

static void dma_bus_advance(t_dma *dma, t_latch *latch,
    x86_core_machine *machine, lib_bool word)
{
    dma_cycle_context context = { dma, latch, machine, word };
    x86_dma_bus bus = { dma_bus_cycle, dma_bus_terminal, &context };
    x86_dma_advance(dma->device, &bus);
}

static void x86_nxvm_board_dma_set_drq(t_dma *primary, t_dma *secondary,
    lib_u8 channel, lib_bool asserted)
{
    t_dma *dma = channel < 4u ? primary : secondary;
    if (channel == 4u || channel > 7u || dma->device == LIB_NULL) return;
    x86_dma_set_request(dma->device, channel & 3u, asserted);
    x86_nxvm_board_dma_update_cascade(primary);
}

lib_status x86_nxvm_board_dma_bind_channel(x86_nxvm_board_dma_bus *bus, lib_u8 drq_id,
    const x86_nxvm_board_dma_channel_provider *provider, void *owner,
    x86_nxvm_board_dma_request_binding *out_binding)
{
    t_dma *dma;
    lib_u8 channel;
    t_latch *latch;
    t_dma *primary;
    t_dma *secondary;

    if (bus == LIB_NULL ||
        provider == LIB_NULL || out_binding == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    latch = &bus->latch;
    primary = &bus->primary;
    secondary = &bus->secondary;
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
        primary->connect.request_token = x86_nxvm_board_dma_request_token_allocate();
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

void x86_nxvm_board_dma_request_assert(x86_nxvm_board_dma_bus *bus,
    const x86_nxvm_board_dma_request_binding *binding)
{
    if (bus == LIB_NULL) return;
    t_dma *primary = &bus->primary;
    t_dma *secondary = &bus->secondary;
    if (binding == LIB_NULL ||
        binding->core_token == 0u ||
        binding->core_token != primary->connect.request_token ||
        primary->connect.peer != secondary) return;
    x86_nxvm_board_dma_set_drq(primary, secondary,
        binding->channel, LIB_TRUE);
}

void x86_nxvm_board_dma_request_deassert(x86_nxvm_board_dma_bus *bus,
    const x86_nxvm_board_dma_request_binding *binding)
{
    if (bus == LIB_NULL) return;
    t_dma *primary = &bus->primary;
    t_dma *secondary = &bus->secondary;
    if (binding == LIB_NULL ||
        binding->core_token == 0u ||
        binding->core_token != primary->connect.request_token ||
        primary->connect.peer != secondary) return;
    x86_nxvm_board_dma_set_drq(primary, secondary,
        binding->channel, LIB_FALSE);
}

void x86_nxvm_board_dma_request_terminate(x86_nxvm_board_dma_bus *bus,
    const x86_nxvm_board_dma_request_binding *binding)
{
    t_dma *dma;
    lib_u8 channel;
    if (bus == LIB_NULL) return;
    t_dma *primary = &bus->primary;
    t_dma *secondary = &bus->secondary;

    if (binding == LIB_NULL ||
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


lib_status x86_nxvm_board_dma_initialize(x86_nxvm_board_dma_bus **out_bus,
    x86_core_machine *machine, lib_u8 controller_count,
    lib_bool page_ports_plain_scratch)
{
    static const lib_u16 primary_page_ports[] = {
        0x0081, 0x0082, 0x0083
    };
    static const lib_u16 secondary_page_ports[] = {
        0x0087, 0x0089, 0x008a, 0x008b, 0x008f
    };
    static const lib_u16 spare_page_ports[] = {
        0x0080, 0x0084, 0x0085, 0x0086, 0x0088, 0x008c, 0x008d, 0x008e
    };
    x86_core_machine_port_route routes[48];
    lib_size count = 0u;
    lib_uptr index;
    lib_status status;
    x86_nxvm_board_dma_bus *bus;
    t_latch *latch;
    t_dma *primary;
    t_dma *secondary;

    if (out_bus == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_bus = LIB_NULL;
    if (machine == LIB_NULL ||
        (controller_count != 1u && controller_count != 2u) ||
        (page_ports_plain_scratch != LIB_FALSE &&
         page_ports_plain_scratch != LIB_TRUE)) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    bus = lib_allocate_zero(1u, sizeof(*bus));
    if (bus == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    latch = &bus->latch;
    primary = &bus->primary;
    secondary = &bus->secondary;
    primary->connect.latch = latch;
    primary->connect.peer = secondary;
    primary->connect.page_ports_plain_scratch = page_ports_plain_scratch;
    secondary->connect.latch = latch;
    secondary->connect.peer = primary;
    status = x86_dma_create(&primary->device);
    if (status == LIB_STATUS_OK && controller_count == 2u) {
        status = x86_dma_create(&secondary->device);
    }
    if (status != LIB_STATUS_OK) {
        x86_nxvm_board_dma_finalize(bus);
        return status;
    }
    for (index = 0; index < 0x10u; ++index) {
        routes[count++] = (x86_core_machine_port_route) {(lib_u16)index,
            index <= 8u || index == 13u ? dma_port_read : LIB_NULL,
            dma_port_write, primary, LIB_FALSE, 0u};
    }
    if (page_ports_plain_scratch) {
        for (index = 0u; index < 0x10u; ++index) {
            routes[count++] = (x86_core_machine_port_route) {
                (lib_u16)(0x0080u + index), dma_port_read, dma_port_write,
                primary, LIB_FALSE, 0x0090u};
        }
    } else {
        for (index = 0; index < sizeof(primary_page_ports) /
                sizeof(primary_page_ports[0]);
             ++index) {
            routes[count++] = (x86_core_machine_port_route) {primary_page_ports[index],
                dma_port_read, dma_port_write, primary, LIB_FALSE, 0x0090u};
        }
    }
    if (controller_count == 2u && !page_ports_plain_scratch) {
        for (index = 0; index < sizeof(spare_page_ports) /
            sizeof(spare_page_ports[0]); ++index) {
            routes[count++] = (x86_core_machine_port_route) {spare_page_ports[index],
                dma_port_read, dma_port_write, primary, LIB_FALSE, 0x0090u};
        }
        for (index = 0; index < sizeof(secondary_page_ports) /
                sizeof(secondary_page_ports[0]); ++index) {
            routes[count++] = (x86_core_machine_port_route) {secondary_page_ports[index],
                dma_port_read, dma_port_write, primary, LIB_FALSE, 0x0090u};
        }
    }
    if (controller_count == 2u) {
        for (index = 0u; index < 0x10u; ++index) {
            routes[count++] = (x86_core_machine_port_route) {
                (lib_u16)(0x00c0u + index * 2u),
                index <= 8u || index == 13u ? dma_port_read : LIB_NULL,
                dma_port_write, primary, LIB_FALSE, 0u};
        }
    }
    status = x86_core_machine_install_port_routes(machine, routes, count);
    if (status != LIB_STATUS_OK) {
        x86_nxvm_board_dma_finalize(bus);
    } else {
        *out_bus = bus;
    }
    return status;
}


void x86_nxvm_board_dma_reset(x86_nxvm_board_dma_bus *bus)
{
    if (bus == LIB_NULL) return;
    t_latch *latch = &bus->latch;
    t_dma *primary = &bus->primary;
    t_dma *secondary = &bus->secondary;
    lib_memory_set(&latch->data, 0u, sizeof(latch->data));
    x86_nxvm_board_dma_controller_reset(primary);
    x86_nxvm_board_dma_controller_reset(secondary);
}

static void x86_nxvm_board_dma_advance_one(t_latch *latch, t_dma *primary,
    t_dma *secondary, x86_core_machine *machine)
{
    x86_dma_signals first = x86_dma_get_signals(primary->device);
    x86_dma_signals second;
    lib_u8 requests;
    lib_u8 channel;
    if (secondary->device == LIB_NULL) {
        if (!first.enabled) return;
        if (first.active_channel < 4u) dma_bus_advance(primary, latch, machine, LIB_FALSE);
        else if (first.requests) x86_dma_grant(primary->device,
            x86_dma_select(primary->device, first.requests), LIB_FALSE);
        return;
    }
    second = x86_dma_get_signals(secondary->device);
    if (!second.enabled) return;
    if (second.active_channel < 4u) {
        if (second.active_channel != 0u) {
            dma_bus_advance(secondary, latch, machine, LIB_TRUE);
        } else if (first.enabled && first.active_channel < 4u) {
            dma_bus_advance(primary, latch, machine, LIB_FALSE);
            if (x86_dma_get_signals(primary->device).active_channel == 4u) {
                x86_dma_release(secondary->device);
            }
        }
        return;
    }
    if (first.enabled && first.active_channel < 4u) {
        dma_bus_advance(primary, latch, machine, LIB_FALSE);
        return;
    }
    requests = second.requests & 0x0eu;
    if (first.enabled && first.requests) requests |= second.requests & 1u;
    channel = x86_dma_select(secondary->device, requests);
    if (channel == 4u) return;
    x86_dma_grant(secondary->device, channel, channel == 0u);
    if (channel == 0u) {
        x86_dma_grant(primary->device,
            x86_dma_select(primary->device, first.requests), LIB_FALSE);
    }
}

lib_bool x86_nxvm_board_dma_has_pending_request(const x86_nxvm_board_dma_bus *bus)
{
    x86_dma_signals first;
    x86_dma_signals second;
    if (bus == LIB_NULL || bus->primary.device == LIB_NULL) return LIB_FALSE;
    const t_dma *primary = &bus->primary;
    const t_dma *secondary = &bus->secondary;
    first = x86_dma_get_signals(primary->device);
    if (secondary->device == LIB_NULL) {
        return first.enabled && (first.requests || first.active_channel < 4u);
    }
    second = x86_dma_get_signals(secondary->device);
    if (!second.enabled) return LIB_FALSE;
    return second.active_channel < 4u || (second.requests & 0x0eu) ||
        (first.enabled && (first.active_channel < 4u || first.requests) &&
            (second.requests & 1u));
}

void x86_nxvm_board_dma_advance_transaction(x86_nxvm_board_dma_bus *bus,
    x86_core_machine *machine, lib_u64 elapsed_ticks)
{
    lib_u64 tick;
    if (bus == LIB_NULL || bus->primary.device == LIB_NULL || machine == LIB_NULL) return;
    for (tick = 0u; tick < elapsed_ticks; ++tick) {
        x86_nxvm_board_dma_advance_one(&bus->latch, &bus->primary, &bus->secondary, machine);
        x86_nxvm_board_dma_update_cascade(&bus->primary);
    }
}

void x86_nxvm_board_dma_finalize(x86_nxvm_board_dma_bus *bus)
{
    if (bus == LIB_NULL) return;
    x86_dma_destroy(bus->primary.device);
    x86_dma_destroy(bus->secondary.device);
    lib_release(bus);
}

x86_dma_signals x86_nxvm_board_dma_get_signals(const x86_nxvm_board_dma_bus *bus,
    lib_u8 controller)
{
    const x86_dma *chip = bus == LIB_NULL || controller > 1u ? LIB_NULL :
        (controller ? bus->secondary.device : bus->primary.device);
    return chip != LIB_NULL ? x86_dma_get_signals(chip) :
        (x86_dma_signals) {0u, 4u, LIB_FALSE};
}

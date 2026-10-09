/* Copyright 2012-2026 Neko. */
#ifndef CORE_MACHINE_DMA_BUS_INTERFACE_H
#define CORE_MACHINE_DMA_BUS_INTERFACE_H
#include "lib/types/types_interface.h"
#include "core/x86/port_interface.h"
#include "core/chips/dma8237/dma8237_interface.h"
typedef struct core_machine_dma_bus core_machine_dma_bus;
/* A copied bus-issued nonce, not a controller or guest-memory pointer. */
typedef struct core_machine_dma_request_binding {
    lib_uptr core_token;
    lib_u8 channel;
} core_machine_dma_request_binding;
/* Scoped transfer scratch value. A provider may update it during this call,
 * but must not retain its address or recursively advance/reset/destroy DMA.
 * Request/EOP changes through the supplied binding are allowed. */
typedef struct t_latch {
    union { lib_u8 byte; lib_u16 word; } data;
} t_latch;
typedef void (*core_machine_dma_device_provider)(void *owner, t_latch *value);
typedef struct core_machine_dma_channel_provider {
    core_machine_dma_device_provider read_device;
    core_machine_dma_device_provider write_device;
    core_machine_dma_device_provider terminal_count;
} core_machine_dma_channel_provider;
/* Serialized construction/runtime access. The bus owns its controllers,
 * pages and latch; provider contexts are borrowed until finalization.
 * Core copies port routes atomically. Stop providers and dispatch before
 * finalize, including Core attachment teardown before routes are discarded.
 * Failure publishes no bus. Reset preserves provider bindings and nonce. */
lib_status core_machine_dma_initialize(core_machine_dma_bus **out_bus,
    core_machine *machine, lib_u8 controller_count);
void core_machine_dma_finalize(core_machine_dma_bus *bus);
void core_machine_dma_reset(core_machine_dma_bus *bus);
void core_machine_dma_advance_transaction(core_machine_dma_bus *bus,
    core_machine *machine, lib_u64 elapsed_ticks);
lib_bool core_machine_dma_has_pending_request(const core_machine_dma_bus *bus);
lib_status core_machine_dma_bind_channel(core_machine_dma_bus *bus,
    lib_u8 channel, const core_machine_dma_channel_provider *provider,
    void *owner, core_machine_dma_request_binding *out_binding);
void core_machine_dma_request_assert(core_machine_dma_bus *bus,
    const core_machine_dma_request_binding *binding);
void core_machine_dma_request_deassert(core_machine_dma_bus *bus,
    const core_machine_dma_request_binding *binding);
void core_machine_dma_request_terminate(core_machine_dma_bus *bus,
    const core_machine_dma_request_binding *binding);
/* Copied live signals; controller 0/1 selects primary/secondary. An absent
 * controller reports disabled/no requests/no active grant (channel 4). */
x86_dma_signals core_machine_dma_get_signals(const core_machine_dma_bus *bus,
    lib_u8 controller);
#endif

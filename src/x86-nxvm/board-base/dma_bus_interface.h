/* Copyright 2012-2026 Neko. */
#ifndef X86_NXVM_BOARD_DMA_BUS_INTERFACE_H
#define X86_NXVM_BOARD_DMA_BUS_INTERFACE_H
#include "lib/types/types_interface.h"
#include "x86-core/machine/port_interface.h"
#include "x86-nxvm/chips/dma8237/dma8237_interface.h"
typedef struct x86_nxvm_board_dma_bus x86_nxvm_board_dma_bus;
/* A copied bus-issued nonce, not a controller or guest-memory pointer. */
typedef struct x86_nxvm_board_dma_request_binding {
    lib_uptr core_token;
    lib_u8 channel;
} x86_nxvm_board_dma_request_binding;
/* Scoped transfer scratch value. A provider may update it during this call,
 * but must not retain its address or recursively advance/reset/destroy DMA.
 * Request/EOP changes through the supplied binding are allowed. */
typedef struct t_latch {
    union { lib_u8 byte; lib_u16 word; } data;
} t_latch;
typedef void (*x86_nxvm_board_dma_device_provider)(void *owner, t_latch *value);
typedef struct x86_nxvm_board_dma_channel_provider {
    x86_nxvm_board_dma_device_provider read_device;
    x86_nxvm_board_dma_device_provider write_device;
    x86_nxvm_board_dma_device_provider terminal_count;
} x86_nxvm_board_dma_channel_provider;
/* Serialized construction/runtime access. The bus owns its controllers,
 * pages and latch; provider contexts are borrowed until finalization.
 * Core copies port routes atomically. Stop providers and dispatch before
 * finalize, including Core attachment teardown before routes are discarded.
 * Failure publishes no bus. Reset preserves provider bindings and nonce.
 * page_ports_plain_scratch selects one frozen board personality for 80h--8Fh:
 * false keeps standard DMA page registers; true selects sixteen independent
 * zero-reset read/write bytes. */
lib_status x86_nxvm_board_dma_initialize(x86_nxvm_board_dma_bus **out_bus,
    x86_core_machine *machine, lib_u8 controller_count,
    lib_bool page_ports_plain_scratch);
void x86_nxvm_board_dma_finalize(x86_nxvm_board_dma_bus *bus);
void x86_nxvm_board_dma_reset(x86_nxvm_board_dma_bus *bus);
void x86_nxvm_board_dma_advance_transaction(x86_nxvm_board_dma_bus *bus,
    x86_core_machine *machine, lib_u64 elapsed_ticks);
lib_bool x86_nxvm_board_dma_has_pending_request(const x86_nxvm_board_dma_bus *bus);
lib_status x86_nxvm_board_dma_bind_channel(x86_nxvm_board_dma_bus *bus,
    lib_u8 channel, const x86_nxvm_board_dma_channel_provider *provider,
    void *owner, x86_nxvm_board_dma_request_binding *out_binding);
void x86_nxvm_board_dma_request_assert(x86_nxvm_board_dma_bus *bus,
    const x86_nxvm_board_dma_request_binding *binding);
void x86_nxvm_board_dma_request_deassert(x86_nxvm_board_dma_bus *bus,
    const x86_nxvm_board_dma_request_binding *binding);
void x86_nxvm_board_dma_request_terminate(x86_nxvm_board_dma_bus *bus,
    const x86_nxvm_board_dma_request_binding *binding);
/* Copied live signals; controller 0/1 selects primary/secondary. An absent
 * controller reports disabled/no requests/no active grant (channel 4). */
x86_dma_signals x86_nxvm_board_dma_get_signals(const x86_nxvm_board_dma_bus *bus,
    lib_u8 controller);
#endif

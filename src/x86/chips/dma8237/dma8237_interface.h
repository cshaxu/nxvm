/* Copyright 2012-2026 Neko. */
#ifndef X86_DMA8237_INTERFACE_H
#define X86_DMA8237_INTERFACE_H
#include "lib/types/types_interface.h"

typedef struct x86_dma x86_dma;

typedef enum x86_dma_cycle {
    X86_DMA_VERIFY,
    X86_DMA_DEVICE_TO_MEMORY,
    X86_DMA_MEMORY_TO_DEVICE,
    X86_DMA_MEMORY_READ,
    X86_DMA_MEMORY_WRITE
} x86_dma_cycle;

/* A provider implements one physical cycle. Address is the controller's local
 * 16-bit address; width/page expansion belongs to the board. byte is used only
 * by MEMORY_READ/WRITE, never a pointer into controller state. */
typedef struct x86_dma_bus {
    lib_status (*cycle)(void *context, x86_dma_cycle kind, lib_u8 channel,
        lib_u16 address, lib_u8 *byte);
    void (*terminal)(void *context, lib_u8 channel);
    void *context;
} x86_dma_bus;

typedef struct x86_dma_signals {
    lib_u8 requests;
    lib_u8 active_channel; /* 4 means no active grant. */
    lib_bool enabled;
} x86_dma_signals;

/* One execution owner. No peer, physical memory, port map or host resources.
 * Providers may change request/EOP inputs but not recursively advance/reset
 * or destroy this controller. Stop all callers before destruction. */
lib_status x86_dma_create(x86_dma **out_dma);
void x86_dma_destroy(x86_dma *dma);
void x86_dma_reset(x86_dma *dma);
/* Selectors are the sixteen chip register addresses, not PC port numbers.
 * Undefined reads preserve *io_value. Status reads clear terminal-count bits. */
void x86_dma_read_register(x86_dma *dma, lib_u8 selector, lib_u8 *io_value);
void x86_dma_write_register(x86_dma *dma, lib_u8 selector, lib_u8 value);
void x86_dma_set_request(x86_dma *dma, lib_u8 channel, lib_bool asserted);
void x86_dma_terminate(x86_dma *dma);
x86_dma_signals x86_dma_get_signals(const x86_dma *dma);
/* Select from board-resolved candidates using this chip's priority. Returns
 * 4 when none. Grant starts service but never performs the first transfer. */
lib_u8 x86_dma_select(const x86_dma *dma, lib_u8 requests);
/* cascade marks a board-delegated grant: no local transfer is advanced; its
 * priority slot is consumed on grant. Release it when downstream service ends. */
void x86_dma_grant(x86_dma *dma, lib_u8 channel, lib_bool cascade);
void x86_dma_release(x86_dma *dma);
/* Advance exactly one input clock of an existing grant. The scoped provider
 * is not retained. Failure releases service without committing address/count. */
void x86_dma_advance(x86_dma *dma, const x86_dma_bus *bus);
#endif

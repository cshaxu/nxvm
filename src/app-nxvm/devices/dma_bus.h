/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_DMA_BUS_H
#define CORE_MACHINE_DMA_BUS_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "lib/types/types_interface.h"
#include "x86/chips/dma8237/dma8237_interface.h"

#include "app-nxvm/devices/controller_interface.h"
#include "app-nxvm/devices/port.h"

#define CORE_MACHINE_DEVICE_DMA "Intel 8237A"

    typedef lib_u8 t_page;
    typedef struct t_latch t_latch;
    typedef struct t_ram t_ram;
    typedef struct core_machine_transaction_state core_machine_transaction_state;
    typedef void (*core_machine_dma_device_provider)(void *owner, t_latch *latch);

    typedef struct core_machine_dma_channel_provider {
        core_machine_dma_device_provider read_device;
        core_machine_dma_device_provider write_device;
        core_machine_dma_device_provider terminal_count;
    } core_machine_dma_channel_provider;

#define VDMA_CHANNEL_COUNT 4

    typedef struct {
        t_page page[VDMA_CHANNEL_COUNT];
        t_page page_spare[8];
    } t_dma_data;

    typedef struct
    {
        /* Non-owning links to the same core-machine-owned DMA pair. */
        t_latch *latch;
        struct t_dma *peer;
        core_machine_dma_device_provider read_provider[VDMA_CHANNEL_COUNT];
        core_machine_dma_device_provider write_provider[VDMA_CHANNEL_COUNT];
        core_machine_dma_device_provider close_provider[VDMA_CHANNEL_COUNT];
        void *device_owner[VDMA_CHANNEL_COUNT];
        lib_uptr request_token;
    } t_dma_connect;

    typedef struct t_dma
    {
        x86_dma *device;
        t_dma_data data;
        t_dma_connect connect;
    } t_dma;

    typedef union
    {
        lib_u8 byte;
        lib_u16 word;
    } t_latch_data;

    struct t_latch
    {
        t_latch_data data;
    };

lib_status core_machine_dma_initialize(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_port *port, lib_u8 controller_count);
    void core_machine_dma_reset(t_latch *latch, t_dma *primary,
                                  t_dma *secondary);
    void core_machine_dma_advance_transaction(t_latch *latch,
        t_dma *primary, t_dma *secondary, t_ram *ram,
        core_machine_transaction_state *transaction,
        lib_u64 elapsed_ticks);
    lib_i32 core_machine_dma_has_pending_request(const t_dma *primary,
        const t_dma *secondary);
    lib_status core_machine_dma_bind_channel(t_latch *latch, t_dma *primary,
        t_dma *secondary, lib_u8 channel,
        const core_machine_dma_channel_provider *provider, void *device_owner,
        core_machine_dma_request_binding *out_binding);
    void core_machine_dma_request_assert(t_dma *primary, t_dma *secondary,
        const core_machine_dma_request_binding *binding);
    void core_machine_dma_request_deassert(t_dma *primary, t_dma *secondary,
        const core_machine_dma_request_binding *binding);
    void core_machine_dma_request_terminate(t_dma *primary, t_dma *secondary,
        const core_machine_dma_request_binding *binding);
    void core_machine_dma_finalize(t_latch *latch, t_dma *primary,
                                     t_dma *secondary);

#ifdef __cplusplus
} /*_EOCD_*/
#endif

#endif

/* Copyright 2012-2026 Neko. */
#ifndef CORE_MACHINE_DMA_BUS_H
#define CORE_MACHINE_DMA_BUS_H
#include "core/board-base/dma_bus_interface.h"
#define VDMA_CHANNEL_COUNT 4u
typedef struct t_dma {
    x86_dma *device;
    struct {
        lib_u8 page[VDMA_CHANNEL_COUNT];
        lib_u8 page_spare[8];
    } data;
    struct {
        t_latch *latch;
        struct t_dma *peer;
        core_machine_dma_device_provider read_provider[VDMA_CHANNEL_COUNT];
        core_machine_dma_device_provider write_provider[VDMA_CHANNEL_COUNT];
        core_machine_dma_device_provider close_provider[VDMA_CHANNEL_COUNT];
        void *device_owner[VDMA_CHANNEL_COUNT];
        lib_uptr request_token;
    } connect;
} t_dma;
struct core_machine_dma_bus {
    t_latch latch;
    t_dma primary;
    t_dma secondary;
};
#endif

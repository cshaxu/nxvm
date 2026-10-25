/* Copyright 2012-2026 Neko. */
#ifndef X86_NXVM_BOARD_DMA_BUS_H
#define X86_NXVM_BOARD_DMA_BUS_H
#include "x86-nxvm/board-base/dma_bus_interface.h"
#define VDMA_CHANNEL_COUNT 4u
typedef struct t_dma {
    x86_dma *device;
    struct {
        lib_u8 page[VDMA_CHANNEL_COUNT];
        lib_u8 page_spare[8];
        lib_u8 page_scratch[16];
    } data;
    struct {
        t_latch *latch;
        struct t_dma *peer;
        x86_nxvm_board_dma_device_provider read_provider[VDMA_CHANNEL_COUNT];
        x86_nxvm_board_dma_device_provider write_provider[VDMA_CHANNEL_COUNT];
        x86_nxvm_board_dma_device_provider close_provider[VDMA_CHANNEL_COUNT];
        void *device_owner[VDMA_CHANNEL_COUNT];
        lib_uptr request_token;
        lib_bool page_ports_plain_scratch;
    } connect;
} t_dma;
struct x86_nxvm_board_dma_bus {
    t_latch latch;
    t_dma primary;
    t_dma secondary;
};
#endif

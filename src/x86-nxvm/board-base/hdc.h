#ifndef X86_NXVM_BOARD_HDC_H
#define X86_NXVM_BOARD_HDC_H
#include "x86-nxvm/board-base/hdc_interface.h"
#include "x86-nxvm/chips/hdc/hdc_interface.h"
#include "x86-nxvm/board-base/controller_interface.h"
#include "x86-nxvm/board-base/dma_bus_interface.h"
#include "x86-nxvm/board-base/media_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-core/machine/port_interface.h"

typedef struct x86_nxvm_board_hdc_connection {
    const x86_nxvm_board_media_registry *media_registry;
    x86_nxvm_board_media_id media_id;
    x86_nxvm_board_media_id slave_media_id;
    x86_nxvm_board_pic_irq_source *irq_source;
    x86_nxvm_board_dma_request_binding dma_request;
    void (*dma_request_assert)(void *owner,
        const x86_nxvm_board_dma_request_binding *binding);
    void (*dma_request_deassert)(void *owner,
        const x86_nxvm_board_dma_request_binding *binding);
    void *dma_request_owner;
    x86_nxvm_board_hdc_config config;
} x86_nxvm_board_hdc_connection;


struct x86_nxvm_board_hdc {
    x86_hdc *chips[2];
    lib_u8 selected_controller;
    x86_nxvm_board_hdc_connection connect;
};

#endif

/* Copyright 2012-2026 Neko. */
#ifndef X86_NXVM_BOARD_FDC_INTERFACE_H
#define X86_NXVM_BOARD_FDC_INTERFACE_H

#include "x86-nxvm/board-base/controller_interface.h"
#include "x86-nxvm/board-base/fdc_observation_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-core/machine/port_interface.h"

typedef struct x86_nxvm_board_fdc x86_nxvm_board_fdc;
typedef void (*x86_nxvm_board_fdc_dma_request_operation)(void *owner,
    const x86_nxvm_board_dma_request_binding *binding);

/* A construction candidate starts without routes or a chip. Its stable
 * address can be bound to DMA before the frozen drive topology is applied.
 * The board owns it until serialized Core teardown; failed configuration
 * leaves that candidate reusable. No callback may outlive destruction. */
lib_status x86_nxvm_board_fdc_create(x86_nxvm_board_fdc **out_fdc);
void x86_nxvm_board_fdc_destroy(x86_nxvm_board_fdc *fdc);
lib_status x86_nxvm_board_fdc_configure(x86_nxvm_board_fdc *fdc,
    const x86_nxvm_board_media_registry *media_registry,
    const x86_nxvm_board_fdc_drive_bindings *drives,
    const x86_nxvm_board_dma_request_binding *dma_request,
    x86_nxvm_board_fdc_dma_request_operation request_assert,
    x86_nxvm_board_fdc_dma_request_operation request_deassert,
    void *request_owner, x86_nxvm_board_pic_bus *pic_master,
    x86_nxvm_board_pic_bus *pic_slave, x86_core_machine *machine,
    const x86_nxvm_board_fdc_config *config,
    const x86_nxvm_board_fdc_terminal_observation_provider *observation_provider);
const x86_nxvm_board_dma_channel_provider *x86_nxvm_board_fdc_dma_provider(void);
void x86_nxvm_board_fdc_reset(x86_nxvm_board_fdc *fdc);
void x86_nxvm_board_fdc_advance_at(x86_nxvm_board_fdc *fdc, lib_u64 tick);
lib_status x86_nxvm_board_fdc_next_due_tick(const x86_nxvm_board_fdc *fdc,
    lib_u64 *out_due_tick);
void x86_nxvm_board_fdc_refresh(x86_nxvm_board_fdc *fdc);

#endif

/* Copyright 2012-2026 Neko. */
#ifndef CORE_MACHINE_FDC_INTERFACE_H
#define CORE_MACHINE_FDC_INTERFACE_H

#include "ibmpc/board-common/controller_interface.h"
#include "ibmpc/board-common/fdc_observation_interface.h"
#include "ibmpc/board-common/pic_bus_interface.h"
#include "x86/core/port_interface.h"

typedef struct core_machine_fdc core_machine_fdc;
typedef void (*core_machine_fdc_dma_request_operation)(void *owner,
    const core_machine_dma_request_binding *binding);

/* A construction candidate starts without routes or a chip. Its stable
 * address can be bound to DMA before the frozen drive topology is applied.
 * The board owns it until serialized Core teardown; failed configuration
 * leaves that candidate reusable. No callback may outlive destruction. */
lib_status core_machine_fdc_create(core_machine_fdc **out_fdc);
void core_machine_fdc_destroy(core_machine_fdc *fdc);
lib_status core_machine_fdc_configure(core_machine_fdc *fdc,
    const core_machine_media_registry *media_registry,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_dma_request_binding *dma_request,
    core_machine_fdc_dma_request_operation request_assert,
    core_machine_fdc_dma_request_operation request_deassert,
    void *request_owner, core_machine_pic_bus *pic_master,
    core_machine_pic_bus *pic_slave, core_machine *machine,
    const core_machine_fdc_config *config,
    const core_machine_fdc_terminal_observation_provider *observation_provider);
const core_machine_dma_channel_provider *core_machine_fdc_dma_provider(void);
void core_machine_fdc_reset(core_machine_fdc *fdc);
void core_machine_fdc_advance_at(core_machine_fdc *fdc, lib_u64 tick);
lib_status core_machine_fdc_next_due_tick(const core_machine_fdc *fdc,
    lib_u64 *out_due_tick);
void core_machine_fdc_refresh(core_machine_fdc *fdc);

#endif

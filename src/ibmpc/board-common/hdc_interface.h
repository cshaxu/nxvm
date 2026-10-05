#ifndef CORE_MACHINE_HDC_INTERFACE_H
#define CORE_MACHINE_HDC_INTERFACE_H

#include "ibmpc/board-common/controller_interface.h"
#include "ibmpc/board-common/pic_bus_interface.h"
#include "x86/core/port_interface.h"

typedef struct core_machine_hdc core_machine_hdc;

/* The board owns this opaque candidate through serialized Core teardown.
 * Configuration creates the chip once; the board publishes its port routes
 * through Core's sole route transaction. Provider callbacks borrow the handle
 * and cannot retain transfer scratch or survive destruction. */
lib_status core_machine_hdc_create(core_machine_hdc **out_hdc);
void core_machine_hdc_destroy(core_machine_hdc *hdc);
/* Construction rollback only, after the caller has revoked any published
 * Core routes. Keep the stable candidate address for a later valid attempt. */
void core_machine_hdc_clear_configuration(core_machine_hdc *hdc);
lib_status core_machine_hdc_configure(core_machine_hdc *hdc,
    const core_machine_media_registry *media_registry,
    core_machine_media_id media_id, core_machine_media_id slave_media_id,
    core_machine_pic_bus *pic_master, core_machine_pic_bus *pic_slave,
    const core_machine_hdc_config *config);
void core_machine_hdc_bind_dma_request(core_machine_hdc *hdc,
    const core_machine_dma_request_binding *binding,
    void (*request_assert)(void *owner,
        const core_machine_dma_request_binding *binding),
    void (*request_deassert)(void *owner,
        const core_machine_dma_request_binding *binding), void *owner);
const core_machine_port_provider *core_machine_hdc_port_provider(void);
const core_machine_dma_channel_provider *core_machine_hdc_dma_provider(void);
void core_machine_hdc_reset(core_machine_hdc *hdc);
void core_machine_hdc_advance_at(core_machine_hdc *hdc, lib_u64 tick);
lib_status core_machine_hdc_next_due_tick(const core_machine_hdc *hdc,
    lib_u64 *out_due_tick);
lib_u8 core_machine_hdc_irq_pending(const core_machine_hdc *hdc);

#endif

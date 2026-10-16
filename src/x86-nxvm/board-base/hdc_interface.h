#ifndef X86_NXVM_BOARD_HDC_INTERFACE_H
#define X86_NXVM_BOARD_HDC_INTERFACE_H

#include "x86-nxvm/board-base/controller_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-core/machine/port_interface.h"

typedef struct x86_nxvm_board_hdc x86_nxvm_board_hdc;

/* The board owns this opaque candidate through serialized Core teardown.
 * Configuration creates one controller, or independent Compaq C/D controllers
 * for its C/D endpoints. An unbound image is absent media, not controller
 * hotplug. The board publishes one set of port routes
 * through Core's sole route transaction. Provider callbacks borrow the handle
 * and cannot retain transfer scratch or survive destruction. */
lib_status x86_nxvm_board_hdc_create(x86_nxvm_board_hdc **out_hdc);
void x86_nxvm_board_hdc_destroy(x86_nxvm_board_hdc *hdc);
/* Construction rollback only, after the caller has revoked any published
 * Core routes. Keep the stable candidate address for a later valid attempt. */
void x86_nxvm_board_hdc_clear_configuration(x86_nxvm_board_hdc *hdc);
lib_status x86_nxvm_board_hdc_configure(x86_nxvm_board_hdc *hdc,
    const x86_nxvm_board_media_registry *media_registry,
    x86_nxvm_board_media_id media_id, x86_nxvm_board_media_id slave_media_id,
    x86_nxvm_board_pic_bus *pic_master, x86_nxvm_board_pic_bus *pic_slave,
    const x86_nxvm_board_hdc_config *config);
void x86_nxvm_board_hdc_bind_dma_request(x86_nxvm_board_hdc *hdc,
    const x86_nxvm_board_dma_request_binding *binding,
    void (*request_assert)(void *owner,
        const x86_nxvm_board_dma_request_binding *binding),
    void (*request_deassert)(void *owner,
        const x86_nxvm_board_dma_request_binding *binding), void *owner);
const x86_core_machine_port_provider *x86_nxvm_board_hdc_port_provider(void);
const x86_nxvm_board_dma_channel_provider *x86_nxvm_board_hdc_dma_provider(void);
void x86_nxvm_board_hdc_reset(x86_nxvm_board_hdc *hdc);
void x86_nxvm_board_hdc_advance_at(x86_nxvm_board_hdc *hdc, lib_u64 tick);
lib_status x86_nxvm_board_hdc_next_due_tick(const x86_nxvm_board_hdc *hdc,
    lib_u64 *out_due_tick);
lib_u8 x86_nxvm_board_hdc_irq_pending(const x86_nxvm_board_hdc *hdc);

#endif

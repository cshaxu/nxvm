#ifndef CORE_MACHINE_HDC_H
#define CORE_MACHINE_HDC_H
#include "lib/types/types_interface.h"
#include "x86/chips/hdc/hdc_interface.h"
#include "app-nxvm/devices/controller_interface.h"
#include "app-nxvm/devices/dma_bus.h"
#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/devices/pic_bus.h"
#include "x86/core/port_interface.h"

typedef struct core_machine_hdc_connection {
    const core_machine_media_registry *media_registry;
    core_machine_media_id media_id;
    core_machine_media_id slave_media_id;
    core_machine_pic_irq_source irq_source;
    core_machine_dma_request_binding dma_request;
    void (*dma_request_assert)(void *owner,
        const core_machine_dma_request_binding *binding);
    void (*dma_request_deassert)(void *owner,
        const core_machine_dma_request_binding *binding);
    void *dma_request_owner;
    core_machine_hdc_config config;
} core_machine_hdc_connection;


typedef struct core_machine_hdc {
    x86_hdc *chip;
    core_machine_hdc_connection connect;
} core_machine_hdc;

void core_machine_hdc_connect(core_machine_hdc *hdc,
    const core_machine_media_registry *media_registry,
    core_machine_media_id media_id, core_machine_media_id slave_media_id,
    core_machine_pic_bus *pic_master, core_machine_pic_bus *pic_slave, const core_machine_hdc_config *config);
void core_machine_hdc_bind_dma_request(core_machine_hdc *hdc,
    const core_machine_dma_request_binding *binding,
    void (*request_assert)(void *owner,
        const core_machine_dma_request_binding *binding),
    void (*request_deassert)(void *owner,
        const core_machine_dma_request_binding *binding), void *owner);
lib_status core_machine_hdc_initialize(core_machine_hdc *hdc);
void core_machine_hdc_reset(core_machine_hdc *hdc);
void core_machine_hdc_advance_at(core_machine_hdc *hdc, lib_u64 now);
lib_status core_machine_hdc_next_due_tick(const core_machine_hdc *hdc,
    lib_u64 *out_due_tick);
void core_machine_hdc_finalize(core_machine_hdc *hdc);
const core_machine_port_provider *core_machine_hdc_port_provider(void);
const core_machine_dma_channel_provider *core_machine_hdc_dma_provider(void);
lib_u8 core_machine_hdc_irq_pending(const core_machine_hdc *hdc);

#endif

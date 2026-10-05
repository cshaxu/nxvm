#ifndef CORE_MACHINE_HDC_H
#define CORE_MACHINE_HDC_H
#include "ibmpc/board-common/hdc_interface.h"
#include "x86/chips/hdc/hdc_interface.h"
#include "ibmpc/board-common/controller_interface.h"
#include "ibmpc/board-common/dma_bus_interface.h"
#include "ibmpc/board-common/media_interface.h"
#include "ibmpc/board-common/pic_bus_interface.h"
#include "x86/core/port_interface.h"

typedef struct core_machine_hdc_connection {
    const core_machine_media_registry *media_registry;
    core_machine_media_id media_id;
    core_machine_media_id slave_media_id;
    core_machine_pic_irq_source *irq_source;
    core_machine_dma_request_binding dma_request;
    void (*dma_request_assert)(void *owner,
        const core_machine_dma_request_binding *binding);
    void (*dma_request_deassert)(void *owner,
        const core_machine_dma_request_binding *binding);
    void *dma_request_owner;
    core_machine_hdc_config config;
} core_machine_hdc_connection;


struct core_machine_hdc {
    x86_hdc *chip;
    core_machine_hdc_connection connect;
};

#endif

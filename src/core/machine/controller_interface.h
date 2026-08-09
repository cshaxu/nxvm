#ifndef CORE_MACHINE_CONTROLLER_INTERFACE_H
#define CORE_MACHINE_CONTROLLER_INTERFACE_H

#include "type.h"

#include "core/machine/media_interface.h"

#define CORE_MACHINE_FDC_DRIVE_COUNT 4u

/* A controller may retain this frozen core-issued nonce, but never DMA
 * registers, guest memory, a controller implementation pointer, or an address.
 * Core validates it against the receiving machine's private DMA state. */
typedef struct core_machine_dma_request_binding {
    type_native_unsigned core_token;
    uint8_t channel;
} core_machine_dma_request_binding;

typedef struct core_machine_fdc_config {
    uint16_t dor_port;
    uint16_t status_port;
    uint16_t data_port;
    uint16_t direction_port;
    uint16_t control_port;
    uint8_t irq;
    uint8_t dma_channel;
} core_machine_fdc_config;

typedef struct core_machine_fdc_drive_bindings {
    core_machine_media_id media_id[CORE_MACHINE_FDC_DRIVE_COUNT];
} core_machine_fdc_drive_bindings;

typedef struct core_machine_hdc_config {
    uint16_t data_port;
    uint16_t error_features_port;
    uint16_t sector_count_port;
    uint16_t sector_number_port;
    uint16_t cylinder_low_port;
    uint16_t cylinder_high_port;
    uint16_t drive_head_port;
    uint16_t status_command_port;
    uint16_t alternate_status_device_control_port;
    uint8_t irq;
    type_bool lba28_supported;
} core_machine_hdc_config;

#endif

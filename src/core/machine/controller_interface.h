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
    type_unsigned_8 channel;
} core_machine_dma_request_binding;

typedef enum core_machine_fdc_unready_read_policy {
    CORE_MACHINE_FDC_UNREADY_READ_GENERIC = 0,
    CORE_MACHINE_FDC_UNREADY_READ_DESKPRO_REFERENCE
} core_machine_fdc_unready_read_policy;

typedef struct core_machine_fdc_config {
    type_unsigned_16 dor_port;
    type_unsigned_16 status_port;
    type_unsigned_16 data_port;
    type_unsigned_16 direction_port;
    type_unsigned_16 control_port;
    type_unsigned_8 irq;
    type_unsigned_8 dma_channel;
    core_machine_fdc_unready_read_policy unready_read_policy;
} core_machine_fdc_config;

typedef struct core_machine_fdc_drive_bindings {
    core_machine_media_id media_id[CORE_MACHINE_FDC_DRIVE_COUNT];
} core_machine_fdc_drive_bindings;

typedef enum core_machine_hdc_protocol {
    CORE_MACHINE_HDC_PROTOCOL_INVALID = 0,
    CORE_MACHINE_HDC_PROTOCOL_ATA_PIO,
    CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB
} core_machine_hdc_protocol;

/* The protocol is explicit, so zero-initialization cannot silently select ATA.
 * Current admitted personalities share this task-file shape. A later
 * non-task-file personality must replace this public shape in its own bounded
 * task rather than inherit invalid ATA semantics. */
typedef struct core_machine_hdc_config {
    core_machine_hdc_protocol protocol;
    type_unsigned_16 data_port;
    type_unsigned_16 error_features_port;
    type_unsigned_16 sector_count_port;
    type_unsigned_16 sector_number_port;
    type_unsigned_16 cylinder_low_port;
    type_unsigned_16 cylinder_high_port;
    type_unsigned_16 drive_head_port;
    type_unsigned_16 status_command_port;
    type_unsigned_16 alternate_status_device_control_port;
    type_unsigned_16 drive_address_port;
    type_unsigned_8 irq;
    type_bool lba28_supported;
} core_machine_hdc_config;

#endif

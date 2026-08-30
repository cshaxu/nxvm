#ifndef CORE_MACHINE_CONTROLLER_INTERFACE_H
#define CORE_MACHINE_CONTROLLER_INTERFACE_H

#include "type.h"

#include "core/machine/media_interface.h"

#define CORE_MACHINE_FDC_DRIVE_COUNT 4u
#define CORE_MACHINE_XEBEC_TYPE_2_LOGICAL_SECTOR_COUNT 41820u
#define CORE_MACHINE_XEBEC_TYPE_2_BYTES_PER_SECTOR 512u
#define CORE_MACHINE_XEBEC_TYPE_2_CYLINDERS 615u
#define CORE_MACHINE_XEBEC_TYPE_2_HEADS 4u
#define CORE_MACHINE_XEBEC_TYPE_2_SECTORS_PER_TRACK 17u

typedef enum core_machine_pit_personality {
    CORE_MACHINE_PIT_PERSONALITY_8254 = 0,
    CORE_MACHINE_PIT_PERSONALITY_8253
} core_machine_pit_personality;

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
    /* Zero omits an optional direction or control endpoint. */
    type_unsigned_16 direction_port;
    type_unsigned_16 control_port;
    type_unsigned_8 irq;
    type_unsigned_8 dma_channel;
    core_machine_fdc_unready_read_policy unready_read_policy;
    /* Frozen board READY inputs, one bit per controller drive select.  They
     * are mechanical wiring facts, never inferred from inserted media. */
    type_unsigned_8 ready_mask;
    /* Frozen conversion into the Core virtual axis.  This is an L2 macro
     * ratio unless a selected board source qualifies it; Core uses it only to
     * convert the 8272A's own microsecond formulas into its elapsed axis. */
    type_unsigned_32 clock_ticks_per_second;
} core_machine_fdc_config;

typedef struct core_machine_fdc_drive_bindings {
    core_machine_media_id media_id[CORE_MACHINE_FDC_DRIVE_COUNT];
    /* Physical units are not inferred from inserted media.  The 8272A status
     * input can observe an installed but empty drive. */
    type_unsigned_8 installed_mask;
    /* ST3 TS is a physical drive property, not a property of removable media. */
    type_unsigned_8 double_sided_mask;
    /* A profile may declare the mechanical cylinder count independently of
     * a mounted medium.  Zero retains the controller's unbounded fallback. */
    type_unsigned_16 cylinder_count[CORE_MACHINE_FDC_DRIVE_COUNT];
    /* Board wiring may invert a drive's raw Track 0 input before it reaches
     * the controller.  Unset preserves the 8272A's ordinary active-high ST3
     * representation. */
    type_unsigned_8 track_zero_active_low_mask;
} core_machine_fdc_drive_bindings;

typedef enum core_machine_hdc_protocol {
    CORE_MACHINE_HDC_PROTOCOL_INVALID = 0,
    CORE_MACHINE_HDC_PROTOCOL_ATA_PIO,
    CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB,
    CORE_MACHINE_HDC_PROTOCOL_IBM_WD1003_ST506,
    CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT
} core_machine_hdc_protocol;

/* One immutable protocol tag selects exactly one bus shape.  Task-file
 * personalities share their registers; Xebec does not inherit them. */
typedef struct core_machine_hdc_task_file_config {
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
    type_bool lba28_supported;
    /* Required only by IBM WD1003: selected Core axis rate for its manual
     * step-rate selector, never a host clock or HDC-owned time axis. */
    type_unsigned_32 clock_ticks_per_second;
} core_machine_hdc_task_file_config;

/* This is an immutable adapter/drive pairing selected by a profile at
 * construction. It is not a runtime media choice; the media provider remains
 * the sole owner of inserted bytes and current media state. */
typedef enum core_machine_xebec_drive_type {
    CORE_MACHINE_XEBEC_DRIVE_TYPE_INVALID = 0,
    CORE_MACHINE_XEBEC_DRIVE_TYPE_2
} core_machine_xebec_drive_type;

typedef struct core_machine_hdc_xebec_config {
    type_unsigned_16 data_port;
    type_unsigned_16 hardware_status_reset_port;
    type_unsigned_16 jumpers_select_port;
    type_unsigned_16 dma_irq_mask_port;
    type_unsigned_8 dma_channel;
    core_machine_xebec_drive_type drive_type;
    core_machine_media_geometry expected_media_geometry;
} core_machine_hdc_xebec_config;

/* Construction-time Core elapsed-tick deadlines.  Command and task-file
 * next-sector are the only separately scheduled logical transitions; a
 * personality without one selects zero.  Core owns scheduling and never
 * interprets these values as host time. */
typedef struct core_machine_hdc_service_config {
    type_unsigned_32 command_ticks;
    type_unsigned_32 next_sector_ticks;
} core_machine_hdc_service_config;

typedef struct core_machine_hdc_config {
    core_machine_hdc_protocol protocol;
    type_unsigned_8 irq;
    /* Frozen Core elapsed-tick deadlines.  Their Manual-L3 or Other-L2
     * provenance belongs to construction evidence, not Core.  Zero remains an
     * immediate owner deadline without inventing a duration. */
    core_machine_hdc_service_config service;
    union {
        core_machine_hdc_task_file_config task_file;
        core_machine_hdc_xebec_config xebec;
    } bus;
} core_machine_hdc_config;

#endif

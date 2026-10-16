#ifndef X86_NXVM_BOARD_CONTROLLER_INTERFACE_H
#define X86_NXVM_BOARD_CONTROLLER_INTERFACE_H
#include "lib/types/types_interface.h"


#include "x86-nxvm/board-base/media_interface.h"
#include "x86-nxvm/board-base/dma_bus_interface.h"

#define X86_NXVM_BOARD_FDC_DRIVE_COUNT 4u
#define X86_NXVM_BOARD_XEBEC_TYPE_2_LOGICAL_SECTOR_COUNT 41820u
#define X86_NXVM_BOARD_XEBEC_TYPE_2_BYTES_PER_SECTOR 512u
#define X86_NXVM_BOARD_XEBEC_TYPE_2_CYLINDERS 615u
#define X86_NXVM_BOARD_XEBEC_TYPE_2_HEADS 4u
#define X86_NXVM_BOARD_XEBEC_TYPE_2_SECTORS_PER_TRACK 17u

typedef struct x86_nxvm_board_fdc_config {
    lib_u16 dor_port;
    lib_u16 status_port;
    lib_u16 data_port;
    /* Zero omits an optional direction or control endpoint. */
    lib_u16 direction_port;
    lib_u16 control_port;
    lib_u8 irq;
    lib_u8 dma_channel;
    /* Frozen board READY inputs, one bit per controller drive select.  They
     * are electrical wiring facts, never inferred from inserted media. */
    lib_u8 ready_mask;
    /* Frozen conversion into the Core virtual axis.  This is an L2 macro
     * ratio unless a selected board source qualifies it; Core uses it only to
     * convert the 8272A's own microsecond formulas into its elapsed axis. */
    lib_u32 clock_ticks_per_second;
    /* Optional board-owned FDC diagnostic endpoint. It is not an 8272A
     * register: a selected board may expose a read-only capability code. */
    lib_u16 diagnostic_port;
    lib_u8 diagnostic_read_value;
    /* The ordinary controller reset rate remains 250 kb/s.  A board whose
     * reset wiring selects another CCR value must opt in explicitly. */
    lib_bool initial_ccr_configured;
    lib_u8 initial_ccr;
} x86_nxvm_board_fdc_config;

/* Flat-media recording is a drive/board contract, not an 8272A identity rule.
 * The borrowed immutable context outlives the controller. A synchronous sample
 * must not re-enter it. False means no readable/formattable track at this
 * rate/position; only the controller chooses command status and completion. */
typedef lib_bool (*x86_nxvm_board_fdc_channel_sample)(const void *context,
    lib_u8 drive, const x86_nxvm_board_media_geometry *geometry,
    lib_u16 physical_cylinder, lib_u32 rate_bps, lib_bool mfm,
    lib_u16 *out_cylinder);

typedef struct x86_nxvm_board_fdc_channel_provider {
    x86_nxvm_board_fdc_channel_sample sample;
    const void *context;
} x86_nxvm_board_fdc_channel_provider;

typedef struct x86_nxvm_board_fdc_drive_bindings {
    x86_nxvm_board_media_id media_id[X86_NXVM_BOARD_FDC_DRIVE_COUNT];
    /* Physical units are not inferred from inserted media.  The 8272A status
     * input can observe an installed but empty drive. */
    lib_u8 installed_mask;
    /* ST3 TS is a physical drive property, not a property of removable media. */
    lib_u8 double_sided_mask;
    /* A profile may declare the mechanical cylinder count independently of
     * a mounted medium.  Zero retains the controller's unbounded fallback. */
    lib_u16 cylinder_count[X86_NXVM_BOARD_FDC_DRIVE_COUNT];
    /* Board wiring may invert a drive's raw Track 0 input before it reaches
     * the controller.  Unset preserves the 8272A's ordinary active-high ST3
     * representation. */
    lib_u8 track_zero_active_low_mask;
    x86_nxvm_board_fdc_channel_provider channel;
} x86_nxvm_board_fdc_drive_bindings;

typedef enum x86_nxvm_board_hdc_protocol {
    X86_NXVM_BOARD_HDC_PROTOCOL_INVALID = 0,
    X86_NXVM_BOARD_HDC_PROTOCOL_ATA_PIO,
    X86_NXVM_BOARD_HDC_PROTOCOL_COMPAQ_WD_40MB,
    X86_NXVM_BOARD_HDC_PROTOCOL_IBM_WD1003_ST506,
    X86_NXVM_BOARD_HDC_PROTOCOL_XEBEC_XT
} x86_nxvm_board_hdc_protocol;

/* One immutable protocol tag selects exactly one bus shape.  Task-file
 * personalities share their registers; Xebec does not inherit them. */
typedef struct x86_nxvm_board_hdc_task_file_config {
    lib_u16 data_port;
    lib_u16 error_features_port;
    lib_u16 sector_count_port;
    lib_u16 sector_number_port;
    lib_u16 cylinder_low_port;
    lib_u16 cylinder_high_port;
    lib_u16 drive_head_port;
    lib_u16 status_command_port;
    lib_u16 alternate_status_device_control_port;
    lib_u16 drive_address_port;
    lib_u8 lba28_supported;
    /* Required only by IBM WD1003: selected Core axis rate for its manual
     * step-rate selector, never a host clock or HDC-owned time axis. */
    lib_u32 clock_ticks_per_second;
} x86_nxvm_board_hdc_task_file_config;

/* This is an immutable adapter/drive pairing selected by a profile at
 * construction. It is not a runtime media choice; the media provider remains
 * the sole owner of inserted bytes and current media state. */
typedef enum x86_nxvm_board_xebec_drive_type {
    X86_NXVM_BOARD_XEBEC_DRIVE_TYPE_INVALID = 0,
    X86_NXVM_BOARD_XEBEC_DRIVE_TYPE_2
} x86_nxvm_board_xebec_drive_type;

typedef struct x86_nxvm_board_hdc_xebec_config {
    lib_u16 data_port;
    lib_u16 hardware_status_reset_port;
    lib_u16 jumpers_select_port;
    lib_u16 dma_irq_mask_port;
    lib_u8 dma_channel;
    x86_nxvm_board_xebec_drive_type drive_type;
    x86_nxvm_board_media_geometry expected_media_geometry;
} x86_nxvm_board_hdc_xebec_config;

/* Construction-time Core elapsed-tick deadlines.  Command and task-file
 * next-sector are the only separately scheduled logical transitions; a
 * personality without one selects zero.  Core owns scheduling and never
 * interprets these values as host time. */
typedef struct x86_nxvm_board_hdc_service_config {
    lib_u32 command_ticks;
    lib_u32 next_sector_ticks;
} x86_nxvm_board_hdc_service_config;

typedef struct x86_nxvm_board_hdc_config {
    x86_nxvm_board_hdc_protocol protocol;
    lib_u8 irq;
    /* Frozen Core elapsed-tick deadlines.  Their Manual-L3 or Other-L2
     * provenance belongs to construction evidence, not Core.  Zero remains an
     * immediate owner deadline without inventing a duration. */
    x86_nxvm_board_hdc_service_config service;
    union {
        x86_nxvm_board_hdc_task_file_config task_file;
        x86_nxvm_board_hdc_xebec_config xebec;
    } bus;
} x86_nxvm_board_hdc_config;

#endif

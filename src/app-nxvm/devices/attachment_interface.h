#ifndef CORE_MACHINE_ATTACHMENT_INTERFACE_H
#define CORE_MACHINE_ATTACHMENT_INTERFACE_H

#include "lib/types/types_interface.h"

typedef struct core_machine core_machine;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine_attachment_deadline_observation {
    lib_u64 source_ticks;
    lib_u8 immediate_due;
    lib_u8 l1_compatibility;
    lib_u8 fast_advance_blocked;
} core_machine_attachment_deadline_observation;
typedef void (*core_machine_attachment_deadline_provider)(void *owner, lib_u64 now,
    lib_bool timing_qualified,
    core_machine_attachment_deadline_observation *out_observation);
typedef void (*core_machine_attachment_ticks_provider)(void *owner,
    lib_u64 source_ticks);
typedef void (*core_machine_attachment_media_provider)(void *owner,
    lib_u64 source_ticks, lib_u64 due_tick);
typedef lib_bool (*core_machine_attachment_refresh_request_provider)(void *owner,
    lib_u8 *out_address);
typedef void (*core_machine_attachment_refresh_complete_provider)(void *owner);
typedef lib_u64 (*core_machine_attachment_dma_ticks_provider)(void *owner,
    lib_u64 source_ticks);
typedef lib_bool (*core_machine_attachment_dma_request_provider)(void *owner);
typedef void (*core_machine_attachment_dma_advance_provider)(void *owner,
    lib_u64 dma_ticks);
typedef struct core_machine_attachment_pit_ticks {
    lib_u64 primary;
    lib_u64 auxiliary;
} core_machine_attachment_pit_ticks;
typedef core_machine_attachment_pit_ticks (*core_machine_attachment_pit_ticks_provider)(
    void *owner, lib_u64 source_ticks);
typedef void (*core_machine_attachment_pit_pic_provider)(void *owner,
    core_machine_attachment_pit_ticks ticks);
typedef lib_bool (*core_machine_attachment_pic_pending_provider)(void *owner);
typedef lib_u8 (*core_machine_attachment_pic_acknowledge_provider)(void *owner);
typedef lib_bool (*core_machine_attachment_shutdown_reset_provider)(
    void *owner);
typedef void (*core_machine_attachment_phase_provider)(void *owner);
typedef lib_status (*core_machine_attachment_firmware_provider)(void *owner);
typedef lib_status (*core_machine_attachment_memory_admission_provider)(void *owner,
    lib_size memory_bytes);

/* Copied once during construction. Context remains valid until finalize_devices
 * returns during Core destruction. Callbacks run on the Core execution owner;
 * they may use bounded Core operations but cannot recursively run or destroy it.
 * Optional operations are null; context and finalize_devices are required.
 * Failed, duplicate or frozen binding leaves the previous binding unchanged. */
typedef struct core_machine_attachment {
    core_machine_attachment_deadline_provider deadline;
    core_machine_attachment_refresh_request_provider refresh_request;
    core_machine_attachment_refresh_complete_provider refresh_complete;
    core_machine_attachment_dma_ticks_provider dma_ticks;
    core_machine_attachment_dma_request_provider dma_request;
    core_machine_attachment_dma_advance_provider dma_advance;
    core_machine_attachment_pit_ticks_provider pit_ticks;
    core_machine_attachment_pit_pic_provider pit_pic;
    core_machine_attachment_pic_pending_provider pic_pending;
    core_machine_attachment_pic_acknowledge_provider pic_acknowledge;
    core_machine_attachment_shutdown_reset_provider shutdown_reset;
    core_machine_attachment_media_provider media;
    core_machine_attachment_ticks_provider rtc;
    core_machine_attachment_ticks_provider peripheral;
    core_machine_attachment_phase_provider reset_devices;
    core_machine_attachment_phase_provider reset_clocks;
    core_machine_attachment_phase_provider refresh_nmi;
    core_machine_attachment_phase_provider finalize_devices;
    core_machine_attachment_firmware_provider firmware;
    /* Optional board admission before stopped RAM replacement. A rejection
     * changes no Core allocation, mapping or lifecycle state. */
    core_machine_attachment_memory_admission_provider memory_admission;
    void *context;
} core_machine_attachment;

lib_status core_machine_bind_attachment(core_machine *machine,
    const core_machine_attachment *attachment);

#ifdef __cplusplus
}
#endif

#endif

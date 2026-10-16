#ifndef X86_CORE_MACHINE_ATTACHMENT_INTERFACE_H
#define X86_CORE_MACHINE_ATTACHMENT_INTERFACE_H

#include "lib/types/types_interface.h"
#include "x86-core/cpu/cpu_interface.h"

typedef struct x86_core_machine x86_core_machine;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct x86_core_machine_attachment_deadline_observation {
    lib_u64 source_ticks;
    lib_u8 immediate_due;
    lib_u8 l1_compatibility;
    lib_u8 fast_advance_blocked;
} x86_core_machine_attachment_deadline_observation;
typedef void (*x86_core_machine_attachment_deadline_provider)(void *owner, lib_u64 now,
    x86_core_machine_attachment_deadline_observation *out_observation);
typedef void (*x86_core_machine_attachment_advance_provider)(void *owner,
    lib_u64 source_ticks, lib_u64 due_tick);
typedef lib_bool (*x86_core_machine_attachment_refresh_request_provider)(void *owner,
    lib_u8 *out_address);
typedef void (*x86_core_machine_attachment_refresh_complete_provider)(void *owner);
/* A board-owned hold may defer CPU retirement while scheduler-owned DMA and
 * device progression continue.  It is not a second transaction owner. */
typedef lib_bool (*x86_core_machine_attachment_cpu_hold_asserted_provider)(void *owner);
typedef lib_u64 (*x86_core_machine_attachment_dma_ticks_provider)(void *owner,
    lib_u64 source_ticks);
typedef lib_bool (*x86_core_machine_attachment_dma_request_provider)(void *owner);
typedef void (*x86_core_machine_attachment_dma_advance_provider)(void *owner,
    lib_u64 dma_ticks);
typedef lib_bool (*x86_core_machine_attachment_interrupt_pending_provider)(void *owner);
typedef lib_u8 (*x86_core_machine_attachment_interrupt_acknowledge_provider)(void *owner);
/*
 * Board adaptation of the CPU TEST input for WAIT.  True means the CPU must
 * remain suspended; the attachment owns any physical signal-polarity mapping.
 */
typedef lib_bool (*x86_core_machine_attachment_wait_test_asserted_provider)(void *owner);
/* The 80186 integrated peripheral block may route D8h--DFh to type 7. */
typedef lib_bool (*x86_core_machine_attachment_escape_trap_provider)(void *owner);
typedef lib_bool (*x86_core_machine_attachment_shutdown_reset_provider)(
    void *owner);
typedef void (*x86_core_machine_attachment_phase_provider)(void *owner);
typedef lib_status (*x86_core_machine_attachment_firmware_provider)(void *owner);
typedef lib_status (*x86_core_machine_attachment_memory_admission_provider)(void *owner,
    lib_size memory_bytes);

/* Copied once during construction. Context remains valid until finalize_devices
 * returns during Core destruction. Callbacks run on the Core execution owner;
 * they may use bounded Core operations but cannot recursively run or destroy it.
 * Optional operations are null; context and finalize_devices are required.
 * Failed, duplicate or frozen binding leaves the previous binding unchanged. */
typedef struct x86_core_machine_attachment {
    x86_core_machine_attachment_deadline_provider deadline;
    x86_core_machine_attachment_refresh_request_provider refresh_request;
    x86_core_machine_attachment_refresh_complete_provider refresh_complete;
    x86_core_machine_attachment_cpu_hold_asserted_provider cpu_hold_asserted;
    x86_core_machine_attachment_dma_ticks_provider dma_ticks;
    x86_core_machine_attachment_dma_request_provider dma_request;
    x86_core_machine_attachment_dma_advance_provider dma_advance;
    x86_core_machine_attachment_interrupt_pending_provider interrupt_pending;
    x86_core_machine_attachment_interrupt_acknowledge_provider interrupt_acknowledge;
    x86_core_machine_attachment_wait_test_asserted_provider wait_test_asserted;
    x86_core_machine_attachment_escape_trap_provider escape_trap_enabled;
    x86_core_machine_attachment_shutdown_reset_provider shutdown_reset;
    /* Timeline and bus arbitration precede these callbacks. Board source work
     * runs before internal FPU/WAIT readiness; remaining board work follows it.
     * Each board owns its device order within these two bounded phases. */
    x86_core_machine_attachment_advance_provider before_internal_advance;
    x86_core_machine_attachment_advance_provider after_internal_advance;
    x86_core_machine_attachment_phase_provider reset_devices;
    x86_core_machine_attachment_phase_provider reset_clocks;
    x86_core_machine_attachment_phase_provider refresh_nmi;
    x86_core_machine_attachment_phase_provider finalize_devices;
    x86_core_machine_attachment_firmware_provider firmware;
    /* Optional board admission before stopped RAM replacement. A rejection
     * changes no Core allocation, mapping or lifecycle state. */
    x86_core_machine_attachment_memory_admission_provider memory_admission;
    /* Optional external cache. A failure leaves any already-completed board
     * effect visible; Core cancels this cycle, without inventing rollback. */
    lib_status (*cache_operation)(void *owner,
        x86_core_cpu_cache_operation operation);
    /* Drive the cache-control pins of this admitted memory cycle. Observation
     * accesses do not invoke this; failure cancels before data publication. */
    lib_status (*memory_cache_control)(void *owner, lib_u32 address,
        lib_u8 bytes, lib_bool write, lib_u8 cache_control);
    /* Physical CPU output only. Board logic owns any SMRAM decode/A20/cache
     * policy; this callback must not run or destroy the Core recursively. */
    void (*system_management_active)(void *owner, lib_bool active);
    /* Optional external special-cycle controller, supplied as a pair. Poll
     * reports local RDY without publishing effects; complete publishes once
     * (commit=true) or releases on cancellation. Core owns the held bus cycle. */
    lib_status (*special_cycle)(void *owner, x86_core_cpu_special_cycle cycle,
        lib_bool *ready);
    void (*complete_special_cycle)(void *owner, x86_core_cpu_special_cycle cycle,
        lib_bool commit);
    void *context;
} x86_core_machine_attachment;

lib_status x86_core_machine_bind_attachment(x86_core_machine *machine,
    const x86_core_machine_attachment *attachment);

#ifdef __cplusplus
}
#endif

#endif

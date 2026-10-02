#ifndef CORE_MACHINE_INTERFACE_H
#define CORE_MACHINE_INTERFACE_H

#include "x86/chips/cpu/cpu_interface.h"
#include "x86/chips/fpu/fpu_interface.h"
#include "app-nxvm/devices/execution_provider.h"
#include "app-nxvm/devices/firmware_interface.h"
#include "app-nxvm/devices/lifecycle_interface.h"
#include "app-nxvm/devices/memory_interface.h"
#include "app-nxvm/devices/port_interface.h"
#include "lib/types/types_interface.h"

#include "app-nxvm/devices/trace_interface.h"
#include "app-nxvm/devices/retirement_observation_interface.h"
#include "app-nxvm/devices/rom_mapping_interface.h"
#include "app-nxvm/devices/entry_plan_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

#define CORE_MACHINE_DEFAULT_MEMORY_BYTES (16u * 1024u * 1024u)
#define CORE_MACHINE_MINIMUM_MEMORY_BYTES (2u * 1024u * 1024u)
#define CORE_MACHINE_MAXIMUM_MEMORY_BYTES (64u * 1024u * 1024u)
/* A domain receives floor((phase + elapsed * numerator) / denominator)
 * ticks. All-zero is retained configuration shorthand for identity 1/1. */
typedef struct core_machine_clock_ratio {
    lib_u32 numerator;
    lib_u32 denominator;
    lib_u32 reset_phase;
} core_machine_clock_ratio;

typedef enum core_machine_time_axis_kind {
    CORE_MACHINE_TIME_AXIS_UNQUALIFIED = 0,
    /* A profile-owned nominal rate can bound host pacing, but does not
     * qualify the Core clock as physical instruction/transaction time. */
    CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL = 1,
    CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL = 2
} core_machine_time_axis_kind;

/* This qualifies the existing Core-owned elapsed-tick axis.  It is immutable
 * plan data, not a second counter or a host-time source. */
typedef struct core_machine_time_axis {
    core_machine_time_axis_kind kind;
    lib_u64 ticks_per_second;
} core_machine_time_axis;

/* A construction-only compatibility policy. It is not a timing source. */
typedef enum core_machine_l1_compatibility_policy {
    CORE_MACHINE_L1_COMPATIBILITY_DISABLED = 0,
    CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS
} core_machine_l1_compatibility_policy;

typedef enum core_machine_retirement_time_contract {
    CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC = 0,
    CORE_MACHINE_RETIREMENT_TIME_PHYSICAL = 1
} core_machine_retirement_time_contract;


/* A profile-selected external CPU-memory-cycle policy. The Core CPU owner
 * charges the declared page result only after a matching lifecycle commit.
 * A hit additionally needs an explicit, still-in-flight sequential overlap;
 * adjacency of completed logical accesses is never an overlap. */
typedef enum core_machine_external_cycle_overlap_policy {
    CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED = 0,
    CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL = 1
} core_machine_external_cycle_overlap_policy;

typedef struct core_machine_external_cycle_timing {
    lib_u32 page_bytes;
    lib_u32 page_miss_ticks;
    lib_u32 page_hit_ticks;
    core_machine_external_cycle_overlap_policy overlap_policy;
    /* Both zero retains legacy all-memory eligibility. */
    lib_u32 first_eligible_address;
    lib_u32 last_eligible_address;
} core_machine_external_cycle_timing;

#define CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY 7u

typedef struct core_machine_external_access_wait_window {
    core_machine_cpu_external_cycle_space space;
    lib_u32 first_address;
    lib_u32 last_address;
    lib_u32 wait_ticks;
} core_machine_external_access_wait_window;

/* Immutable plan-selected policy for the existing Core transaction,
 * availability and arbitration owners. It carries values only: CPU, DMA and
 * device owners retain their state machines and do not receive callbacks. */
typedef struct core_machine_transaction_contract {
    core_machine_external_cycle_timing external_cycle_timing;
    core_machine_external_access_wait_window external_access_wait_windows[
        CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY];
    lib_u32 dma_cycle_wait_quanta;
    lib_u8 dma_cycle_bus_ready_gate_enabled;
    lib_u8 cpu_cycle_bus_ready_gate_enabled;
    lib_u8 cpu_prefetch_reservation_enabled;
} core_machine_transaction_contract;

#define CORE_MACHINE_TIMING_CAPABILITY_COUNT 30u

typedef enum core_machine_timing_capability {
    CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC,
    CORE_MACHINE_TIMING_CAPABILITY_CPU_EXCEPT,
    CORE_MACHINE_TIMING_CAPABILITY_CPU_PREFETCH,
    CORE_MACHINE_TIMING_CAPABILITY_CPU_RETIRE,
    CORE_MACHINE_TIMING_CAPABILITY_CPU_FPU,
    CORE_MACHINE_TIMING_CAPABILITY_TIME_CLOCK,
    CORE_MACHINE_TIMING_CAPABILITY_TIME_LIFECYCLE,
    CORE_MACHINE_TIMING_CAPABILITY_TXN_MEMORY,
    CORE_MACHINE_TIMING_CAPABILITY_TXN_PORT,
    CORE_MACHINE_TIMING_CAPABILITY_TXN_ARBITRATION,
    CORE_MACHINE_TIMING_CAPABILITY_MEM_RAM_A20_PARITY,
    CORE_MACHINE_TIMING_CAPABILITY_MEM_ROM_FIRMWARE,
    CORE_MACHINE_TIMING_CAPABILITY_MACHINE_CONFIG,
    CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIC,
    CORE_MACHINE_TIMING_CAPABILITY_CTRL_DMA,
    CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIT,
    CORE_MACHINE_TIMING_CAPABILITY_CTRL_RTC_CMOS,
    CORE_MACHINE_TIMING_CAPABILITY_CTRL_KBC_NMI,
    CORE_MACHINE_TIMING_CAPABILITY_CTRL_FDC,
    CORE_MACHINE_TIMING_CAPABILITY_CTRL_HDC,
    CORE_MACHINE_TIMING_CAPABILITY_MEDIA_BACKING,
    CORE_MACHINE_TIMING_CAPABILITY_DISPLAY_VADP,
    CORE_MACHINE_TIMING_CAPABILITY_DISPLAY_PRESENT,
    CORE_MACHINE_TIMING_CAPABILITY_INPUT_HOST,
    CORE_MACHINE_TIMING_CAPABILITY_TRACE_DEBUG,
    CORE_MACHINE_TIMING_CAPABILITY_PLATFORM_MAILBOX,
    CORE_MACHINE_TIMING_CAPABILITY_PLATFORM_RESOURCE,
    CORE_MACHINE_TIMING_CAPABILITY_PLATFORM_WAIT,
    CORE_MACHINE_TIMING_CAPABILITY_SESSION_COMMAND,
    CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG
} core_machine_timing_capability;

typedef enum core_machine_timing_disposition {
    CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK,
    CORE_MACHINE_TIMING_DISPOSITION_NON_GUEST_TIME,
    CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED
} core_machine_timing_disposition;

typedef enum core_machine_timing_seam {
    CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM,
    CORE_MACHINE_TIMING_SEAM_RETIREMENT,
    CORE_MACHINE_TIMING_SEAM_CLOCK,
    CORE_MACHINE_TIMING_SEAM_LIFECYCLE,
    CORE_MACHINE_TIMING_SEAM_TRANSACTION,
    CORE_MACHINE_TIMING_SEAM_MEMORY,
    CORE_MACHINE_TIMING_SEAM_CONFIGURATION,
    CORE_MACHINE_TIMING_SEAM_DEVICE,
    CORE_MACHINE_TIMING_SEAM_OBSERVATION
} core_machine_timing_seam;

typedef struct core_machine_timing_declaration {
    core_machine_timing_capability capability;
    core_machine_timing_disposition disposition;
    core_machine_timing_seam seam;
} core_machine_timing_declaration;

typedef enum core_machine_stop_reason {
    CORE_MACHINE_STOP_NONE = 0,
    /* Also used for a successfully delivered synchronous fault: that run
     * retires no instruction and advances no machine time. */
    CORE_MACHINE_STOP_BUDGET,
    CORE_MACHINE_STOP_PAUSED,
    CORE_MACHINE_STOP_GUEST_EXIT,
    CORE_MACHINE_STOP_REQUESTED,
    CORE_MACHINE_STOP_RESET_REQUESTED,
    CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT,
    CORE_MACHINE_STOP_FAULT
} core_machine_stop_reason;

typedef struct core_machine_run_budget {
    lib_u64 instructions;
    lib_u64 ticks;
} core_machine_run_budget;

typedef struct core_machine_run_result {
    core_machine_stop_reason reason;
    lib_u64 executed;
    lib_u64 ticks;
    lib_u64 elapsed_ticks;
    lib_u32 linear_pc;
    lib_u32 detail;
} core_machine_run_result;

typedef struct core_machine_observation {
    core_machine_lifecycle lifecycle;
    lib_u64 elapsed_ticks;
    core_machine_cpu_state cpu;
    core_machine_cpu_diagnostic diagnostic;
} core_machine_observation;

/* A copied classification of the next Core-owned time-progress opportunity.
 * It contains no controller identity, pointer, or requested tick quantity. */
typedef enum core_machine_time_progress_disposition {
    CORE_MACHINE_TIME_PROGRESS_IDLE = 0,
    CORE_MACHINE_TIME_PROGRESS_DEADLINE,
    CORE_MACHINE_TIME_PROGRESS_IMMEDIATE,
    CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY
} core_machine_time_progress_disposition;

/* A copied guest-time observation. `next_deadline_valid` is true only when
 * Core has composed an earliest source-qualified guest-observable deadline.
 * Recurring scheduler maintenance is deliberately not such a deadline. */
typedef struct core_machine_time_observation {
    lib_u64 elapsed_ticks;
    lib_u64 next_deadline_tick;
    lib_u64 pacing_ticks_per_second;
    lib_u64 physical_ticks_per_second;
    lib_u8 next_deadline_valid;
    lib_u8 pacing_time_available;
    lib_u8 physical_time_available;
    core_machine_time_progress_disposition progress_disposition;
} core_machine_time_observation;

typedef struct core_machine_timeline_observation {
    lib_u64 now;
    lib_u64 next_sequence;
    lib_u32 pending_events;
} core_machine_timeline_observation;

lib_status core_machine_get_timing_disposition(const core_machine *machine,
    core_machine_timing_capability capability,
    core_machine_timing_disposition *out_disposition);
lib_status core_machine_get_timing_declaration(const core_machine *machine,
    core_machine_timing_capability capability,
    core_machine_timing_declaration *out_declaration);

lib_status core_machine_reset(core_machine *machine);

lib_status core_machine_reconfigure_memory(core_machine *machine,
    lib_size memory_bytes);

lib_status core_machine_get_lifecycle(
    const core_machine *machine,
    core_machine_lifecycle *out_lifecycle);

lib_status core_machine_get_cpu_state(
    const core_machine *machine,
    core_machine_cpu_state *out_state);

lib_status core_machine_get_cpu_profile(
    const core_machine *machine, core_machine_cpu_profile *out_profile);
lib_status core_machine_get_fpu_profile(
    const core_machine *machine, x86_fpu_profile *out_profile);
lib_status core_machine_get_fpu_state(
    const core_machine *machine, x86_fpu_state *out_state);
lib_status core_machine_get_memory_bytes(
    const core_machine *machine, lib_size *out_memory_bytes);
lib_status core_machine_get_elapsed_ticks(
    const core_machine *machine, lib_u64 *out_elapsed_ticks);
lib_status core_machine_capture_time_observation(const core_machine *machine,
    core_machine_time_observation *out_observation);
/* Core selects and advances to its next valid guest-observable deadline.
 * A false result means an unqualified owner blocks safe fast advance. */
lib_status core_machine_advance_to_next_deadline(core_machine *machine,
    lib_u8 *out_advanced);
/* Turbo may request one bounded, Core-owned escape from a copied L1 state.
 * The caller supplies neither a tick count nor a controller selection. */
lib_status core_machine_advance_l1_compatibility(core_machine *machine,
    lib_u8 *out_advanced);
lib_status core_machine_get_timeline_observation(const core_machine *machine,
    core_machine_timeline_observation *out_observation);

lib_status core_machine_get_cpu_diagnostic(
    const core_machine *machine,
    core_machine_cpu_diagnostic *out_diagnostic);

lib_status core_machine_run(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *result);

lib_status core_machine_request_stop(core_machine *machine);

/* Selected bus adapters drive this level at deterministic guest-time boundaries. */
lib_status core_machine_set_dma_bus_ready(core_machine *machine, lib_i32 ready);
lib_status core_machine_set_cpu_bus_ready(core_machine *machine, lib_i32 ready);
/* VM devices may request the architected NMI mask through this operation;
 * they never borrow CPU storage to change it. */
lib_status core_machine_set_nmi_mask(core_machine *machine, lib_i32 masked);
lib_status core_machine_get_nmi_mask(const core_machine *machine,
    lib_i32 *out_masked);

/* Board signals run on the owning executor thread with a live opaque machine.
 * NMI returns false for a masked signal or null machine; reset is null-safe.
 * CPU owns the pending state; the normal run boundary consumes processor reset. */
lib_bool core_machine_signal_nmi(core_machine *machine);
void core_machine_signal_processor_reset(core_machine *machine);

lib_status core_machine_report_fault(
    core_machine *machine,
    lib_u32 detail);

lib_status core_machine_capture_observation(
    const core_machine *machine, core_machine_observation *out_observation);

lib_status core_machine_bind_execution_provider(core_machine *machine,
    const core_machine_execution_provider *provider, void *context);
lib_status core_machine_freeze_execution_providers(core_machine *machine);

void core_machine_destroy(core_machine *machine);

#ifdef __cplusplus
}
#endif

#endif

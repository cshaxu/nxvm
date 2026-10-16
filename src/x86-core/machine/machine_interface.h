#ifndef X86_CORE_MACHINE_INTERFACE_H
#define X86_CORE_MACHINE_INTERFACE_H

#include "x86-core/cpu/cpu_interface.h"
#include "x86-core/fpu/fpu_interface.h"
#include "x86-core/machine/execution_provider_interface.h"
#include "x86-core/machine/firmware_interface.h"
#include "x86-core/machine/lifecycle_interface.h"
#include "x86-core/machine/attachment_interface.h"
#include "x86-core/machine/memory_interface.h"
#include "x86-core/machine/port_interface.h"
#include "lib/types/types_interface.h"

#include "x86-core/machine/trace_interface.h"
#include "x86-core/machine/retirement_observation_interface.h"
#include "x86-core/machine/rom_mapping_interface.h"
#include "x86-core/machine/entry_plan_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct x86_core_machine x86_core_machine;

/* A domain receives floor((phase + elapsed * numerator) / denominator)
 * ticks. All-zero is retained configuration shorthand for identity 1/1. */
typedef struct x86_core_machine_clock_ratio {
    lib_u32 numerator;
    lib_u32 denominator;
    lib_u32 reset_phase;
} x86_core_machine_clock_ratio;

typedef enum x86_core_machine_time_axis_kind {
    X86_CORE_MACHINE_TIME_AXIS_UNQUALIFIED = 0,
    /* A profile-owned nominal rate can bound host pacing, but does not
     * qualify the Core clock as physical instruction/transaction time. */
    X86_CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL = 1,
    X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL = 2
} x86_core_machine_time_axis_kind;

/* This qualifies the existing Core-owned elapsed-tick axis.  It is immutable
 * plan data, not a second counter or a host-time source. */
typedef struct x86_core_machine_time_axis {
    x86_core_machine_time_axis_kind kind;
    lib_u64 ticks_per_second;
} x86_core_machine_time_axis;

/* A construction-only compatibility policy. It is not a timing source. */
typedef enum x86_core_machine_l1_compatibility_policy {
    X86_CORE_MACHINE_L1_COMPATIBILITY_DISABLED = 0,
    X86_CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS
} x86_core_machine_l1_compatibility_policy;

typedef enum x86_core_machine_retirement_time_contract {
    X86_CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC = 0,
    X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL = 1
} x86_core_machine_retirement_time_contract;


/* A profile-selected external CPU-memory-cycle policy. The Core CPU owner
 * charges the declared page result only after a matching lifecycle commit.
 * A hit additionally needs an explicit, still-in-flight sequential overlap;
 * adjacency of completed logical accesses is never an overlap. */
typedef enum x86_core_machine_external_cycle_overlap_policy {
    X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED = 0,
    X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL = 1
} x86_core_machine_external_cycle_overlap_policy;

typedef struct x86_core_machine_external_cycle_timing {
    lib_u32 page_bytes;
    lib_u32 page_miss_ticks;
    lib_u32 page_hit_ticks;
    x86_core_machine_external_cycle_overlap_policy overlap_policy;
    /* Both zero retains legacy all-memory eligibility. */
    lib_u32 first_eligible_address;
    lib_u32 last_eligible_address;
} x86_core_machine_external_cycle_timing;

#define X86_CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY 7u

typedef struct x86_core_machine_external_access_wait_window {
    x86_core_cpu_external_cycle_space space;
    lib_u32 first_address;
    lib_u32 last_address;
    lib_u32 wait_ticks;
} x86_core_machine_external_access_wait_window;

/* Immutable plan-selected policy for the existing Core transaction,
 * availability and arbitration owners. It carries values only: CPU, DMA and
 * device owners retain their state machines and do not receive callbacks. */
typedef struct x86_core_machine_transaction_contract {
    x86_core_machine_external_cycle_timing external_cycle_timing;
    x86_core_machine_external_access_wait_window external_access_wait_windows[
        X86_CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY];
    lib_u32 dma_cycle_wait_quanta;
    lib_u8 dma_cycle_bus_ready_gate_enabled;
    lib_u8 cpu_cycle_bus_ready_gate_enabled;
    lib_u8 cpu_prefetch_reservation_enabled;
    /* Zero retains point-service refresh. Nonzero occupies the sole bus for
     * this many source ticks; the selected board declares their provenance. */
    lib_u32 refresh_cycle_source_ticks;
} x86_core_machine_transaction_contract;

typedef enum x86_core_machine_stop_reason {
    X86_CORE_MACHINE_STOP_NONE = 0,
    /* Also used for a successfully delivered synchronous fault: that run
     * retires no instruction and advances no machine time. */
    X86_CORE_MACHINE_STOP_BUDGET,
    X86_CORE_MACHINE_STOP_PAUSED,
    X86_CORE_MACHINE_STOP_GUEST_EXIT,
    X86_CORE_MACHINE_STOP_REQUESTED,
    X86_CORE_MACHINE_STOP_RESET_REQUESTED,
    X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT,
    X86_CORE_MACHINE_STOP_FAULT
} x86_core_machine_stop_reason;

typedef struct x86_core_machine_run_budget {
    lib_u64 instructions;
    lib_u64 ticks;
} x86_core_machine_run_budget;

typedef struct x86_core_machine_run_result {
    x86_core_machine_stop_reason reason;
    lib_u64 executed;
    lib_u64 ticks;
    lib_u64 elapsed_ticks;
    lib_u32 linear_pc;
    lib_u32 detail;
} x86_core_machine_run_result;

typedef struct x86_core_machine_observation {
    x86_core_machine_lifecycle lifecycle;
    lib_u64 elapsed_ticks;
    x86_core_cpu_state cpu;
    x86_core_cpu_diagnostic diagnostic;
} x86_core_machine_observation;

/* A copied classification of the next Core-owned time-progress opportunity.
 * It contains no controller identity, pointer, or requested tick quantity. */
typedef enum x86_core_machine_time_progress_disposition {
    X86_CORE_MACHINE_TIME_PROGRESS_IDLE = 0,
    X86_CORE_MACHINE_TIME_PROGRESS_DEADLINE,
    X86_CORE_MACHINE_TIME_PROGRESS_IMMEDIATE,
    X86_CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY
} x86_core_machine_time_progress_disposition;

/* A copied guest-time observation. `next_deadline_valid` is true only when
 * Core has composed an earliest source-qualified guest-observable deadline.
 * Recurring scheduler maintenance is deliberately not such a deadline. */
typedef struct x86_core_machine_time_observation {
    lib_u64 elapsed_ticks;
    lib_u64 next_deadline_tick;
    lib_u64 pacing_ticks_per_second;
    lib_u64 physical_ticks_per_second;
    lib_u8 next_deadline_valid;
    lib_u8 pacing_time_available;
    lib_u8 physical_time_available;
    x86_core_machine_time_progress_disposition progress_disposition;
} x86_core_machine_time_observation;

typedef struct x86_core_machine_timeline_observation {
    lib_u64 now;
    lib_u64 next_sequence;
    lib_u32 pending_events;
} x86_core_machine_timeline_observation;

lib_status x86_core_machine_reset(x86_core_machine *machine);

lib_status x86_core_machine_reconfigure_memory(x86_core_machine *machine,
    lib_size memory_bytes);

/* Board operations share Core's bounded firmware-reentry guard. This is an
 * eligibility check, not a lifecycle transition or a private-state lease. */
lib_i32 x86_core_machine_mutable_operation_is_allowed(const x86_core_machine *machine);

/* Explicit construction values consumed only by the neutral executor. CPU
 * profile must name an admitted model and memory_bytes must be nonzero. The board
 * composition derives this temporary value from its one frozen configuration;
 * Core never retains a second configuration or any board selection. */
typedef struct x86_core_machine_executor_config {
    lib_size memory_bytes;
    x86_core_cpu_profile cpu_profile;
    x86_fpu_profile fpu_profile;
    lib_u8 cpu_80386_cr_mov_ignores_mod;
    x86_core_machine_address_decode address_decode;
    lib_u32 ticks_per_instruction;
    x86_core_cpu_instruction_timing instruction_timing;
    x86_core_machine_transaction_contract transaction_contract;
    x86_core_machine_clock_ratio provider_clock;
    x86_core_machine_time_axis time_axis;
    x86_core_machine_l1_compatibility_policy l1_compatibility_policy;
    x86_core_machine_retirement_time_contract retirement_time_contract;
    const x86_core_machine_retirement_qualification_descriptor *retirement_qualification;
} x86_core_machine_executor_config;

lib_i32 x86_core_machine_neutral_config_is_valid(
    const x86_core_machine_executor_config *config);
/* Success publishes one initialized Core; outputs clear on failure. Attachments
 * are composed separately before freeze. Core owns the returned lifetime. */
lib_status x86_core_machine_neutral_create(
    const x86_core_machine_executor_config *config, x86_core_machine **out_machine);
void x86_core_machine_cpu_bus_refresh_pulse(void *core_owner);

/* Construction eligibility only; board attachments retain no Core layout. */
lib_i32 x86_core_machine_configuration_is_open(const x86_core_machine *machine);

lib_status x86_core_machine_get_lifecycle(
    const x86_core_machine *machine,
    x86_core_machine_lifecycle *out_lifecycle);

lib_status x86_core_machine_get_cpu_state(
    const x86_core_machine *machine,
    x86_core_cpu_state *out_state);

lib_status x86_core_machine_get_cpu_profile(
    const x86_core_machine *machine, x86_core_cpu_profile *out_profile);
lib_status x86_core_machine_get_fpu_profile(
    const x86_core_machine *machine, x86_fpu_profile *out_profile);
lib_status x86_core_machine_get_fpu_state(
    const x86_core_machine *machine, x86_fpu_state *out_state);
lib_status x86_core_machine_get_memory_bytes(
    const x86_core_machine *machine, lib_size *out_memory_bytes);
lib_status x86_core_machine_get_elapsed_ticks(
    const x86_core_machine *machine, lib_u64 *out_elapsed_ticks);
lib_status x86_core_machine_capture_time_observation(const x86_core_machine *machine,
    x86_core_machine_time_observation *out_observation);
/* Core selects and advances to its next valid guest-observable deadline.
 * A false result means an unqualified owner blocks safe fast advance. */
lib_status x86_core_machine_advance_to_next_deadline(x86_core_machine *machine,
    lib_u8 *out_advanced);
/* Turbo may request one bounded, Core-owned escape from a copied L1 state.
 * The caller supplies neither a tick count nor a controller selection. */
lib_status x86_core_machine_advance_l1_compatibility(x86_core_machine *machine,
    lib_u8 *out_advanced);
lib_status x86_core_machine_get_timeline_observation(const x86_core_machine *machine,
    x86_core_machine_timeline_observation *out_observation);

lib_status x86_core_machine_get_cpu_diagnostic(
    const x86_core_machine *machine,
    x86_core_cpu_diagnostic *out_diagnostic);

lib_status x86_core_machine_run(
    x86_core_machine *machine,
    x86_core_machine_run_budget budget,
    x86_core_machine_run_result *result);

lib_status x86_core_machine_request_stop(x86_core_machine *machine);

/* Selected bus adapters drive this level at deterministic guest-time boundaries. */
lib_status x86_core_machine_set_dma_bus_ready(x86_core_machine *machine, lib_i32 ready);
lib_status x86_core_machine_set_cpu_bus_ready(x86_core_machine *machine, lib_i32 ready);
/* VM devices may request the architected NMI mask through this operation;
 * they never borrow CPU storage to change it. */
lib_status x86_core_machine_set_nmi_mask(x86_core_machine *machine, lib_i32 masked);
lib_status x86_core_machine_get_nmi_mask(const x86_core_machine *machine,
    lib_i32 *out_masked);

/* Board signals run on the owning executor thread with a live opaque machine.
 * NMI returns false for a masked signal or null machine; reset is null-safe.
 * CPU owns the pending state; the normal run boundary consumes processor reset. */
lib_bool x86_core_machine_signal_nmi(x86_core_machine *machine);
void x86_core_machine_signal_processor_reset(x86_core_machine *machine);
/* Selected SX SRESET retains CPU SMBASE; RAM, board state and scheduled work
 * remain intact as for processor-only RESET. Consumed at the same run boundary. */
lib_status x86_core_machine_signal_soft_processor_reset(x86_core_machine *machine);
/* Board-owned qualified SMI level; no instruction executes in this call. */
lib_status x86_core_machine_set_smi(x86_core_machine *machine, lib_bool asserted);
/* Qualified CPU power/system inputs. The board owns CLK/PLL timing; these
 * calls only change inputs and do not run the CPU or advance Core time. */
lib_status x86_core_machine_set_stop_clock(x86_core_machine *machine, lib_bool asserted);
lib_status x86_core_machine_set_clock_running(x86_core_machine *machine, lib_bool running);
lib_status x86_core_machine_set_flush(x86_core_machine *machine, lib_bool asserted);
lib_status x86_core_machine_set_nmi(x86_core_machine *machine, lib_bool asserted);

lib_status x86_core_machine_report_fault(
    x86_core_machine *machine,
    lib_u32 detail);

lib_status x86_core_machine_capture_observation(
    const x86_core_machine *machine, x86_core_machine_observation *out_observation);

lib_status x86_core_machine_bind_execution_provider(x86_core_machine *machine,
    const x86_core_machine_execution_provider *provider, void *context);
lib_status x86_core_machine_freeze_execution_providers(x86_core_machine *machine);

void x86_core_machine_destroy(x86_core_machine *machine);

#ifdef __cplusplus
}
#endif

#endif

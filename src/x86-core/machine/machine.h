#ifndef X86_CORE_MACHINE_H
#define X86_CORE_MACHINE_H
#include "lib/types/types_interface.h"






#include "x86-core/cpu/cpu_interface.h"
#include "x86-core/fpu/fpu_interface.h"

#include "x86-core/machine/execution_provider_interface.h"

#include "x86-core/machine/machine_interface.h"

#include "x86-core/machine/clock_interface.h"

#include "x86-core/machine/timeline.h"

#include "x86-core/machine/transaction.h"

#include "x86-core/machine/memory.h"

#include "x86-core/machine/port.h"

#define X86_CORE_MACHINE_TRACE_CAPACITY 32u
#define X86_CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY 8u
#define X86_CORE_MACHINE_RETIREMENT_QUALIFICATION_CAPACITY 128u

typedef struct x86_core_machine_trace_state {
    x86_core_machine_trace_provider provider;
    x86_core_machine_trace_event events[X86_CORE_MACHINE_TRACE_CAPACITY];
    lib_u64 next_sequence;
    lib_size count;
    lib_i32 flushing;
} x86_core_machine_trace_state;

typedef struct x86_core_machine_cpu_diagnostic_state {
    x86_core_cpu_diagnostic snapshot;
    lib_size next_index;
} x86_core_machine_cpu_diagnostic_state;

typedef struct x86_core_machine_retirement_observation_state {
    x86_core_machine_retirement_observation_provider provider;
    x86_core_machine_retirement_observation pending_observation;
    lib_u64 next_sequence;
    lib_u8 pending;
} x86_core_machine_retirement_observation_state;
typedef struct x86_core_machine_immutable_rom_mapping {
    lib_u32 physical_start;
    lib_size bytes;
    lib_u8 *image;
    lib_u8 owns_image;
} x86_core_machine_immutable_rom_mapping;

struct x86_core_machine_firmware_context {
    x86_core_machine *machine;
    lib_status operation_status;
    lib_i32 track_operation_failures;
    lib_i32 active;
    lib_i32 configuring;
};

struct x86_core_machine {
    x86_core_machine_lifecycle lifecycle;
    lib_atomic_i32 stop_requested;
    lib_u32 fault_detail;
    lib_u64 elapsed_ticks;
    x86_core_machine_timeline timeline;
    x86_core_machine_transaction_state transaction;
    x86_core_machine_transaction_contract transaction_contract;
    lib_u32 external_cycle_page_tag;
    lib_u64 external_cycle_round_ticks;
    lib_u64 bus_invalidation_cycles;
    lib_bool bus_invalidation_seen;
    lib_u64 cpu_retirement_wait_ticks;
    lib_u64 cpu_retirement_completion_ticks;
    lib_u64 cpu_retirement_source_ticks;
    lib_bool cpu_retirement_wait_retires;
    lib_u8 external_cycle_page_valid;
    lib_u8 external_cycle_pending_valid;
    x86_core_cpu_external_cycle_space external_cycle_pending_space;
    lib_u32 external_cycle_pending_physical;
    lib_u8 external_cycle_pending_bytes;
    lib_u8 external_cycle_pending_write;
    x86_core_cpu_memory_access_provenance external_cycle_pending_provenance;
    lib_u8 external_cycle_overlap_valid;
    lib_u32 external_cycle_overlap_next_physical;
    lib_u8 external_cycle_round_overflow;
    lib_u8 cpu_retirement_wait_pending;
    lib_u64 maximum_instruction_ticks;
    x86_core_machine_retirement_time_contract retirement_time_contract;
    x86_core_machine_retirement_eligibility_key retirement_eligibility_key;
    lib_u8 retirement_eligibility_key_valid;
    x86_core_machine_retirement_eligibility_key retirement_qualification[
        X86_CORE_MACHINE_RETIREMENT_QUALIFICATION_CAPACITY];
    lib_size retirement_qualification_count;
    x86_core_machine_clock_domain provider_clock;
    x86_core_machine_time_axis time_axis;
    x86_core_machine_l1_compatibility_policy l1_compatibility_policy;
    x86_core_machine_trace_state trace;
    x86_core_machine_cpu_diagnostic_state cpu_diagnostic;
    x86_core_machine_retirement_observation_state retirement_observation;
    x86_core_machine_immutable_rom_mapping
        immutable_rom_mappings[X86_CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY];
    lib_size immutable_rom_mapping_count;
    lib_u8 entry_plan_applied;
    x86_core_cpu_profile cpu_profile;
    lib_u8 cpu_80386_cr_mov_ignores_mod;
    x86_core_cpu_protected_data_reload_policy protected_data_reload_policy;
    x86_fpu *fpu;
    /* The CPU owns its architectural/decoder layout.  The board owns exactly
     * one opaque execution lifetime and never mirrors those fields. */
    x86_core_cpu_execution_context *executor_cpu_execution;
    t_ram executor_memory;
    t_port executor_port;
    const x86_core_machine_firmware_provider *firmware_provider;
    void *firmware_provider_context;
    x86_core_machine_firmware_context firmware_context;
    lib_i32 firmware_operation_active;
    const x86_core_machine_execution_provider *execution_provider;
    void *execution_provider_context;
    lib_i32 execution_provider_frozen;
    /* Core scheduler state is private; attachments use the copied contract. */
    lib_u32 dma_cycle_wait_remaining;
    lib_u32 refresh_cycle_ticks_remaining;
    lib_u8 dma_cycle_bus_ready;
    lib_u8 cpu_cycle_bus_ready;
    x86_core_machine_attachment attachment;
};

#if X86_CORE_MACHINE_RUNTIME_TRACE_ENABLED || defined(X86_CORE_MACHINE_TRACE_IMPLEMENTATION)
void x86_core_machine_trace_initialize(x86_core_machine *machine);
void x86_core_machine_trace_finalize(x86_core_machine *machine);
#else
#define x86_core_machine_trace_initialize(machine) ((void)(machine))
#define x86_core_machine_trace_finalize(machine) ((void)(machine))
#endif
void x86_core_machine_cpu_diagnostic_initialize(x86_core_machine *machine);
void x86_core_machine_cpu_diagnostic_reset(x86_core_machine *machine);
void x86_core_machine_refresh_hold_advance(x86_core_machine *machine, lib_u64 source_ticks);
void x86_core_machine_retirement_observation_initialize(x86_core_machine *machine);
void x86_core_machine_retirement_observation_reset(x86_core_machine *machine);
void x86_core_machine_retirement_observation_capture_instruction(x86_core_machine *machine,
    const x86_core_cpu_instruction_observation *observation);
void x86_core_machine_retirement_observation_capture_eligibility_key(
    x86_core_machine *machine);
void x86_core_machine_retirement_observation_publish(x86_core_machine *machine,
    lib_u64 source_ticks);
lib_status x86_core_machine_register_immutable_rom_mapping_from_firmware(
    x86_core_machine *machine, lib_u32 physical_start, const lib_u8 *image,
    lib_size bytes);
lib_status x86_core_machine_register_immutable_rom_mapping_alias_from_firmware(
    x86_core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes);
void x86_core_machine_rollback_immutable_rom_mappings(x86_core_machine *machine,
    lib_size mapping_count);
/* Private fault injection; production composition uses neutral_create. */
lib_status x86_core_machine_neutral_create_with_test_allocation(
    const x86_core_machine_executor_config *config,
    x86_core_machine_memory_test_allocation *test_allocation,
    x86_core_machine_port_test_allocation *port_test_allocation,
    x86_core_machine **out_machine);

lib_u32 x86_core_machine_linear_pc(const x86_core_machine *machine);
void x86_core_machine_external_cycle_invalidate(x86_core_machine *machine);
extern const x86_core_cpu_bus_provider x86_core_machine_cpu_bus;
void x86_core_machine_cpu_bus_snoop_write(void *opaque, lib_u32 physical, lib_size bytes);
void x86_core_machine_transaction_trace(void *opaque,
    x86_core_machine_transaction_owner owner, x86_core_machine_transaction_kind kind,
    x86_core_machine_transaction_phase phase, lib_u32 address,
    lib_u32 value, lib_u32 detail);
lib_u64 x86_core_machine_cpu_external_cycle_trace(void *opaque,
    x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address,
    lib_u8 bytes, lib_u8 write,
    x86_core_cpu_memory_access_provenance provenance);
void x86_core_machine_cpu_diagnostic_capture(const x86_core_machine *machine,
    x86_core_cpu_diagnostic *out_diagnostic);
void x86_core_machine_cpu_diagnostic_initialize(x86_core_machine *machine);
void x86_core_machine_cpu_diagnostic_reset(x86_core_machine *machine);
extern const x86_core_cpu_execution_diagnostic_provider
    x86_core_machine_cpu_diagnostic_provider;
/* Production always retains fault and delivered-exception snapshots for the
 * runtime debugger.  Per-retirement capture is installed only when an
 * observer or a physical retirement contract actually needs it. */
extern const x86_core_cpu_execution_diagnostic_provider
    x86_core_machine_cpu_fault_diagnostic_provider;
typedef enum x86_core_machine_time_publication_origin {
    X86_CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT,
    X86_CORE_MACHINE_TIME_PUBLICATION_EXTERNAL_WAIT,
    X86_CORE_MACHINE_TIME_PUBLICATION_DEADLINE,
    X86_CORE_MACHINE_TIME_PUBLICATION_DETERMINISTIC_ADVANCE,
    X86_CORE_MACHINE_TIME_PUBLICATION_L1_COMPATIBILITY,
    X86_CORE_MACHINE_TIME_PUBLICATION_CPU_EVENT,
    X86_CORE_MACHINE_TIME_PUBLICATION_CPU_EXECUTION
} x86_core_machine_time_publication_origin;
lib_bool x86_core_machine_retirement_time_is_qualified(const x86_core_machine *machine);
lib_status x86_core_machine_publish_elapsed_ticks(x86_core_machine *machine,
    lib_u64 elapsed_ticks, x86_core_machine_time_publication_origin origin);
/* Deterministic Core-test helper. Product execution advances through the
 * CPU/event boundary or x86_core_machine_advance_to_next_deadline(). */
lib_status x86_core_machine_advance_time(x86_core_machine *machine,
    lib_u64 source_ticks);
void x86_core_machine_capture_time_observation_private(const x86_core_machine *machine,
    x86_core_machine_time_observation *out_observation);
lib_status x86_core_machine_firmware_invoke(x86_core_machine *machine,
    lib_i32 configuring, lib_i32 track_operation_failures,
    lib_status (*callback)(void *, x86_core_machine_firmware_context *));
lib_i32 x86_core_machine_retirement_time_contract_is_valid(
    x86_core_machine_retirement_time_contract contract);
lib_i32 x86_core_machine_external_cycle_timing_is_valid(
    const x86_core_machine_external_cycle_timing *timing);
lib_i32 x86_core_machine_external_access_wait_windows_are_valid(
    const x86_core_machine_external_access_wait_window *windows);
lib_i32 x86_core_machine_transaction_contract_is_valid(
    const x86_core_machine_transaction_contract *contract);
#endif

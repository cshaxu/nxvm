#ifndef CORE_MACHINE_H
#define CORE_MACHINE_H
#include "lib/types/types_interface.h"






#include "x86/chips/cpu/cpu_interface.h"
#include "x86/chips/fpu/fpu_interface.h"

#include "x86/core/execution_provider_interface.h"

#include "x86/core/machine_interface.h"

#include "x86/core/clock_interface.h"

#include "x86/core/timeline.h"

#include "x86/core/transaction.h"

#include "x86/core/memory.h"

#include "x86/core/port.h"

#define CORE_MACHINE_TRACE_CAPACITY 32u
#define CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY 8u
#define CORE_MACHINE_RETIREMENT_QUALIFICATION_CAPACITY 128u

typedef struct core_machine_trace_state {
    core_machine_trace_provider provider;
    core_machine_trace_event events[CORE_MACHINE_TRACE_CAPACITY];
    lib_u64 next_sequence;
    lib_size count;
    lib_i32 flushing;
} core_machine_trace_state;

typedef struct core_machine_cpu_diagnostic_state {
    core_machine_cpu_diagnostic snapshot;
    lib_size next_index;
} core_machine_cpu_diagnostic_state;

typedef struct core_machine_retirement_observation_state {
    core_machine_retirement_observation_provider provider;
    core_machine_retirement_observation pending_observation;
    lib_u64 next_sequence;
    lib_u8 pending;
} core_machine_retirement_observation_state;
typedef struct core_machine_immutable_rom_mapping {
    lib_u32 physical_start;
    lib_size bytes;
    lib_u8 *image;
    lib_u8 owns_image;
} core_machine_immutable_rom_mapping;

struct core_machine_firmware_context {
    core_machine *machine;
    lib_status operation_status;
    lib_i32 track_operation_failures;
    lib_i32 active;
    lib_i32 configuring;
};

struct core_machine {
    core_machine_lifecycle lifecycle;
    lib_atomic_i32 stop_requested;
    lib_u32 fault_detail;
    lib_u64 elapsed_ticks;
    core_machine_timeline timeline;
    core_machine_timing_declaration timing_declarations[
        CORE_MACHINE_TIMING_CAPABILITY_COUNT];
    lib_u8 timing_declarations_copied;
    core_machine_transaction_state transaction;
    core_machine_transaction_contract transaction_contract;
    lib_u32 external_cycle_page_tag;
    lib_u64 external_cycle_round_ticks;
    lib_u64 cpu_retirement_wait_ticks;
    lib_u64 cpu_retirement_completion_ticks;
    lib_u64 cpu_retirement_source_ticks;
    lib_bool cpu_retirement_wait_retires;
    lib_u8 external_cycle_page_valid;
    lib_u8 external_cycle_pending_valid;
    core_machine_cpu_external_cycle_space external_cycle_pending_space;
    lib_u32 external_cycle_pending_physical;
    lib_u8 external_cycle_pending_bytes;
    lib_u8 external_cycle_pending_write;
    core_machine_cpu_memory_access_provenance external_cycle_pending_provenance;
    lib_u8 external_cycle_overlap_valid;
    lib_u32 external_cycle_overlap_next_physical;
    lib_u8 external_cycle_round_overflow;
    lib_u8 cpu_retirement_wait_pending;
    lib_u64 maximum_instruction_ticks;
    core_machine_retirement_time_contract retirement_time_contract;
    core_machine_retirement_eligibility_key retirement_eligibility_key;
    lib_u8 retirement_eligibility_key_valid;
    core_machine_retirement_eligibility_key retirement_qualification[
        CORE_MACHINE_RETIREMENT_QUALIFICATION_CAPACITY];
    lib_size retirement_qualification_count;
    core_machine_clock_domain provider_clock;
    core_machine_time_axis time_axis;
    core_machine_l1_compatibility_policy l1_compatibility_policy;
    core_machine_trace_state trace;
    core_machine_cpu_diagnostic_state cpu_diagnostic;
    core_machine_retirement_observation_state retirement_observation;
    core_machine_immutable_rom_mapping
        immutable_rom_mappings[CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY];
    lib_size immutable_rom_mapping_count;
    lib_u8 entry_plan_applied;
    core_machine_cpu_profile cpu_profile;
    lib_u8 cpu_80386_cr_mov_ignores_mod;
    x86_fpu *fpu;
    /* The CPU owns its architectural/decoder layout.  The board owns exactly
     * one opaque execution lifetime and never mirrors those fields. */
    core_machine_cpu_execution_context *executor_cpu_execution;
    t_ram executor_memory;
    t_port executor_port;
    const core_machine_firmware_provider *firmware_provider;
    void *firmware_provider_context;
    core_machine_firmware_context firmware_context;
    lib_i32 firmware_operation_active;
    const core_machine_execution_provider *execution_provider;
    void *execution_provider_context;
    lib_i32 execution_provider_frozen;
    /* Core scheduler state is private; attachments use the copied contract. */
    lib_u32 dma_cycle_wait_remaining;
    lib_u8 dma_cycle_bus_ready;
    lib_u8 cpu_cycle_bus_ready;
    core_machine_attachment attachment;
};

#if CORE_MACHINE_RUNTIME_TRACE_ENABLED || defined(CORE_MACHINE_TRACE_IMPLEMENTATION)
void core_machine_trace_initialize(core_machine *machine);
void core_machine_trace_finalize(core_machine *machine);
#else
#define core_machine_trace_initialize(machine) ((void)(machine))
#define core_machine_trace_finalize(machine) ((void)(machine))
#endif
void core_machine_cpu_diagnostic_initialize(core_machine *machine);
void core_machine_cpu_diagnostic_reset(core_machine *machine);
void core_machine_retirement_observation_initialize(core_machine *machine);
void core_machine_retirement_observation_reset(core_machine *machine);
void core_machine_retirement_observation_capture_instruction(core_machine *machine,
    const core_machine_cpu_instruction_observation *observation);
void core_machine_retirement_observation_capture_eligibility_key(
    core_machine *machine);
void core_machine_retirement_observation_publish(core_machine *machine,
    lib_u64 source_ticks);
lib_status core_machine_register_immutable_rom_mapping_from_firmware(
    core_machine *machine, lib_u32 physical_start, const lib_u8 *image,
    lib_size bytes);
lib_status core_machine_register_immutable_rom_mapping_alias_from_firmware(
    core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes);
void core_machine_rollback_immutable_rom_mappings(core_machine *machine,
    lib_size mapping_count);
/* Private fault injection; production composition uses neutral_create. */
lib_status core_machine_neutral_create_with_test_allocation(
    const core_machine_executor_config *config,
    core_machine_memory_test_allocation *test_allocation,
    core_machine_port_test_allocation *port_test_allocation,
    core_machine **out_machine);

lib_u32 core_machine_linear_pc(const core_machine *machine);
void core_machine_external_cycle_invalidate(core_machine *machine);
extern const core_machine_cpu_bus_provider core_machine_cpu_bus;
void core_machine_transaction_trace(void *opaque,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    core_machine_transaction_phase phase, lib_u32 address,
    lib_u32 value, lib_u32 detail);
void core_machine_cpu_external_cycle_trace(void *opaque,
    core_machine_cpu_external_cycle_phase phase,
    core_machine_cpu_external_cycle_space space, lib_u32 address,
    lib_u8 bytes, lib_u8 write,
    core_machine_cpu_memory_access_provenance provenance);
void core_machine_cpu_diagnostic_capture(const core_machine *machine,
    core_machine_cpu_diagnostic *out_diagnostic);
void core_machine_cpu_diagnostic_initialize(core_machine *machine);
void core_machine_cpu_diagnostic_reset(core_machine *machine);
extern const core_machine_cpu_execution_diagnostic_provider
    core_machine_cpu_diagnostic_provider;
/* Production always retains fault and delivered-exception snapshots for the
 * runtime debugger.  Per-retirement capture is installed only when an
 * observer or a physical retirement contract actually needs it. */
extern const core_machine_cpu_execution_diagnostic_provider
    core_machine_cpu_fault_diagnostic_provider;
typedef enum core_machine_time_publication_origin {
    CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT,
    CORE_MACHINE_TIME_PUBLICATION_EXTERNAL_WAIT,
    CORE_MACHINE_TIME_PUBLICATION_DEADLINE,
    CORE_MACHINE_TIME_PUBLICATION_DETERMINISTIC_ADVANCE,
    CORE_MACHINE_TIME_PUBLICATION_L1_COMPATIBILITY
} core_machine_time_publication_origin;
lib_status core_machine_publish_elapsed_ticks(core_machine *machine,
    lib_u64 elapsed_ticks, core_machine_time_publication_origin origin);
/* Deterministic Core-test helper. Product composition advances only through
 * CPU retirement or core_machine_advance_to_next_deadline(). */
lib_status core_machine_advance_time(core_machine *machine,
    lib_u64 source_ticks);
void core_machine_capture_time_observation_private(const core_machine *machine,
    core_machine_time_observation *out_observation);
lib_status core_machine_firmware_invoke(core_machine *machine,
    lib_i32 configuring, lib_i32 track_operation_failures,
    lib_status (*callback)(void *, core_machine_firmware_context *));
lib_i32 core_machine_retirement_time_contract_is_valid(
    core_machine_retirement_time_contract contract);
lib_i32 core_machine_timing_capability_is_valid(
    core_machine_timing_capability capability);
lib_i32 core_machine_external_cycle_timing_is_valid(
    const core_machine_external_cycle_timing *timing);
lib_i32 core_machine_external_access_wait_windows_are_valid(
    const core_machine_external_access_wait_window *windows);
lib_i32 core_machine_transaction_contract_is_valid(
    const core_machine_transaction_contract *contract);
#endif

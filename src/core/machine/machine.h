#ifndef CORE_MACHINE_H
#define CORE_MACHINE_H

#include "type.h"





#include "core/machine/cpu.h"

#include "core/machine/cpu_instructions.h"
#include "core/machine/fpu.h"

#include "core/machine/execution_provider.h"

#include "core/machine/machine_interface.h"

#include "core/machine/clock.h"

#include "core/machine/timeline.h"

#include "core/machine/transaction.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

#include "core/machine/pic.h"

#include "core/machine/pit.h"

#include "core/machine/dma.h"

#include "core/machine/d4_memory.h"

#include "core/machine/rtc.h"

#include "core/machine/fdc.h"

#include "core/machine/hdc.h"

#include "core/machine/kbc.h"

#include "core/machine/xt_ppi_keyboard.h"
#include "core/machine/xt_keyboard.h"

#include "core/machine/vadp.h"

#define CORE_MACHINE_TRACE_CAPACITY 32u
#define CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY 8u
#define CORE_MACHINE_RETIREMENT_QUALIFICATION_CAPACITY 128u
#define CORE_MACHINE_PLAN_MEMORY_DEVICE_COUNT 4u

#ifndef CORE_MACHINE_RUNTIME_TRACE_ENABLED
#define CORE_MACHINE_RUNTIME_TRACE_ENABLED 1
#endif

typedef struct core_machine_plan_memory_device {
    type_unsigned_32 physical_start;
    STD_SIZE_T bytes;
    core_machine_memory_device_callbacks callbacks;
    C_VOID *owner;
} core_machine_plan_memory_device;

typedef struct core_machine_fdc_topology {
    const core_machine_media_registry *media_registry;
    core_machine_fdc_drive_bindings drives;
    core_machine_dma_request_binding dma_request;
    core_machine_fdc_config config;
    core_machine_fdc_terminal_observation_provider observation_provider;
} core_machine_fdc_topology;

typedef struct core_machine_hdc_topology {
    const core_machine_media_registry *media_registry;
    core_machine_media_id media_id;
    core_machine_media_id slave_media_id;
    core_machine_hdc_config config;
} core_machine_hdc_topology;

struct core_machine_plan {
    core_machine_config configuration;
    core_machine_plan_topology topology;
    core_machine_timing_declaration declarations[
        CORE_MACHINE_TIMING_CAPABILITY_COUNT];
    STD_SIZE_T declaration_count;
    core_machine_controller_timing_rules controller_timing;
    core_machine_plan_memory_device memory_devices[
        CORE_MACHINE_PLAN_MEMORY_DEVICE_COUNT];
    STD_SIZE_T memory_device_count;
    core_machine_d4_memory_config d4_memory;
    const core_machine_media_registry *media_registry;
    core_machine_display_provider_slot *display_provider;
    core_machine_fdc_terminal_observation_provider fdc_observation_provider;
};

typedef struct core_machine_trace_state {
    core_machine_trace_provider provider;
    core_machine_trace_event events[CORE_MACHINE_TRACE_CAPACITY];
    type_unsigned_64 next_sequence;
    STD_SIZE_T count;
    C_INT flushing;
} core_machine_trace_state;

typedef struct core_machine_cpu_diagnostic_state {
    core_machine_cpu_diagnostic snapshot;
    STD_SIZE_T next_index;
} core_machine_cpu_diagnostic_state;

typedef struct core_machine_retirement_observation_state {
    core_machine_retirement_observation_provider provider;
    core_machine_retirement_observation pending_observation;
    type_unsigned_64 next_sequence;
    type_bool pending;
} core_machine_retirement_observation_state;
typedef struct core_machine_immutable_rom_mapping {
    type_unsigned_32 physical_start;
    STD_SIZE_T bytes;
    type_unsigned_8 *image;
    type_bool owns_image;
} core_machine_immutable_rom_mapping;

typedef struct core_machine_absent_memory {
    core_machine_absent_memory_config config;
    type_bool configured;
} core_machine_absent_memory;

struct core_machine_firmware_context {
    core_machine *machine;
    type_status operation_status;
    C_INT track_operation_failures;
    C_INT active;
    C_INT configuring;
};

struct core_machine {
    core_machine_lifecycle lifecycle;
    STD_ATOMIC_BOOL stop_requested;
    type_unsigned_32 fault_detail;
    type_unsigned_64 elapsed_ticks;
    core_machine_timeline timeline;
    core_machine_plan timing_plan;
    type_bool timing_plan_copied;
    core_machine_d4_memory d4_memory;
    core_machine_transaction_state transaction;
    core_machine_instruction_timing instruction_timing;
    core_machine_transaction_contract transaction_contract;
    type_unsigned_32 external_cycle_page_tag;
    type_unsigned_64 external_cycle_round_ticks;
    type_unsigned_64 cpu_retirement_wait_ticks;
    type_unsigned_64 cpu_retirement_completion_ticks;
    type_unsigned_64 cpu_retirement_source_ticks;
    type_bool external_cycle_page_valid;
    type_bool external_cycle_pending_valid;
    core_machine_cpu_external_cycle_space external_cycle_pending_space;
    type_unsigned_32 external_cycle_pending_physical;
    type_unsigned_8 external_cycle_pending_bytes;
    type_bool external_cycle_pending_write;
    core_machine_cpu_memory_access_provenance external_cycle_pending_provenance;
    type_bool external_cycle_overlap_valid;
    type_unsigned_32 external_cycle_overlap_next_physical;
    type_bool external_cycle_round_overflow;
    type_bool cpu_retirement_wait_pending;
    type_unsigned_64 maximum_instruction_ticks;
    core_machine_retirement_time_contract retirement_time_contract;
    type_bool source_timing_unallocated;
    core_machine_retirement_timing_origin source_timing_origin;
    type_unsigned_32 source_timing_form_id;
    type_unsigned_32 source_timing_key_id;
    type_unsigned_32 source_timing_formula_inputs;
    core_machine_retirement_repeat_phase source_timing_repeat_phase;
    core_machine_retirement_eligibility_key retirement_eligibility_key;
    type_bool retirement_eligibility_key_valid;
    core_machine_retirement_eligibility_key retirement_qualification[
        CORE_MACHINE_RETIREMENT_QUALIFICATION_CAPACITY];
    STD_SIZE_T retirement_qualification_count;
    type_bool source_repeat_active;
    type_unsigned_16 source_repeat_cs;
    type_unsigned_32 source_repeat_eip;
    type_unsigned_8 source_repeat_opcode;
    type_unsigned_8 source_repeat_prefix;
    type_bool source_repeat_operand_size;
    type_bool source_repeat_address_size;
    core_machine_clock_domain dma_clock;
    core_machine_clock_domain pit_clock;
    core_machine_clock_domain auxiliary_pit_clock;
    core_machine_clock_domain rtc_clock;
    core_machine_clock_domain vadp_clock;
    core_machine_clock_domain kbc_clock;
    core_machine_clock_domain provider_clock;
    core_machine_time_axis time_axis;
    core_machine_l1_compatibility_policy l1_compatibility_policy;
    type_unsigned_32 kbc_typematic_initial_ticks;
    type_unsigned_32 kbc_typematic_repeat_ticks;
    type_unsigned_32 kbc_command_response_ticks;
    type_unsigned_32 kbc_serial_delivery_ticks;
    type_bool kbc_input_port_configured;
    type_unsigned_8 kbc_input_port;
    core_machine_keyboard_topology keyboard_topology;
    core_machine_display_port_topology display_ports;
    type_bool display_configured;
    core_machine_dma_wiring dma_wiring;
    core_machine_dma_request_binding fdc_dma_request;
    core_machine_dma_request_binding hdc_dma_request;
    core_machine_dma_request_binding refresh_dma_request;
    type_bool dma_configured;
    core_machine_rtc_cmos_config rtc_cmos_config;
    type_bool rtc_cmos_configured;
    core_machine_planar_parity_config planar_parity_config;
    type_unsigned_8 planar_parity_port_b;
    type_bool planar_parity_configured;
    type_bool planar_parity_latched;
    type_bool planar_parity_nmi_signaled;
    type_bool xt_ppi_speaker_configured;
    type_bool xt_ppi_speaker_gate;
    type_bool xt_ppi_speaker_data_enabled;
    type_bool speaker_output;
    core_machine_d4_platform_config d4_platform_config;
    type_unsigned_8 d4_platform_port_b;
    type_bool d4_platform_configured;
    type_bool d4_platform_iochk_latched;
    type_bool d4_platform_failsafe_latched;
    type_bool d4_platform_nmi_signaled;
    core_machine_absent_memory absent_memory[CORE_MACHINE_ABSENT_MEMORY_WINDOW_COUNT];
    core_machine_fdc_topology fdc_topology;
    type_bool fdc_configured;
    core_machine_hdc_topology hdc_topology;
    type_bool hdc_configured;
    core_machine_trace_state trace;
    core_machine_cpu_diagnostic_state cpu_diagnostic;
    core_machine_retirement_observation_state retirement_observation;
    core_machine_immutable_rom_mapping
        immutable_rom_mappings[CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY];
    STD_SIZE_T immutable_rom_mapping_count;
    type_bool entry_plan_applied;
    core_machine_cpu_profile cpu_profile;
    type_bool cpu_80386_cr_mov_ignores_mod;
    core_machine_fpu fpu;
    t_cpu executor_cpu;
    t_cpuins executor_cpu_instructions;
    core_machine_cpu_execution_context executor_cpu_execution;
    t_ram executor_memory;
    t_port executor_port;
    t_pic shared_pic_master;
    t_pic shared_pic_slave;
    core_machine_pic_irq_source shared_pit_irq0_source;
    t_pit shared_pit;
    t_pit auxiliary_pit;
    type_bool auxiliary_pit_configured;
    t_latch shared_dma_latch;
    t_dma shared_dma_primary;
    t_dma shared_dma_secondary;
    core_machine_rtc shared_rtc;
    core_machine_fdc fdc;
    core_machine_hdc hdc;
    t_kbc shared_kbc;
    core_machine_xt_ppi_keyboard xt_ppi_keyboard;
    core_machine_xt_keyboard xt_keyboard;
    t_vadp shared_vadp;
    const core_machine_firmware_provider *firmware_provider;
    C_VOID *firmware_provider_context;
    core_machine_firmware_context firmware_context;
    C_INT firmware_operation_active;
    const core_machine_execution_provider *execution_provider;
    C_VOID *execution_provider_context;
    C_INT execution_provider_frozen;
    /* Append-only scheduler state keeps existing internal offsets stable. */
    type_unsigned_32 dma_cycle_wait_remaining;
    type_bool dma_cycle_bus_ready;
    type_bool cpu_cycle_bus_ready;
    /* D4 refresh request state; it is serviced at the shared arbitration point. */
    type_bool d4_refresh_hold_pending;
    type_bool d4_refresh_pulse_active;
    type_unsigned_8 d4_refresh_address;
};

type_status core_machine_bus_initialize(core_machine *machine);
C_VOID core_machine_bus_finalize(core_machine *machine);
#if CORE_MACHINE_RUNTIME_TRACE_ENABLED || defined(CORE_MACHINE_TRACE_IMPLEMENTATION)
C_VOID core_machine_trace_initialize(core_machine *machine);
C_VOID core_machine_trace_finalize(core_machine *machine);
C_VOID core_machine_trace_record(
    core_machine *machine,
    core_machine_trace_event_type type,
    type_unsigned_32 address,
    type_unsigned_32 value,
    type_unsigned_32 detail);
#else
#define core_machine_trace_initialize(machine) ((C_VOID)(machine))
#define core_machine_trace_finalize(machine) ((C_VOID)(machine))
#define core_machine_trace_record(machine, type, address, value, detail) \
    do { \
        (C_VOID)sizeof(machine); \
        (C_VOID)sizeof(type); \
        (C_VOID)sizeof(address); \
        (C_VOID)sizeof(value); \
        (C_VOID)sizeof(detail); \
    } while (0)
#endif
C_VOID core_machine_cpu_diagnostic_initialize(core_machine *machine);
C_VOID core_machine_cpu_diagnostic_reset(core_machine *machine);
C_VOID core_machine_retirement_observation_initialize(core_machine *machine);
C_VOID core_machine_retirement_observation_reset(core_machine *machine);
C_VOID core_machine_retirement_observation_capture_instruction(core_machine *machine,
    const t_cpu *cpu, const t_cpuins *instructions);
C_VOID core_machine_retirement_observation_capture_eligibility_key(
    core_machine *machine);
C_VOID core_machine_retirement_observation_publish(core_machine *machine,
    type_unsigned_64 source_ticks);
C_INT core_machine_configuration_is_open(const core_machine *machine);
type_status core_machine_register_reset_rom_alias(core_machine *machine);
C_INT core_machine_mutable_operation_is_allowed(const core_machine *machine);
type_status core_machine_register_immutable_rom_mapping_from_firmware(
    core_machine *machine, type_unsigned_32 physical_start, const type_unsigned_8 *image,
    STD_SIZE_T bytes);
type_status core_machine_register_immutable_rom_mapping_alias_from_firmware(
    core_machine *machine, type_unsigned_32 source_start,
    type_unsigned_32 physical_start, STD_SIZE_T bytes);
C_VOID core_machine_rollback_immutable_rom_mappings(core_machine *machine,
    STD_SIZE_T mapping_count);
/* Private test-only create seam; the public create contract remains unchanged. */
type_status core_machine_create_with_test_memory_allocation(
    const core_machine_config *config, core_machine **out_machine,
    core_machine_memory_test_allocation *test_allocation);
type_status core_machine_create_with_test_port_allocation(
    const core_machine_config *config, core_machine **out_machine,
    core_machine_port_test_allocation *test_allocation);
type_status core_machine_configure_fdc(core_machine *machine,
    const core_machine_fdc_topology *topology);
type_status core_machine_configure_hdc(core_machine *machine,
    const core_machine_hdc_topology *topology);
type_unsigned_32 core_machine_linear_pc(const core_machine *machine);
C_VOID core_machine_external_cycle_invalidate(core_machine *machine);
C_VOID core_machine_transaction_trace(C_VOID *opaque,
    core_machine_transaction_owner owner, core_machine_transaction_kind kind,
    core_machine_transaction_phase phase, type_unsigned_32 address,
    type_unsigned_32 value, type_unsigned_32 detail);
C_VOID core_machine_cpu_external_cycle_trace(C_VOID *opaque,
    core_machine_cpu_external_cycle_phase phase,
    core_machine_cpu_external_cycle_space space, type_unsigned_32 address,
    type_unsigned_8 bytes, type_bool write,
    core_machine_cpu_memory_access_provenance provenance);
C_VOID core_machine_cpu_diagnostic_capture(const core_machine *machine,
    core_machine_cpu_diagnostic *out_diagnostic);
C_VOID core_machine_cpu_diagnostic_initialize(core_machine *machine);
C_VOID core_machine_cpu_diagnostic_reset(core_machine *machine);
extern const core_machine_cpu_execution_diagnostic_provider
    core_machine_cpu_diagnostic_provider;
C_VOID core_machine_board_cold_reset(core_machine *machine);
C_VOID core_machine_board_after_pit_reset(core_machine *machine);
C_VOID core_machine_board_refresh_nmi(core_machine *machine);
C_VOID core_machine_board_configure_xt_ppi_speaker(core_machine *machine);
C_VOID core_machine_board_set_xt_ppi_speaker(core_machine *machine,
    type_bool timer_gate, type_bool data_enabled);
typedef enum core_machine_time_publication_origin {
    CORE_MACHINE_TIME_PUBLICATION_CPU_RETIREMENT,
    CORE_MACHINE_TIME_PUBLICATION_EXTERNAL_WAIT,
    CORE_MACHINE_TIME_PUBLICATION_DEADLINE,
    CORE_MACHINE_TIME_PUBLICATION_DETERMINISTIC_ADVANCE,
    CORE_MACHINE_TIME_PUBLICATION_L1_COMPATIBILITY
} core_machine_time_publication_origin;
type_status core_machine_publish_elapsed_ticks(core_machine *machine,
    type_unsigned_64 elapsed_ticks, core_machine_time_publication_origin origin);
/* Deterministic Core-test helper. Product composition advances only through
 * CPU retirement or core_machine_advance_to_next_deadline(). */
type_status core_machine_advance_time(core_machine *machine,
    type_unsigned_64 source_ticks);
C_VOID core_machine_capture_time_observation_private(const core_machine *machine,
    core_machine_time_observation *out_observation);
type_status core_machine_firmware_invoke(core_machine *machine,
    C_INT configuring, C_INT track_operation_failures,
    type_status (*callback)(C_VOID *, core_machine_firmware_context *));
type_status core_machine_firmware_handle_software_interrupt(C_VOID *opaque,
    type_unsigned_8 vector, const core_machine_firmware_interrupt_frame *frame,
    core_machine_firmware_interrupt_result *result, type_bool *out_handled);
type_status core_machine_plan_validate(const core_machine_plan *plan);
type_status core_machine_plan_apply_topology(core_machine *machine,
    const core_machine_plan *plan);
C_INT core_machine_retirement_time_contract_is_valid(
    core_machine_retirement_time_contract contract);
C_INT core_machine_timing_capability_is_valid(
    core_machine_timing_capability capability);
C_INT core_machine_external_cycle_timing_is_valid(
    const core_machine_external_cycle_timing *timing);
C_INT core_machine_external_access_wait_windows_are_valid(
    const core_machine_external_access_wait_window *windows);
C_INT core_machine_transaction_contract_is_valid(
    const core_machine_transaction_contract *contract);
C_INT core_machine_clock_plan_is_valid(const core_machine_clock_plan *plan);
const core_machine_timing_declaration *core_machine_plan_declaration_find(
    const core_machine_plan *plan, core_machine_timing_capability capability);
#endif

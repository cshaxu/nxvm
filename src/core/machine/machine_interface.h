#ifndef CORE_MACHINE_INTERFACE_H
#define CORE_MACHINE_INTERFACE_H


#include "core/machine/cpu_interface.h"
#include "core/machine/controller_interface.h"
#include "core/machine/display_interface.h"
#include "core/machine/fpu_interface.h"
#include "core/machine/execution_provider.h"
#include "core/machine/firmware_interface.h"
#include "core/machine/lifecycle_interface.h"
#include "core/machine/memory_interface.h"
#include "core/machine/port_interface.h"
#include "type.h"
#include "core/machine/trace_interface.h"
#include "core/machine/rom_mapping_interface.h"
#include "core/machine/entry_plan_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

#define CORE_MACHINE_DEFAULT_MEMORY_BYTES (16u * 1024u * 1024u)
#define CORE_MACHINE_MINIMUM_MEMORY_BYTES (2u * 1024u * 1024u)
#define CORE_MACHINE_MAXIMUM_MEMORY_BYTES (64u * 1024u * 1024u)
#define CORE_MACHINE_RTC_TYPE_DISK_FLOPPY 0x10u
#define CORE_MACHINE_RTC_TYPE_DISK_FIXED 0x12u
#define CORE_MACHINE_RTC_EQUIPMENT 0x14u
#define CORE_MACHINE_RTC_BASEMEM_LSB 0x15u
#define CORE_MACHINE_RTC_BASEMEM_MSB 0x16u
#define CORE_MACHINE_RTC_TYPE_DISK_FIXED_EXTENDED_0 0x19u

/* A domain receives floor((phase + elapsed * numerator) / denominator)
 * ticks. All-zero is retained configuration shorthand for identity 1/1. */
typedef struct core_machine_clock_ratio {
    type_unsigned_32 numerator;
    type_unsigned_32 denominator;
    type_unsigned_32 reset_phase;
} core_machine_clock_ratio;

/* Ratios are relative to core_machine elapsed ticks, not host time. */
typedef struct core_machine_clock_plan {
    core_machine_clock_ratio dma;
    core_machine_clock_ratio pit;
    core_machine_clock_ratio vadp;
    core_machine_clock_ratio kbc;
    core_machine_clock_ratio provider;
} core_machine_clock_plan;

/* Level 2 costs are relative to one completed executor refresh. Zero keeps the
 * legacy ticks_per_instruction base and disables the corresponding surcharge. */
typedef struct core_machine_instruction_timing {
    type_unsigned_32 base_ticks;
    type_unsigned_32 prefix_surcharge;
    type_unsigned_32 taken_branch_surcharge;
    type_unsigned_32 data_memory_surcharge;
    type_unsigned_32 io_surcharge;
    type_unsigned_32 rep_iteration_surcharge;
} core_machine_instruction_timing;

typedef struct core_machine_config {
    STD_SIZE_T memory_bytes;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    /* Compatibility base-cost shorthand when instruction_timing.base_ticks is 0. */
    type_unsigned_32 ticks_per_instruction;
    core_machine_instruction_timing instruction_timing;
    core_machine_clock_plan clock_plan;
    type_unsigned_32 kbc_typematic_initial_ticks;
    type_unsigned_32 kbc_typematic_repeat_ticks;
    type_unsigned_32 kbc_command_response_ticks;
} core_machine_config;

typedef struct core_machine_display_port_topology {
    type_unsigned_16 attribute_first;
    type_unsigned_16 attribute_last;
    type_unsigned_16 sequencer_first;
    type_unsigned_16 sequencer_last;
    type_unsigned_16 graphics_first;
    type_unsigned_16 graphics_last;
    type_unsigned_16 crtc_first;
    type_unsigned_16 crtc_last;
} core_machine_display_port_topology;

/* Composition binds the neutral provider slot; core freezes it when this
 * one-time display declaration has been applied. */
typedef struct core_machine_display_config {
    core_machine_vadp_text_timing text_timing;
    core_machine_vadp_ega_sequencer_config ega_sequencer;
    core_machine_vadp_ega_controller_config ega_controllers;
    core_machine_display_port_topology ports;
    core_machine_display_provider_slot *provider;
} core_machine_display_config;

#define CORE_MACHINE_RTC_DEFAULT_COUNT 6u

typedef struct core_machine_rtc_default_byte {
    type_unsigned_8 index;
    type_unsigned_8 value;
} core_machine_rtc_default_byte;

typedef struct core_machine_rtc_cmos_config {
    type_unsigned_16 index_port;
    type_unsigned_16 data_port;
    type_unsigned_8 irq;
    type_unsigned_8 nmi_mask_bit;
    type_unsigned_32 ticks_per_second;
    core_machine_rtc_default_byte defaults[CORE_MACHINE_RTC_DEFAULT_COUNT];
    STD_SIZE_T default_count;
} core_machine_rtc_cmos_config;

/* The current DMA consumer is embedded core FDC storage; composition receives
 * only the resulting frozen request binding, never DMA controller storage. */
typedef struct core_machine_dma_wiring {
    type_unsigned_8 fdc_channel;
} core_machine_dma_wiring;

/* Composition retains the media provider policy, while core copies this
 * neutral controller topology and owns the connected controller state. */
typedef struct core_machine_fdc_topology {
    const core_machine_media_registry *media_registry;
    core_machine_fdc_drive_bindings drives;
    core_machine_dma_request_binding dma_request;
    core_machine_fdc_config config;
} core_machine_fdc_topology;

typedef struct core_machine_hdc_topology {
    const core_machine_media_registry *media_registry;
    core_machine_media_id media_id;
    core_machine_hdc_config config;
} core_machine_hdc_topology;

typedef enum core_machine_stop_reason {
    CORE_MACHINE_STOP_NONE = 0,
    CORE_MACHINE_STOP_BUDGET,
    CORE_MACHINE_STOP_PAUSED,
    CORE_MACHINE_STOP_GUEST_EXIT,
    CORE_MACHINE_STOP_REQUESTED,
    CORE_MACHINE_STOP_RESET_REQUESTED,
    CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT,
    CORE_MACHINE_STOP_FAULT
} core_machine_stop_reason;

typedef struct core_machine_run_budget {
    type_unsigned_64 instructions;
    type_unsigned_64 ticks;
} core_machine_run_budget;

typedef struct core_machine_run_result {
    core_machine_stop_reason reason;
    type_unsigned_64 executed;
    type_unsigned_64 ticks;
    type_unsigned_64 elapsed_ticks;
    type_unsigned_32 linear_pc;
    type_unsigned_32 detail;
} core_machine_run_result;

typedef struct core_machine_observation {
    core_machine_lifecycle lifecycle;
    type_unsigned_64 elapsed_ticks;
    core_machine_cpu_state cpu;
    core_machine_cpu_diagnostic diagnostic;
} core_machine_observation;

type_status core_machine_create(
    const core_machine_config *config,
    core_machine **out_machine);

type_status core_machine_reset(core_machine *machine);

type_status core_machine_reconfigure_memory(core_machine *machine,
    STD_SIZE_T memory_bytes);

type_status core_machine_get_lifecycle(
    const core_machine *machine,
    core_machine_lifecycle *out_lifecycle);

type_status core_machine_get_cpu_state(
    const core_machine *machine,
    core_machine_cpu_state *out_state);

type_status core_machine_get_cpu_profile(
    const core_machine *machine, core_machine_cpu_profile *out_profile);
type_status core_machine_get_fpu_profile(
    const core_machine *machine, core_machine_fpu_profile *out_profile);
type_status core_machine_get_fpu_state(
    const core_machine *machine, core_machine_fpu_state *out_state);
type_status core_machine_get_memory_bytes(
    const core_machine *machine, STD_SIZE_T *out_memory_bytes);
type_status core_machine_get_elapsed_ticks(
    const core_machine *machine, type_unsigned_64 *out_elapsed_ticks);

type_status core_machine_get_cpu_diagnostic(
    const core_machine *machine,
    core_machine_cpu_diagnostic *out_diagnostic);

type_status core_machine_run(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *result);

type_status core_machine_request_stop(core_machine *machine);

/* VM devices may request the architected NMI mask through this operation;
 * they never borrow CPU storage to change it. */
type_status core_machine_set_nmi_mask(core_machine *machine, C_INT masked);
type_status core_machine_get_nmi_mask(const core_machine *machine,
    C_INT *out_masked);

type_status core_machine_keyboard_submit_scan_code(core_machine *machine,
    type_unsigned_8 scan_code);
type_status core_machine_keyboard_submit_scan_codes(core_machine *machine,
    const type_unsigned_8 *scan_codes, STD_SIZE_T count);
type_status core_machine_mouse_submit_relative(core_machine *machine,
    type_signed_16 delta_x, type_signed_16 delta_y, type_unsigned_8 buttons);

type_status core_machine_capture_display_snapshot(const core_machine *machine,
    core_machine_display_snapshot *out_snapshot);

type_status core_machine_configure_display(core_machine *machine,
    const core_machine_display_config *config);
type_status core_machine_configure_dma(core_machine *machine,
    const core_machine_dma_wiring *wiring,
    core_machine_dma_request_binding *out_fdc_request);
type_status core_machine_configure_rtc_cmos(core_machine *machine,
    const core_machine_rtc_cmos_config *config);
type_status core_machine_configure_fdc(core_machine *machine,
    const core_machine_fdc_topology *topology);
type_status core_machine_configure_hdc(core_machine *machine,
    const core_machine_hdc_topology *topology);

type_status core_machine_report_fault(
    core_machine *machine,
    type_unsigned_32 detail);

type_status core_machine_capture_observation(
    const core_machine *machine, core_machine_observation *out_observation);

type_status core_machine_bind_execution_provider(core_machine *machine,
    const core_machine_execution_provider *provider, C_VOID *context);
type_status core_machine_freeze_execution_providers(core_machine *machine);

C_VOID core_machine_destroy(core_machine *machine);

#ifdef __cplusplus
}
#endif

#endif

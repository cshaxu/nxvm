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
#include "core/machine/pic_interface.h"
#include "type.h"
#include "core/machine/trace_interface.h"
#include "core/machine/retirement_observation_interface.h"
#include "core/machine/fdc_observation_interface.h"
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
#define CORE_MACHINE_RTC_EXTMEM_LSB 0x17u
#define CORE_MACHINE_RTC_EXTMEM_MSB 0x18u
#define CORE_MACHINE_RTC_TYPE_DISK_FIXED_EXTENDED_0 0x19u
#define CORE_MACHINE_PC_AT_PORT_B 0x0061u

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
    /* An optional second PIT chip is a distinct board clock consumer. */
    core_machine_clock_ratio auxiliary_pit;
    core_machine_clock_ratio rtc;
    core_machine_clock_ratio vadp;
    core_machine_clock_ratio kbc;
    core_machine_clock_ratio provider;
} core_machine_clock_plan;

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
    type_unsigned_64 ticks_per_second;
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

/* A profile-selected external CPU-memory-cycle policy. The Core CPU owner
 * charges the declared page result only after a matching lifecycle commit.
 * A hit additionally needs an explicit, still-in-flight sequential overlap;
 * adjacency of completed logical accesses is never an overlap. */
typedef enum core_machine_external_cycle_overlap_policy {
    CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_DISABLED = 0,
    CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL = 1
} core_machine_external_cycle_overlap_policy;

typedef struct core_machine_external_cycle_timing {
    type_unsigned_32 page_bytes;
    type_unsigned_32 page_miss_ticks;
    type_unsigned_32 page_hit_ticks;
    core_machine_external_cycle_overlap_policy overlap_policy;
    /* Both zero retains legacy all-memory eligibility. */
    type_unsigned_32 first_eligible_address;
    type_unsigned_32 last_eligible_address;
} core_machine_external_cycle_timing;

#define CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY 7u

typedef struct core_machine_external_access_wait_window {
    core_machine_cpu_external_cycle_space space;
    type_unsigned_32 first_address;
    type_unsigned_32 last_address;
    type_unsigned_32 wait_ticks;
} core_machine_external_access_wait_window;

/* Immutable plan-selected policy for the existing Core transaction,
 * availability and arbitration owners. It carries values only: CPU, DMA and
 * device owners retain their state machines and do not receive callbacks. */
typedef struct core_machine_transaction_contract {
    core_machine_external_cycle_timing external_cycle_timing;
    core_machine_external_access_wait_window external_access_wait_windows[
        CORE_MACHINE_EXTERNAL_ACCESS_WAIT_WINDOW_CAPACITY];
    type_unsigned_32 dma_cycle_wait_quanta;
    type_bool dma_cycle_bus_ready_gate_enabled;
    type_bool cpu_cycle_bus_ready_gate_enabled;
    type_bool cpu_prefetch_reservation_enabled;
} core_machine_transaction_contract;

/* The product selects its keyboard controller once when it freezes the Core
 * configuration.  XT PPI is the IBM 5160 system-board attachment, not a
 * keyboard-only variant of the PC/AT 8042. */
typedef enum core_machine_keyboard_topology {
    CORE_MACHINE_KEYBOARD_TOPOLOGY_8042 = 0,
    CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI = 1
} core_machine_keyboard_topology;

/* IBM 5160 PPI port-C fault inputs. A selected board source supplies the
 * current electrical condition; the XT PPI owns its port and NMI meaning. */
typedef enum core_machine_xt_ppi_fault_input {
    CORE_MACHINE_XT_PPI_FAULT_IO_CHECK = 0,
    CORE_MACHINE_XT_PPI_FAULT_RAM_PARITY = 1
} core_machine_xt_ppi_fault_input;

typedef struct core_machine_xt_ppi_keyboard_config {
    type_unsigned_16 port_a;
    type_unsigned_16 port_b;
    type_unsigned_16 port_c;
    type_unsigned_16 control_port;
    type_unsigned_8 irq;
    /* IBM 5160 system-board DIP electrical values.  PB3 selects low
     * (switches 1--4) or high (switches 5--8) nibble at PC0--PC3. */
    type_unsigned_8 switches_low;
    type_unsigned_8 switches_high;
} core_machine_xt_ppi_keyboard_config;

typedef struct core_machine_config {
    STD_SIZE_T memory_bytes;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    /* Original 80386 silicon accepts MOV CR ModR/M forms with MOD other
     * than 11b, using the r/m field as the general-register selector. */
    type_bool cpu_80386_cr_mov_ignores_mod;
    core_machine_a20_wrap_policy a20_wrap_policy;
    /* Compatibility base-cost shorthand when instruction_timing.base_ticks is 0. */
    type_unsigned_32 ticks_per_instruction;
    core_machine_instruction_timing instruction_timing;
    core_machine_transaction_contract transaction_contract;
    core_machine_clock_plan clock_plan;
    /* Frozen shared system-PIT chip selection; zero preserves 8254 users. */
    core_machine_pit_personality shared_pit_personality;
    core_machine_pic_topology pic_topology;
    /* Product profiles select one or two controllers explicitly.  Zero is
     * retained only for direct Core fixture compatibility and resolves to two. */
    type_unsigned_8 dma_controller_count;
    core_machine_time_axis time_axis;
    core_machine_pic_irq_timing pic_irq_timing;
    core_machine_l1_compatibility_policy l1_compatibility_policy;
    /* Physical mode refuses an unallocated successful retirement before it can
     * be published into a clock-domain plan. */
    core_machine_retirement_time_contract retirement_time_contract;
    /* Read synchronously by create and copied into Core-owned storage; the
     * caller retains no lifetime obligation after core_machine_create(). */
    const core_machine_retirement_qualification_descriptor *retirement_qualification;
    type_unsigned_32 kbc_typematic_initial_ticks;
    type_unsigned_32 kbc_typematic_repeat_ticks;
    type_unsigned_32 kbc_command_response_ticks;
    /* Optional board-provided response visibility phase.  A nonzero value
     * holds a completed KBC command reply through this many status reads. */
    type_unsigned_8 kbc_command_response_status_polls;
    type_unsigned_32 kbc_serial_delivery_ticks;
    /* Optional product-selected 8254 topology; no output consumer is implied. */
    type_bool auxiliary_pit_present;
    type_unsigned_16 auxiliary_pit_base_port;
    /* False preserves PC/AT AUX; true selects a keyboard-only 8042 topology. */
    type_bool kbc_aux_absent;
    /* A board may freeze the electrical 8042 input pins observed by command
     * C0h.  Unconfigured machines retain the controller's AT default. */
    type_bool kbc_input_port_configured;
    type_unsigned_8 kbc_input_port;
    /* Frozen board output-pin state applied whenever the selected 8042 resets.
     * It is an electrical input to the generic controller, not a profile name. */
    type_bool kbc_reset_output_port_configured;
    type_unsigned_8 kbc_reset_output_port;
    core_machine_keyboard_topology keyboard_topology;
    core_machine_xt_ppi_keyboard_config xt_ppi_keyboard;
} core_machine_config;

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

/* Construction-plan qualifications consume copied Core values. They are not
 * runtime controller setters; profile provenance remains outside Core. */
typedef enum core_machine_controller_timing_rule {
    CORE_MACHINE_CONTROLLER_TIMING_RULE_L2_FALLBACK = 0,
    CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
    CORE_MACHINE_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES
} core_machine_controller_timing_rule;

typedef struct core_machine_controller_timing_rules {
    core_machine_controller_timing_rule pic_visibility;
    core_machine_controller_timing_rule dma_clock;
    core_machine_controller_timing_rule dma_service;
    core_machine_controller_timing_rule pit_clock;
    core_machine_controller_timing_rule rtc_clock;
} core_machine_controller_timing_rules;

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

/* This remains a copied board declaration. Its output provider is registered
 * separately on the Core-owned plan. */
typedef struct core_machine_display_config {
    core_machine_vadp_text_timing text_timing;
    core_machine_vadp_text_glyph_config text_glyphs;
    type_bool cga_vram_present;
    type_bool ega_present;
    core_machine_vadp_ega_personality ega_personality;
    core_machine_vadp_cecg_config cecg;
    core_machine_vadp_ega_sequencer_config ega_sequencer;
    core_machine_vadp_ega_controller_config ega_controllers;
    core_machine_display_port_topology ports;
} core_machine_display_config;

#define CORE_MACHINE_RTC_DEFAULT_COUNT 6u
/* A board seed contributes the MC146818 NVRAM window 0Eh--3Fh. Calendar and
 * status registers remain Core-owned. */
#define CORE_MACHINE_RTC_DEFAULT_CAPACITY 50u

/* Board composition supplies a copied RTC phase scale.  L3 means the values
 * are a direct selected-board conversion; L2 means a board ratio estimate.
 * Core consumes ticks only and never receives a host clock or callback. */
typedef enum core_machine_rtc_timing_provenance {
    CORE_MACHINE_RTC_TIMING_L2_RATIO = 0,
    CORE_MACHINE_RTC_TIMING_L3_SOURCE = 1
} core_machine_rtc_timing_provenance;

typedef struct core_machine_rtc_timing_plan {
    type_unsigned_32 uip_lead_ticks;
    type_unsigned_32 update_ticks;
    core_machine_rtc_timing_provenance provenance;
} core_machine_rtc_timing_plan;

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
    core_machine_rtc_timing_plan timing;
    core_machine_rtc_default_byte defaults[CORE_MACHINE_RTC_DEFAULT_CAPACITY];
    STD_SIZE_T default_count;
    /* Unit-only synthetic board defaults may ask Core to derive the AT
     * configuration checksum.  A session-provided board seed owns its
     * complete NVRAM image, including 2Eh/2Fh, and clears this flag. */
    type_bool derive_configuration_checksum;
} core_machine_rtc_cmos_config;

typedef enum core_machine_planar_parity_refresh_status_source {
    /* Port B reflects the directly wired PIT counter 1 output. */
    CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1 = 0,
    /* A board-provided period derives the readable refresh signal from the
     * sole Core elapsed-tick axis. This is a board signal model, not a second
     * clock or a writable runtime policy. */
    CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE
} core_machine_planar_parity_refresh_status_source;

typedef struct core_machine_planar_parity_config {
    /* IBM PC/AT system-board port B; zero memory_bytes selects its timer and
     * speaker wiring without claiming a parity-memory producer. */
    type_unsigned_16 port;
    STD_SIZE_T memory_bytes;
    core_machine_planar_parity_refresh_status_source refresh_status_source;
    /* Required only for ELAPSED_TICK_TOGGLE: ticks between output edges. */
    type_unsigned_32 refresh_status_toggle_ticks;
} core_machine_planar_parity_config;

typedef struct core_machine_planar_parity_observation {
    C_INT configured;
    C_INT enabled;
    C_INT latched;
    C_INT nmi_signaled;
} core_machine_planar_parity_observation;

/* DeskPro D4 platform port B.  This is distinct from IBM planar parity even
 * though both selected boards decode port 61h. */
typedef struct core_machine_d4_platform_config {
    type_unsigned_16 port;
    type_unsigned_8 failsafe_pit_counter;
} core_machine_d4_platform_config;

typedef struct core_machine_d4_platform_observation {
    C_INT configured;
    C_INT iochk_enabled;
    C_INT failsafe_enabled;
    C_INT iochk_latched;
    C_INT failsafe_latched;
    C_INT nmi_signaled;
} core_machine_d4_platform_observation;

/* Construction-only input for the selected DeskPro D4 RAM controller.  ROM
 * decoding remains owned by the immutable firmware mapping. */
typedef struct core_machine_d4_memory_config {
    type_bool present;
    type_unsigned_8 diagnostic_low;
    type_unsigned_8 diagnostic_high;
    type_unsigned_16 ram_setup;
} core_machine_d4_memory_config;
/* Copied logical speaker-line state. The Core owns port-B and PIT sampling;
 * host audio is a separate, optional consumer. */
typedef struct core_machine_speaker_observation {
    C_INT configured;
    C_INT timer_gate;
    C_INT data_enabled;
    C_INT timer_output;
    C_INT output;
} core_machine_speaker_observation;

/* A profile-selected, bounded unpopulated memory window. Reads return the
 * declared fallback byte and writes are deliberately discarded; it never adds
 * installed RAM. This is for board models, not a generic memory default. */
typedef struct core_machine_absent_memory_config {
    type_unsigned_32 physical_start;
    STD_SIZE_T bytes;
    type_unsigned_8 read_value;
} core_machine_absent_memory_config;

#define CORE_MACHINE_ABSENT_MEMORY_WINDOW_COUNT 4u

/* A profile-declared physical alias into installed Core RAM.  This preserves
 * one RAM owner while allowing board address decoding to select it twice. */
typedef struct core_machine_memory_alias_config {
    type_unsigned_32 physical_start;
    type_unsigned_32 backing_start;
    STD_SIZE_T bytes;
} core_machine_memory_alias_config;

#define CORE_MACHINE_MEMORY_ALIAS_COUNT 4u

#define CORE_MACHINE_DMA_CONTROLLER_COUNT 2u
#define CORE_MACHINE_DMA_CASCADE_CHANNEL 4u
#define CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND 0xffu

/* The plan selects the controller/refresh topology independently of an FDC.
 * When present, fdc_channel is the one Core-issued FDC request binding; the
 * unbound value leaves that later device route absent. */
typedef struct core_machine_dma_wiring {
    type_unsigned_8 fdc_channel;
    type_unsigned_8 controller_count;
    type_unsigned_8 cascade_channel;
} core_machine_dma_wiring;

/* Every optional topology is copied into the Core-owned plan before machine
 * creation. Runtime endpoints are registered separately and never enter this
 * public declaration. */
typedef struct core_machine_plan_topology {
    type_unsigned_8 absent_memory_count;
    core_machine_absent_memory_config absent_memory[CORE_MACHINE_ABSENT_MEMORY_WINDOW_COUNT];
    type_unsigned_8 memory_alias_count;
    core_machine_memory_alias_config memory_alias[CORE_MACHINE_MEMORY_ALIAS_COUNT];
    type_bool planar_parity_present;
    core_machine_planar_parity_config planar_parity;
    type_bool d4_platform_present;
    core_machine_d4_platform_config d4_platform;
    type_bool display_present;
    core_machine_display_config display;
    type_bool dma_present;
    core_machine_dma_wiring dma;
    type_bool rtc_cmos_present;
    core_machine_rtc_cmos_config rtc_cmos;
    type_bool fdc_present;
    core_machine_fdc_drive_bindings fdc_drives;
    core_machine_fdc_config fdc;
    type_bool hdc_present;
    core_machine_media_id hdc_media_id;
    core_machine_media_id hdc_slave_media_id;
    core_machine_hdc_config hdc;
} core_machine_plan_topology;

typedef struct core_machine_plan core_machine_plan;

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
    type_unsigned_64 elapsed_ticks;
    type_unsigned_64 next_deadline_tick;
    type_unsigned_64 pacing_ticks_per_second;
    type_unsigned_64 physical_ticks_per_second;
    type_bool next_deadline_valid;
    type_bool pacing_time_available;
    type_bool physical_time_available;
    core_machine_time_progress_disposition progress_disposition;
} core_machine_time_observation;

typedef struct core_machine_timeline_observation {
    type_unsigned_64 now;
    type_unsigned_64 next_sequence;
    type_unsigned_32 pending_events;
} core_machine_timeline_observation;

type_status core_machine_create(
    const core_machine_config *config,
    core_machine **out_machine);

type_status core_machine_plan_create(const core_machine_config *configuration,
    core_machine_plan **out_plan);
C_VOID core_machine_plan_destroy(core_machine_plan *plan);
type_status core_machine_plan_set_topology(core_machine_plan *plan,
    const core_machine_plan_topology *topology);
type_status core_machine_plan_set_controller_timing_rules(core_machine_plan *plan,
    const core_machine_controller_timing_rules *rules);
type_status core_machine_plan_bind_media_registry(core_machine_plan *plan,
    const core_machine_media_registry *registry);
type_status core_machine_plan_bind_display_provider(core_machine_plan *plan,
    core_machine_display_provider_slot *provider);
type_status core_machine_plan_bind_fdc_terminal_observation(core_machine_plan *plan,
    core_machine_fdc_terminal_observation_provider provider);
type_status core_machine_plan_configure_fdc(core_machine_plan *plan,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_fdc_config *config);
type_status core_machine_plan_configure_hdc(core_machine_plan *plan,
    core_machine_media_id media_id, core_machine_media_id slave_media_id,
    const core_machine_hdc_config *config);
type_status core_machine_plan_configure_d4_memory(core_machine_plan *plan,
    const core_machine_d4_memory_config *config);
type_status core_machine_plan_register_memory_device(core_machine_plan *plan,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    const core_machine_memory_device_callbacks *callbacks, C_VOID *owner);
type_status core_machine_create_from_plan(const core_machine_plan *plan,
    core_machine **out_machine);
type_status core_machine_get_timing_disposition(const core_machine *machine,
    core_machine_timing_capability capability,
    core_machine_timing_disposition *out_disposition);
type_status core_machine_get_timing_declaration(const core_machine *machine,
    core_machine_timing_capability capability,
    core_machine_timing_declaration *out_declaration);

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
type_status core_machine_capture_time_observation(const core_machine *machine,
    core_machine_time_observation *out_observation);
/* Core selects and advances to its next valid guest-observable deadline.
 * A false result means an unqualified owner blocks safe fast advance. */
type_status core_machine_advance_to_next_deadline(core_machine *machine,
    type_bool *out_advanced);
/* Turbo may request one bounded, Core-owned escape from a copied L1 state.
 * The caller supplies neither a tick count nor a controller selection. */
type_status core_machine_advance_l1_compatibility(core_machine *machine,
    type_bool *out_advanced);
type_status core_machine_get_timeline_observation(const core_machine *machine,
    core_machine_timeline_observation *out_observation);

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

/* A serial byte received from the keyboard attached to this machine's 8042.
 * Product host adapters query the selected scan set before forming this
 * device-native stream. */
typedef enum core_machine_keyboard_scan_set {
    CORE_MACHINE_KEYBOARD_SCAN_SET_1 = 1u,
    CORE_MACHINE_KEYBOARD_SCAN_SET_2 = 2u
} core_machine_keyboard_scan_set;

type_status core_machine_keyboard_get_native_scan_set(const core_machine *machine,
    type_unsigned_8 *out_scan_set);
type_status core_machine_keyboard_receive_native_byte(core_machine *machine,
    type_unsigned_8 native_byte);
type_status core_machine_keyboard_receive_native_bytes(core_machine *machine,
    const type_unsigned_8 *native_bytes, STD_SIZE_T count);
type_status core_machine_set_xt_ppi_fault_input(core_machine *machine,
    core_machine_xt_ppi_fault_input input, C_INT asserted);
/* A relative report received from the machine's attached pointing device. */
type_status core_machine_mouse_receive_relative(core_machine *machine,
    type_signed_16 delta_x, type_signed_16 delta_y, type_unsigned_8 buttons);

type_status core_machine_capture_display_snapshot(const core_machine *machine,
    core_machine_display_snapshot *out_snapshot);
type_status core_machine_observe_display_snapshot(const core_machine *machine,
    type_bool acknowledged_generation_valid,
    type_unsigned_64 acknowledged_generation,
    core_machine_display_snapshot_observation *out_observation);

type_status core_machine_configure_display(core_machine *machine,
    const core_machine_display_config *config);
type_status core_machine_configure_dma(core_machine *machine,
    const core_machine_dma_wiring *wiring,
    core_machine_dma_request_binding *out_fdc_request);
type_status core_machine_get_fdc_dma_request_binding(const core_machine *machine,
    core_machine_dma_request_binding *out_binding);
/* Selected bus adapters drive this level at deterministic guest-time boundaries. */
type_status core_machine_set_dma_bus_ready(core_machine *machine, C_INT ready);
type_status core_machine_set_cpu_bus_ready(core_machine *machine, C_INT ready);
type_status core_machine_configure_rtc_cmos(core_machine *machine,
    const core_machine_rtc_cmos_config *config);
type_status core_machine_configure_planar_parity(core_machine *machine,
    const core_machine_planar_parity_config *config);
type_status core_machine_configure_d4_platform(core_machine *machine,
    const core_machine_d4_platform_config *config);
type_status core_machine_configure_absent_memory(core_machine *machine,
    const core_machine_absent_memory_config *config);
type_status core_machine_report_planar_parity_fault(core_machine *machine);
type_status core_machine_clear_d4_iochk_fault(core_machine *machine);
type_status core_machine_report_d4_iochk_fault(core_machine *machine);
type_status core_machine_get_planar_parity_observation(const core_machine *machine,
    core_machine_planar_parity_observation *out_observation);
type_status core_machine_get_d4_platform_observation(const core_machine *machine,
    core_machine_d4_platform_observation *out_observation);
type_status core_machine_get_speaker_observation(const core_machine *machine,
    core_machine_speaker_observation *out_observation);

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

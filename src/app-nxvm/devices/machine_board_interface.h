#ifndef CORE_MACHINE_BOARD_INTERFACE_H
#define CORE_MACHINE_BOARD_INTERFACE_H

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/controller_interface.h"
#include "app-nxvm/devices/display_interface.h"
#include "app-nxvm/devices/pic_bus_interface.h"
#include "app-nxvm/devices/fdc_observation_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CORE_MACHINE_RTC_TYPE_DISK_FLOPPY 0x10u
#define CORE_MACHINE_RTC_TYPE_DISK_FIXED 0x12u
#define CORE_MACHINE_RTC_EQUIPMENT 0x14u
#define CORE_MACHINE_RTC_BASEMEM_LSB 0x15u
#define CORE_MACHINE_RTC_BASEMEM_MSB 0x16u
#define CORE_MACHINE_RTC_EXTMEM_LSB 0x17u
#define CORE_MACHINE_RTC_EXTMEM_MSB 0x18u
#define CORE_MACHINE_RTC_TYPE_DISK_FIXED_EXTENDED_0 0x19u
#define CORE_MACHINE_PC_AT_PORT_B 0x0061u

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
    lib_u16 port_a;
    lib_u16 port_b;
    lib_u16 port_c;
    lib_u16 control_port;
    lib_u8 irq;
    /* IBM 5160 system-board DIP electrical values.  PB3 selects low
     * (switches 1--4) or high (switches 5--8) nibble at PC0--PC3. */
    lib_u8 switches_low;
    lib_u8 switches_high;
} core_machine_xt_ppi_keyboard_config;

typedef struct core_machine_config {
    lib_size memory_bytes;
    core_machine_cpu_profile cpu_profile;
    x86_fpu_profile fpu_profile;
    /* Original 80386 silicon accepts MOV CR ModR/M forms with MOD other
     * than 11b, using the r/m field as the general-register selector. */
    lib_u8 cpu_80386_cr_mov_ignores_mod;
    core_machine_a20_wrap_policy a20_wrap_policy;
    /* Compatibility base-cost shorthand when instruction_timing.base_ticks is 0. */
    lib_u32 ticks_per_instruction;
    core_machine_instruction_timing instruction_timing;
    core_machine_transaction_contract transaction_contract;
    core_machine_clock_plan clock_plan;
    /* Frozen shared system-PIT chip selection; zero preserves 8254 users. */
    x86_pit_personality shared_pit_personality;
    core_machine_pic_topology pic_topology;
    /* Product profiles select one or two controllers explicitly.  Zero is
     * retained only for direct Core fixture compatibility and resolves to two. */
    lib_u8 dma_controller_count;
    core_machine_time_axis time_axis;
    core_machine_pic_irq_timing pic_irq_timing;
    core_machine_l1_compatibility_policy l1_compatibility_policy;
    /* Physical mode refuses an unallocated successful retirement before it can
     * be published into a clock-domain plan. */
    core_machine_retirement_time_contract retirement_time_contract;
    /* Read synchronously by create and copied into Core-owned storage; the
     * caller retains no lifetime obligation after core_machine_create(). */
    const core_machine_retirement_qualification_descriptor *retirement_qualification;
    lib_u32 kbc_typematic_initial_ticks;
    lib_u32 kbc_typematic_repeat_ticks;
    lib_u32 kbc_command_response_ticks;
    /* Optional board-provided response visibility phase.  A nonzero value
     * holds a completed KBC command reply through this many status reads. */
    lib_u8 kbc_command_response_status_polls;
    lib_u32 kbc_serial_delivery_ticks;
    /* Optional product-selected 8254 topology; no output consumer is implied. */
    lib_u8 auxiliary_pit_present;
    lib_u16 auxiliary_pit_base_port;
    /* False preserves PC/AT AUX; true selects a keyboard-only 8042 topology. */
    lib_u8 kbc_aux_absent;
    /* A board may freeze the electrical 8042 input pins observed by command
     * C0h.  Unconfigured machines retain the controller's AT default. */
    lib_u8 kbc_input_port_configured;
    lib_u8 kbc_input_port;
    /* Frozen board output-pin state applied whenever the selected 8042 resets.
     * It is an electrical input to the generic controller, not a profile name. */
    lib_u8 kbc_reset_output_port_configured;
    lib_u8 kbc_reset_output_port;
    core_machine_keyboard_topology keyboard_topology;
    core_machine_xt_ppi_keyboard_config xt_ppi_keyboard;
} core_machine_config;

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
    lib_u16 attribute_first;
    lib_u16 attribute_last;
    lib_u16 sequencer_first;
    lib_u16 sequencer_last;
    lib_u16 graphics_first;
    lib_u16 graphics_last;
    lib_u16 crtc_first;
    lib_u16 crtc_last;
} core_machine_display_port_topology;

/* This remains a copied board declaration. Its output provider is registered
 * separately on the Core-owned plan. */
typedef struct core_machine_display_config {
    x86_video_text_timing text_timing;
    x86_video_text_glyph_config text_glyphs;
    lib_u8 cga_vram_present;
    lib_u8 ega_present;
    x86_video_ega_personality ega_personality;
    x86_video_cecg_config cecg;
    x86_video_ega_sequencer_config ega_sequencer;
    x86_video_ega_controller_config ega_controllers;
    core_machine_display_port_topology ports;
    lib_bool vga_present;
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
    lib_u32 uip_lead_ticks;
    lib_u32 update_ticks;
    core_machine_rtc_timing_provenance provenance;
} core_machine_rtc_timing_plan;

typedef struct core_machine_rtc_default_byte {
    lib_u8 index;
    lib_u8 value;
} core_machine_rtc_default_byte;

typedef struct core_machine_rtc_cmos_config {
    lib_u16 index_port;
    lib_u16 data_port;
    lib_u8 irq;
    lib_u8 nmi_mask_bit;
    lib_u32 ticks_per_second;
    core_machine_rtc_timing_plan timing;
    core_machine_rtc_default_byte defaults[CORE_MACHINE_RTC_DEFAULT_CAPACITY];
    lib_size default_count;
    /* Unit-only synthetic board defaults may ask Core to derive the AT
     * configuration checksum.  A session-provided board seed owns its
     * complete NVRAM image, including 2Eh/2Fh, and clears this flag. */
    lib_u8 derive_configuration_checksum;
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
    lib_u16 port;
    lib_size memory_bytes;
    core_machine_planar_parity_refresh_status_source refresh_status_source;
    /* Required only for ELAPSED_TICK_TOGGLE: ticks between output edges. */
    lib_u32 refresh_status_toggle_ticks;
} core_machine_planar_parity_config;

typedef struct core_machine_planar_parity_observation {
    lib_i32 configured;
    lib_i32 enabled;
    lib_i32 latched;
    lib_i32 nmi_signaled;
} core_machine_planar_parity_observation;

/* DeskPro D4 platform port B.  This is distinct from IBM planar parity even
 * though both selected boards decode port 61h. */
typedef struct core_machine_d4_platform_config {
    lib_u16 port;
    lib_u8 failsafe_pit_counter;
} core_machine_d4_platform_config;

typedef struct core_machine_d4_platform_observation {
    lib_i32 configured;
    lib_i32 iochk_enabled;
    lib_i32 failsafe_enabled;
    lib_i32 iochk_latched;
    lib_i32 failsafe_latched;
    lib_i32 nmi_signaled;
} core_machine_d4_platform_observation;

/* Construction-only input for the selected DeskPro D4 RAM controller.  ROM
 * decoding remains owned by the immutable firmware mapping. */
typedef struct core_machine_d4_memory_config {
    lib_u8 present;
    lib_u8 diagnostic_low;
    lib_u8 diagnostic_high;
    lib_u16 ram_setup;
} core_machine_d4_memory_config;
/* Copied logical speaker-line state. The Core owns port-B and PIT sampling;
 * host audio is a separate, optional consumer. */
typedef struct core_machine_speaker_observation {
    lib_i32 configured;
    lib_i32 timer_gate;
    lib_i32 data_enabled;
    lib_i32 timer_output;
    lib_i32 output;
} core_machine_speaker_observation;

/* A profile-selected, bounded unpopulated memory window. Reads return the
 * declared fallback byte and writes are deliberately discarded; it never adds
 * installed RAM. This is for board models, not a generic memory default. */
typedef struct core_machine_absent_memory_config {
    lib_u32 physical_start;
    lib_size bytes;
    lib_u8 read_value;
} core_machine_absent_memory_config;

#define CORE_MACHINE_ABSENT_MEMORY_WINDOW_COUNT 4u

/* A profile-declared physical alias into installed Core RAM.  This preserves
 * one RAM owner while allowing board address decoding to select it twice. */
typedef struct core_machine_memory_alias_config {
    lib_u32 physical_start;
    lib_u32 backing_start;
    lib_size bytes;
} core_machine_memory_alias_config;

#define CORE_MACHINE_MEMORY_ALIAS_COUNT 4u

#define CORE_MACHINE_DMA_CONTROLLER_COUNT 2u
#define CORE_MACHINE_DMA_CASCADE_CHANNEL 4u
#define CORE_MACHINE_DMA_FDC_CHANNEL_UNBOUND 0xffu

/* The plan selects the controller/refresh topology independently of an FDC.
 * When present, fdc_channel is the one Core-issued FDC request binding; the
 * unbound value leaves that later device route absent. */
typedef struct core_machine_dma_wiring {
    lib_u8 fdc_channel;
    lib_u8 controller_count;
    lib_u8 cascade_channel;
} core_machine_dma_wiring;

/* Every optional topology is copied into the Core-owned plan before machine
 * creation. Runtime endpoints are registered separately and never enter this
 * public declaration. */
typedef struct core_machine_plan_topology {
    lib_u8 absent_memory_count;
    core_machine_absent_memory_config absent_memory[CORE_MACHINE_ABSENT_MEMORY_WINDOW_COUNT];
    lib_u8 memory_alias_count;
    core_machine_memory_alias_config memory_alias[CORE_MACHINE_MEMORY_ALIAS_COUNT];
    lib_u8 planar_parity_present;
    core_machine_planar_parity_config planar_parity;
    lib_u8 d4_platform_present;
    core_machine_d4_platform_config d4_platform;
    lib_u8 display_present;
    core_machine_display_config display;
    lib_u8 dma_present;
    core_machine_dma_wiring dma;
    lib_u8 rtc_cmos_present;
    core_machine_rtc_cmos_config rtc_cmos;
    lib_u8 fdc_present;
    core_machine_fdc_drive_bindings fdc_drives;
    core_machine_fdc_config fdc;
    lib_u8 hdc_present;
    core_machine_media_id hdc_media_id;
    core_machine_media_id hdc_slave_media_id;
    core_machine_hdc_config hdc;
} core_machine_plan_topology;

typedef struct core_machine_plan core_machine_plan;

lib_status core_machine_create(
    const core_machine_config *config,
    core_machine **out_machine);

lib_status core_machine_plan_create(const core_machine_config *configuration,
    core_machine_plan **out_plan);
void core_machine_plan_destroy(core_machine_plan *plan);
lib_status core_machine_plan_set_topology(core_machine_plan *plan,
    const core_machine_plan_topology *topology);
lib_status core_machine_plan_set_controller_timing_rules(core_machine_plan *plan,
    const core_machine_controller_timing_rules *rules);
lib_status core_machine_plan_bind_media_registry(core_machine_plan *plan,
    const core_machine_media_registry *registry);
lib_status core_machine_plan_bind_display_provider(core_machine_plan *plan,
    core_machine_display_provider_slot *provider);
lib_status core_machine_plan_bind_fdc_terminal_observation(core_machine_plan *plan,
    core_machine_fdc_terminal_observation_provider provider);
lib_status core_machine_plan_configure_fdc(core_machine_plan *plan,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_fdc_config *config);
lib_status core_machine_plan_configure_hdc(core_machine_plan *plan,
    core_machine_media_id media_id, core_machine_media_id slave_media_id,
    const core_machine_hdc_config *config);
lib_status core_machine_plan_configure_d4_memory(core_machine_plan *plan,
    const core_machine_d4_memory_config *config);
lib_status core_machine_create_from_plan(const core_machine_plan *plan,
    core_machine **out_machine);
/* A serial byte received from the keyboard attached to this machine's 8042.
 * Product host adapters query the selected scan set before forming this
 * device-native stream. */
typedef enum core_machine_keyboard_scan_set {
    CORE_MACHINE_KEYBOARD_SCAN_SET_1 = 1u,
    CORE_MACHINE_KEYBOARD_SCAN_SET_2 = 2u
} core_machine_keyboard_scan_set;

lib_status core_machine_keyboard_get_native_scan_set(const core_machine *machine,
    lib_u8 *out_scan_set);
lib_status core_machine_keyboard_receive_native_byte(core_machine *machine,
    lib_u8 native_byte);
lib_status core_machine_keyboard_receive_native_bytes(core_machine *machine,
    const lib_u8 *native_bytes, lib_size count);
lib_status core_machine_set_xt_ppi_fault_input(core_machine *machine,
    core_machine_xt_ppi_fault_input input, lib_i32 asserted);
/* A relative report received from the machine's attached pointing device. */
lib_status core_machine_mouse_receive_relative(core_machine *machine,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons);

lib_status core_machine_capture_display_snapshot(const core_machine *machine,
    x86_video_snapshot *out_snapshot);
lib_status core_machine_observe_display_snapshot(const core_machine *machine,
    lib_u8 acknowledged_generation_valid,
    lib_u64 acknowledged_generation,
    x86_video_snapshot_observation *out_observation);

lib_status core_machine_configure_display(core_machine *machine,
    const core_machine_display_config *config);
lib_status core_machine_configure_dma(core_machine *machine,
    const core_machine_dma_wiring *wiring,
    core_machine_dma_request_binding *out_fdc_request);
lib_status core_machine_get_fdc_dma_request_binding(const core_machine *machine,
    core_machine_dma_request_binding *out_binding);
lib_status core_machine_configure_rtc_cmos(core_machine *machine,
    const core_machine_rtc_cmos_config *config);
lib_status core_machine_configure_planar_parity(core_machine *machine,
    const core_machine_planar_parity_config *config);
lib_status core_machine_configure_d4_platform(core_machine *machine,
    const core_machine_d4_platform_config *config);
lib_status core_machine_configure_absent_memory(core_machine *machine,
    const core_machine_absent_memory_config *config);
lib_status core_machine_report_planar_parity_fault(core_machine *machine);
lib_status core_machine_clear_d4_iochk_fault(core_machine *machine);
lib_status core_machine_report_d4_iochk_fault(core_machine *machine);
lib_status core_machine_get_planar_parity_observation(const core_machine *machine,
    core_machine_planar_parity_observation *out_observation);
lib_status core_machine_get_d4_platform_observation(const core_machine *machine,
    core_machine_d4_platform_observation *out_observation);
lib_status core_machine_get_speaker_observation(const core_machine *machine,
    core_machine_speaker_observation *out_observation);

#ifdef __cplusplus
}
#endif

#endif

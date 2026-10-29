#ifndef X86_NXVM_BOARD_INTERFACE_H
#define X86_NXVM_BOARD_INTERFACE_H

#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/timing_interface.h"
#include "x86-nxvm/board-base/trace_interface.h"
#include "x86-nxvm/board-base/controller_interface.h"
#include "x86-nxvm/board-base/display_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-nxvm/board-base/fdc_observation_interface.h"
#include "x86-nxvm/chips/pit825x/pit825x_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct x86_nxvm_board_state x86_nxvm_board_state;
#include "x86-nxvm/board-base/board_profile_interface.h"

#define X86_NXVM_BOARD_DEFAULT_MEMORY_BYTES (16u * 1024u * 1024u)
/* Stopped PC RAM replacement limits, not allocator or construction minima. */
#define X86_NXVM_BOARD_MINIMUM_MEMORY_BYTES (2u * 1024u * 1024u)
#define X86_NXVM_BOARD_MAXIMUM_MEMORY_BYTES (64u * 1024u * 1024u)

/* Construction-only lifetime transfer; rejects replacement and incomplete
 * bindings. On failure the caller still owns the Profile object. */
lib_status x86_nxvm_board_bind_profile(x86_nxvm_board_state *board,
    const x86_nxvm_board_profile_binding *binding);
lib_status x86_nxvm_board_construct_profile(x86_nxvm_board_state *board,
    x86_nxvm_board_profile_factory factory, void *context);

#define X86_NXVM_BOARD_RTC_TYPE_DISK_FLOPPY 0x10u
#define X86_NXVM_BOARD_RTC_TYPE_DISK_FIXED 0x12u
#define X86_NXVM_BOARD_RTC_EQUIPMENT 0x14u
#define X86_NXVM_BOARD_RTC_BASEMEM_LSB 0x15u
#define X86_NXVM_BOARD_RTC_BASEMEM_MSB 0x16u
#define X86_NXVM_BOARD_RTC_EXTMEM_LSB 0x17u
#define X86_NXVM_BOARD_RTC_EXTMEM_MSB 0x18u
#define X86_NXVM_BOARD_RTC_TYPE_DISK_FIXED_EXTENDED_0 0x19u
#define X86_NXVM_BOARD_PC_AT_PORT_B 0x0061u

/* Ratios are relative to x86_core_machine elapsed ticks, not host time. */
typedef struct x86_nxvm_board_clock_plan {
    x86_core_machine_clock_ratio dma;
    x86_core_machine_clock_ratio pit;
    /* An optional second PIT chip is a distinct board clock consumer. */
    x86_core_machine_clock_ratio auxiliary_pit;
    x86_core_machine_clock_ratio rtc;
    x86_core_machine_clock_ratio vadp;
    x86_core_machine_clock_ratio kbc;
    x86_core_machine_clock_ratio provider;
} x86_nxvm_board_clock_plan;

/* The product selects its keyboard controller once when it freezes the Core
 * configuration.  XT PPI is the IBM 5160 system-board attachment, not a
 * keyboard-only variant of the PC/AT 8042. */
typedef enum x86_nxvm_board_keyboard_topology {
    X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_8042 = 0,
    X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_XT_PPI = 1,
    /* The board owns no keyboard endpoint; a product receiver may attach one. */
    X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_NONE = 2,
    /* An 8042-style serial transport without PC/AT output-port wiring. */
    X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_8042_NEUTRAL = 3
} x86_nxvm_board_keyboard_topology;

/* Zero keeps the existing PC video default for all existing board inputs. */
typedef enum x86_nxvm_board_display_topology {
    X86_NXVM_BOARD_DISPLAY_TOPOLOGY_VADP = 0,
    X86_NXVM_BOARD_DISPLAY_TOPOLOGY_NONE = 1
} x86_nxvm_board_display_topology;

/* The legacy facade groups only the forced PC/AT port-92 and reset-alias
 * policy.  It does not describe an actual chipset. */
typedef enum x86_nxvm_board_system_facade {
    X86_NXVM_BOARD_SYSTEM_FACADE_PC_AT = 0,
    X86_NXVM_BOARD_SYSTEM_FACADE_NONE = 1
} x86_nxvm_board_system_facade;

#include "x86-nxvm/board-xt/xt_ppi_keyboard_interface.h"

typedef enum x86_nxvm_board_a20_wrap_policy {
    X86_NXVM_BOARD_A20_WRAP_GLOBAL_MASK = 0,
    X86_NXVM_BOARD_A20_WRAP_FIRST_TO_SECOND_MIB = 1
} x86_nxvm_board_a20_wrap_policy;

typedef struct x86_nxvm_board_config {
    lib_size memory_bytes;
    x86_core_cpu_profile cpu_profile;
    x86_fpu_profile fpu_profile;
    /* Original 80386 silicon accepts MOV CR ModR/M forms with MOD other
     * than 11b, using the r/m field as the general-register selector. */
    lib_u8 cpu_80386_cr_mov_ignores_mod;
    x86_core_cpu_protected_data_reload_policy protected_data_reload_policy;
    x86_nxvm_board_a20_wrap_policy a20_wrap_policy;
    /* Compatibility base-cost shorthand when instruction_timing.base_ticks is 0. */
    lib_u32 ticks_per_instruction;
    x86_core_cpu_instruction_timing instruction_timing;
    x86_core_machine_transaction_contract transaction_contract;
    x86_nxvm_board_clock_plan clock_plan;
    /* Frozen shared system-PIT chip selection; zero preserves 8254 users. */
    x86_pit_personality shared_pit_personality;
    x86_nxvm_board_pic_topology pic_topology;
    /* Product profiles select one or two controllers explicitly.  Zero is
     * retained only for direct Core fixture compatibility and resolves to two. */
    lib_u8 dma_controller_count;
    /* False retains ordinary PC DMA page registers at 80h--8Fh; true selects
     * a board-qualified independent scratch file for that entire range. */
    lib_bool dma_page_ports_plain_scratch;
    x86_core_machine_time_axis time_axis;
    x86_nxvm_board_pic_irq_timing pic_irq_timing;
    x86_core_machine_l1_compatibility_policy l1_compatibility_policy;
    /* Physical mode refuses an unallocated successful retirement before it can
     * be published into a clock-domain plan. */
    x86_core_machine_retirement_time_contract retirement_time_contract;
    /* Read synchronously by create and copied into Core-owned storage; the
     * caller retains no lifetime obligation after x86_nxvm_board_create(). */
    const x86_core_machine_retirement_qualification_descriptor *retirement_qualification;
    lib_u32 kbc_typematic_initial_ticks;
    lib_u32 kbc_typematic_repeat_ticks;
    lib_u32 kbc_command_response_ticks;
    /* Optional board-provided response visibility phase.  A nonzero value
     * holds a completed KBC command reply through this many status reads. */
    lib_u8 kbc_command_response_status_polls;
    lib_u32 kbc_serial_delivery_ticks;
    /* Existing KBC service-axis units; zero is an order-only pulse model. */
    lib_u32 kbc_output_pulse_ticks;
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
    /* Optional system-controller self-test status visible at reset through
     * 8042 port 64h bit 2.  It is a board electrical fact, not BIOS policy. */
    lib_u8 kbc_reset_system_flag_configured;
    lib_u8 kbc_reset_system_flag;
    /* Optional full command byte sampled immediately after 8042 reset. */
    lib_u8 kbc_reset_command_byte_configured;
    lib_u8 kbc_reset_command_byte;
    x86_nxvm_board_keyboard_topology keyboard_topology;
    x86_nxvm_board_display_topology display_topology;
    x86_nxvm_board_system_facade system_facade;
    x86_nxvm_board_xt_ppi_keyboard_config xt_ppi_keyboard;
    /* Select the existing 92h A20/reset port without the PC/AT facade.
     * The facade retains its existing decode; both use one Board owner.
     * Functional L2 behavior does not assert physical reset-pulse timing. */
    lib_bool system_control_port_present;
} x86_nxvm_board_config;

/* Construction-plan qualifications consume copied Core values. They are not
 * runtime controller setters; profile provenance remains outside Core. */
typedef enum x86_nxvm_board_controller_timing_rule {
    X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_L2_FALLBACK = 0,
    X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_RATIONAL_CLOCK,
    X86_NXVM_BOARD_CONTROLLER_TIMING_RULE_SOURCE_DMA_SERVICE_PHASES
} x86_nxvm_board_controller_timing_rule;

typedef struct x86_nxvm_board_controller_timing_rules {
    x86_nxvm_board_controller_timing_rule pic_visibility;
    x86_nxvm_board_controller_timing_rule dma_clock;
    x86_nxvm_board_controller_timing_rule dma_service;
    x86_nxvm_board_controller_timing_rule pit_clock;
    x86_nxvm_board_controller_timing_rule rtc_clock;
} x86_nxvm_board_controller_timing_rules;

#include "x86-nxvm/board-base/vadp_interface.h"

#define X86_NXVM_BOARD_RTC_DEFAULT_COUNT 6u
/* A board seed contributes the MC146818 NVRAM window 0Eh--3Fh. Calendar and
 * status registers remain RTC-chip-owned. */
#define X86_NXVM_BOARD_RTC_DEFAULT_CAPACITY 114u

/* Board composition supplies a copied RTC phase scale.  L3 means the values
 * are a direct selected-board conversion; L2 means a board ratio estimate.
 * Core consumes ticks only and never receives a host clock or callback. */
typedef enum x86_nxvm_board_rtc_timing_provenance {
    X86_NXVM_BOARD_RTC_TIMING_L2_RATIO = 0,
    X86_NXVM_BOARD_RTC_TIMING_L3_SOURCE = 1
} x86_nxvm_board_rtc_timing_provenance;

typedef struct x86_nxvm_board_rtc_timing_plan {
    lib_u32 uip_lead_ticks;
    lib_u32 update_ticks;
    x86_nxvm_board_rtc_timing_provenance provenance;
} x86_nxvm_board_rtc_timing_plan;

typedef struct x86_nxvm_board_rtc_default_byte {
    lib_u8 index;
    lib_u8 value;
} x86_nxvm_board_rtc_default_byte;

typedef struct x86_nxvm_board_rtc_cmos_config {
    lib_u16 index_port;
    lib_u16 data_port;
    lib_u8 irq;
    lib_u8 nmi_mask_bit;
    lib_u32 ticks_per_second;
    x86_nxvm_board_rtc_timing_plan timing;
    x86_nxvm_board_rtc_default_byte defaults[X86_NXVM_BOARD_RTC_DEFAULT_CAPACITY];
    lib_size default_count;
    /* Unit-only synthetic board defaults may ask Board to derive the AT
     * configuration checksum.  A session-provided board seed owns its
     * complete NVRAM image, including 2Eh/2Fh, and clears this flag. */
    lib_u8 derive_configuration_checksum;
    /* False preserves the classic six-bit CMOS address decode. */
    lib_bool extended_registers;
} x86_nxvm_board_rtc_cmos_config;

#include "x86-nxvm/board-at/parity_interface.h"




/* Copied logical speaker-line state. Board owns speaker wiring and PIT sampling;
 * host audio is a separate, optional consumer. */
typedef struct x86_nxvm_board_speaker_observation {
    lib_i32 configured;
    lib_i32 timer_gate;
    lib_i32 data_enabled;
    lib_i32 timer_output;
    lib_i32 output;
} x86_nxvm_board_speaker_observation;

/* A profile-selected, bounded unpopulated memory window. Reads return the
 * declared fallback byte and writes are deliberately discarded; it never adds
 * installed RAM. This is for board models, not a generic memory default. */
typedef struct x86_nxvm_board_absent_memory_config {
    lib_u32 physical_start;
    lib_size bytes;
    lib_u8 read_value;
} x86_nxvm_board_absent_memory_config;

#define X86_NXVM_BOARD_ABSENT_MEMORY_WINDOW_COUNT 4u

#define X86_NXVM_BOARD_MEMORY_ALIAS_COUNT 4u

#define X86_NXVM_BOARD_DMA_CONTROLLER_COUNT 2u
#define X86_NXVM_BOARD_DMA_CASCADE_CHANNEL 4u
#define X86_NXVM_BOARD_DMA_FDC_CHANNEL_UNBOUND 0xffu

/* The plan selects the controller/refresh topology independently of an FDC.
 * When present, fdc_channel selects the Board DMA bus's FDC request binding; the
 * unbound value leaves that later device route absent. */
typedef struct x86_nxvm_board_dma_wiring {
    lib_u8 fdc_channel;
    lib_u8 controller_count;
    lib_u8 cascade_channel;
} x86_nxvm_board_dma_wiring;

/* Every optional topology is copied into the Board-owned plan before machine
 * creation. Runtime endpoints are registered separately and never enter this
 * public declaration. */
typedef struct x86_nxvm_board_plan_topology {
    lib_u8 absent_memory_count;
    x86_nxvm_board_absent_memory_config absent_memory[X86_NXVM_BOARD_ABSENT_MEMORY_WINDOW_COUNT];
    lib_u8 memory_alias_count;
    x86_core_machine_memory_alias_config memory_alias[X86_NXVM_BOARD_MEMORY_ALIAS_COUNT];
    lib_u8 planar_parity_present;
    x86_nxvm_board_planar_parity_config planar_parity;
    lib_u8 display_present;
    x86_nxvm_board_display_config display;
    lib_u8 dma_present;
    x86_nxvm_board_dma_wiring dma;
    lib_u8 rtc_cmos_present;
    x86_nxvm_board_rtc_cmos_config rtc_cmos;
    lib_u8 fdc_present;
    x86_nxvm_board_fdc_drive_bindings fdc_drives;
    x86_nxvm_board_fdc_config fdc;
    lib_u8 hdc_present;
    x86_nxvm_board_media_id hdc_media_id;
    x86_nxvm_board_media_id hdc_slave_media_id;
    x86_nxvm_board_hdc_config hdc;
} x86_nxvm_board_plan_topology;

typedef struct x86_nxvm_board_plan x86_nxvm_board_plan;

/* Optional board output is borrowed until x86_core_machine_destroy. Supplied
 * outputs clear on failure; construction has one allocation/cleanup owner. */
lib_status x86_nxvm_board_create(
    const x86_nxvm_board_config *config,
    x86_core_machine **out_machine, x86_nxvm_board_state **out_board);

lib_status x86_nxvm_board_observe_a20(const x86_nxvm_board_state *board,
    lib_bool *out_enabled);

lib_status x86_nxvm_board_plan_create(const x86_nxvm_board_config *configuration,
    x86_nxvm_board_plan **out_plan);
void x86_nxvm_board_plan_destroy(x86_nxvm_board_plan *plan);
lib_status x86_nxvm_board_plan_set_topology(x86_nxvm_board_plan *plan,
    const x86_nxvm_board_plan_topology *topology);
lib_status x86_nxvm_board_plan_set_controller_timing_rules(x86_nxvm_board_plan *plan,
    const x86_nxvm_board_controller_timing_rules *rules);
lib_status x86_nxvm_board_plan_bind_media_registry(x86_nxvm_board_plan *plan,
    const x86_nxvm_board_media_registry *registry);
lib_status x86_nxvm_board_plan_bind_display_provider(x86_nxvm_board_plan *plan,
    x86_nxvm_board_display_provider_slot *provider);
lib_status x86_nxvm_board_plan_bind_fdc_terminal_observation(x86_nxvm_board_plan *plan,
    x86_nxvm_board_fdc_terminal_observation_provider provider);
lib_status x86_nxvm_board_plan_configure_fdc(x86_nxvm_board_plan *plan,
    const x86_nxvm_board_fdc_drive_bindings *drives,
    const x86_nxvm_board_fdc_config *config);
lib_status x86_nxvm_board_plan_configure_hdc(x86_nxvm_board_plan *plan,
    x86_nxvm_board_media_id media_id, x86_nxvm_board_media_id slave_media_id,
    const x86_nxvm_board_hdc_config *config);
/* Context is borrowed until create_from_plan returns, not kept by the board.
 * A construction output in that context is not usable before full success. */
lib_status x86_nxvm_board_plan_bind_profile_factory(x86_nxvm_board_plan *plan,
    x86_nxvm_board_profile_factory factory, void *context);
/* Both outputs publish only after complete construction. Board is borrowed:
 * x86_core_machine_destroy releases it through the installed attachment. */
lib_status x86_nxvm_board_create_from_plan(const x86_nxvm_board_plan *plan,
    x86_core_machine **out_machine, x86_nxvm_board_state **out_board);
/* A serial byte received from the keyboard attached to this machine's 8042.
 * Product host adapters query the selected scan set before forming this
 * device-native stream. */
typedef enum x86_nxvm_board_keyboard_scan_set {
    X86_NXVM_BOARD_KEYBOARD_SCAN_SET_1 = 1u,
    X86_NXVM_BOARD_KEYBOARD_SCAN_SET_2 = 2u,
    X86_NXVM_BOARD_KEYBOARD_SCAN_SET_3 = 3u
} x86_nxvm_board_keyboard_scan_set;

lib_status x86_nxvm_board_keyboard_get_native_scan_set(const x86_nxvm_board_state *board,
    lib_u8 *out_scan_set);
lib_status x86_nxvm_board_keyboard_receive_native_byte(x86_nxvm_board_state *board,
    lib_u8 native_byte);
lib_status x86_nxvm_board_keyboard_receive_native_bytes(x86_nxvm_board_state *board,
    const lib_u8 *native_bytes, lib_size count);
lib_status x86_nxvm_board_set_xt_ppi_fault_input(x86_nxvm_board_state *board,
    x86_nxvm_board_xt_ppi_fault_input input, lib_i32 asserted);
/* A relative report received from the machine's attached pointing device. */
lib_status x86_nxvm_board_mouse_receive_relative(x86_nxvm_board_state *board,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons);

lib_status x86_nxvm_board_capture_display_snapshot(const x86_nxvm_board_state *board,
    x86_video_snapshot *out_snapshot);
lib_status x86_nxvm_board_observe_display_snapshot(const x86_nxvm_board_state *board,
    lib_u8 acknowledged_generation_valid,
    lib_u64 acknowledged_generation,
    x86_video_snapshot_observation *out_observation);

lib_status x86_nxvm_board_configure_display(x86_nxvm_board_state *board,
    const x86_nxvm_board_display_config *config);
lib_status x86_nxvm_board_configure_dma(x86_nxvm_board_state *board,
    const x86_nxvm_board_dma_wiring *wiring,
    x86_nxvm_board_dma_request_binding *out_fdc_request);
lib_status x86_nxvm_board_get_fdc_dma_request_binding(const x86_nxvm_board_state *board,
    x86_nxvm_board_dma_request_binding *out_binding);
lib_status x86_nxvm_board_configure_rtc_cmos(x86_nxvm_board_state *board,
    const x86_nxvm_board_rtc_cmos_config *config);
/* Capture CMOS RAM 0Eh onward after the machine executor is quiescent.
 * The caller supplies exactly 50 or 114 bytes for the configured RTC. */
lib_status x86_nxvm_board_capture_cmos(x86_nxvm_board_state *board,
    lib_u8 *bytes, lib_size byte_count);
/* Restore the same configuration range at the caller-owned quiescent boundary.
 * An invalid size is rejected before any register is changed. */
lib_status x86_nxvm_board_restore_cmos(x86_nxvm_board_state *board,
    const lib_u8 *bytes, lib_size byte_count);
/* Caller owns the quiescent boundary. Register C is never read or restored. */
lib_status x86_nxvm_board_capture_rtc(x86_nxvm_board_state *board, lib_u8 *bytes,
    lib_size byte_count);
lib_status x86_nxvm_board_restore_rtc(x86_nxvm_board_state *board, const lib_u8 *bytes,
    lib_size byte_count);
lib_status x86_nxvm_board_configure_planar_parity(x86_nxvm_board_state *board,
    const x86_nxvm_board_planar_parity_config *config);
lib_status x86_nxvm_board_configure_absent_memory(x86_nxvm_board_state *board,
    const x86_nxvm_board_absent_memory_config *config);
lib_status x86_nxvm_board_report_planar_parity_fault(x86_nxvm_board_state *board);
lib_status x86_nxvm_board_get_planar_parity_observation(const x86_nxvm_board_state *board,
    x86_nxvm_board_planar_parity_observation *out_observation);
lib_status x86_nxvm_board_get_speaker_observation(const x86_nxvm_board_state *board,
    x86_nxvm_board_speaker_observation *out_observation);

#ifdef __cplusplus
}
#endif

#endif

#ifndef X86_NXVM_BOARD_PC_AT_PROFILE_INTERFACE_H
#define X86_NXVM_BOARD_PC_AT_PROFILE_INTERFACE_H
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-at/wiring_interface.h"
#include "x86-nxvm/board-base/at_contract_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/vadp_interface.h"
#include "x86-nxvm/board-base/profile_contract_interface.h"

typedef struct x86_nxvm_board_pc_at_rom_mapping {
    lib_u32 linear_start;
    lib_u32 physical_start;
    lib_size bytes;
    lib_u16 reset_segment;
    lib_u16 reset_offset;
} x86_nxvm_board_pc_at_rom_mapping;

typedef enum x86_nxvm_board_pc_at_firmware_slot {
    X86_NXVM_BOARD_PC_AT_FIRMWARE_SLOT_GENERIC,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT
} x86_nxvm_board_pc_at_firmware_slot;

typedef struct x86_nxvm_board_pc_at_cmos_defaults {
    lib_u8 equipment;
    lib_u16 base_memory_kib;
    lib_u8 floppy_type;
    lib_u8 fixed_disk_type;
    lib_u8 fixed_disk_type_extended_0;
    lib_u8 boot_floppy_drive;
    lib_u8 boot_hard_disk_drive;
} x86_nxvm_board_pc_at_cmos_defaults;

typedef enum x86_nxvm_board_pc_at_firmware_hook {
    X86_NXVM_BOARD_PC_AT_FIRMWARE_CMOS_POST,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_TIMER_IRQ0,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_TIMER_INT1A,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_KEYBOARD_IRQ1,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_KEYBOARD_INT16,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_DMA_POST,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_FDC_POST,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_FDC_IRQ6,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_FDC_INT13,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_FDC_INT40,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_HDC_INT13,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_PIT_POST,
    X86_NXVM_BOARD_PC_AT_FIRMWARE_PIC_POST
} x86_nxvm_board_pc_at_firmware_hook;

typedef struct x86_nxvm_board_pc_at_firmware_service {
    x86_nxvm_board_pc_at_firmware_hook hook;
    lib_u8 vector;
} x86_nxvm_board_pc_at_firmware_service;

typedef struct x86_nxvm_board_pc_at_cpu_contract {
    x86_core_cpu_profile cpu_profile;
    x86_fpu_profile fpu_profile;
    lib_u32 ticks_per_instruction;
    x86_core_cpu_instruction_timing instruction_timing;
    x86_core_machine_transaction_contract transaction_contract;
    x86_nxvm_board_clock_plan clock_plan;
    x86_core_machine_time_axis time_axis;
    x86_nxvm_board_controller_timing_rules controller_timing_rules;
    x86_nxvm_board_pic_irq_timing pic_irq_timing;
    lib_u32 kbc_typematic_initial_ticks;
    lib_u32 kbc_typematic_repeat_ticks;
    lib_u32 kbc_command_response_ticks;
    lib_u8 kbc_command_response_status_polls;
} x86_nxvm_board_pc_at_cpu_contract;

typedef struct x86_nxvm_board_pc_at_descriptor {
    const char *identity;
    lib_u32 compatibility_revision;
    x86_core_cpu_profile cpu_profile;
    x86_fpu_profile fpu_profile;
    lib_u32 ticks_per_instruction;
    x86_core_cpu_instruction_timing instruction_timing;
    x86_core_machine_transaction_contract transaction_contract;
    x86_nxvm_board_clock_plan clock_plan;
    x86_core_machine_time_axis time_axis;
    x86_nxvm_board_controller_timing_rules controller_timing_rules;
    x86_nxvm_board_pic_irq_timing pic_irq_timing;
    lib_u32 kbc_typematic_initial_ticks;
    lib_u32 kbc_typematic_repeat_ticks;
    lib_u32 kbc_command_response_ticks;
    lib_u8 kbc_command_response_status_polls;
    lib_u8 kbc_reset_output_port_configured;
    lib_u8 kbc_reset_output_port;
    /* 8042 C0h is a frozen system-board jumper input, not keyboard state. */
    lib_u8 kbc_input_port_configured;
    lib_u8 kbc_input_port;
    lib_u32 rtc_ticks_per_second;
    x86_video_text_timing cga_text_timing;
    x86_video_ega_sequencer_config ega_sequencer;
    x86_video_ega_controller_config ega_controllers;
    lib_size default_memory_bytes;
    lib_u8 unpopulated_extended_memory;
    /* 8237A boundary-transfer workspace, expressed as a real-mode segment. */
    lib_u16 fdc_bounce_segment;
    /* Frozen board READY input per unit select, independent of fitted drives. */
    lib_u8 fdc_ready_mask;
    lib_u8 hdc_present;
    lib_u8 planar_parity_present;
    /* Port 61h bit 4 is a frozen board input.  It is independent of PIT1's
     * refresh request wiring, which remains owned by Board. */
    x86_nxvm_board_planar_parity_refresh_status_source refresh_status_source;
    lib_u32 refresh_status_toggle_ticks;
    lib_u8 ega_present;
    lib_u8 cga_vram_present;
    lib_u8 monochrome_aperture_absent;
    x86_nxvm_board_pc_at_firmware_slot firmware_slot;
    lib_u8 diskette_drive_a_field_upgrade;
    x86_nxvm_board_pc_at_rom_mapping rom;
    x86_nxvm_board_pc_at_cmos_defaults cmos;
    const x86_nxvm_board_at_port_leaf *port_leaves;
    lib_size port_leaf_count;
    const x86_nxvm_board_at_route *routes;
    lib_size route_count;
    x86_nxvm_board_hdc_config hdc;
    const x86_nxvm_board_pc_at_firmware_service *firmware_services;
    lib_size firmware_service_count;
    /* Immutable mechanics and board inputs for the FDC's physical units.
     * Mounted media is deliberately supplied later by the session provider. */
    lib_u8 fdc_installed_mask;
    lib_u8 fdc_double_sided_mask;
    lib_u16 fdc_cylinder_count[X86_NXVM_BOARD_FDC_DRIVE_COUNT];
    lib_u8 fdc_track_zero_active_low_mask;
    lib_u16 fdc_diagnostic_port;
    lib_u8 fdc_diagnostic_read_value;
    lib_bool kbc_aux_absent;
    /* Model constraints stay with the immutable App definition. */
    lib_i32 (*validate)(const struct x86_nxvm_board_pc_at_descriptor *descriptor);
    lib_u32 kbc_output_pulse_ticks;
} x86_nxvm_board_pc_at_descriptor;

#define X86_NXVM_BOARD_PC_AT_PLAN_PORT_LEAF_CAPACITY 96u
#define X86_NXVM_BOARD_PC_AT_PLAN_ROUTE_CAPACITY 8u
#define X86_NXVM_BOARD_PC_AT_PLAN_FIRMWARE_SERVICE_CAPACITY 16u

/* In-place immutable result: its descriptor points only at its own copied
 * arrays, so it can become the later session input without static aliases. */
typedef struct x86_nxvm_board_pc_at_plan_snapshot {
    x86_nxvm_board_contract_values values;
    x86_nxvm_board_pc_at_descriptor descriptor;
    x86_nxvm_board_plan_topology topology;
    x86_nxvm_board_at_port_leaf
        port_leaves[X86_NXVM_BOARD_PC_AT_PLAN_PORT_LEAF_CAPACITY];
    x86_nxvm_board_at_route routes[X86_NXVM_BOARD_PC_AT_PLAN_ROUTE_CAPACITY];
    x86_nxvm_board_pc_at_firmware_service
        firmware_services[X86_NXVM_BOARD_PC_AT_PLAN_FIRMWARE_SERVICE_CAPACITY];
} x86_nxvm_board_pc_at_plan_snapshot;

lib_i32 x86_nxvm_board_pc_at_cpu_profile_is_valid(
    x86_core_cpu_profile profile);
lib_i32 x86_nxvm_board_pc_at_fpu_profile_is_valid(
    x86_fpu_profile profile);
lib_i32 x86_nxvm_board_pc_at_cpu_contract_select(
    const x86_nxvm_board_pc_at_descriptor *descriptor,
    x86_core_cpu_profile requested_cpu,
    x86_fpu_profile requested_fpu,
    x86_nxvm_board_pc_at_cpu_contract *out_contract);
lib_i32 x86_nxvm_board_pc_at_core_config_materialize(
    const x86_nxvm_board_pc_at_descriptor *descriptor,
    const x86_nxvm_board_pc_at_cpu_contract *contract,
    x86_nxvm_board_config *out_config,
    x86_nxvm_board_controller_timing_rules *out_timing_rules);
lib_status x86_nxvm_board_pc_at_topology_materialize(
    const x86_nxvm_board_pc_at_descriptor *descriptor,
    const x86_nxvm_board_controller_timing_rules *timing_rules,
    x86_nxvm_board_plan_topology *out_topology);
lib_status x86_nxvm_board_pc_at_values_create(
    const x86_nxvm_board_pc_at_descriptor *descriptor,
    x86_core_cpu_profile cpu_profile, x86_fpu_profile fpu_profile,
    x86_nxvm_board_contract_values *out_values);
lib_status x86_nxvm_board_pc_at_snapshot_copy(
    x86_nxvm_board_pc_at_plan_snapshot *out_profile,
    const x86_nxvm_board_pc_at_descriptor *descriptor, const char *identity,
    lib_u8 floppy_cmos_type);
lib_u32 x86_nxvm_board_pc_at_enabled_devices(
    const x86_nxvm_board_pc_at_descriptor *descriptor);
lib_i32 x86_nxvm_board_pc_at_descriptor_is_valid(
    const x86_nxvm_board_pc_at_descriptor *descriptor);
extern const x86_nxvm_board_pc_at_firmware_service x86_nxvm_board_pc_at_firmware_services[13u];

#endif

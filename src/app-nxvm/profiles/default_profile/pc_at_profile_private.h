#ifndef VM_PROFILE_DEFAULT_PC_AT_PROFILE_PRIVATE_H
#define VM_PROFILE_DEFAULT_PC_AT_PROFILE_PRIVATE_H
#include "lib/types/types_interface.h"


#include "app-nxvm/devices/cpu_interface.h"
#include "app-nxvm/devices/fpu_interface.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/vadp.h"
#include "app-nxvm/profiles/profile_contract_interface.h"

typedef enum vm_profile_default_pc_at_device_role {
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_KBC,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_MEMORY_CONTROL,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_BOARD
} vm_profile_default_pc_at_device_role;

#define VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL 0xffu

typedef struct vm_profile_default_pc_at_port_leaf {
    vm_profile_default_pc_at_device_role device;
    lib_u16 port;
    lib_u8 read;
    lib_u8 write;
} vm_profile_default_pc_at_port_leaf;

typedef enum vm_profile_default_pc_at_route_source {
    VM_PROFILE_DEFAULT_PC_AT_ROUTE_PIT_IRQ0,
    VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_KEYBOARD_IRQ1,
    VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_AUX_IRQ12,
    VM_PROFILE_DEFAULT_PC_AT_ROUTE_CMOS_IRQ8,
    VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2
} vm_profile_default_pc_at_route_source;

typedef struct vm_profile_default_pc_at_route {
    vm_profile_default_pc_at_route_source source;
    lib_u8 irq;
    lib_u8 dma_channel;
} vm_profile_default_pc_at_route;

typedef struct vm_profile_default_pc_at_rom_mapping {
    lib_u32 linear_start;
    lib_u32 physical_start;
    lib_size bytes;
    lib_u16 reset_segment;
    lib_u16 reset_offset;
} vm_profile_default_pc_at_rom_mapping;

typedef enum vm_profile_default_pc_at_firmware_slot {
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_GENERIC,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT
} vm_profile_default_pc_at_firmware_slot;

typedef struct vm_profile_default_pc_at_cmos_defaults {
    lib_u8 equipment;
    lib_u16 base_memory_kib;
    lib_u8 floppy_type;
    lib_u8 fixed_disk_type;
    lib_u8 fixed_disk_type_extended_0;
    lib_u8 boot_floppy_drive;
    lib_u8 boot_hard_disk_drive;
} vm_profile_default_pc_at_cmos_defaults;

typedef enum vm_profile_default_pc_at_firmware_hook {
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_CMOS_POST,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_TIMER_IRQ0,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_TIMER_INT1A,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_IRQ1,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_INT16,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_DMA_POST,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_POST,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_IRQ6,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_INT13,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_INT40,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_HDC_INT13,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIT_POST,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIC_POST
} vm_profile_default_pc_at_firmware_hook;

typedef struct vm_profile_default_pc_at_firmware_service {
    vm_profile_default_pc_at_firmware_hook hook;
    lib_u8 vector;
} vm_profile_default_pc_at_firmware_service;

typedef struct vm_profile_default_pc_at_cpu_contract {
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    lib_u32 ticks_per_instruction;
    core_machine_instruction_timing instruction_timing;
    core_machine_transaction_contract transaction_contract;
    core_machine_clock_plan clock_plan;
    core_machine_time_axis time_axis;
    core_machine_controller_timing_rules controller_timing_rules;
    core_machine_pic_irq_timing pic_irq_timing;
    lib_u32 kbc_typematic_initial_ticks;
    lib_u32 kbc_typematic_repeat_ticks;
    lib_u32 kbc_command_response_ticks;
    lib_u8 kbc_command_response_status_polls;
} vm_profile_default_pc_at_cpu_contract;

typedef struct vm_profile_default_pc_at_descriptor {
    const char *identity;
    lib_u32 compatibility_revision;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    lib_u32 ticks_per_instruction;
    core_machine_instruction_timing instruction_timing;
    core_machine_transaction_contract transaction_contract;
    core_machine_clock_plan clock_plan;
    core_machine_time_axis time_axis;
    core_machine_controller_timing_rules controller_timing_rules;
    core_machine_pic_irq_timing pic_irq_timing;
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
    core_machine_vadp_text_timing cga_text_timing;
    core_machine_vadp_ega_sequencer_config ega_sequencer;
    core_machine_vadp_ega_controller_config ega_controllers;
    lib_size default_memory_bytes;
    lib_u8 unpopulated_extended_memory;
    /* 8237A boundary-transfer workspace, expressed as a real-mode segment. */
    lib_u16 fdc_bounce_segment;
    /* Frozen board READY inputs, one bit per physically fitted FDC drive. */
    lib_u8 fdc_ready_mask;
    lib_u8 hdc_present;
    lib_u8 planar_parity_present;
    /* Port 61h bit 4 is a frozen board input.  It is independent of PIT1's
     * DMA-refresh request path, which remains owned by Core. */
    core_machine_planar_parity_refresh_status_source refresh_status_source;
    lib_u32 refresh_status_toggle_ticks;
    lib_u8 ega_present;
    lib_u8 cga_vram_present;
    lib_u8 monochrome_aperture_absent;
    vm_profile_default_pc_at_firmware_slot firmware_slot;
    lib_u8 diskette_drive_a_field_upgrade;
    vm_profile_default_pc_at_rom_mapping rom;
    vm_profile_default_pc_at_cmos_defaults cmos;
    const vm_profile_default_pc_at_port_leaf *port_leaves;
    lib_size port_leaf_count;
    const vm_profile_default_pc_at_route *routes;
    lib_size route_count;
    core_machine_hdc_config hdc;
    const vm_profile_default_pc_at_firmware_service *firmware_services;
    lib_size firmware_service_count;
    /* Immutable mechanics and board inputs for the FDC's physical units.
     * Mounted media is deliberately supplied later by the session provider. */
    lib_u8 fdc_installed_mask;
    lib_u8 fdc_double_sided_mask;
    lib_u16 fdc_cylinder_count[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_u8 fdc_track_zero_active_low_mask;
    lib_u16 fdc_diagnostic_port;
    lib_u8 fdc_diagnostic_read_value;
} vm_profile_default_pc_at_descriptor;

#define VM_PROFILE_DEFAULT_PC_AT_PLAN_PORT_LEAF_CAPACITY 96u
#define VM_PROFILE_DEFAULT_PC_AT_PLAN_ROUTE_CAPACITY 8u
#define VM_PROFILE_DEFAULT_PC_AT_PLAN_FIRMWARE_SERVICE_CAPACITY 16u

/* In-place immutable result: its descriptor points only at its own copied
 * arrays, so it can become the later session input without static aliases. */
typedef struct vm_profile_default_pc_at_plan_snapshot {
    vm_profile_contract_values values;
    vm_profile_default_pc_at_descriptor descriptor;
    core_machine_plan_topology topology;
    vm_profile_default_pc_at_port_leaf
        port_leaves[VM_PROFILE_DEFAULT_PC_AT_PLAN_PORT_LEAF_CAPACITY];
    vm_profile_default_pc_at_route routes[VM_PROFILE_DEFAULT_PC_AT_PLAN_ROUTE_CAPACITY];
    vm_profile_default_pc_at_firmware_service
        firmware_services[VM_PROFILE_DEFAULT_PC_AT_PLAN_FIRMWARE_SERVICE_CAPACITY];
} vm_profile_default_pc_at_plan_snapshot;

#define VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU 0x01u
#define VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY 0x02u
#define VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY 0x04u

typedef struct vm_profile_default_at_request {
    lib_u32 requested_options;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    lib_size memory_bytes;
    lib_u8 floppy_cmos_type;
} vm_profile_default_at_request;

const vm_profile_default_pc_at_descriptor *
vm_profile_default_pc_at_descriptor_get(void);
const vm_profile_default_pc_at_descriptor *
vm_profile_ibm_5170_model_339_descriptor_get(void);
lib_i32 vm_profile_default_pc_at_cpu_contract_select(
    const vm_profile_default_pc_at_descriptor *descriptor,
    core_machine_cpu_profile requested_cpu,
    core_machine_fpu_profile requested_fpu,
    vm_profile_default_pc_at_cpu_contract *out_contract);
lib_i32 vm_profile_default_pc_at_core_config_materialize(
    const vm_profile_default_pc_at_descriptor *descriptor,
    const vm_profile_default_pc_at_cpu_contract *contract,
    core_machine_config *out_config,
    core_machine_controller_timing_rules *out_timing_rules);
lib_status vm_profile_default_pc_at_topology_materialize(
    const vm_profile_default_pc_at_descriptor *descriptor,
    const core_machine_controller_timing_rules *timing_rules,
    core_machine_plan_topology *out_topology);
lib_status vm_profile_ibm_5170_values_create(lib_size memory_bytes,
    vm_profile_contract_values *out_values);
lib_status vm_profile_ibm_5170_plan_create(
    vm_profile_default_pc_at_plan_snapshot *out_profile);
/* IBM's 128 KiB conventional-memory option and 512 KiB extended-memory
 * options are selected only while the frozen 5170 profile is constructed. */
lib_status vm_profile_ibm_5170_plan_create_memory(lib_size memory_bytes,
    vm_profile_default_pc_at_plan_snapshot *out_profile);
lib_status vm_profile_default_at_plan_create(
    const vm_profile_default_at_request *request,
    vm_profile_default_pc_at_plan_snapshot *out_profile);
const vm_profile_default_pc_at_port_leaf *
vm_profile_default_pc_at_port_leaf_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device, lib_u16 port);
const vm_profile_default_pc_at_port_leaf *
vm_profile_default_pc_at_port_leaf_at(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device, lib_size ordinal);
const vm_profile_default_pc_at_route *vm_profile_default_pc_at_route_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_route_source source);
lib_i32 vm_profile_default_pc_at_descriptor_is_valid(
    const vm_profile_default_pc_at_descriptor *descriptor);

#endif

#ifndef VM_PROFILE_DEFAULT_PC_AT_PROFILE_PRIVATE_H
#define VM_PROFILE_DEFAULT_PC_AT_PROFILE_PRIVATE_H

#include "type.h"

#include "core/machine/cpu_interface.h"
#include "core/machine/fpu_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/vadp.h"
#include "vm/profile/profile_resolver_interface.h"

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
    type_unsigned_16 port;
    type_bool read;
    type_bool write;
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
    type_unsigned_8 irq;
    type_unsigned_8 dma_channel;
} vm_profile_default_pc_at_route;

typedef struct vm_profile_default_pc_at_rom_mapping {
    type_unsigned_32 linear_start;
    type_unsigned_32 physical_start;
    STD_SIZE_T bytes;
    type_unsigned_16 reset_segment;
    type_unsigned_16 reset_offset;
} vm_profile_default_pc_at_rom_mapping;

typedef enum vm_profile_default_pc_at_firmware_slot {
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_GENERIC,
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_SLOT_IBM_5170_REV3_ABSTRACT
} vm_profile_default_pc_at_firmware_slot;

typedef struct vm_profile_default_pc_at_cmos_defaults {
    type_unsigned_8 equipment;
    type_unsigned_16 base_memory_kib;
    type_unsigned_8 floppy_type;
    type_unsigned_8 fixed_disk_type;
    type_unsigned_8 fixed_disk_type_extended_0;
    type_unsigned_8 boot_floppy_drive;
    type_unsigned_8 boot_hard_disk_drive;
} vm_profile_default_pc_at_cmos_defaults;

typedef enum vm_profile_default_pc_at_firmware_hook {
    VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10,
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
    type_unsigned_8 vector;
} vm_profile_default_pc_at_firmware_service;

typedef struct vm_profile_default_pc_at_cpu_contract {
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    type_unsigned_32 ticks_per_instruction;
    core_machine_instruction_timing instruction_timing;
    core_machine_transaction_contract transaction_contract;
    core_machine_clock_plan clock_plan;
    core_machine_time_axis time_axis;
    core_machine_controller_timing_rules controller_timing_rules;
    type_unsigned_32 kbc_typematic_initial_ticks;
    type_unsigned_32 kbc_typematic_repeat_ticks;
    type_unsigned_32 kbc_command_response_ticks;
} vm_profile_default_pc_at_cpu_contract;

typedef struct vm_profile_default_pc_at_descriptor {
    const C_CHAR *identity;
    type_unsigned_32 compatibility_revision;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    type_unsigned_32 ticks_per_instruction;
    core_machine_instruction_timing instruction_timing;
    core_machine_transaction_contract transaction_contract;
    core_machine_clock_plan clock_plan;
    core_machine_time_axis time_axis;
    core_machine_controller_timing_rules controller_timing_rules;
    type_unsigned_32 kbc_typematic_initial_ticks;
    type_unsigned_32 kbc_typematic_repeat_ticks;
    type_unsigned_32 kbc_command_response_ticks;
    type_unsigned_32 rtc_ticks_per_second;
    core_machine_vadp_text_timing cga_text_timing;
    core_machine_vadp_ega_sequencer_config ega_sequencer;
    core_machine_vadp_ega_controller_config ega_controllers;
    STD_SIZE_T default_memory_bytes;
    type_bool unpopulated_extended_memory;
    /* 8237A boundary-transfer workspace, expressed as a real-mode segment. */
    type_unsigned_16 fdc_bounce_segment;
    type_bool hdc_present;
    type_bool planar_parity_present;
    type_bool ega_present;
    type_bool cga_vram_present;
    type_bool monochrome_aperture_absent;
    vm_profile_default_pc_at_firmware_slot firmware_slot;
    type_bool diskette_drive_a_field_upgrade;
    vm_profile_default_pc_at_rom_mapping rom;
    vm_profile_default_pc_at_cmos_defaults cmos;
    const vm_profile_default_pc_at_port_leaf *port_leaves;
    STD_SIZE_T port_leaf_count;
    const vm_profile_default_pc_at_route *routes;
    STD_SIZE_T route_count;
    core_machine_hdc_config hdc;
    const vm_profile_default_pc_at_firmware_service *firmware_services;
    STD_SIZE_T firmware_service_count;
} vm_profile_default_pc_at_descriptor;

#define VM_PROFILE_DEFAULT_PC_AT_RESOLVED_PORT_LEAF_CAPACITY 96u
#define VM_PROFILE_DEFAULT_PC_AT_RESOLVED_ROUTE_CAPACITY 8u
#define VM_PROFILE_DEFAULT_PC_AT_RESOLVED_FIRMWARE_SERVICE_CAPACITY 16u

/* In-place immutable result: its descriptor points only at its own copied
 * arrays, so it can become the later session input without static aliases. */
typedef struct vm_profile_default_pc_at_resolved_profile {
    vm_resolved_profile resolved;
    vm_profile_default_pc_at_descriptor descriptor;
    core_machine_plan_topology topology;
    vm_profile_default_pc_at_port_leaf
        port_leaves[VM_PROFILE_DEFAULT_PC_AT_RESOLVED_PORT_LEAF_CAPACITY];
    vm_profile_default_pc_at_route routes[VM_PROFILE_DEFAULT_PC_AT_RESOLVED_ROUTE_CAPACITY];
    vm_profile_default_pc_at_firmware_service
        firmware_services[VM_PROFILE_DEFAULT_PC_AT_RESOLVED_FIRMWARE_SERVICE_CAPACITY];
} vm_profile_default_pc_at_resolved_profile;

#define VM_PROFILE_DEFAULT_AT_SESSION_OPTION_CPU_FPU 0x01u
#define VM_PROFILE_DEFAULT_AT_SESSION_OPTION_MEMORY 0x02u
#define VM_PROFILE_DEFAULT_AT_SESSION_OPTION_FLOPPY 0x04u

typedef struct vm_profile_default_at_request {
    type_unsigned_32 requested_options;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    STD_SIZE_T memory_bytes;
    type_unsigned_8 floppy_cmos_type;
} vm_profile_default_at_request;

const vm_profile_default_pc_at_descriptor *
vm_profile_default_pc_at_descriptor_get(C_VOID);
const vm_profile_default_pc_at_descriptor *
vm_profile_ibm_5170_model_339_descriptor_get(C_VOID);
C_INT vm_profile_default_pc_at_cpu_contract_select(
    const vm_profile_default_pc_at_descriptor *descriptor,
    core_machine_cpu_profile requested_cpu,
    core_machine_fpu_profile requested_fpu,
    vm_profile_default_pc_at_cpu_contract *out_contract);
C_INT vm_profile_default_pc_at_core_config_materialize(
    const vm_profile_default_pc_at_descriptor *descriptor,
    const vm_profile_default_pc_at_cpu_contract *contract,
    core_machine_config *out_config,
    core_machine_controller_timing_rules *out_timing_rules);
type_status vm_profile_default_pc_at_topology_materialize(
    const vm_profile_default_pc_at_descriptor *descriptor,
    const core_machine_controller_timing_rules *timing_rules,
    core_machine_plan_topology *out_topology);
type_status vm_profile_ibm_5170_root_declaration_create(
    vm_profile_resolver_declaration *out_declaration);
type_status vm_profile_ibm_5170_root_resolve(
    vm_profile_default_pc_at_resolved_profile *out_profile);
type_status vm_profile_default_at_child_declaration_create(
    const vm_profile_resolver_declaration *parent,
    const vm_profile_default_at_request *request,
    vm_profile_resolver_declaration *out_declaration);
type_status vm_profile_default_at_child_resolve(
    const vm_profile_default_at_request *request,
    vm_profile_default_pc_at_resolved_profile *out_profile);
const vm_profile_default_pc_at_port_leaf *
vm_profile_default_pc_at_port_leaf_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device, type_unsigned_16 port);
const vm_profile_default_pc_at_port_leaf *
vm_profile_default_pc_at_port_leaf_at(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device, STD_SIZE_T ordinal);
const vm_profile_default_pc_at_route *vm_profile_default_pc_at_route_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_route_source source);
C_INT vm_profile_default_pc_at_descriptor_is_valid(
    const vm_profile_default_pc_at_descriptor *descriptor);

#endif

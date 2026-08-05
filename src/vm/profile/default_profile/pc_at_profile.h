#ifndef VM_PROFILE_DEFAULT_PC_AT_PROFILE_H
#define VM_PROFILE_DEFAULT_PC_AT_PROFILE_H

#include "type.h"

#include "core/machine/cpu_interface.h"
#include "core/machine/fpu_interface.h"
#include "core/machine/vadp.h"

typedef enum vm_profile_default_pc_at_device_role {
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_KBC,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC,
    VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC
} vm_profile_default_pc_at_device_role;

#define VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL 0xffu

typedef struct vm_profile_default_pc_at_port_range {
    vm_profile_default_pc_at_device_role device;
    uint16_t first;
    uint16_t last;
} vm_profile_default_pc_at_port_range;

typedef struct vm_profile_default_pc_at_route {
    vm_profile_default_pc_at_device_role device;
    uint8_t irq;
    uint8_t dma_channel;
} vm_profile_default_pc_at_route;

typedef struct vm_profile_default_pc_at_hdc_pio {
    uint16_t data_port;
    uint16_t error_features_port;
    uint16_t sector_count_port;
    uint16_t sector_number_port;
    uint16_t cylinder_low_port;
    uint16_t cylinder_high_port;
    uint16_t drive_head_port;
    uint16_t status_command_port;
    uint16_t alternate_status_device_control_port;
    uint8_t irq;
    uint8_t dma_channel;
    uint8_t data_width_bits;
    uint8_t register_width_bits;
    type_bool lba28_supported;
    type_bool slave_present;
    type_bool secondary_channel_present;
} vm_profile_default_pc_at_hdc_pio;

typedef struct vm_profile_default_pc_at_rom_mapping {
    uint32_t linear_start;
    uint32_t physical_start;
    STD_SIZE_T bytes;
    uint16_t reset_segment;
    uint16_t reset_offset;
} vm_profile_default_pc_at_rom_mapping;

typedef struct vm_profile_default_pc_at_cmos_defaults {
    uint8_t equipment;
    uint16_t base_memory_kib;
    uint8_t floppy_type;
    uint8_t fixed_disk_type;
    uint8_t boot_floppy_drive;
    uint8_t boot_hard_disk_drive;
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
    uint8_t vector;
} vm_profile_default_pc_at_firmware_service;

typedef struct vm_profile_default_pc_at_descriptor {
    const C_CHAR *identity;
    uint32_t compatibility_revision;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    uint32_t ticks_per_instruction;
    uint32_t pit_elapsed_ticks_per_input_tick;
    uint32_t kbc_typematic_initial_ticks;
    uint32_t kbc_typematic_repeat_ticks;
    uint32_t kbc_command_response_ticks;
    uint32_t rtc_ticks_per_second;
    core_machine_vadp_text_timing cga_text_timing;
    STD_SIZE_T default_memory_bytes;
    vm_profile_default_pc_at_rom_mapping rom;
    vm_profile_default_pc_at_cmos_defaults cmos;
    const vm_profile_default_pc_at_port_range *port_ranges;
    STD_SIZE_T port_range_count;
    const vm_profile_default_pc_at_route *routes;
    STD_SIZE_T route_count;
    vm_profile_default_pc_at_hdc_pio hdc_pio;
    const vm_profile_default_pc_at_firmware_service *firmware_services;
    STD_SIZE_T firmware_service_count;
} vm_profile_default_pc_at_descriptor;

const vm_profile_default_pc_at_descriptor *
vm_profile_default_pc_at_descriptor_get(C_VOID);
const vm_profile_default_pc_at_port_range *
vm_profile_default_pc_at_port_range_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device);
const vm_profile_default_pc_at_route *vm_profile_default_pc_at_route_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device);

#endif

#include "vm/profile/default_profile/pc_at_profile.h"

static const vm_profile_default_pc_at_port_range default_pc_at_port_ranges[] = {
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC, 0x0020u, 0x0021u },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC, 0x00a0u, 0x00a1u },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT, 0x0040u, 0x0043u },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0000u, 0x000fu },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_KBC, 0x0060u, 0x0064u },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER, 0x03c4u, 0x03c5u },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE, 0x03c0u, 0x03c1u },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS, 0x03ceu, 0x03cfu },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03d4u, 0x03dau },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 0x0070u, 0x0071u },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0x03f2u, 0x03f7u }
};

static const vm_profile_default_pc_at_route default_pc_at_routes[] = {
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT, 0u, 0u },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_KBC, 1u, 0u },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 8u, VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 6u, 2u }
};

static const vm_profile_default_pc_at_firmware_service
default_pc_at_firmware_services[] = {
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10, 0x10u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_CMOS_POST, 0u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_TIMER_IRQ0, 0x08u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_TIMER_INT1A, 0x1au },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_IRQ1, 0x09u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_KEYBOARD_INT16, 0x16u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_DMA_POST, 0u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_POST, 0u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_IRQ6, 0x0eu },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_INT13, 0x13u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_FDC_INT40, 0x40u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_HDC_INT13, 0x13u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIT_POST, 0u },
    { VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_PIC_POST, 0u }
};

static const vm_profile_default_pc_at_descriptor default_pc_at_descriptor = {
    "default-pc-at",
    1u,
    CORE_MACHINE_CPU_PROFILE_80386,
    CORE_MACHINE_FPU_PROFILE_NONE,
    1u,
    { 1u, 0u, 0u, 0u, 0u, 0u },
    { { 1u, 1u, 0u }, { 1u, 4u, 0u }, { 1u, 1u, 0u },
        { 1u, 1u, 0u }, { 1u, 1u, 0u } },
    0u,
    0u,
    0u,
    50000u,
    { 48u, 8u, 8u },
    { CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, TYPE_TRUE },
    { { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u } },
    16u * 1024u * 1024u,
    { 0xfffffff0u, 0x000ffff0u, 16u, 0xf000u, 0xfff0u },
    { 0x21u, 0x027fu, 0x40u, 0u, 0u, 0x80u },
    default_pc_at_port_ranges,
    sizeof(default_pc_at_port_ranges) / sizeof(default_pc_at_port_ranges[0]),
    default_pc_at_routes,
    sizeof(default_pc_at_routes) / sizeof(default_pc_at_routes[0]),
    { 0x01f0u, 0x01f1u, 0x01f2u, 0x01f3u, 0x01f4u, 0x01f5u, 0x01f6u,
        0x01f7u, 0x03f6u, 14u, VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL,
        16u, 8u, TYPE_TRUE, TYPE_FALSE, TYPE_FALSE },
    default_pc_at_firmware_services,
    sizeof(default_pc_at_firmware_services) /
        sizeof(default_pc_at_firmware_services[0])
};

const vm_profile_default_pc_at_descriptor *
vm_profile_default_pc_at_descriptor_get(C_VOID)
{
    return &default_pc_at_descriptor;
}

const vm_profile_default_pc_at_port_range *
vm_profile_default_pc_at_port_range_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device)
{
    STD_SIZE_T index;

    if (descriptor == STD_NULL) return STD_NULL;
    for (index = 0u; index < descriptor->port_range_count; ++index) {
        if (descriptor->port_ranges[index].device == device) {
            return &descriptor->port_ranges[index];
        }
    }
    return STD_NULL;
}

const vm_profile_default_pc_at_route *vm_profile_default_pc_at_route_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device)
{
    STD_SIZE_T index;

    if (descriptor == STD_NULL) return STD_NULL;
    for (index = 0u; index < descriptor->route_count; ++index) {
        if (descriptor->routes[index].device == device) {
            return &descriptor->routes[index];
        }
    }
    return STD_NULL;
}

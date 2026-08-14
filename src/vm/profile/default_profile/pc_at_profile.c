#include "vm/profile/default_profile/pc_at_profile.h"

static const vm_profile_default_pc_at_port_leaf default_pc_at_port_leaves[] = {
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC, 0x0020u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC, 0x0021u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC, 0x00a0u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIC, 0x00a1u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT, 0x0040u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT, 0x0041u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT, 0x0042u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_PIT, 0x0043u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0000u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0001u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0002u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0003u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0004u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0005u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0006u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0007u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0008u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0009u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000au, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000bu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000cu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000du, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000eu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x000fu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0081u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0082u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0083u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0087u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x0089u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x008au, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x008bu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x008fu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00c0u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00c2u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00c4u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00c6u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00c8u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00cau, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00ccu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00ceu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00d0u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00d2u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00d4u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00d6u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00d8u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00dau, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00dcu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_DMA, 0x00deu, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_KBC, 0x0060u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_KBC, 0x0064u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_MEMORY_CONTROL, 0x0092u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE, 0x03c0u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_ATTRIBUTE, 0x03c1u, TYPE_TRUE, TYPE_FALSE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER, 0x03c4u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_SEQUENCER, 0x03c5u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS, 0x03ceu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP_GRAPHICS, 0x03cfu, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03d4u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03d5u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03d8u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03d9u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_VADP, 0x03dau, TYPE_TRUE, TYPE_FALSE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 0x0070u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS, 0x0071u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0x03f2u, TYPE_FALSE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0x03f4u, TYPE_TRUE, TYPE_FALSE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0x03f5u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC, 0x03f7u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f0u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f1u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f2u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f3u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f4u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f5u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f6u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x01f7u, TYPE_TRUE, TYPE_TRUE },
    { VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC, 0x03f6u, TYPE_TRUE, TYPE_TRUE }
};

static const vm_profile_default_pc_at_route default_pc_at_routes[] = {
    { VM_PROFILE_DEFAULT_PC_AT_ROUTE_PIT_IRQ0, 0u,
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
    { VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_KEYBOARD_IRQ1, 1u,
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
    { VM_PROFILE_DEFAULT_PC_AT_ROUTE_KBC_AUX_IRQ12, 12u,
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
    { VM_PROFILE_DEFAULT_PC_AT_ROUTE_CMOS_IRQ8, 8u,
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL },
    { VM_PROFILE_DEFAULT_PC_AT_ROUTE_FDC_IRQ6_DMA2, 6u, 2u }
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

static const vm_profile_default_pc_at_firmware_service
ibm_5170_model_339_firmware_services[] = {
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
    { { 1u, 1u, 0u }, { 1u, 4u, 0u }, { 1u, 1u, 0u }, { 1u, 1u, 0u },
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
    TYPE_TRUE,
    TYPE_FALSE,
    { 0xfffffff0u, 0x000ffff0u, 16u, 0xf000u, 0xfff0u },
    { 0x21u, 0x027fu, 0x40u, 0xf0u, 0x2fu, 0u, 0x80u },
    default_pc_at_port_leaves,
    sizeof(default_pc_at_port_leaves) / sizeof(default_pc_at_port_leaves[0]),
    default_pc_at_routes,
    sizeof(default_pc_at_routes) / sizeof(default_pc_at_routes[0]),
    { 0x01f0u, 0x01f1u, 0x01f2u, 0x01f3u, 0x01f4u, 0x01f5u, 0x01f6u,
        0x01f7u, 0x03f6u, 14u, VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL,
        16u, 8u, TYPE_TRUE, TYPE_FALSE, TYPE_FALSE },
    default_pc_at_firmware_services,
    sizeof(default_pc_at_firmware_services) /
        sizeof(default_pc_at_firmware_services[0])
};

static const vm_profile_default_pc_at_descriptor ibm_5170_model_339_descriptor = {
    "ibm-5170-model-339",
    1u,
    CORE_MACHINE_CPU_PROFILE_80286,
    CORE_MACHINE_FPU_PROFILE_NONE,
    1u,
    { 1u, 0u, 0u, 0u, 0u, 0u },
    { { 1u, 1u, 0u }, { 1u, 4u, 0u }, { 1u, 1u, 0u }, { 1u, 1u, 0u },
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
    512u * 1024u,
    TYPE_FALSE,
    TYPE_TRUE,
    { 0xfffffff0u, 0x000ffff0u, 16u, 0xf000u, 0xfff0u },
    { 0x21u, 0x0200u, 0x40u, 0x00u, 0x00u, 0u, 0x80u },
    default_pc_at_port_leaves,
    sizeof(default_pc_at_port_leaves) / sizeof(default_pc_at_port_leaves[0]),
    default_pc_at_routes,
    sizeof(default_pc_at_routes) / sizeof(default_pc_at_routes[0]),
    { 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL,
        0u, 0u, TYPE_FALSE, TYPE_FALSE, TYPE_FALSE },
    ibm_5170_model_339_firmware_services,
    sizeof(ibm_5170_model_339_firmware_services) /
        sizeof(ibm_5170_model_339_firmware_services[0])
};

const vm_profile_default_pc_at_descriptor *
vm_profile_default_pc_at_descriptor_get(C_VOID)
{
    return &default_pc_at_descriptor;
}

const vm_profile_default_pc_at_descriptor *
vm_profile_ibm_5170_model_339_descriptor_get(C_VOID)
{
    return &ibm_5170_model_339_descriptor;
}

const vm_profile_default_pc_at_port_leaf *
vm_profile_default_pc_at_port_leaf_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device, type_unsigned_16 port)
{
    STD_SIZE_T index;

    if (descriptor == STD_NULL) return STD_NULL;
    for (index = 0u; index < descriptor->port_leaf_count; ++index) {
        if (descriptor->port_leaves[index].device == device &&
            (device != VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC ||
            descriptor->hdc_present) &&
            descriptor->port_leaves[index].port == port) {
            return &descriptor->port_leaves[index];
        }
    }
    return STD_NULL;
}

const vm_profile_default_pc_at_port_leaf *
vm_profile_default_pc_at_port_leaf_at(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_device_role device, STD_SIZE_T ordinal)
{
    STD_SIZE_T index;

    if (descriptor == STD_NULL) return STD_NULL;
    for (index = 0u; index < descriptor->port_leaf_count; ++index) {
        if (descriptor->port_leaves[index].device == device &&
            (device != VM_PROFILE_DEFAULT_PC_AT_DEVICE_HDC ||
            descriptor->hdc_present)) {
            if (ordinal == 0u) return &descriptor->port_leaves[index];
            --ordinal;
        }
    }
    return STD_NULL;
}

const vm_profile_default_pc_at_route *vm_profile_default_pc_at_route_find(
    const vm_profile_default_pc_at_descriptor *descriptor,
    vm_profile_default_pc_at_route_source source)
{
    STD_SIZE_T index;

    if (descriptor == STD_NULL) return STD_NULL;
    for (index = 0u; index < descriptor->route_count; ++index) {
        if (descriptor->routes[index].source == source) {
            return &descriptor->routes[index];
        }
    }
    return STD_NULL;
}

C_INT vm_profile_default_pc_at_descriptor_is_valid(
    const vm_profile_default_pc_at_descriptor *descriptor)
{
    STD_SIZE_T index;

    if (descriptor == &ibm_5170_model_339_descriptor) {
        return descriptor->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
            descriptor->default_memory_bytes == 512u * 1024u &&
            !descriptor->hdc_present && descriptor->planar_parity_present &&
            descriptor->cmos.base_memory_kib == 0x0200u &&
            descriptor->cmos.floppy_type == 0x40u &&
            descriptor->cmos.fixed_disk_type == 0u &&
            descriptor->cmos.fixed_disk_type_extended_0 == 0u &&
            descriptor->firmware_service_count ==
            sizeof(ibm_5170_model_339_firmware_services) /
                sizeof(ibm_5170_model_339_firmware_services[0]);
    }
    if (descriptor == STD_NULL || descriptor->port_leaves == STD_NULL ||
        descriptor->routes == STD_NULL || descriptor->port_leaf_count !=
        sizeof(default_pc_at_port_leaves) / sizeof(default_pc_at_port_leaves[0]) ||
        descriptor->route_count != sizeof(default_pc_at_routes) /
        sizeof(default_pc_at_routes[0])) return 0;
    for (index = 0u; index < descriptor->port_leaf_count; ++index) {
        if (STD_MEMCMP(&descriptor->port_leaves[index],
                &default_pc_at_port_leaves[index],
                sizeof(default_pc_at_port_leaves[index])) != 0) return 0;
    }
    for (index = 0u; index < descriptor->route_count; ++index) {
        if (STD_MEMCMP(&descriptor->routes[index], &default_pc_at_routes[index],
                sizeof(default_pc_at_routes[index])) != 0) return 0;
    }
    return descriptor->hdc_present && !descriptor->planar_parity_present &&
        descriptor->hdc_pio.data_port == 0x01f0u &&
        descriptor->hdc_pio.error_features_port == 0x01f1u &&
        descriptor->hdc_pio.sector_count_port == 0x01f2u &&
        descriptor->hdc_pio.sector_number_port == 0x01f3u &&
        descriptor->hdc_pio.cylinder_low_port == 0x01f4u &&
        descriptor->hdc_pio.cylinder_high_port == 0x01f5u &&
        descriptor->hdc_pio.drive_head_port == 0x01f6u &&
        descriptor->hdc_pio.status_command_port == 0x01f7u &&
        descriptor->hdc_pio.alternate_status_device_control_port == 0x03f6u &&
        descriptor->hdc_pio.irq == 14u &&
        descriptor->hdc_pio.dma_channel ==
            VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL &&
        descriptor->hdc_pio.data_width_bits == 16u &&
        descriptor->hdc_pio.register_width_bits == 8u &&
        descriptor->hdc_pio.lba28_supported && !descriptor->hdc_pio.slave_present &&
        !descriptor->hdc_pio.secondary_channel_present;
}

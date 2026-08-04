#include "type.h"

#include "vm/profile/default_profile/pc_at_profile.h"

C_INT main(C_VOID)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();
    const vm_profile_default_pc_at_port_range *cmos_ports;
    const vm_profile_default_pc_at_port_range *fdc_ports;
    const vm_profile_default_pc_at_route *fdc_route;

    if (profile == STD_NULL ||
        STD_STRCMP(profile->identity, "default-pc-at") != 0 ||
        profile->rom.linear_start != 0xfffffff0u ||
        profile->rom.physical_start != 0x000ffff0u ||
        profile->rom.bytes != 16u ||
        profile->rom.reset_segment != 0xf000u ||
        profile->rom.reset_offset != 0xfff0u ||
        profile->cmos.equipment != 0x21u ||
        profile->cmos.base_memory_kib != 0x027fu ||
        profile->firmware_service_count != 14u) return 1;

    cmos_ports = vm_profile_default_pc_at_port_range_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_CMOS);
    fdc_ports = vm_profile_default_pc_at_port_range_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC);
    fdc_route = vm_profile_default_pc_at_route_find(profile,
        VM_PROFILE_DEFAULT_PC_AT_DEVICE_FDC);
    if (cmos_ports == STD_NULL || cmos_ports->first != 0x0070u ||
        cmos_ports->last != 0x0071u || fdc_ports == STD_NULL ||
        fdc_ports->first != 0x03f2u || fdc_ports->last != 0x03f7u ||
        fdc_route == STD_NULL || fdc_route->irq != 6u ||
        fdc_route->dma_channel != 2u) return 1;

    puts("M5:T208:S2:DEFAULT-PC-AT-PROFILE:OK");
    return 0;
}

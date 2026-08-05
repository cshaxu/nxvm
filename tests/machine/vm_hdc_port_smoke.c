#include "type.h"

#include "vm/profile/default_profile/pc_at_profile.h"

C_INT main(C_VOID)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();
    const vm_profile_default_pc_at_hdc_pio *hdc;

    if (profile == STD_NULL) return 1;
    hdc = &profile->hdc_pio;
    if (hdc->data_port != 0x01f0u || hdc->error_features_port != 0x01f1u ||
        hdc->sector_count_port != 0x01f2u ||
        hdc->sector_number_port != 0x01f3u ||
        hdc->cylinder_low_port != 0x01f4u ||
        hdc->cylinder_high_port != 0x01f5u ||
        hdc->drive_head_port != 0x01f6u ||
        hdc->status_command_port != 0x01f7u ||
        hdc->alternate_status_device_control_port != 0x03f6u ||
        hdc->irq != 14u ||
        hdc->dma_channel != VM_PROFILE_DEFAULT_PC_AT_NO_DMA_CHANNEL ||
        hdc->data_width_bits != 16u || hdc->register_width_bits != 8u) {
        return 1;
    }
    STD_PRINTF("M5:T213:S1:HDC:PORT:OK data=%04X status=%04X alt=%04X irq=%u dma=none\n",
        hdc->data_port, hdc->status_command_port,
        hdc->alternate_status_device_control_port, (C_UINT)hdc->irq);
    return 0;
}

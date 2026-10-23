#include "app-mypc110/pc110_machine_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"

int main(void)
{
    static const lib_u8 bios[MYPC110_TFT_BIOS_BYTES] = {0};
    mypc110_base_composition composition = {0};
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    failed |= mypc110_base_composition_create(bios, sizeof(bios), &composition) !=
        LIB_STATUS_OK;
    failed |= !failed && (composition.core == LIB_NULL || composition.board == LIB_NULL ||
        composition.configuration_chip == LIB_NULL);
    failed |= !failed && (!x86_core_machine_immutable_rom_mapping_contains(
        composition.core, MYPC110_TFT_BIOS_PHYSICAL_START, MYPC110_TFT_BIOS_BYTES) ||
        x86_core_machine_immutable_rom_mapping_contains(composition.core,
            0xfffffff0u, 1u));
    failed |= !failed && x86_core_machine_reset(composition.core) != LIB_STATUS_OK;
    failed |= !failed && (x86_core_machine_bus_read(composition.core, 0x20u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x40u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x60u, &value) !=
        LIB_STATUS_UNSUPPORTED || x86_core_machine_bus_read(composition.core, 0x92u,
            &value) != LIB_STATUS_UNSUPPORTED || x86_core_machine_bus_read(composition.core,
            0x3dau, &value) != LIB_STATUS_UNSUPPORTED);

    mypc110_base_composition_destroy(&composition);
    return failed;
}

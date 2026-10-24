#include "app-mypc110/pc110_machine_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"

int main(void)
{
    static const lib_u8 bios[MYPC110_TFT_BIOS_BYTES] = {0};
    static const lib_u8 font[MYPC110_FONT_ROM_BYTES] = {0};
    mypc110_base_composition composition = {0};
    lib_u8 write = 0x5au;
    lib_u8 read = 0u;
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    failed |= mypc110_base_composition_create(bios, sizeof(bios), font,
        sizeof(font), &composition) != LIB_STATUS_OK ||
        composition.display_controller == LIB_NULL ||
        x86_core_machine_reset(composition.core) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core,
            X86_F65535_EXTENSION_INDEX_PORT, 0u) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(composition.core,
            X86_F65535_EXTENSION_DATA_PORT, &value) != LIB_STATUS_OK ||
        value != X86_F65535_PC110_CHIP_VERSION ||
        x86_core_machine_memory_write(composition.core,
            MYPC110_DISPLAY_MEMORY_START + MYPC110_DISPLAY_MEMORY_BYTES - 1u,
            &write, 1u) != LIB_STATUS_OK || x86_core_machine_memory_read(
            composition.core, MYPC110_DISPLAY_MEMORY_START +
            MYPC110_DISPLAY_MEMORY_BYTES - 1u, &read, 1u) != LIB_STATUS_OK ||
        read != write || x86_core_machine_memory_write(composition.core,
            MYPC110_DISPLAY_MEMORY_START + MYPC110_DISPLAY_MEMORY_BYTES,
            &write, 1u) != LIB_STATUS_OK;
    mypc110_base_composition_destroy(&composition);
    return failed;
}

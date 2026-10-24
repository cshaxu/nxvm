#include "app-mypc110/pc110_machine_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"

int main(void)
{
    static const lib_u8 bios[MYPC110_TFT_BIOS_BYTES] = {0};
    static lib_u8 font[MYPC110_FONT_ROM_BYTES];
    static const lib_u8 make_break[] = {0x1cu, 0xf0u, 0x1cu};
    mypc110_base_composition composition = {0};
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    font[0] = 0x55u;
    font[MYPC110_FONT_BANK_BYTES] = 0xaau;
    failed |= mypc110_base_composition_create(bios, sizeof(bios), font,
        sizeof(font), &composition) != LIB_STATUS_OK;
    failed |= !failed && (composition.core == LIB_NULL || composition.board == LIB_NULL ||
        composition.configuration_chip == LIB_NULL);
    failed |= !failed && (!x86_core_machine_immutable_rom_mapping_contains(
        composition.core, MYPC110_TFT_BIOS_PHYSICAL_START, MYPC110_TFT_BIOS_BYTES) ||
        !x86_core_machine_immutable_rom_mapping_contains(composition.core,
            0xfffffff0u, 1u));
    failed |= !failed && x86_core_machine_reset(composition.core) != LIB_STATUS_OK;
    failed |= !failed && (x86_core_machine_bus_read(composition.core, 0x20u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x40u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x60u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x64u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x92u,
            &value) != LIB_STATUS_UNSUPPORTED || x86_core_machine_bus_read(composition.core,
            0x3dau, &value) != LIB_STATUS_OK);
    failed |= !failed && (x86_core_machine_memory_read(composition.core,
        MYPC110_FONT_PHYSICAL_START, &value, 1u) != LIB_STATUS_OK ||
        value != 0x55u || x86_core_machine_bus_write(composition.core,
            MYPC110_FONT_BANK_PORT, 0x81u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(composition.core,
            MYPC110_FONT_PHYSICAL_START, &value, 1u) != LIB_STATUS_OK ||
        value != 0xaau);
    failed |= !failed && (x86_core_machine_bus_write(composition.core, 0x64u, 0x60u) !=
        LIB_STATUS_OK || x86_core_machine_bus_write(composition.core, 0x60u, 0x01u) !=
        LIB_STATUS_OK || x86_nxvm_board_keyboard_receive_native_bytes(composition.board,
            make_break, sizeof(make_break)) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(composition.core, 0x60u, &value) != LIB_STATUS_OK ||
        value != make_break[0] || x86_core_machine_bus_read(composition.core, 0x60u,
            &value) != LIB_STATUS_OK || value != make_break[1] ||
        x86_core_machine_bus_read(composition.core, 0x60u, &value) != LIB_STATUS_OK ||
        value != make_break[2]);
    failed |= !failed && (x86_core_machine_bus_write(composition.core, 0x64u, 0xa8u) !=
        LIB_STATUS_OK || x86_core_machine_bus_write(composition.core, 0x64u, 0xd4u) !=
        LIB_STATUS_OK || x86_core_machine_bus_write(composition.core, 0x60u, 0xf2u) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x60u, &value) !=
        LIB_STATUS_OK || value != 0xfau || x86_core_machine_bus_read(composition.core,
            0x60u, &value) != LIB_STATUS_OK || value != 0x00u ||
        x86_core_machine_bus_write(composition.core, 0x64u, 0xd4u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x60u, 0xf4u) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(composition.core, 0x60u, &value) != LIB_STATUS_OK ||
        value != 0xfau || x86_nxvm_board_mouse_receive_relative(composition.board,
            5, -3, 1u) != LIB_STATUS_OK || x86_core_machine_bus_read(composition.core,
            0x60u, &value) != LIB_STATUS_OK || value != 0x29u ||
        x86_core_machine_bus_read(composition.core, 0x60u, &value) != LIB_STATUS_OK ||
        value != 0x05u || x86_core_machine_bus_read(composition.core, 0x60u, &value) !=
        LIB_STATUS_OK || value != 0xfdu);

    mypc110_base_composition_destroy(&composition);
    return failed;
}

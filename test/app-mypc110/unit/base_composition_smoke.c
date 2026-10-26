#include "app-mypc110/pc110_machine_interface.h"
#include "lib/types/file.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

int main(void)
{
    static lib_u8 bios[MYPC110_FIRMWARE_BYTES];
    static lib_u8 font[MYPC110_FONT_ROM_BYTES];
    static const lib_u8 make_break[] = {0x1cu, 0xf0u, 0x1cu};
    static const lib_u8 shadow_patch = 0x44u;
    static const lib_u8 high_memory_pattern[] = {0x55u, 0xaau, 0x55u, 0xaau};
    mypc110_base_composition composition = {0};
    lib_u32 value = 0u;
    lib_u8 byte = 0u;
    lib_bool a20_enabled = LIB_FALSE;
    lib_bool advanced = LIB_FALSE;
    lib_i32 failed = 0;

    bios[2u * MYPC110_FIRMWARE_BANK_BYTES] = 0x11u;
    font[0] = 0x55u;
    font[MYPC110_FONT_BANK_BYTES] = 0xaau;
    failed |= mypc110_base_composition_create(bios, sizeof(bios), font,
        sizeof(font), &composition) != LIB_STATUS_OK;
    failed |= !failed && (composition.core == LIB_NULL || composition.board == LIB_NULL ||
        composition.configuration_chip == LIB_NULL);
    failed |= !failed && x86_core_machine_reset(composition.core) != LIB_STATUS_OK;
    failed |= !failed && (x86_core_machine_bus_write(composition.core, 0x64u, 0x20u) !=
        LIB_STATUS_OK || x86_core_machine_advance_to_next_deadline(composition.core,
            &advanced) != LIB_STATUS_OK || !advanced ||
        x86_core_machine_bus_read(composition.core, 0x60u, &value) != LIB_STATUS_OK ||
        value != 0x65u || x86_nxvm_board_mouse_receive_relative(composition.board,
            1, 1, 0u) != LIB_STATUS_INVALID_STATE);
    failed |= !failed && (x86_nxvm_board_observe_a20(composition.board,
        &a20_enabled) != LIB_STATUS_OK || !a20_enabled);
    failed |= !failed && (x86_core_machine_memory_write(composition.core,
        0x013f0000u, high_memory_pattern, sizeof(high_memory_pattern)) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(composition.core, 0x013f0000u, &value,
            sizeof(value)) != LIB_STATUS_OK || value != 0xaa55aa55u);
    failed |= !failed && (x86_core_machine_memory_read(composition.core,
        0x01400000u, &byte, 1u) != LIB_STATUS_OK || byte != 0xffu ||
        x86_core_machine_memory_write(composition.core, 0x01400000u,
            &shadow_patch, 1u) != LIB_STATUS_OK || x86_core_machine_memory_read(
                composition.core, 0x01400000u, &byte, 1u) != LIB_STATUS_OK ||
        byte != 0xffu);
    failed |= !failed && (x86_core_machine_memory_read(composition.core,
        0x000c0000u, &byte, 1u) != LIB_STATUS_OK || byte != 0xffu ||
        x86_core_machine_memory_write(composition.core, 0x000c0000u,
            &shadow_patch, 1u) != LIB_STATUS_OK || x86_core_machine_memory_read(
                composition.core, 0x000c0000u, &byte, 1u) != LIB_STATUS_OK ||
        byte != 0xffu || x86_core_machine_memory_read(composition.core,
            0x000d0000u, &byte, 1u) != LIB_STATUS_OK || byte != 0xffu);
    failed |= !failed && (x86_core_machine_bus_read(composition.core, 0x20u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x40u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x60u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x64u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x92u,
            &value) != LIB_STATUS_OK || value != 0u || x86_core_machine_bus_read(composition.core,
            0x3dau, &value) != LIB_STATUS_OK);
    for (lib_u16 port = 0x0080u; !failed && port <= 0x008fu; ++port) {
        failed |= x86_core_machine_bus_read(composition.core, port, &value) !=
                LIB_STATUS_OK || value != 0u ||
            x86_core_machine_bus_write(composition.core, port,
                (lib_u32)(port ^ 0x005au)) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(composition.core, port, &value) !=
                LIB_STATUS_OK || value != (lib_u32)(port ^ 0x005au);
    }
    failed |= !failed && x86_core_machine_reset(composition.core) != LIB_STATUS_OK;
    for (lib_u16 port = 0x0080u; !failed && port <= 0x008fu; ++port) {
        failed |= x86_core_machine_bus_read(composition.core, port, &value) !=
            LIB_STATUS_OK || value != 0u;
    }
    failed |= !failed && (x86_core_machine_memory_read(composition.core,
        MYPC110_FONT_PHYSICAL_START, &value, 1u) != LIB_STATUS_OK ||
        value != 0x55u || x86_core_machine_bus_write(composition.core,
            MYPC110_FONT_BANK_PORT, 0x81u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(composition.core,
            MYPC110_FONT_PHYSICAL_START, &value, 1u) != LIB_STATUS_OK ||
        value != 0xaau);
    /* The BIOS drains early OBF before waiting for the reset ACK. The first
     * completed-write status sample must not expose that ACK for draining. */
    if (!failed) {
        failed = x86_core_machine_bus_write(composition.core, 0x64u, 0x60u) !=
            LIB_STATUS_OK || x86_core_machine_bus_write(composition.core, 0x60u, 0x6du) !=
            LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x60u, &value) !=
            LIB_STATUS_OK || value != 0xaau ||
            x86_core_machine_bus_write(composition.core, 0x60u, 0xffu) !=
            LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x64u, &value) !=
            LIB_STATUS_OK || (value & 3u) != 0u ||
            x86_core_machine_advance_to_next_deadline(composition.core, &advanced) !=
            LIB_STATUS_OK || !advanced || x86_core_machine_bus_read(composition.core, 0x60u,
                &value) != LIB_STATUS_OK || value != 0xfau;
        if (failed) lib_c_printf("PC110:KBC:reset-order failed value=%08x\n", value);
    }
    failed |= !failed && x86_core_machine_reset(composition.core) != LIB_STATUS_OK;
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
        LIB_STATUS_OK || x86_core_machine_bus_read(composition.core, 0x64u, &value) !=
        LIB_STATUS_OK || (value & 1u) != 0u ||
        x86_core_machine_advance_to_next_deadline(composition.core, &advanced) !=
        LIB_STATUS_OK || !advanced ||
        x86_core_machine_bus_read(composition.core, 0x60u, &value) !=
        LIB_STATUS_OK || value != 0xfau || x86_core_machine_bus_read(composition.core,
            0x60u, &value) != LIB_STATUS_OK || value != 0x00u ||
        x86_core_machine_bus_write(composition.core, 0x64u, 0xd4u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x60u, 0xe9u) != LIB_STATUS_OK ||
        x86_core_machine_advance_to_next_deadline(composition.core, &advanced) !=
        LIB_STATUS_OK || !advanced ||
        x86_core_machine_bus_read(composition.core, 0x60u, &value) != LIB_STATUS_OK ||
        value != 0xfau || x86_core_machine_bus_read(composition.core, 0x60u,
            &value) != LIB_STATUS_OK || value != 0x00u ||
        x86_core_machine_bus_read(composition.core, 0x60u, &value) != LIB_STATUS_OK ||
        value != 0x02u || x86_core_machine_bus_read(composition.core, 0x60u,
            &value) != LIB_STATUS_OK || value != 0x64u ||
        x86_core_machine_bus_write(composition.core, 0x64u, 0xd4u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x60u, 0xf4u) != LIB_STATUS_OK ||
        x86_core_machine_advance_to_next_deadline(composition.core, &advanced) !=
        LIB_STATUS_OK || !advanced ||
        x86_core_machine_bus_read(composition.core, 0x60u, &value) != LIB_STATUS_OK ||
        value != 0xfau || x86_nxvm_board_mouse_receive_relative(composition.board,
            5, -3, 1u) != LIB_STATUS_OK || x86_core_machine_bus_read(composition.core,
            0x60u, &value) != LIB_STATUS_OK || value != 0x29u ||
        x86_core_machine_bus_read(composition.core, 0x60u, &value) != LIB_STATUS_OK ||
        value != 0x05u || x86_core_machine_bus_read(composition.core, 0x60u, &value) !=
        LIB_STATUS_OK || value != 0xfdu);

    if (failed) lib_c_printf("PC110:BASE:composition failed value=%08x\n", value);
    mypc110_base_composition_destroy(&composition);
    return failed;
}

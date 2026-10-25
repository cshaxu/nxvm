#include "app-mypc110/pc110_machine_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"

int main(void)
{
    static const lib_u8 bios[MYPC110_FIRMWARE_BYTES] = {0};
    static const lib_u8 font[MYPC110_FONT_ROM_BYTES] = {0};
    mypc110_base_composition composition = {0};
    x86_video_snapshot snapshot = {0};
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
        x86_core_machine_bus_write(composition.core, 0x03ceu, 6u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03cfu, 0x0eu) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03c4u, 4u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03c5u, 2u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d4u, 1u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d5u, 0x4fu) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d4u, 7u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d5u, 0x1fu) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d4u, 9u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d5u, 0x0fu) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d4u, 18u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d5u, 0x8fu) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(composition.core,
            0x000b8000u,
            &write, 1u) != LIB_STATUS_OK || x86_core_machine_memory_read(
            composition.core, 0x000b8000u, &read, 1u) != LIB_STATUS_OK ||
        read != write || x86_core_machine_memory_write(composition.core,
            MYPC110_DISPLAY_MEMORY_START + MYPC110_DISPLAY_MEMORY_BYTES,
            &write, 1u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(composition.display_controller,
            &snapshot) != LIB_STATUS_OK || snapshot.columns != 80u ||
        snapshot.rows != 25u || snapshot.characters[0] != write;
    /* Exercise the board's existing standard-VGA route into F65535 mode 12
     * capture, without introducing an App-local display path. */
    failed |= x86_core_machine_bus_write(composition.core, 0x03ceu, 6u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03cfu, 5u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03c4u, 4u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03c5u, 4u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d4u, 1u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d5u, 79u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d4u, 7u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d5u, 2u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d4u, 18u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d5u, 0xdfu) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d4u, 19u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03d5u, 40u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03c4u, 2u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(composition.core, 0x03c5u, 15u) != LIB_STATUS_OK ||
        x86_core_machine_memory_write(composition.core, 0x000a0000u,
            &write, 1u) != LIB_STATUS_OK ||
        x86_f65535_capture_snapshot(composition.display_controller,
            &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_VGA_640X480X16 ||
        snapshot.pixel_width != 640u || snapshot.pixel_height != 480u ||
        snapshot.pixels[1] != 15u;
    mypc110_base_composition_destroy(&composition);
    return failed;
}

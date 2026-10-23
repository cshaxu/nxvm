#include "app-mypc110/pc110_font_interface.h"

int main(void)
{
    static lib_u8 image[MYPC110_FONT_ROM_BYTES];
    const x86_core_machine_executor_config config = {
        .memory_bytes = 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX
    };
    mypc110_font_rom font;
    x86_core_machine *machine = LIB_NULL;
    lib_u8 bytes[2] = {0};
    lib_u8 write = 0xa5u;
    x86_core_machine_memory_route route;
    lib_u32 value = 0u;
    lib_i32 failed = LIB_FALSE;

    image[0] = 0x55u;
    image[1] = 0xaau;
    image[2u * MYPC110_FONT_BANK_BYTES] = 0x31u;
    image[2u * MYPC110_FONT_BANK_BYTES + MYPC110_FONT_BANK_BYTES - 1u] = 0x7cu;
    failed |= mypc110_font_rom_create(LIB_NULL, sizeof(image), &font) !=
        LIB_STATUS_INVALID_ARGUMENT || mypc110_font_rom_create(image,
            sizeof(image) - 1u, &font) != LIB_STATUS_INVALID_ARGUMENT ||
        mypc110_font_rom_create(image, sizeof(image), &font) != LIB_STATUS_OK ||
        x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        mypc110_font_rom_install(machine, &font) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, MYPC110_FONT_PHYSICAL_START,
            bytes, sizeof(bytes)) != LIB_STATUS_OK || bytes[0] != 0x55u ||
        bytes[1] != 0xaau || x86_core_machine_bus_write(machine,
            MYPC110_FONT_BANK_PORT, 0x82u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, MYPC110_FONT_PHYSICAL_START,
            &bytes[0], 1u) != LIB_STATUS_OK || bytes[0] != 0x31u ||
        x86_core_machine_memory_read(machine, MYPC110_FONT_PHYSICAL_START +
            MYPC110_FONT_BANK_BYTES - 1u, &bytes[0], 1u) != LIB_STATUS_OK ||
        bytes[0] != 0x7cu || x86_core_machine_memory_write(machine,
            MYPC110_FONT_PHYSICAL_START, &write, 1u) != LIB_STATUS_OK ||
        x86_core_machine_memory_read(machine, MYPC110_FONT_PHYSICAL_START,
            &bytes[0], 1u) != LIB_STATUS_OK || bytes[0] != 0x31u ||
        x86_core_machine_memory_query(machine, MYPC110_FONT_PHYSICAL_START,
            1u, X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
        route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER ||
        x86_core_machine_memory_query(machine, MYPC110_FONT_PHYSICAL_START - 1u,
            1u, X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
        route != X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM ||
        x86_core_machine_bus_read(machine, MYPC110_FONT_BANK_PORT, &value) !=
            LIB_STATUS_UNSUPPORTED;
    x86_core_machine_destroy(machine);
    return failed ? 1 : 0;
}

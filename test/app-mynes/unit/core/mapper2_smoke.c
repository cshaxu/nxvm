#include <assert.h>

#include "core/bus.h"
#include "core/machine.h"

#define MAPPER2_IMAGE_BYTES (16u + 8u * 16384u)

static void make_image(lib_u8 *image, lib_bool vertical)
{
    lib_size index;

    lib_memory_set(image, 0, MAPPER2_IMAGE_BYTES);
    image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1au;
    image[4] = 8u;
    image[6] = (lib_u8)(0x20u | (vertical ? 1u : 0u));
    for (index = 0u; index < 8u * 16384u; ++index)
        image[16u + index] = (lib_u8)(index >> 14u);
}

static lib_u8 bus_read(core_machine *machine, lib_u16 address)
{
    lib_u8 value;

    assert(core_bus_read(machine, address, &value) == LIB_STATUS_OK);
    return value;
}

static void bus_write(core_machine *machine, lib_u16 address, lib_u8 value)
{
    assert(core_bus_write(machine, address, value) == LIB_STATUS_OK);
}

int main(void)
{
    lib_u8 image[MAPPER2_IMAGE_BYTES];
    core_machine *machine = LIB_NULL;
    core_machine_options options = { 0 };

    make_image(image, LIB_FALSE);
    assert(core_machine_create(&machine, image, sizeof(image), &options) ==
        LIB_STATUS_OK);
    assert(machine->cartridge->mapper == 2u &&
        machine->cartridge->chr_ram == LIB_TRUE);
    assert(bus_read(machine, 0x8000u) == 0u);
    assert(bus_read(machine, 0xc000u) == 7u);
    bus_write(machine, 0xc000u, 5u);
    assert(bus_read(machine, 0x8000u) == 5u);
    assert(bus_read(machine, 0xc000u) == 7u);
    /* The currently mapped bank 5 drives the bus at $8000, so writing 7
     * produces the hardware AND result 5 rather than selecting bank 7. */
    bus_write(machine, 0x8000u, 7u);
    assert(bus_read(machine, 0x8000u) == 5u);
    bus_write(machine, 0xc000u, 0xffu);
    assert(bus_read(machine, 0x8000u) == 7u);
    bus_write(machine, 0x7fffu, 0u);
    assert(bus_read(machine, 0x8000u) == 7u);
    assert(core_cartridge_ppu_write(machine->cartridge, 0u, 0x5au));
    assert(core_cartridge_ppu_write(machine->cartridge, 0x1fffu, 0xa5u));
    assert(core_cartridge_ppu_read(machine->cartridge, 0u) == 0x5au);
    assert(core_cartridge_ppu_read(machine->cartridge, 0x1fffu) == 0xa5u);
    assert(core_cartridge_ciram_address(machine->cartridge, 0x2000u) ==
        core_cartridge_ciram_address(machine->cartridge, 0x2400u));
    core_machine_destroy(machine);

    make_image(image, LIB_TRUE);
    assert(core_machine_create(&machine, image, sizeof(image), &options) ==
        LIB_STATUS_OK);
    bus_write(machine, 0xc000u, 3u);
    assert(bus_read(machine, 0x8000u) == 3u);
    assert(core_cartridge_ciram_address(machine->cartridge, 0x2000u) ==
        core_cartridge_ciram_address(machine->cartridge, 0x2800u));
    assert(core_cartridge_ciram_address(machine->cartridge, 0x2000u) !=
        core_cartridge_ciram_address(machine->cartridge, 0x2400u));
    core_machine_destroy(machine);
    return 0;
}

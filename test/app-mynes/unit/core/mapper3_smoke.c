#include <assert.h>

#include "core/bus.h"
#include "core/machine.h"

#define MAPPER3_IMAGE_BYTES (16u + 32768u + 4u * 8192u)

static void make_image(lib_u8 *image, lib_bool vertical)
{
    lib_size index;

    lib_memory_set(image, 0, MAPPER3_IMAGE_BYTES);
    image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1au;
    image[4] = 2u; image[5] = 4u;
    image[6] = (lib_u8)(0x30u | (vertical ? 1u : 0u));
    for (index = 0u; index < 4u; ++index)
        lib_memory_set(image + 16u + 32768u + index * 8192u, (lib_i32)index, 8192u);
    image[16u + 0u] = 2u;
    image[16u + 0x4000u] = 0u;
    image[16u + 0x4001u] = 1u;
    image[16u + 0x4002u] = 2u;
    image[16u + 0x4003u] = 3u;
}

static void bus_write(core_machine *machine, lib_u16 address, lib_u8 value)
{
    assert(core_bus_write(machine, address, value) == LIB_STATUS_OK);
}

int main(void)
{
    lib_u8 image[MAPPER3_IMAGE_BYTES];
    core_machine *machine = LIB_NULL;
    core_machine_options options = { 0 };
    lib_size index;

    make_image(image, LIB_FALSE);
    assert(core_machine_create(&machine, image, sizeof(image), &options) ==
        LIB_STATUS_OK);
    assert(machine->cartridge->mapper == 3u && !machine->cartridge->chr_ram);
    assert(core_cartridge_ppu_read(machine->cartridge, 0u) == 0u);
    for (index = 0u; index < 4u; ++index) {
        bus_write(machine, (lib_u16)(0xc000u + index), (lib_u8)index);
        assert(core_cartridge_ppu_read(machine->cartridge, 0u) == index);
    }
    /* ROM byte 2 at $8000 turns a write of 3 into selected bank 2. */
    bus_write(machine, 0x8000u, 3u);
    assert(core_cartridge_ppu_read(machine->cartridge, 0u) == 2u);
    bus_write(machine, 0x7fffu, 0u);
    assert(core_cartridge_ppu_read(machine->cartridge, 0u) == 2u);
    assert(!core_cartridge_ppu_write(machine->cartridge, 0u, 0xffu));
    assert(core_cartridge_ciram_address(machine->cartridge, 0x2000u) ==
        core_cartridge_ciram_address(machine->cartridge, 0x2400u));
    core_machine_destroy(machine);

    make_image(image, LIB_TRUE);
    assert(core_machine_create(&machine, image, sizeof(image), &options) ==
        LIB_STATUS_OK);
    bus_write(machine, 0xc003u, 3u);
    assert(core_cartridge_ppu_read(machine->cartridge, 0x1fffu) == 3u);
    assert(core_cartridge_ciram_address(machine->cartridge, 0x2000u) ==
        core_cartridge_ciram_address(machine->cartridge, 0x2800u));
    assert(core_cartridge_ciram_address(machine->cartridge, 0x2000u) !=
        core_cartridge_ciram_address(machine->cartridge, 0x2400u));
    core_machine_destroy(machine);
    return 0;
}

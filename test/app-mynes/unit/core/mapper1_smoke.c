#include <assert.h>
#include <stdlib.h>

#include "core/bus.h"
#include "core/machine.h"
#include "lib/storage/file_interface.h"

static void write_serial(core_machine *machine, lib_u16 address, lib_u8 value)
{
    lib_u8 bit;
    for (bit = 0u; bit < 5u; ++bit)
        assert(core_bus_write(machine, address, (lib_u8)((value >> bit) & 1u)) ==
            LIB_STATUS_OK);
}

static void ppu_address(core_machine *machine, lib_u16 address)
{
    core_ppu_cpu_write(&machine->ppu, machine->cartridge, 6u,
        (lib_u8)(address >> 8u));
    core_ppu_cpu_write(&machine->ppu, machine->cartridge, 6u,
        (lib_u8)address);
}

static lib_u8 ppu_read(core_machine *machine, lib_u16 address)
{
    ppu_address(machine, address);
    (void)core_ppu_cpu_read(&machine->ppu, machine->cartridge, 7u);
    return core_ppu_cpu_read(&machine->ppu, machine->cartridge, 7u);
}

static void owner_local_header_load(void)
{
    const char *path = getenv("MYNES_OWNER_MMC1_ROM");
    core_machine *machine = LIB_NULL;
    core_machine_options options = { 0 };
    void *bytes = LIB_NULL;
    lib_size byte_count = 0u;

    if (path == LIB_NULL || path[0] == '\0') return;
    assert(lib_storage_file_read_owned(path, 65552u, &bytes, &byte_count) == LIB_STATUS_OK);
    assert(core_machine_create(&machine, bytes, byte_count, &options) == LIB_STATUS_OK);
    assert(machine->cartridge->mapper == 1u);
    assert(machine->cartridge->prg_bytes == 32768u && machine->cartridge->chr_bytes == 32768u);
    core_machine_destroy(machine);
    lib_release(bytes);
}

int main(void)
{
    lib_u8 image[16u + 32768u + 32768u];
    core_machine *machine = LIB_NULL;
    core_machine_options options = { 0 };
    lib_size index;
    lib_u8 value;

    lib_memory_set(image, 0, sizeof(image));
    image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1au;
    image[4] = 2u; image[5] = 4u; image[6] = 0x10u;
    for (index = 0u; index < 32768u; ++index)
        image[16u + index] = (lib_u8)(index >> 14u);
    for (index = 0u; index < 32768u; ++index)
        image[16u + 32768u + index] = (lib_u8)(index >> 12u);
    image[16u + 32768u - 4u] = 0u;
    image[16u + 32768u - 3u] = 0x80u;
    assert(core_machine_create(&machine, image, sizeof(image), &options) == LIB_STATUS_OK);

    assert(core_bus_read(machine, 0x8000u, &value) == LIB_STATUS_OK && value == 0u);
    assert(core_bus_read(machine, 0xc000u, &value) == LIB_STATUS_OK && value == 1u);
    write_serial(machine, 0x8000u, 0x1eu);
    write_serial(machine, 0xa000u, 3u);
    write_serial(machine, 0xc000u, 6u);
    assert(ppu_read(machine, 0u) == 3u);
    assert(ppu_read(machine, 0x1000u) == 6u);
    write_serial(machine, 0xe000u, 1u);
    assert(core_bus_read(machine, 0x8000u, &value) == LIB_STATUS_OK && value == 1u);
    write_serial(machine, 0x8000u, 2u);
    ppu_address(machine, 0x2000u);
    core_ppu_cpu_write(&machine->ppu, machine->cartridge, 7u, 0x5au);
    assert(ppu_read(machine, 0x2800u) == 0x5au);
    assert(ppu_read(machine, 0x2400u) != 0x5au);

    core_machine_destroy(machine);
    owner_local_header_load();
    return 0;
}

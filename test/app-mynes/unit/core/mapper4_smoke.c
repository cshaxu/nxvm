#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "core/bus.h"
#include "core/driver.h"
#include "core/machine.h"

#define MAPPER4_IMAGE_BYTES (16u + 131072u + 131072u)
#define MAPPER4_CHR_RAM_IMAGE_BYTES (16u + 32768u)

static void make_image(lib_u8 *image)
{
    lib_size index;

    memset(image, 0, MAPPER4_IMAGE_BYTES);
    image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1au;
    image[4] = 8u; image[5] = 16u; image[6] = 0x43u; image[8] = 1u;
    for (index = 0u; index < 131072u; ++index)
        image[16u + index] = (lib_u8)(index >> 13u);
    for (index = 0u; index < 131072u; ++index)
        image[16u + 131072u + index] = (lib_u8)(index >> 10u);
    image[16u + 131072u - 4u] = 0u;
    image[16u + 131072u - 3u] = 0x80u;
}

static void make_chr_ram_image(lib_u8 *image)
{
    memset(image, 0, MAPPER4_CHR_RAM_IMAGE_BYTES);
    image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1au;
    image[4] = 2u; image[5] = 0u; image[6] = 0x40u;
    image[MAPPER4_CHR_RAM_IMAGE_BYTES - 4u] = 0u;
    image[MAPPER4_CHR_RAM_IMAGE_BYTES - 3u] = 0x80u;
}

static void mapper_write(core_machine *machine, lib_u16 address, lib_u8 value)
{
    assert(core_bus_write(machine, address, value) == LIB_STATUS_OK);
}

static lib_u8 bus_read(core_machine *machine, lib_u16 address)
{
    lib_u8 value;
    assert(core_bus_read(machine, address, &value) == LIB_STATUS_OK);
    return value;
}

static lib_u8 ppu_read(core_machine *machine, lib_u16 address)
{
    core_ppu_cpu_write(&machine->ppu, machine->cartridge, 6u,
        (lib_u8)(address >> 8u));
    core_ppu_cpu_write(&machine->ppu, machine->cartridge, 6u, (lib_u8)address);
    (void)core_ppu_cpu_read(&machine->ppu, machine->cartridge, 7u);
    return core_ppu_cpu_read(&machine->ppu, machine->cartridge, 7u);
}

static void qualified_edge(core_cartridge *cartridge)
{
    lib_u8 index;

    for (index = 0u; index < 8u; ++index)
        core_cartridge_ppu_a12_tick(cartridge, LIB_FALSE);
    core_cartridge_ppu_a12_tick(cartridge, LIB_TRUE);
}

static void owner_local_load(void)
{
    const char *path = getenv("MYNES_OWNER_MMC3_ROM");
    core_driver *driver = LIB_NULL;

    if (path == LIB_NULL || path[0] == '\0') return;
    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_set_media(driver, path, LIB_STORAGE_MEDIUM_READONLY));
    assert(driver->machine != LIB_NULL && driver->machine->cartridge->mapper == 4u);
    assert(driver->machine->cartridge->prg_bytes >= 32768u &&
        driver->machine->cartridge->prg_bytes <= 524288u);
    assert(driver->machine->cartridge->chr_bytes >= 8192u &&
        driver->machine->cartridge->chr_bytes <= 262144u);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
}

int main(void)
{
    lib_u8 image[MAPPER4_IMAGE_BYTES];
    lib_u8 chr_ram_image[MAPPER4_CHR_RAM_IMAGE_BYTES];
    lib_u8 saved_ram[8192];
    core_machine *machine = LIB_NULL;
    core_machine_options options = { 0 };
    lib_u32 index;

    make_image(image);
    assert(core_machine_create(&machine, image, sizeof(image), &options) == LIB_STATUS_OK);
    assert(bus_read(machine, 0x8000u) == 0u);
    assert(bus_read(machine, 0xa000u) == 0u);
    assert(bus_read(machine, 0xc000u) == 14u);
    assert(bus_read(machine, 0xe000u) == 15u);

    mapper_write(machine, 0x8000u, 6u);
    mapper_write(machine, 0x8001u, 5u);
    mapper_write(machine, 0x8000u, 7u);
    mapper_write(machine, 0x8001u, 6u);
    assert(bus_read(machine, 0x8000u) == 5u);
    assert(bus_read(machine, 0xa000u) == 6u);
    mapper_write(machine, 0x8000u, 0x46u);
    assert(bus_read(machine, 0x8000u) == 14u);
    assert(bus_read(machine, 0xc000u) == 5u);

    mapper_write(machine, 0x8000u, 0u);
    mapper_write(machine, 0x8001u, 4u);
    mapper_write(machine, 0x8000u, 2u);
    mapper_write(machine, 0x8001u, 7u);
    assert(ppu_read(machine, 0x0000u) == 4u);
    assert(ppu_read(machine, 0x0400u) == 5u);
    assert(ppu_read(machine, 0x1000u) == 7u);
    mapper_write(machine, 0x8000u, 0x80u);
    assert(ppu_read(machine, 0x1000u) == 4u);

    mapper_write(machine, 0xa000u, 0u);
    assert(core_cartridge_ciram_address(machine->cartridge, 0x2000u) ==
        core_cartridge_ciram_address(machine->cartridge, 0x2800u));
    mapper_write(machine, 0xa000u, 1u);
    assert(core_cartridge_ciram_address(machine->cartridge, 0x2000u) ==
        core_cartridge_ciram_address(machine->cartridge, 0x2400u));

    mapper_write(machine, 0xa001u, 0x80u);
    mapper_write(machine, 0x6000u, 0x5au);
    assert(bus_read(machine, 0x6000u) == 0x5au);
    assert(machine->cartridge->battery_backed && machine->cartridge->prg_ram_dirty);
    assert(core_cartridge_battery_ram_byte_count(machine->cartridge) == sizeof(saved_ram));
    assert(core_cartridge_export_battery_ram(machine->cartridge, saved_ram,
        sizeof(saved_ram)) == LIB_STATUS_OK && saved_ram[0] == 0x5au);
    saved_ram[0] = 0x3cu;
    assert(core_cartridge_import_battery_ram(machine->cartridge, saved_ram,
        sizeof(saved_ram)) == LIB_STATUS_OK && !machine->cartridge->prg_ram_dirty);
    assert(bus_read(machine, 0x6000u) == 0x3cu);
    mapper_write(machine, 0xa001u, 0xc0u);
    mapper_write(machine, 0x6000u, 0xa5u);
    assert(bus_read(machine, 0x6000u) == 0x3cu);
    mapper_write(machine, 0xa001u, 0u);
    assert(bus_read(machine, 0x6000u) == 0u);

    mapper_write(machine, 0xc000u, 2u);
    mapper_write(machine, 0xc001u, 0u);
    mapper_write(machine, 0xe001u, 0u);
    qualified_edge(machine->cartridge);
    assert(!core_cartridge_irq_asserted(machine->cartridge));
    qualified_edge(machine->cartridge);
    assert(!core_cartridge_irq_asserted(machine->cartridge));
    qualified_edge(machine->cartridge);
    assert(core_cartridge_irq_asserted(machine->cartridge));
    (void)bus_read(machine, 0u);
    assert(machine->irq_asserted);
    mapper_write(machine, 0xe000u, 0u);
    assert(!core_cartridge_irq_asserted(machine->cartridge));
    assert(!machine->cartridge->mmc3_irq_enabled);
    assert(!machine->irq_asserted);

    /* $C001 clears a live counter.  Its latch is not visible until the next
     * qualified A12 edge, so it cannot inherit a prior partial countdown. */
    mapper_write(machine, 0xc000u, 3u);
    mapper_write(machine, 0xc001u, 0u);
    qualified_edge(machine->cartridge);
    qualified_edge(machine->cartridge);
    assert(machine->cartridge->mmc3_irq_counter == 2u);
    mapper_write(machine, 0xc001u, 0u);
    assert(machine->cartridge->mmc3_irq_counter == 0u);
    assert(machine->cartridge->mmc3_irq_reload);
    qualified_edge(machine->cartridge);
    assert(machine->cartridge->mmc3_irq_counter == 3u);
    assert(!machine->cartridge->mmc3_irq_reload);

    /* $2006 exposes the completed PPU address on the cartridge bus.  After
     * eight low dots, a $0000->$1000 address transition is a qualified MMC3
     * rising edge even before rendering starts. */
    mapper_write(machine, 0xc000u, 0u);
    mapper_write(machine, 0xc001u, 0u);
    mapper_write(machine, 0xe001u, 0u);
    core_ppu_cpu_write(&machine->ppu, machine->cartridge, 6u, 0u);
    core_ppu_cpu_write(&machine->ppu, machine->cartridge, 6u, 0u);
    for (index = 0u; index < 8u; ++index)
        core_ppu_tick(&machine->ppu, machine->cartridge);
    core_ppu_cpu_write(&machine->ppu, machine->cartridge, 6u, 0x10u);
    core_ppu_cpu_write(&machine->ppu, machine->cartridge, 6u, 0u);
    assert(core_cartridge_irq_asserted(machine->cartridge));

    core_machine_destroy(machine);
    make_chr_ram_image(chr_ram_image);
    assert(core_machine_create(&machine, chr_ram_image, sizeof(chr_ram_image),
        &options) == LIB_STATUS_OK);
    core_cartridge_ppu_write(machine->cartridge, 0x0000u, 0x5au);
    core_cartridge_ppu_write(machine->cartridge, 0x1fffu, 0xa5u);
    assert(core_cartridge_ppu_read(machine->cartridge, 0x0000u) == 0x5au);
    assert(core_cartridge_ppu_read(machine->cartridge, 0x1fffu) == 0xa5u);
    core_machine_destroy(machine);
    owner_local_load();
    return 0;
}

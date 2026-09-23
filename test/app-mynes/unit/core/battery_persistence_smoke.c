#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "core/bus.h"
#include "core/driver.h"

#define IMAGE_BYTES (16u + 131072u + 131072u)

static void write_image(const char *path, lib_u8 distinguishing_byte)
{
    lib_u8 image[IMAGE_BYTES];
    FILE *file;

    memset(image, 0, sizeof(image));
    image[0] = 'N'; image[1] = 'E'; image[2] = 'S'; image[3] = 0x1au;
    image[4] = 8u; image[5] = 16u; image[6] = 0x43u; image[8] = 1u;
    image[16u] = distinguishing_byte;
    image[16u + 131072u - 4u] = 0u;
    image[16u + 131072u - 3u] = 0x80u;
    file = fopen(path, "wb");
    assert(file != NULL);
    assert(fwrite(image, 1u, sizeof(image), file) == sizeof(image));
    assert(fclose(file) == 0);
}

static core_driver *load_driver(const char *path)
{
    core_driver *driver = LIB_NULL;

    assert(core_driver_create(&driver, &(core_driver_options) { 0 }) == LIB_STATUS_OK);
    assert(core_driver_set_media(driver, path, LIB_STORAGE_MEDIUM_READONLY));
    assert(core_bus_write(driver->machine, 0xa001u, 0x80u) == LIB_STATUS_OK);
    return driver;
}

static lib_u8 ram_at(core_driver *driver)
{
    lib_u8 value = 0u;
    assert(core_bus_read(driver->machine, 0x6000u, &value) == LIB_STATUS_OK);
    return value;
}

static void write_damaged_save(const char *path)
{
    FILE *file = fopen(path, "wb");
    assert(file != NULL);
    assert(fwrite("bad", 1u, 3u, file) == 3u);
    assert(fclose(file) == 0);
}

int main(void)
{
    const char *rom_a = "mynes-battery-persistence-a.nes";
    const char *rom_b = "mynes-battery-persistence-b.nes";
    const char *save = "mynes-battery-persistence.sav";
    core_driver *driver;

    (void)remove(rom_a); (void)remove(rom_b); (void)remove(save);
    write_image(rom_a, 0x11u);
    write_image(rom_b, 0x22u);
    driver = load_driver(rom_a);
    assert(core_driver_save_battery_ram(driver, save) == LIB_STATUS_INVALID_STATE);
    assert(core_bus_write(driver->machine, 0x6000u, 0x5au) == LIB_STATUS_OK);
    assert(core_driver_save_battery_ram(driver, save) == LIB_STATUS_OK);
    assert(core_bus_write(driver->machine, 0x6000u, 0x3cu) == LIB_STATUS_OK);
    assert(remove(save) == 0);
    assert(core_driver_load_battery_ram(driver, save) != LIB_STATUS_OK);
    assert(ram_at(driver) == 0x3cu);
    assert(core_bus_write(driver->machine, 0x6000u, 0x5au) == LIB_STATUS_OK);
    assert(core_driver_save_battery_ram(driver, save) == LIB_STATUS_OK);
    assert(core_bus_write(driver->machine, 0x6000u, 0x3cu) == LIB_STATUS_OK);
    write_damaged_save(save);
    assert(core_driver_load_battery_ram(driver, save) != LIB_STATUS_OK);
    assert(ram_at(driver) == 0x3cu);
    assert(core_bus_write(driver->machine, 0x6000u, 0x5au) == LIB_STATUS_OK);
    assert(core_driver_save_battery_ram(driver, save) == LIB_STATUS_OK);
    assert(core_bus_write(driver->machine, 0x6000u, 0x3cu) == LIB_STATUS_OK);
    assert(core_driver_load_battery_ram(driver, save) == LIB_STATUS_OK);
    assert(ram_at(driver) == 0x5au);
    assert(!driver->machine->cartridge->prg_ram_dirty);

    assert(core_driver_set_media(driver, rom_b, LIB_STORAGE_MEDIUM_READONLY));
    assert(core_bus_write(driver->machine, 0xa001u, 0x80u) == LIB_STATUS_OK);
    assert(core_driver_load_battery_ram(driver, save) == LIB_STATUS_INVALID_ARGUMENT);
    assert(ram_at(driver) == 0u);
    assert(core_driver_destroy(driver) == LIB_STATUS_OK);
    assert(remove(rom_a) == 0 && remove(rom_b) == 0 && remove(save) == 0);
    return 0;
}

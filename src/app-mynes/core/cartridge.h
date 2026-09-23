#ifndef CORE_CARTRIDGE_H
#define CORE_CARTRIDGE_H

#include "lib/types/types_interface.h"

typedef struct core_cartridge {
    lib_u8 *prg;
    lib_u8 *chr;
    lib_u8 *prg_ram;
    lib_size prg_bytes;
    lib_size chr_bytes;
    lib_u8 mapper;
    lib_u64 content_identity;
    lib_u8 mirroring;
    lib_u8 uxrom_prg_bank;
    lib_u8 cnrom_chr_bank;
    lib_u8 mmc1_control;
    lib_u8 mmc1_chr_bank0;
    lib_u8 mmc1_chr_bank1;
    lib_u8 mmc1_prg_bank;
    lib_u8 mmc1_shift_data;
    lib_u8 mmc1_shift_count;
    lib_u8 mmc3_bank_select;
    lib_u8 mmc3_bank_data[8];
    lib_u8 mmc3_irq_latch;
    lib_u8 mmc3_irq_counter;
    lib_u8 mmc3_a12_low_ticks;
    lib_bool mmc3_irq_reload;
    lib_bool mmc3_irq_enabled;
    lib_bool mmc3_irq_asserted;
    lib_bool mmc3_a12_high;
    lib_bool mmc3_prg_ram_enabled;
    lib_bool mmc3_prg_ram_protected;
    lib_bool battery_backed;
    lib_bool prg_ram_dirty;
    lib_bool chr_ram;
} core_cartridge;

/* This is the largest finite M5 iNES payload plus the only accepted legacy
 * display suffix. Media uses it only as a read bound; cartridge still validates
 * the exact descriptor before it allocates or publishes a machine. */
lib_size core_cartridge_maximum_image_bytes(void);
lib_bool core_cartridge_normalize_ines_size(const lib_u8 *bytes,
    lib_size *in_out_byte_count);
lib_status core_cartridge_create(core_cartridge **out_cartridge,
    const lib_u8 *bytes, lib_size byte_count);
void core_cartridge_destroy(core_cartridge *cartridge);
lib_u8 core_cartridge_cpu_read(const core_cartridge *cartridge, lib_u16 address);
lib_bool core_cartridge_cpu_write(core_cartridge *cartridge, lib_u16 address,
    lib_u8 value);
lib_u8 core_cartridge_ppu_read(const core_cartridge *cartridge, lib_u16 address);
lib_bool core_cartridge_ppu_write(core_cartridge *cartridge, lib_u16 address,
    lib_u8 value);
lib_u16 core_cartridge_ciram_address(const core_cartridge *cartridge,
    lib_u16 address);
void core_cartridge_ppu_a12_tick(core_cartridge *cartridge, lib_bool high);
lib_bool core_cartridge_irq_asserted(const core_cartridge *cartridge);
lib_size core_cartridge_battery_ram_byte_count(const core_cartridge *cartridge);
lib_u64 core_cartridge_battery_identity(const core_cartridge *cartridge);
lib_status core_cartridge_export_battery_ram(const core_cartridge *cartridge,
    void *bytes, lib_size byte_count);
lib_status core_cartridge_import_battery_ram(core_cartridge *cartridge,
    const void *bytes, lib_size byte_count);

#endif

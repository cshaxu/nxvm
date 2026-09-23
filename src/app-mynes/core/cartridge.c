#include "core/cartridge.h"

#define CORE_CARTRIDGE_HEADER_BYTES 16u
#define CORE_CARTRIDGE_PRG_BANK_BYTES 16384u
#define CORE_CARTRIDGE_CHR_BANK_BYTES 8192u
#define CORE_CARTRIDGE_CHR_PAGE_BYTES 4096u
#define CORE_CARTRIDGE_LEGACY_SUFFIX_BYTES 128u

enum {
    CORE_CARTRIDGE_MAPPER_NROM = 0u,
    CORE_CARTRIDGE_MAPPER_MMC1 = 1u,
    CORE_CARTRIDGE_MAPPER_UXROM = 2u,
    CORE_CARTRIDGE_MAPPER_CNROM = 3u,
    CORE_CARTRIDGE_MAPPER_MMC3 = 4u,
    CORE_CARTRIDGE_MIRROR_SINGLE_LOW = 0u,
    CORE_CARTRIDGE_MIRROR_SINGLE_HIGH = 1u,
    CORE_CARTRIDGE_MIRROR_VERTICAL = 2u,
    CORE_CARTRIDGE_MIRROR_HORIZONTAL = 3u
};

typedef enum core_cartridge_profile {
    CORE_CARTRIDGE_PROFILE_NROM,
    CORE_CARTRIDGE_PROFILE_MMC1,
    CORE_CARTRIDGE_PROFILE_UXROM,
    CORE_CARTRIDGE_PROFILE_CNROM,
    CORE_CARTRIDGE_PROFILE_MMC3
} core_cartridge_profile;

typedef struct core_cartridge_descriptor {
    lib_size prg_bytes;
    lib_size chr_bytes;
    lib_u8 mapper;
    lib_u8 mirroring;
    core_cartridge_profile profile;
    lib_bool chr_ram;
    lib_bool battery_backed;
} core_cartridge_descriptor;

static lib_bool core_cartridge_profile_select(const lib_u8 *bytes,
    lib_u8 mapper, core_cartridge_descriptor *out_descriptor)
{
    core_cartridge_profile profile;

    if (out_descriptor == LIB_NULL) return LIB_FALSE;
    if (mapper == CORE_CARTRIDGE_MAPPER_NROM) {
        if ((bytes[4] != 1u && bytes[4] != 2u) || bytes[5] > 1u) return LIB_FALSE;
        profile = CORE_CARTRIDGE_PROFILE_NROM;
    } else if (mapper == CORE_CARTRIDGE_MAPPER_MMC1) {
        if (bytes[4] != 2u || bytes[5] != 4u) return LIB_FALSE;
        profile = CORE_CARTRIDGE_PROFILE_MMC1;
    } else if (mapper == 2u) {
        if (bytes[4] != 8u || bytes[5] != 0u) return LIB_FALSE;
        profile = CORE_CARTRIDGE_PROFILE_UXROM;
    } else if (mapper == 3u) {
        if (bytes[4] != 2u || bytes[5] == 0u || bytes[5] > 4u) return LIB_FALSE;
        profile = CORE_CARTRIDGE_PROFILE_CNROM;
    } else if (mapper == CORE_CARTRIDGE_MAPPER_MMC3) {
        if (bytes[4] < 2u || bytes[4] > 32u || bytes[5] > 32u) return LIB_FALSE;
        profile = CORE_CARTRIDGE_PROFILE_MMC3;
    } else return LIB_FALSE;
    out_descriptor->prg_bytes = (lib_size)bytes[4] * CORE_CARTRIDGE_PRG_BANK_BYTES;
    out_descriptor->chr_bytes = (lib_size)bytes[5] * CORE_CARTRIDGE_CHR_BANK_BYTES;
    out_descriptor->mapper = mapper;
    out_descriptor->mirroring = (bytes[6] & 1u) != 0u ?
        CORE_CARTRIDGE_MIRROR_VERTICAL : CORE_CARTRIDGE_MIRROR_HORIZONTAL;
    out_descriptor->profile = profile;
    out_descriptor->chr_ram = bytes[5] == 0u;
    return LIB_TRUE;
}

static lib_bool core_cartridge_descriptor_create(const lib_u8 *bytes,
    lib_size byte_count, core_cartridge_descriptor *out_descriptor)
{
    lib_u8 mapper;
    lib_size payload;
    lib_size index;

    if (bytes == LIB_NULL || out_descriptor == LIB_NULL ||
        byte_count < CORE_CARTRIDGE_HEADER_BYTES) return LIB_FALSE;
    if (bytes[0] != 0x4eu || bytes[1] != 0x45u ||
        bytes[2] != 0x53u || bytes[3] != 0x1au) return LIB_FALSE;
    if ((bytes[7] & 0x0cu) == 0x08u || (bytes[6] & 0x0cu) != 0u ||
        (bytes[7] & 0x0fu) != 0u) return LIB_FALSE;
    for (index = 9u; index < CORE_CARTRIDGE_HEADER_BYTES; ++index)
        if (bytes[index] != 0u) return LIB_FALSE;
    mapper = (lib_u8)((bytes[6] >> 4u) | (bytes[7] & 0xf0u));
    if (bytes[8] != 0u && (mapper != CORE_CARTRIDGE_MAPPER_MMC3 ||
        bytes[8] != 1u)) return LIB_FALSE;
    if (!core_cartridge_profile_select(bytes, mapper, out_descriptor)) return LIB_FALSE;
    if ((bytes[6] & 2u) != 0u && (mapper != CORE_CARTRIDGE_MAPPER_MMC3 ||
        bytes[8] != 1u)) return LIB_FALSE;
    out_descriptor->battery_backed = (bytes[6] & 2u) != 0u;
    if (out_descriptor->prg_bytes > LIB_SIZE_MAX - CORE_CARTRIDGE_HEADER_BYTES ||
        out_descriptor->chr_bytes > LIB_SIZE_MAX - CORE_CARTRIDGE_HEADER_BYTES -
            out_descriptor->prg_bytes) return LIB_FALSE;
    payload = CORE_CARTRIDGE_HEADER_BYTES + out_descriptor->prg_bytes +
        out_descriptor->chr_bytes;
    return payload == byte_count;
}

static lib_u64 core_cartridge_content_identity(const lib_u8 *bytes,
    lib_size byte_count)
{
    lib_u64 value = 1469598103934665603ull;
    lib_size index;

    for (index = 0u; index < byte_count; ++index)
        value = (value ^ bytes[index]) * 1099511628211ull;
    return value;
}

lib_size core_cartridge_maximum_image_bytes(void)
{
    return CORE_CARTRIDGE_HEADER_BYTES + 32u * CORE_CARTRIDGE_PRG_BANK_BYTES +
        32u * CORE_CARTRIDGE_CHR_BANK_BYTES + CORE_CARTRIDGE_LEGACY_SUFFIX_BYTES;
}

lib_bool core_cartridge_normalize_ines_size(const lib_u8 *bytes,
    lib_size *in_out_byte_count)
{
    lib_size payload;

    if (bytes == LIB_NULL || in_out_byte_count == LIB_NULL ||
        *in_out_byte_count < CORE_CARTRIDGE_HEADER_BYTES ||
        bytes[0] != 'N' || bytes[1] != 'E' || bytes[2] != 'S' || bytes[3] != 0x1au)
        return LIB_FALSE;
    payload = CORE_CARTRIDGE_HEADER_BYTES + (lib_size)bytes[4] *
        CORE_CARTRIDGE_PRG_BANK_BYTES + (lib_size)bytes[5] * CORE_CARTRIDGE_CHR_BANK_BYTES;
    if (*in_out_byte_count == payload) return LIB_TRUE;
    if (*in_out_byte_count != payload + CORE_CARTRIDGE_LEGACY_SUFFIX_BYTES - 1u &&
        *in_out_byte_count != payload + CORE_CARTRIDGE_LEGACY_SUFFIX_BYTES) return LIB_FALSE;
    *in_out_byte_count = payload;
    return LIB_TRUE;
}

lib_status core_cartridge_create(core_cartridge **out_cartridge,
    const lib_u8 *bytes, lib_size byte_count)
{
    core_cartridge *cartridge;
    core_cartridge_descriptor descriptor;

    if (out_cartridge == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_cartridge = LIB_NULL;
    if (!core_cartridge_descriptor_create(bytes, byte_count, &descriptor))
        return LIB_STATUS_INVALID_ARGUMENT;
    cartridge = lib_allocate_zero(1u, sizeof(*cartridge));
    if (cartridge == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    cartridge->prg = lib_allocate(descriptor.prg_bytes);
    cartridge->chr = descriptor.chr_ram ?
        lib_allocate_zero(1u, CORE_CARTRIDGE_CHR_BANK_BYTES) :
        lib_allocate(descriptor.chr_bytes);
    cartridge->prg_ram = descriptor.profile == CORE_CARTRIDGE_PROFILE_MMC3 ?
        lib_allocate_zero(1u, 8192u) : LIB_NULL;
    if (cartridge->prg == LIB_NULL || cartridge->chr == LIB_NULL ||
        (descriptor.profile == CORE_CARTRIDGE_PROFILE_MMC3 && cartridge->prg_ram == LIB_NULL)) {
        core_cartridge_destroy(cartridge);
        return LIB_STATUS_NO_MEMORY;
    }
    lib_memory_copy(cartridge->prg, bytes + CORE_CARTRIDGE_HEADER_BYTES,
        descriptor.prg_bytes);
    if (!descriptor.chr_ram) lib_memory_copy(cartridge->chr,
        bytes + CORE_CARTRIDGE_HEADER_BYTES + descriptor.prg_bytes,
        descriptor.chr_bytes);
    cartridge->prg_bytes = descriptor.prg_bytes;
    cartridge->chr_bytes = descriptor.chr_ram ? CORE_CARTRIDGE_CHR_BANK_BYTES :
        descriptor.chr_bytes;
    cartridge->chr_ram = descriptor.chr_ram;
    cartridge->battery_backed = descriptor.battery_backed;
    cartridge->mapper = descriptor.mapper;
    cartridge->mirroring = descriptor.mirroring;
    cartridge->content_identity = core_cartridge_content_identity(bytes, byte_count);
    cartridge->mmc1_control = 0x0cu;
    *out_cartridge = cartridge;
    return LIB_STATUS_OK;
}

void core_cartridge_destroy(core_cartridge *cartridge)
{
    if (cartridge == LIB_NULL) return;
    lib_release(cartridge->prg);
    lib_release(cartridge->chr);
    lib_release(cartridge->prg_ram);
    lib_release(cartridge);
}

lib_u8 core_cartridge_cpu_read(const core_cartridge *cartridge, lib_u16 address)
{
    lib_size offset;
    lib_size banks;
    lib_size bank;

    if (cartridge == LIB_NULL) return 0u;
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_MMC3 &&
        address >= 0x6000u && address < 0x8000u)
        return cartridge->mmc3_prg_ram_enabled ? cartridge->prg_ram[address & 0x1fffu] : 0u;
    if (address < 0x8000u) return 0u;
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_NROM) {
        offset = (lib_size)(address - 0x8000u);
        if (cartridge->prg_bytes == CORE_CARTRIDGE_PRG_BANK_BYTES)
            offset %= CORE_CARTRIDGE_PRG_BANK_BYTES;
        return cartridge->prg[offset];
    }
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_UXROM) {
        banks = cartridge->prg_bytes / CORE_CARTRIDGE_PRG_BANK_BYTES;
        bank = address < 0xc000u ?
            (lib_size)(cartridge->uxrom_prg_bank % banks) : banks - 1u;
        return cartridge->prg[bank * CORE_CARTRIDGE_PRG_BANK_BYTES +
            (address & 0x3fffu)];
    }
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_CNROM)
        return cartridge->prg[address - 0x8000u];
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_MMC3) {
        banks = cartridge->prg_bytes / 8192u;
        if (address < 0xa000u)
            bank = (cartridge->mmc3_bank_select & 0x40u) != 0u ? banks - 2u :
                (lib_size)(cartridge->mmc3_bank_data[6u] % banks);
        else if (address < 0xc000u) bank = cartridge->mmc3_bank_data[7u] % banks;
        else if (address < 0xe000u)
            bank = (cartridge->mmc3_bank_select & 0x40u) != 0u ?
                (lib_size)(cartridge->mmc3_bank_data[6u] % banks) : banks - 2u;
        else bank = banks - 1u;
        return cartridge->prg[bank * 8192u + (address & 0x1fffu)];
    }
    banks = cartridge->prg_bytes / CORE_CARTRIDGE_PRG_BANK_BYTES;
    if ((cartridge->mmc1_control & 0x0cu) <= 4u) {
        bank = ((lib_size)(cartridge->mmc1_prg_bank & 0x0eu) % banks) *
            CORE_CARTRIDGE_PRG_BANK_BYTES;
        offset = bank + (address - 0x8000u);
    } else if ((cartridge->mmc1_control & 0x0cu) == 8u) {
        bank = address < 0xc000u ? 0u :
            ((lib_size)(cartridge->mmc1_prg_bank & 0x0fu) % banks);
        offset = bank * CORE_CARTRIDGE_PRG_BANK_BYTES + (address & 0x3fffu);
    } else {
        bank = address < 0xc000u ?
            ((lib_size)(cartridge->mmc1_prg_bank & 0x0fu) % banks) : banks - 1u;
        offset = bank * CORE_CARTRIDGE_PRG_BANK_BYTES + (address & 0x3fffu);
    }
    return cartridge->prg[offset];
}

lib_bool core_cartridge_cpu_write(core_cartridge *cartridge, lib_u16 address,
    lib_u8 value)
{
    lib_u8 *target;

    if (cartridge == LIB_NULL) return LIB_FALSE;
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_MMC3 &&
        address >= 0x6000u && address < 0x8000u) {
        if (cartridge->mmc3_prg_ram_enabled && !cartridge->mmc3_prg_ram_protected) {
            cartridge->prg_ram[address & 0x1fffu] = value;
            if (cartridge->battery_backed) cartridge->prg_ram_dirty = LIB_TRUE;
        }
        return LIB_TRUE;
    }
    if (address < 0x8000u) return LIB_FALSE;
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_NROM) return LIB_TRUE;
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_UXROM) {
        lib_u8 rom_value = core_cartridge_cpu_read(cartridge, address);
        lib_size banks = cartridge->prg_bytes / CORE_CARTRIDGE_PRG_BANK_BYTES;

        cartridge->uxrom_prg_bank = (lib_u8)((value & rom_value) % banks);
        return LIB_TRUE;
    }
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_CNROM) {
        lib_u8 rom_value = core_cartridge_cpu_read(cartridge, address);
        lib_size banks = cartridge->chr_bytes / CORE_CARTRIDGE_CHR_BANK_BYTES;

        cartridge->cnrom_chr_bank = (lib_u8)((value & rom_value) % banks);
        return LIB_TRUE;
    }
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_MMC3) {
        if (address < 0xa000u) {
            if ((address & 1u) == 0u) cartridge->mmc3_bank_select = value;
            else cartridge->mmc3_bank_data[cartridge->mmc3_bank_select & 7u] = value;
        } else if (address < 0xc000u) {
            if ((address & 1u) == 0u)
                cartridge->mirroring = (value & 1u) != 0u ?
                    CORE_CARTRIDGE_MIRROR_HORIZONTAL : CORE_CARTRIDGE_MIRROR_VERTICAL;
            else {
                cartridge->mmc3_prg_ram_enabled = (value & 0x80u) != 0u;
                cartridge->mmc3_prg_ram_protected = (value & 0x40u) != 0u;
            }
        } else if (address < 0xe000u) {
            if ((address & 1u) == 0u) cartridge->mmc3_irq_latch = value;
            else cartridge->mmc3_irq_reload = LIB_TRUE;
        } else if ((address & 1u) == 0u) {
            cartridge->mmc3_irq_enabled = LIB_FALSE;
            cartridge->mmc3_irq_asserted = LIB_FALSE;
        } else cartridge->mmc3_irq_enabled = LIB_TRUE;
        return LIB_TRUE;
    }
    if ((value & 0x80u) != 0u) {
        cartridge->mmc1_shift_data = 0u;
        cartridge->mmc1_shift_count = 0u;
        cartridge->mmc1_control |= 0x0cu;
        return LIB_TRUE;
    }
    cartridge->mmc1_shift_data |= (lib_u8)((value & 1u) << cartridge->mmc1_shift_count);
    ++cartridge->mmc1_shift_count;
    if (cartridge->mmc1_shift_count != 5u) return LIB_TRUE;
    if (address < 0xa000u) target = &cartridge->mmc1_control;
    else if (address < 0xc000u) target = &cartridge->mmc1_chr_bank0;
    else if (address < 0xe000u) target = &cartridge->mmc1_chr_bank1;
    else target = &cartridge->mmc1_prg_bank;
    *target = cartridge->mmc1_shift_data;
    cartridge->mmc1_shift_data = 0u;
    cartridge->mmc1_shift_count = 0u;
    if (target == &cartridge->mmc1_control)
        cartridge->mirroring = cartridge->mmc1_control & 3u;
    return LIB_TRUE;
}

static lib_size core_cartridge_mmc3_chr_offset(const core_cartridge *cartridge,
    lib_u16 address)
{
    lib_u8 register_index;
    lib_u8 bank;

    if ((cartridge->mmc3_bank_select & 0x80u) == 0u) {
        if (address < 0x0800u) register_index = 0u;
        else if (address < 0x1000u) register_index = 1u;
        else register_index = (lib_u8)(2u + ((address - 0x1000u) >> 10u));
    } else {
        if (address < 0x1000u) register_index = (lib_u8)(2u + (address >> 10u));
        else if (address < 0x1800u) register_index = 0u;
        else register_index = 1u;
    }
    bank = cartridge->mmc3_bank_data[register_index];
    if (register_index < 2u)
        bank = (lib_u8)((bank & 0xfeu) + ((address >> 10u) & 1u));
    return ((lib_size)bank % (cartridge->chr_bytes / 1024u)) * 1024u +
        (address & 0x03ffu);
}

lib_u8 core_cartridge_ppu_read(const core_cartridge *cartridge, lib_u16 address)
{
    lib_size pages;
    lib_size page;
    if (cartridge == LIB_NULL || address >= 0x2000u) return 0u;
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_NROM ||
        cartridge->mapper == CORE_CARTRIDGE_MAPPER_UXROM) return cartridge->chr[address];
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_CNROM) {
        return cartridge->chr[(lib_size)cartridge->cnrom_chr_bank *
            CORE_CARTRIDGE_CHR_BANK_BYTES + address];
    }
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_MMC3)
        return cartridge->chr[core_cartridge_mmc3_chr_offset(cartridge, address)];
    pages = cartridge->chr_bytes / CORE_CARTRIDGE_CHR_PAGE_BYTES;
    if ((cartridge->mmc1_control & 0x10u) == 0u)
        page = ((lib_size)(cartridge->mmc1_chr_bank0 & 0x1eu) % pages) +
            (address >= 0x1000u ? 1u : 0u);
    else page = (lib_size)(address < 0x1000u ? cartridge->mmc1_chr_bank0 :
        cartridge->mmc1_chr_bank1) % pages;
    return cartridge->chr[page * CORE_CARTRIDGE_CHR_PAGE_BYTES + (address & 0x0fffu)];
}

lib_bool core_cartridge_ppu_write(core_cartridge *cartridge, lib_u16 address,
    lib_u8 value)
{
    if (cartridge == LIB_NULL || address >= 0x2000u || !cartridge->chr_ram)
        return LIB_FALSE;
    if (cartridge->mapper == CORE_CARTRIDGE_MAPPER_MMC3)
        cartridge->chr[core_cartridge_mmc3_chr_offset(cartridge, address)] = value;
    else cartridge->chr[address] = value;
    return LIB_TRUE;
}

lib_u16 core_cartridge_ciram_address(const core_cartridge *cartridge,
    lib_u16 address)
{
    lib_u16 table = (lib_u16)((address - 0x2000u) >> 10u);
    lib_u16 offset = (lib_u16)(address & 0x03ffu);
    lib_u8 mirroring = cartridge == LIB_NULL ? CORE_CARTRIDGE_MIRROR_HORIZONTAL :
        cartridge->mirroring;

    if (mirroring == CORE_CARTRIDGE_MIRROR_SINGLE_LOW) table = 0u;
    else if (mirroring == CORE_CARTRIDGE_MIRROR_SINGLE_HIGH) table = 1u;
    else if (mirroring == CORE_CARTRIDGE_MIRROR_VERTICAL) table &= 1u;
    else table >>= 1u;
    return (lib_u16)(table * 1024u + offset);
}

void core_cartridge_ppu_a12_tick(core_cartridge *cartridge, lib_bool high)
{
    if (cartridge == LIB_NULL || cartridge->mapper != CORE_CARTRIDGE_MAPPER_MMC3)
        return;
    if (!high) {
        if (cartridge->mmc3_a12_low_ticks != 255u) ++cartridge->mmc3_a12_low_ticks;
        cartridge->mmc3_a12_high = LIB_FALSE;
        return;
    }
    if (cartridge->mmc3_a12_high || cartridge->mmc3_a12_low_ticks < 8u) return;
    cartridge->mmc3_a12_high = LIB_TRUE;
    cartridge->mmc3_a12_low_ticks = 0u;
    if (cartridge->mmc3_irq_reload || cartridge->mmc3_irq_counter == 0u) {
        cartridge->mmc3_irq_counter = cartridge->mmc3_irq_latch;
        cartridge->mmc3_irq_reload = LIB_FALSE;
    } else --cartridge->mmc3_irq_counter;
    if (cartridge->mmc3_irq_counter == 0u && cartridge->mmc3_irq_enabled)
        cartridge->mmc3_irq_asserted = LIB_TRUE;
}

lib_bool core_cartridge_irq_asserted(const core_cartridge *cartridge)
{
    return cartridge != LIB_NULL && cartridge->mmc3_irq_asserted;
}

lib_size core_cartridge_battery_ram_byte_count(const core_cartridge *cartridge)
{ return cartridge != LIB_NULL && cartridge->battery_backed ? 8192u : 0u; }

lib_u64 core_cartridge_battery_identity(const core_cartridge *cartridge)
{ return core_cartridge_battery_ram_byte_count(cartridge) == 0u ? 0u :
    cartridge->content_identity; }

lib_status core_cartridge_export_battery_ram(const core_cartridge *cartridge,
    void *bytes, lib_size byte_count)
{
    if (bytes == LIB_NULL || byte_count != core_cartridge_battery_ram_byte_count(cartridge))
        return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(bytes, cartridge->prg_ram, byte_count);
    return LIB_STATUS_OK;
}

lib_status core_cartridge_import_battery_ram(core_cartridge *cartridge,
    const void *bytes, lib_size byte_count)
{
    if (bytes == LIB_NULL || byte_count != core_cartridge_battery_ram_byte_count(cartridge))
        return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_copy(cartridge->prg_ram, bytes, byte_count);
    cartridge->prg_ram_dirty = LIB_FALSE;
    return LIB_STATUS_OK;
}

#ifndef TESTS_SUPPORT_VM_MODEL40_BYOB_FIXTURE_H
#define TESTS_SUPPORT_VM_MODEL40_BYOB_FIXTURE_H
#include "lib/types/types_interface.h"

#include "type.h"
#include "app-nxvm/devices/rtc.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/profiles/model40/model40_private.h"

static inline C_VOID vm_model40_fixture_cmos_seed(
    lib_u8 bytes[VM_MACHINE_CMOS_SEED_BYTES])
{
    lib_u16 checksum = 0u;
    lib_u8 index;

    lib_memory_set(bytes, 0, VM_MACHINE_CMOS_SEED_BYTES);
    bytes[CORE_MACHINE_RTC_TYPE_DISK_FLOPPY] = 0x22u;
    bytes[CORE_MACHINE_RTC_TYPE_DISK_FIXED] = 0x80u;
    bytes[CORE_MACHINE_RTC_EQUIPMENT] = 0x41u;
    bytes[CORE_MACHINE_RTC_BASEMEM_LSB] = 0x80u;
    bytes[CORE_MACHINE_RTC_BASEMEM_MSB] = 0x02u;
    bytes[CORE_MACHINE_RTC_EXTMEM_LSB] = 0x00u;
    bytes[CORE_MACHINE_RTC_EXTMEM_MSB] = 0x04u;
    for (index = 0x10u; index < 0x2eu; ++index) checksum =
        (lib_u16)(checksum + bytes[index]);
    bytes[0x2eu] = TYPE_MASK_UNSIGNED_8(checksum >> 8u);
    bytes[0x2fu] = TYPE_MASK_UNSIGNED_8(checksum);
}

static inline type_status vm_model40_fixture_create_bytes_with_floppy_format(
    const lib_u8 *even_bytes, const lib_u8 *odd_bytes,
    vm_machine_floppy_format floppy_format, vm_machine **out_session)
{
    vm_machine_config config = {0};
    lib_u8 cmos_seed[VM_MACHINE_CMOS_SEED_BYTES];

    vm_machine_assets assets = { .bios = {
        { even_bytes, VM_PROFILE_MODEL40_ROM_CHIP_BYTES },
        { odd_bytes, VM_PROFILE_MODEL40_ROM_CHIP_BYTES }
    } };

    /* Unit fixtures deliberately keep their bytes in process memory. */
    vm_model40_fixture_cmos_seed(cmos_seed);
    assets.cmos_seed = (vm_machine_asset_bytes) { cmos_seed, sizeof(cmos_seed) };
    config.profile_kind = VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.bios_count = 2u;
    config.floppy_format = floppy_format;
    return vm_machine_create_from_assets(&config, &assets, out_session);
}

static inline type_status vm_model40_fixture_create_bytes(
    const lib_u8 *even_bytes, const lib_u8 *odd_bytes,
    vm_machine **out_session)
{
    return vm_model40_fixture_create_bytes_with_floppy_format(even_bytes, odd_bytes,
        VM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT, out_session);
}

static inline type_status vm_model40_fixture_create(vm_machine **out_session)
{
    lib_u8 even_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES] = {0};
    lib_u8 odd_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];

    lib_memory_set(odd_bytes, 1, sizeof(odd_bytes));
    return vm_model40_fixture_create_bytes(even_bytes, odd_bytes, out_session);
}

#endif

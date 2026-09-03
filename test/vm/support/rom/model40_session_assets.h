#ifndef TESTS_SUPPORT_VM_MODEL40_BYOB_FIXTURE_H
#define TESTS_SUPPORT_VM_MODEL40_BYOB_FIXTURE_H

#include "type.h"
#include "core/machine/rtc.h"
#include "vm/composition/session/session_interface.h"

static inline C_VOID vm_model40_fixture_cmos_seed(
    type_unsigned_8 bytes[VM_SESSION_CMOS_SEED_BYTES])
{
    type_unsigned_16 checksum = 0u;
    type_unsigned_8 index;

    STD_MEMSET(bytes, 0, VM_SESSION_CMOS_SEED_BYTES);
    bytes[CORE_MACHINE_RTC_TYPE_DISK_FLOPPY] = 0x22u;
    bytes[CORE_MACHINE_RTC_TYPE_DISK_FIXED] = 0x80u;
    bytes[CORE_MACHINE_RTC_EQUIPMENT] = 0x41u;
    bytes[CORE_MACHINE_RTC_BASEMEM_LSB] = 0x80u;
    bytes[CORE_MACHINE_RTC_BASEMEM_MSB] = 0x02u;
    bytes[CORE_MACHINE_RTC_EXTMEM_LSB] = 0x00u;
    bytes[CORE_MACHINE_RTC_EXTMEM_MSB] = 0x04u;
    for (index = 0x10u; index < 0x2eu; ++index) checksum =
        (type_unsigned_16)(checksum + bytes[index]);
    bytes[0x2eu] = TYPE_MASK_UNSIGNED_8(checksum >> 8u);
    bytes[0x2fu] = TYPE_MASK_UNSIGNED_8(checksum);
}

static inline type_status vm_model40_fixture_create_bytes_with_floppy_format(
    const type_unsigned_8 *even_bytes, const type_unsigned_8 *odd_bytes,
    vm_session_floppy_format floppy_format, vm_session **out_session)
{
    vm_session_config config = {0};
    type_unsigned_8 cmos_seed[VM_SESSION_CMOS_SEED_BYTES];

    vm_session_assets assets = { .bios = {
        { even_bytes, VM_PROFILE_MODEL40_ROM_CHIP_BYTES },
        { odd_bytes, VM_PROFILE_MODEL40_ROM_CHIP_BYTES }
    } };

    /* Unit fixtures deliberately keep their bytes in process memory. */
    vm_model40_fixture_cmos_seed(cmos_seed);
    assets.cmos_seed = (vm_session_asset_bytes) { cmos_seed, sizeof(cmos_seed) };
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.bios_count = 2u;
    config.floppy_format = floppy_format;
    return vm_session_create_from_assets(&config, &assets, out_session);
}

static inline type_status vm_model40_fixture_create_bytes(
    const type_unsigned_8 *even_bytes, const type_unsigned_8 *odd_bytes,
    vm_session **out_session)
{
    return vm_model40_fixture_create_bytes_with_floppy_format(even_bytes, odd_bytes,
        VM_SESSION_FLOPPY_FORMAT_PROFILE_DEFAULT, out_session);
}

static inline type_status vm_model40_fixture_create(vm_session **out_session)
{
    type_unsigned_8 even_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES] = {0};
    type_unsigned_8 odd_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];

    STD_MEMSET(odd_bytes, 1, sizeof(odd_bytes));
    return vm_model40_fixture_create_bytes(even_bytes, odd_bytes, out_session);
}

#endif

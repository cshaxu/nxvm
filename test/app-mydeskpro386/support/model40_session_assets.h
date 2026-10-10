#include "core/machine/machine_interface.h"
#ifndef TEST_CORE_MACHINE_QUALIFICATION_MODEL40_SESSION_ASSETS_H
#define TEST_CORE_MACHINE_QUALIFICATION_MODEL40_SESSION_ASSETS_H
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "core/x86/device_support_interface.h"

#include "core/chips/rtc146818/rtc146818_interface.h"
#include "core/machine/machine_interface.h"
#include "app-mydeskpro386/profiles/model40_private.h"
#include "app-mydeskpro386/profiles/construction_interface.h"

static inline lib_status vm_model40_fixture_machine_create_from_assets(
    const vm_machine_config *config, const vm_machine_assets *assets,
    vm_machine **out_session)
{
    vm_machine_construction construction;
    lib_status status;

    if (config == LIB_NULL || assets == LIB_NULL || out_session == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_session = LIB_NULL;
    status = vm_profile_machine_plan_create_model40(config, assets, &construction);
    if (status != LIB_STATUS_OK) return status;
    return vm_machine_create(config, &construction, out_session);
}

static inline void vm_model40_fixture_cmos_seed(
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
    bytes[0x2eu] = CORE_MACHINE_MASK_U8(checksum >> 8u);
    bytes[0x2fu] = CORE_MACHINE_MASK_U8(checksum);
}

static inline lib_status vm_model40_fixture_create_bytes_with_floppy_format(
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
    config.bios_count = 2u;
    config.floppy_format = floppy_format;
    return vm_model40_fixture_machine_create_from_assets(&config, &assets, out_session);
}

static inline lib_status vm_model40_fixture_create_bytes(
    const lib_u8 *even_bytes, const lib_u8 *odd_bytes,
    vm_machine **out_session)
{
    return vm_model40_fixture_create_bytes_with_floppy_format(even_bytes, odd_bytes,
        VM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT, out_session);
}

static inline lib_status vm_model40_fixture_create(vm_machine **out_session)
{
    lib_u8 even_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES] = {0};
    lib_u8 odd_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];

    lib_memory_set(odd_bytes, 1, sizeof(odd_bytes));
    return vm_model40_fixture_create_bytes(even_bytes, odd_bytes, out_session);
}

#endif

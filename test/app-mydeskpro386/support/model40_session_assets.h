#include "x86-nxvm/machine/machine_interface.h"
#ifndef TEST_X86_NXVM_MACHINE_QUALIFICATION_MODEL40_SESSION_ASSETS_H
#define TEST_X86_NXVM_MACHINE_QUALIFICATION_MODEL40_SESSION_ASSETS_H
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

#include "x86-nxvm/chips/rtc146818/rtc146818_interface.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "app-mydeskpro386/machine_config.h"
#include "app-mydeskpro386/machine_plan_interface.h"

static inline lib_status vm_model40_fixture_machine_create_from_assets(
    const x86_nxvm_machine_config *config, const x86_nxvm_machine_assets *assets,
    x86_nxvm_machine **out_session)
{
    x86_nxvm_machine_construction construction;
    lib_status status;

    if (config == LIB_NULL || assets == LIB_NULL || out_session == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_session = LIB_NULL;
    status = mydeskpro386_machine_plan_create(config, assets, &construction);
    if (status != LIB_STATUS_OK) return status;
    return x86_nxvm_machine_create(config, &construction, out_session);
}

static inline void vm_model40_fixture_cmos_seed(
    lib_u8 bytes[X86_NXVM_MACHINE_CMOS_SEED_BYTES])
{
    lib_u16 checksum = 0u;
    lib_u8 index;

    lib_memory_set(bytes, 0, X86_NXVM_MACHINE_CMOS_SEED_BYTES);
    bytes[X86_NXVM_BOARD_RTC_TYPE_DISK_FLOPPY] = 0x22u;
    bytes[X86_NXVM_BOARD_RTC_TYPE_DISK_FIXED] = 0x80u;
    bytes[X86_NXVM_BOARD_RTC_EQUIPMENT] = 0x41u;
    bytes[X86_NXVM_BOARD_RTC_BASEMEM_LSB] = 0x80u;
    bytes[X86_NXVM_BOARD_RTC_BASEMEM_MSB] = 0x02u;
    bytes[X86_NXVM_BOARD_RTC_EXTMEM_LSB] = 0x00u;
    bytes[X86_NXVM_BOARD_RTC_EXTMEM_MSB] = 0x04u;
    for (index = 0x10u; index < 0x2eu; ++index) checksum =
        (lib_u16)(checksum + bytes[index]);
    bytes[0x2eu] = ((lib_u8)(checksum >> 8u));
    bytes[0x2fu] = ((lib_u8)(checksum));
}

static inline lib_status vm_model40_fixture_create_bytes_with_floppy_format(
    const lib_u8 *even_bytes, const lib_u8 *odd_bytes,
    x86_nxvm_machine_floppy_format floppy_format, x86_nxvm_machine **out_session)
{
    x86_nxvm_machine_config config = {0};
    lib_u8 cmos_seed[X86_NXVM_MACHINE_CMOS_SEED_BYTES];

    x86_nxvm_machine_assets assets = { .bios = {
        { even_bytes, MYDESKPRO386_ROM_CHIP_BYTES },
        { odd_bytes, MYDESKPRO386_ROM_CHIP_BYTES }
    } };

    /* Unit fixtures deliberately keep their bytes in process memory. */
    vm_model40_fixture_cmos_seed(cmos_seed);
    assets.cmos_seed = (x86_nxvm_machine_asset_bytes) { cmos_seed, sizeof(cmos_seed) };
    config.bios_count = 2u;
    config.floppy_format = floppy_format;
    return vm_model40_fixture_machine_create_from_assets(&config, &assets, out_session);
}

static inline lib_status vm_model40_fixture_create_bytes(
    const lib_u8 *even_bytes, const lib_u8 *odd_bytes,
    x86_nxvm_machine **out_session)
{
    return vm_model40_fixture_create_bytes_with_floppy_format(even_bytes, odd_bytes,
        X86_NXVM_MACHINE_FLOPPY_FORMAT_PROFILE_DEFAULT, out_session);
}

static inline lib_status vm_model40_fixture_create(x86_nxvm_machine **out_session)
{
    lib_u8 even_bytes[MYDESKPRO386_ROM_CHIP_BYTES] = {0};
    lib_u8 odd_bytes[MYDESKPRO386_ROM_CHIP_BYTES];

    lib_memory_set(odd_bytes, 1, sizeof(odd_bytes));
    return vm_model40_fixture_create_bytes(even_bytes, odd_bytes, out_session);
}

#endif

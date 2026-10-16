#include "../support/profile.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "lib/types/file.h"

#include "../../x86-nxvm/board-base/composition/composition_fixture.h"
#include "x86-nxvm/chips/rtc146818/rtc146818_interface.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_private.h"
#include "../support/model40_session_assets.h"

static lib_u8 vm_model40_cmos_read(x86_nxvm_machine *session,
    lib_u8 index)
{
    test_x86_nxvm_write_port_after_run(session->x86_core_machine, 0x0070u, index);
    return (lib_u8)test_x86_nxvm_read_port_after_run(session->x86_core_machine, 0x0071u);
}

static void vm_model40_cmos_write(x86_nxvm_machine *session, lib_u8 index,
    lib_u8 value)
{
    test_x86_nxvm_write_port_after_run(session->x86_core_machine, 0x0070u, index);
    test_x86_nxvm_write_port_after_run(session->x86_core_machine, 0x0071u, value);
}

static lib_i32 vm_model40_cmos_checksum_is_valid(x86_nxvm_machine *session)
{
    lib_u16 checksum = 0u;
    lib_u8 index;

    for (index = 0x10u; index < 0x2eu; ++index) {
        checksum = (lib_u16)(checksum + vm_model40_cmos_read(session, index));
    }
    return vm_model40_cmos_read(session, 0x2eu) ==
        ((lib_u8)(checksum >> 8u)) &&
        vm_model40_cmos_read(session, 0x2fu) == ((lib_u8)(checksum));
}

static lib_i32 vm_model40_cmos_seed_matches(x86_nxvm_machine *session)
{
    return vm_model40_cmos_read(session, X86_NXVM_BOARD_RTC_TYPE_DISK_FLOPPY) == 0x22u &&
        vm_model40_cmos_read(session, X86_NXVM_BOARD_RTC_TYPE_DISK_FIXED) == 0x80u &&
        vm_model40_cmos_read(session, X86_NXVM_BOARD_RTC_TYPE_DISK_FIXED_EXTENDED_0) == 0u &&
        vm_model40_cmos_read(session, X86_NXVM_BOARD_RTC_EQUIPMENT) == 0x41u &&
        vm_model40_cmos_read(session, X86_NXVM_BOARD_RTC_BASEMEM_LSB) == 0x80u &&
        vm_model40_cmos_read(session, X86_NXVM_BOARD_RTC_BASEMEM_MSB) == 0x02u &&
        vm_model40_cmos_read(session, X86_NXVM_BOARD_RTC_EXTMEM_LSB) == 0x00u &&
        vm_model40_cmos_read(session, X86_NXVM_BOARD_RTC_EXTMEM_MSB) == 0x04u &&
        vm_model40_cmos_checksum_is_valid(session);
}

static void vm_model40_cmos_seed_bytes(
    lib_u8 bytes[X86_NXVM_MACHINE_CMOS_SEED_BYTES])
{
    lib_memory_set(bytes, 0, X86_NXVM_MACHINE_CMOS_SEED_BYTES);
    bytes[X86_NXVM_BOARD_RTC_TYPE_DISK_FLOPPY] = 0x22u;
    bytes[X86_NXVM_BOARD_RTC_TYPE_DISK_FIXED] = 0x80u;
    /* 33h is Model-40 board NVRAM, not a topology-derived CMOS field. */
    bytes[0x33u] = 0x5au;
    bytes[X86_NXVM_BOARD_RTC_BASEMEM_LSB] = 0x80u;
    bytes[X86_NXVM_BOARD_RTC_BASEMEM_MSB] = 0x02u;
    bytes[X86_NXVM_BOARD_RTC_EXTMEM_LSB] = 0x00u;
    bytes[X86_NXVM_BOARD_RTC_EXTMEM_MSB] = 0x04u;
    {
        lib_u16 checksum = 0u;
        lib_u8 index;

        for (index = 0x10u; index < 0x2eu; ++index) {
            checksum = (lib_u16)(checksum + bytes[index]);
        }
        bytes[0x2eu] = ((lib_u8)(checksum >> 8u));
        bytes[0x2fu] = ((lib_u8)(checksum));
    }
}

static lib_status vm_model40_cmos_seed_session_create(
    const lib_u8 seed[X86_NXVM_MACHINE_CMOS_SEED_BYTES], x86_nxvm_machine **out_session)
{
    lib_u8 even_bytes[MYDESKPRO386_ROM_CHIP_BYTES] = {0};
    lib_u8 odd_bytes[MYDESKPRO386_ROM_CHIP_BYTES];
    lib_u8 default_seed[X86_NXVM_MACHINE_CMOS_SEED_BYTES];
    x86_nxvm_machine_config config = {0};
    x86_nxvm_machine_assets assets = {0};

    lib_memory_set(odd_bytes, 1, sizeof(odd_bytes));
    config.bios_count = 2u;
    assets.bios[0u] = (x86_nxvm_machine_asset_bytes) { even_bytes, sizeof(even_bytes) };
    assets.bios[1u] = (x86_nxvm_machine_asset_bytes) { odd_bytes, sizeof(odd_bytes) };
    vm_model40_fixture_cmos_seed(default_seed);
    assets.cmos_seed = (x86_nxvm_machine_asset_bytes) { seed == LIB_NULL ? default_seed : seed,
        X86_NXVM_MACHINE_CMOS_SEED_BYTES };
    return vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        &config, &assets, out_session);
}

lib_i32 main(void)
{
    x86_nxvm_machine *first = LIB_NULL;
    x86_nxvm_machine *second = LIB_NULL;
    x86_nxvm_machine *seeded = LIB_NULL;
    lib_u8 seed[X86_NXVM_MACHINE_CMOS_SEED_BYTES];
    lib_i32 failed = 0;

    failed = failed || vm_model40_cmos_seed_session_create(LIB_NULL, &first) != LIB_STATUS_OK ||
        first == LIB_NULL;
    if (!failed) failed = !vm_model40_cmos_seed_matches(first);
    if (!failed) {
        vm_model40_cmos_write(first, X86_NXVM_BOARD_RTC_EQUIPMENT, 0x5au);
        failed = x86_nxvm_machine_reset(first) != LIB_STATUS_OK ||
            vm_model40_cmos_read(first, X86_NXVM_BOARD_RTC_EQUIPMENT) != 0x5au;
    }
    failed = failed || vm_model40_cmos_seed_session_create(LIB_NULL, &second) != LIB_STATUS_OK ||
        second == LIB_NULL;
    if (!failed) failed = !vm_model40_cmos_seed_matches(second);
    vm_model40_cmos_seed_bytes(seed);
    failed = failed || vm_model40_cmos_seed_session_create(seed, &seeded) !=
        LIB_STATUS_OK || seeded == LIB_NULL;
    if (!failed) {
        failed = vm_model40_cmos_read(seeded, X86_NXVM_BOARD_RTC_EQUIPMENT) !=
                seed[X86_NXVM_BOARD_RTC_EQUIPMENT] ||
            vm_model40_cmos_read(seeded, 0x33u) != seed[0x33u] ||
            vm_model40_cmos_read(seeded, 0x2eu) != seed[0x2eu] ||
            vm_model40_cmos_read(seeded, 0x2fu) != seed[0x2fu] ||
            !vm_model40_cmos_checksum_is_valid(seeded);
    }
    x86_nxvm_machine_destroy(seeded);
    x86_nxvm_machine_destroy(second);
    x86_nxvm_machine_destroy(first);
    if (failed) return 1;
    lib_c_printf("MODEL40-CMOS-SEED:OK\n");
    return 0;
}

#include "../support/profile.h"
#include "core/board-base/pc_at_rom_interface.h"
#include "core/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include <stdio.h>
#include "core/x86/device_support_interface.h"

#include "core/board-base/pic_bus_interface.h"
#include "../../../core/board-base/composition/bus_fixture.h"
#include "../support/ibmpc/board-common/cmos_fixture.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine_private.h"
#include "core/chips/rtc146818/rtc146818_interface.h"
#include "../support/rom/session_assets.h"

static void cmos_write(core_machine *core, lib_u8 reg, lib_u8 value)
{
    test_core_machine_fixture_write_port(core, 0x0070u, reg);
    test_core_machine_fixture_write_port(core, 0x0071u, value);
}

static lib_u8 cmos_read(core_machine *core, lib_u8 reg)
{
    test_core_machine_fixture_write_port(core, 0x0070u, reg);
    return (lib_u8)test_core_machine_fixture_read_bus(core, 0x0071u);
}

static void initialize_pic(core_machine *core)
{
    test_core_machine_fixture_write_port(core, 0x0020u, 0x11u);
    test_core_machine_fixture_write_port(core, 0x0021u, 0x08u);
    test_core_machine_fixture_write_port(core, 0x0021u, 0x04u);
    test_core_machine_fixture_write_port(core, 0x0021u, 0x01u);
    test_core_machine_fixture_write_port(core, 0x00a0u, 0x11u);
    test_core_machine_fixture_write_port(core, 0x00a1u, 0x70u);
    test_core_machine_fixture_write_port(core, 0x00a1u, 0x02u);
    test_core_machine_fixture_write_port(core, 0x00a1u, 0x01u);
}

static lib_i32 default_at_cmos_seed_is_loaded(void)
{
    lib_u8 seed[VM_MACHINE_CMOS_SEED_BYTES] = {0};
    vm_machine_config config = {0};
    vm_machine_assets assets;
    vm_machine *session = LIB_NULL;
    lib_u16 checksum = 0u;
    lib_size index;
    lib_i32 failed = 0;

    for (index = 0x0eu; index < VM_MACHINE_CMOS_SEED_BYTES; ++index) {
        seed[index] = (lib_u8)(0xa5u ^ index);
    }
    /* A session seed owns the whole board-NVRAM image, not just vendor bytes.
     * Supply a valid AT checksum exactly as an external .cmos asset would. */
    for (index = 0x10u; index < 0x2eu; ++index) {
        checksum = (lib_u16)(checksum + seed[index]);
    }
    seed[0x2eu] = CORE_MACHINE_MASK_U8(checksum >> 8u);
    seed[0x2fu] = CORE_MACHINE_MASK_U8(checksum);
    vm_test_default_pc_at_assets(&assets,
        (lib_u8[VM_PROFILE_EXTERNAL_PC_AT_ROM_BYTES]) {0});
    /* The helper's ROM array must outlive composition only; session copies it. */
    assets.cmos_seed = (vm_machine_asset_bytes) { seed, sizeof(seed) };
    config.bios_count = 1u;
    failed |= vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_DEFAULT_PC_AT,
        &config, &assets, &session) != LIB_STATUS_OK ||
        session == LIB_NULL;
    if (!failed) {
        core_machine *core = session->core_machine;

        for (index = 0x0eu; index < VM_MACHINE_CMOS_SEED_BYTES; ++index) {
            failed |= cmos_read(core, (lib_u8)index) != seed[index];
        }
    }
    vm_machine_destroy(session);
    return failed;
}

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    core_machine *core;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) return 1;
    core = session->core_machine;
    if (!session->active) failed = 1;
    initialize_pic(core);

    if (cmos_read(core, X86_RTC_REG_D) != X86_RTC_REG_D_VRT) failed |= 0x0001;
    if (cmos_read(core, X86_RTC_SECOND) != 0x00u) failed |= 0x0002;
    test_board_cmos_advance(session->board, 50000u);
    if (cmos_read(core, X86_RTC_SECOND) != 0x01u) failed |= 0x0004;

    cmos_write(core, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_UIE);
    test_board_cmos_advance(session->board, 50000u);
    if (!test_board_cmos_scan_interrupt(session->board)) failed |= 0x0008;
    if (test_board_cmos_get_interrupt(session->board) != 0x70u) failed |= 0x0010;
    if ((cmos_read(core, X86_RTC_REG_C) &
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_UF)) !=
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_UF)) failed |= 0x0020;
    test_core_machine_fixture_write_port(core, 0x00a0u, 0x20u);
    test_core_machine_fixture_write_port(core, 0x0020u, 0x20u);
    if (test_board_cmos_scan_interrupt(session->board)) failed |= 0x0040;

    cmos_write(core, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_PIE);
    test_board_cmos_advance(session->board, 50u);
    if (!test_board_cmos_scan_interrupt(session->board) ||
        test_board_cmos_get_interrupt(session->board) != 0x70u) failed |= 0x0080;
    if ((cmos_read(core, X86_RTC_REG_C) &
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_PF)) !=
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_PF)) failed |= 0x0100;
    test_core_machine_fixture_write_port(core, 0x00a0u, 0x20u);
    test_core_machine_fixture_write_port(core, 0x0020u, 0x20u);

    cmos_write(core, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_SET);
    cmos_write(core, X86_RTC_SECOND, 0x11u);
    test_board_cmos_advance(session->board, 100000u);
    if (cmos_read(core, X86_RTC_SECOND) != 0x11u) failed |= 0x0200;
    cmos_write(core, X86_RTC_REG_B, X86_RTC_REG_B_24H);

    cmos_write(core, X86_RTC_REG_B, X86_RTC_REG_B_DM);
    cmos_write(core, X86_RTC_HOUR, 0x81u);
    if (cmos_read(core, X86_RTC_HOUR) != 0x81u) failed |= 0x0400;

    cmos_write(core, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_SET);
    cmos_write(core, X86_RTC_SECOND, 0x58u);
    cmos_write(core, X86_RTC_MINUTE, 0x00u);
    cmos_write(core, X86_RTC_HOUR, 0x00u);
    cmos_write(core, X86_RTC_SECOND_ALARM, 0x59u);
    cmos_write(core, X86_RTC_MINUTE_ALARM, 0x00u);
    cmos_write(core, X86_RTC_HOUR_ALARM, 0x00u);
    cmos_write(core, X86_RTC_REG_B, X86_RTC_REG_B_24H | X86_RTC_REG_B_AIE);
    test_board_cmos_advance(session->board, 50000u);
    if (!test_board_cmos_scan_interrupt(session->board) ||
        test_board_cmos_get_interrupt(session->board) != 0x70u) failed |= 0x0800;
    if ((cmos_read(core, X86_RTC_REG_C) &
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_AF)) !=
        (X86_RTC_REG_C_IRQF | X86_RTC_REG_C_AF)) failed |= 0x1000;
    test_core_machine_fixture_write_port(core, 0x00a0u, 0x20u);
    test_core_machine_fixture_write_port(core, 0x0020u, 0x20u);

    cmos_write(core, CORE_MACHINE_RTC_EQUIPMENT, 0x5au);
    test_board_cmos_reset(session->board);
    if (cmos_read(core, CORE_MACHINE_RTC_EQUIPMENT) != 0x5au) failed |= 0x2000;
    if (cmos_read(core, X86_RTC_SECOND) != 0x59u) failed |= 0x4000;

    failed |= default_at_cmos_seed_is_loaded();
    if (failed) {
        test_board_cmos_report_failure(session->board, failed);
    }
    vm_machine_destroy(session);
    if (failed) return 1;
    puts("CMOS-RTC-PORT:OK");
    return 0;
}

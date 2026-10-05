#include "../../../support/profile.h"
#include "ibmpc/machine/machine_interface.h"
#include "../../../support/media.h"
#include "lib/types/types_interface.h"
#include "../../../../ibmpc/board-common/composition_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include <stdio.h>

#include "x86/core/debug_interface.h"
#include "ibmpc/machine/lifecycle.h"
#include "ibmpc/machine/machine_interface.h"
#include "ibmpc/machine/machine_private.h"
#include "../../../../x86/core/time_fixture.h"
#include "../../../../ibmpc/board-common/cmos_fixture.h"
#include "support/rom/session_assets.h"

static lib_i32 vm_default_pc_at_fdd_format_is_valid(
    vm_machine_floppy_format format, lib_u16 cylinders,
    lib_u16 sectors, lib_u8 cmos_type)
{
    vm_machine_config config = {0};
    vm_machine *session = LIB_NULL;
    lib_u32 port_b;
    lib_u32 next_port_b;
    lib_u32 tick;
    lib_u16 checksum = 0u;
    lib_u8 index;
    lib_u8 observed_type;
    lib_i32 valid = 0;

    config.floppy_format = format;
    if (vm_test_default_pc_at_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) goto done;
    observed_type = test_board_cmos_read_register(session->board,
        CORE_MACHINE_RTC_TYPE_DISK_FLOPPY);
    if (vm_test_fdd_info(session->fdd).geometry.cylinders != cylinders || vm_test_fdd_info(session->fdd).geometry.heads != 2u ||
        vm_test_fdd_info(session->fdd).geometry.sectors_per_track != sectors || vm_test_fdd_info(session->fdd).geometry.bytes_per_sector != 512u ||
        observed_type != cmos_type) {
        printf("FDD setup format=%u cmos=%02x expected=%02x\n",
            (unsigned int)format, (unsigned int)observed_type,
            (unsigned int)cmos_type);
        goto done;
    }
    for (index = 0x10u; index < 0x2eu; ++index) {
        checksum = (lib_u16)(checksum +
            test_board_cmos_read_register(session->board, index));
    }
    if (test_board_cmos_read_register(session->board, 0x2eu) !=
            (lib_u8)(checksum >> 8u) ||
        test_board_cmos_read_register(session->board, 0x2fu) !=
            (lib_u8)checksum) {
        goto done;
    }
    if (core_machine_bus_read(session->core_machine, 0x0061u, &port_b) !=
        LIB_STATUS_OK) {
        goto done;
    }
    for (tick = 0u; tick < 200u; ++tick) {
        if (test_core_machine_advance_time(session->core_machine, 1u) != LIB_STATUS_OK ||
            core_machine_bus_read(session->core_machine, 0x0061u, &next_port_b) !=
                LIB_STATUS_OK) {
            goto done;
        }
        if ((port_b & 0x10u) != (next_port_b & 0x10u)) break;
    }
    if (tick == 200u) goto done;
    valid = 1;
done:
    vm_machine_destroy(session);
    return valid;
}

static lib_i32 vm_default_pc_at_80186_refresh_polling_is_live(void)
{
    static const lib_u8 program[] = {
        0xb4u, 0x10u, 0xe4u, 0x61u, 0x24u, 0x10u,
        0x3au, 0xc4u, 0x74u, 0xf8u, 0xf4u
    };
    vm_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80186
    };
    vm_machine *session = LIB_NULL;
    core_machine_run_result result = {0};
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP),
        .values = { [CORE_MACHINE_DEBUG_EIP] = 0x0500u,
            [CORE_MACHINE_DEBUG_ESP] = 0xfffeu }
    };
    lib_u32 port_b;
    lib_u32 tick;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) {
        vm_machine_destroy(session);
        return 0;
    }
    for (tick = 0u; tick < 200u; ++tick) {
        if (core_machine_bus_read(session->core_machine, 0x0061u, &port_b) !=
                LIB_STATUS_OK) {
            failed = 1;
            break;
        }
        if ((port_b & 0x10u) == 0u) break;
        if (test_core_machine_advance_time(session->core_machine, 1u) != LIB_STATUS_OK) {
            failed = 1;
            break;
        }
    }
    if (!failed && (tick == 200u || core_machine_memory_write(session->core_machine,
            0x0500u, program, sizeof(program)) != LIB_STATUS_OK)) {
        failed = 1;
    }
    if (!failed) {
        failed = core_machine_debug_patch_registers(session->core_machine,
            &entry) != LIB_STATUS_OK || core_machine_run(session->core_machine,
            (core_machine_run_budget) {1000u, 0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    }
    vm_machine_destroy(session);
    return !failed;
}

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    test_board_composition_observation board;
    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) {
        vm_machine_destroy(session);
        return 1;
    }
    board = test_board_capture_composition(session->board);
    if (!session->active || vm_test_profile_construction(session)->profile.context == LIB_NULL ||
        board.fdc.dor_port != 0x03f2u ||
        board.fdc.status_port != 0x03f4u ||
        board.fdc.data_port != 0x03f5u ||
        board.fdc.direction_port != 0x03f7u ||
        board.fdc.irq != 6u ||
        board.fdc.dma_channel != 2u ||
        board.fdc.ready_mask != 0x0fu) {
        vm_machine_destroy(session);
        return 1;
    }
    if (test_board_cmos_read_register(session->board, CORE_MACHINE_RTC_EQUIPMENT) !=
            0x21u || test_board_cmos_read_register(session->board, CORE_MACHINE_RTC_BASEMEM_LSB) != 0x7fu ||
        test_board_cmos_read_register(session->board, CORE_MACHINE_RTC_BASEMEM_MSB) != 0x02u) {
        vm_machine_destroy(session);
        return 1;
    }
    vm_machine_destroy(session);
    if (!vm_default_pc_at_fdd_format_is_valid(VM_MACHINE_FLOPPY_FORMAT_360K,
            40u, 9u, 0x10u) ||
        !vm_default_pc_at_fdd_format_is_valid(VM_MACHINE_FLOPPY_FORMAT_720K,
            80u, 9u, 0x30u) ||
        !vm_default_pc_at_fdd_format_is_valid(VM_MACHINE_FLOPPY_FORMAT_1200K,
            80u, 15u, 0x20u) ||
        !vm_default_pc_at_fdd_format_is_valid(VM_MACHINE_FLOPPY_FORMAT_1440K,
            80u, 18u, 0x40u)) {
        return 1;
    }
    if (!vm_default_pc_at_80186_refresh_polling_is_live()) return 1;
    puts("M5:T208:S3:DEFAULT-PC-AT-APPLY:OK");
    return 0;
}

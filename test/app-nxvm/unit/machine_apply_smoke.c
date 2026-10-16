#include "../support/profile.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "../../x86-nxvm/machine/support/media.h"
#include "lib/types/types_interface.h"
#include "../../x86-nxvm/board-base/support/composition_fixture.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "lib/types/file.h"

#include "x86-core/machine/debug_interface.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "../../x86-nxvm/board-base/support/cmos_fixture.h"
#include "../support/rom/session_assets.h"

static lib_i32 vm_default_pc_at_fdd_format_is_valid(
    x86_nxvm_machine_floppy_format format, lib_u16 cylinders,
    lib_u16 sectors, lib_u8 cmos_type)
{
    x86_nxvm_machine_config config = {0};
    x86_nxvm_machine *session = LIB_NULL;
    lib_u32 port_b;
    lib_u32 next_port_b;
    lib_u32 tick;
    lib_u64 first_tick = 0u;
    lib_u16 checksum = 0u;
    lib_u8 index;
    lib_u8 observed_type;
    lib_i32 valid = 0;

    config.floppy_format = format;
    if (vm_test_default_pc_at_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) goto done;
    observed_type = test_board_cmos_read_register(session->board,
        X86_NXVM_BOARD_RTC_TYPE_DISK_FLOPPY);
    if (vm_test_fdd_info(session->floppy[0u]).geometry.cylinders != cylinders || vm_test_fdd_info(session->floppy[0u]).geometry.heads != 2u ||
        vm_test_fdd_info(session->floppy[0u]).geometry.sectors_per_track != sectors || vm_test_fdd_info(session->floppy[0u]).geometry.bytes_per_sector != 512u ||
        observed_type != cmos_type) {
        lib_c_printf("FDD setup format=%u cmos=%02x expected=%02x\n",
            (lib_u32)format, (lib_u32)observed_type, (lib_u32)cmos_type);
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
    if (x86_core_machine_get_elapsed_ticks(session->x86_core_machine, &first_tick) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(session->x86_core_machine, 0x0061u, &port_b) !=
        LIB_STATUS_OK) {
        goto done;
    }
    for (tick = 0u; tick < 200u; ++tick) {
        lib_u8 advanced = LIB_FALSE;
        lib_u64 elapsed = 0u;

        if (x86_core_machine_advance_to_next_deadline(session->x86_core_machine,
                &advanced) != LIB_STATUS_OK || !advanced ||
            x86_core_machine_get_elapsed_ticks(session->x86_core_machine, &elapsed) != LIB_STATUS_OK ||
            elapsed - first_tick > 200u ||
            x86_core_machine_bus_read(session->x86_core_machine, 0x0061u, &next_port_b) !=
                LIB_STATUS_OK) {
            goto done;
        }
        if ((port_b & 0x10u) != (next_port_b & 0x10u)) break;
    }
    if (tick == 200u) goto done;
    valid = 1;
done:
    x86_nxvm_machine_destroy(session);
    return valid;
}

static lib_i32 vm_default_pc_at_80186_refresh_polling_is_live(void)
{
    static const lib_u8 program[] = {
        0xb4u, 0x10u, 0xe4u, 0x61u, 0x24u, 0x10u,
        0x3au, 0xc4u, 0x74u, 0xf8u, 0xf4u
    };
    x86_nxvm_machine_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80186
    };
    x86_nxvm_machine *session = LIB_NULL;
    x86_core_machine_run_result result = {0};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ES) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP),
        .values = { [X86_CORE_CPU_DEBUG_EIP] = 0x0500u,
            [X86_CORE_CPU_DEBUG_ESP] = 0xfffeu }
    };
    lib_u32 port_b;
    lib_u32 tick;
    lib_u64 first_tick = 0u;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(&config, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) {
        x86_nxvm_machine_destroy(session);
        return 0;
    }
    if (x86_core_machine_get_elapsed_ticks(session->x86_core_machine, &first_tick) != LIB_STATUS_OK) {
        x86_nxvm_machine_destroy(session);
        return 0;
    }
    for (tick = 0u; tick < 200u; ++tick) {
        lib_u8 advanced = LIB_FALSE;
        lib_u64 elapsed = 0u;
        if (x86_core_machine_bus_read(session->x86_core_machine, 0x0061u, &port_b) !=
                LIB_STATUS_OK) {
            failed = 1;
            break;
        }
        if ((port_b & 0x10u) == 0u) break;
        if (x86_core_machine_advance_to_next_deadline(session->x86_core_machine,
                &advanced) != LIB_STATUS_OK || !advanced ||
            x86_core_machine_get_elapsed_ticks(session->x86_core_machine, &elapsed) != LIB_STATUS_OK ||
            elapsed - first_tick > 200u) {
            failed = 1;
            break;
        }
    }
    if (!failed && (tick == 200u || x86_core_machine_memory_write(session->x86_core_machine,
            0x0500u, program, sizeof(program)) != LIB_STATUS_OK)) {
        failed = 1;
    }
    if (!failed) {
        failed = x86_core_machine_debug_patch_registers(session->x86_core_machine,
            &entry) != LIB_STATUS_OK || x86_core_machine_run(session->x86_core_machine,
            (x86_core_machine_run_budget) {1000u, 0u}, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    }
    x86_nxvm_machine_destroy(session);
    return !failed;
}

lib_i32 main(void)
{
    x86_nxvm_machine *session = LIB_NULL;
    test_board_composition_observation board;
    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) {
        x86_nxvm_machine_destroy(session);
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
        x86_nxvm_machine_destroy(session);
        return 1;
    }
    if (test_board_cmos_read_register(session->board, X86_NXVM_BOARD_RTC_EQUIPMENT) !=
            0x21u || test_board_cmos_read_register(session->board, X86_NXVM_BOARD_RTC_BASEMEM_LSB) != 0x7fu ||
        test_board_cmos_read_register(session->board, X86_NXVM_BOARD_RTC_BASEMEM_MSB) != 0x02u) {
        x86_nxvm_machine_destroy(session);
        return 1;
    }
    x86_nxvm_machine_destroy(session);
    if (!vm_default_pc_at_fdd_format_is_valid(X86_NXVM_MACHINE_FLOPPY_FORMAT_360K,
            40u, 9u, 0x10u) ||
        !vm_default_pc_at_fdd_format_is_valid(X86_NXVM_MACHINE_FLOPPY_FORMAT_720K,
            80u, 9u, 0x30u) ||
        !vm_default_pc_at_fdd_format_is_valid(X86_NXVM_MACHINE_FLOPPY_FORMAT_1200K,
            80u, 15u, 0x20u) ||
        !vm_default_pc_at_fdd_format_is_valid(X86_NXVM_MACHINE_FLOPPY_FORMAT_1440K,
            80u, 18u, 0x40u)) {
        return 1;
    }
    if (!vm_default_pc_at_80186_refresh_polling_is_live()) return 1;
    lib_c_printf("DEFAULT-PC-AT-APPLY:OK\n");
    return 0;
}

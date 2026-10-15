#include "app-mydeskpro386/machine_observation_interface.h"
#include "../../core/machine/support/guest_input.h"
#include "../support/profile.h"
#include "core/machine/machine_interface.h"
#include "../support/model40.h"
#include "../../core/board-base/composition/composition_fixture.h"
#include "../../core/board-base/support/composition_fixture.h"
#include "../../core/board-base/support/cmos_fixture.h"
#include "../../core/board-base/support/kbc_state_fixture.h"
#include "../../core/board-base/support/controller_fixture.h"
#include "core/board-base/machine_board_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "core/machine/machine_private.h"
#include "core/machine/machine_interface.h"
#include "core/machine/lifecycle.h"
#include "core/machine/display.h"
#include "../support/model40_session_assets.h"

static lib_i32 model40_kbc_extension_contract(vm_machine *session)
{
    lib_u32 status = 0u;
    lib_u32 value = 0u;

    if (session == LIB_NULL || session->core_machine == LIB_NULL ||
        core_machine_bus_write(session->core_machine, 0x0064u, 0xa3u) !=
            LIB_STATUS_OK ||
        core_machine_bus_write(session->core_machine, 0x0064u, 0xa4u) !=
            LIB_STATUS_OK ||
        core_machine_bus_read(session->core_machine, 0x0064u, &status) !=
            LIB_STATUS_OK || (status & 0x01u) != 0u ||
        core_machine_bus_write(session->core_machine, 0x0064u, 0xa5u) !=
            LIB_STATUS_OK ||
        core_machine_bus_read(session->core_machine, 0x0064u, &status) !=
            LIB_STATUS_OK || (status & 0x01u) != 0u ||
        core_machine_bus_read(session->core_machine, 0x0060u, &value) !=
            LIB_STATUS_OK || value != 0xb4u ||
        core_machine_bus_write(session->core_machine, 0x0064u, 0x60u) !=
            LIB_STATUS_OK ||
        core_machine_bus_write(session->core_machine, 0x0060u, 0x20u) !=
            LIB_STATUS_OK ||
        core_machine_bus_write(session->core_machine, 0x0064u, 0xa5u) !=
            LIB_STATUS_OK ||
        core_machine_bus_read(session->core_machine, 0x0060u, &value) !=
            LIB_STATUS_OK || value != 0xa4u ||
        core_machine_bus_write(session->core_machine, 0x0064u, 0x60u) !=
            LIB_STATUS_OK ||
        core_machine_bus_write(session->core_machine, 0x0060u, 0x43u) !=
            LIB_STATUS_OK) return 0;
    return 1;
}

lib_i32 main(void)
{
    static lib_u8 even[MYDESKPRO386_ROM_CHIP_BYTES];
    static lib_u8 odd[MYDESKPRO386_ROM_CHIP_BYTES];
    vm_machine_config invalid_config = {0};
    vm_machine_assets missing_assets = {0};
    vm_machine *session = LIB_NULL;
    x86_core_cpu_profile cpu_profile;
    lib_size memory_bytes;
    mydeskpro386_d4_platform_observation d4;
    lib_u32 value = 0u;
    lib_u8 rom_byte = 0u;
    lib_u8 sense_status = 0u;
    lib_u8 sense_cylinder = 0u;
    lib_u8 reset_status[CORE_MACHINE_FDC_DRIVE_COUNT] = {0};
    test_board_composition_observation composition = {0};
    lib_u8 floppy_type = 0u;
    lib_i32 failed = 1;
    lib_i32 stage = 0;

    even[0x3ff8u] = 0xa5u;

    stage = 1;
    if (vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        &invalid_config, &missing_assets, &session) !=
        LIB_STATUS_INVALID_ARGUMENT || session != LIB_NULL ||
        vm_model40_fixture_create_bytes(even, odd, &session) !=
        LIB_STATUS_OK || session == LIB_NULL || session->core_machine == LIB_NULL ||
        !vm_test_profile_is_model40(vm_test_profile_construction(session)) ||
        core_machine_get_cpu_profile(session->core_machine, &cpu_profile) !=
            LIB_STATUS_OK || cpu_profile != X86_CORE_CPU_PROFILE_80386 ||
        core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
            LIB_STATUS_OK || memory_bytes != 2u * 1024u * 1024u ||
        vm_test_model40_d4_observe(session, &d4) !=
            LIB_STATUS_OK || !d4.configured || d4.iochk_enabled ||
        d4.failsafe_enabled ||
        core_machine_bus_read(session->core_machine, 0x07c6u, &value) !=
            LIB_STATUS_OK || value != 0x0du ||
        core_machine_bus_read(session->core_machine, 0x0bc6u, &value) !=
            LIB_STATUS_OK || value != 0x30u ||
        core_machine_bus_read(session->core_machine, 0x0fc6u, &value) !=
            LIB_STATUS_OK || value != 0x01u ||
        core_machine_bus_read(session->core_machine, 0x0061u, &value) !=
            LIB_STATUS_OK || value != 0x1fu ||
        core_machine_memory_read(session->core_machine, 0x000ffff0u, &rom_byte,
            sizeof(rom_byte)) != LIB_STATUS_OK || rom_byte != 0xa5u ||
        test_board_kbc_aux_enabled(session->board) ||
        !test_board_kbc_command_matches(session->board,
            session->core_machine, 0x20u,
            0x20u, 0u) || !model40_kbc_extension_contract(session)) goto done;
    stage = 2;
    {
        core_machine_guest_input_event event = {0};

        event.kind = CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE;
        event.data.relative_mouse.delta_x = 1;
        event.data.relative_mouse.delta_y = 1;
        event.data.relative_mouse.buttons = 1u;
        if (core_machine_bus_read(session->core_machine, 0x64u, &value) != LIB_STATUS_OK ||
            (value & 0x01u) != 0u ||
            vm_test_submit_host_input(session, &event) != LIB_STATUS_OK ||
            core_machine_bus_read(session->core_machine, 0x64u, &value) != LIB_STATUS_OK ||
            (value & 0x01u) != 0u ||
            core_machine_bus_write(session->core_machine, 0x0064u, 0xa8u) != LIB_STATUS_OK ||
            test_board_kbc_aux_enabled(session->board) ||
            !test_board_kbc_command_matches(session->board,
            session->core_machine, 0x20u,
            0x20u, 0u)) goto done;
        composition = test_board_capture_composition(session->board);
        if (composition.drives.installed_mask != 0x03u ||
            composition.drives.double_sided_mask != 0x03u ||
            composition.drives.cylinder_count[0u] != 80u ||
            composition.drives.cylinder_count[1u] != 80u ||
            composition.drives.track_zero_active_low_mask != 0u) goto done;
        floppy_type = test_board_cmos_read_register(session->board, CORE_MACHINE_RTC_TYPE_DISK_FLOPPY);
        if (floppy_type != 0x22u ||
            core_machine_bus_write(session->core_machine, 0x0060u, 0xf5u) != LIB_STATUS_OK ||
            test_board_kbc_read_reply(session->board, session->core_machine) != 0xfau ||
            core_machine_bus_write(session->core_machine, 0x0064u, 0xd4u) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x0060u, 0xf4u) != LIB_STATUS_OK ||
            test_board_keyboard_scanning(session->board) ||
            core_machine_bus_read(session->core_machine, 0x64u, &value) != LIB_STATUS_OK ||
            (value & 0x01u) != 0u ||
            core_machine_bus_write(session->core_machine, 0x60u, 0xeeu) != LIB_STATUS_OK ||
            test_board_kbc_read_reply(session->board, session->core_machine) != 0xeeu) goto done;
    }
    stage = 3;
    if (vm_machine_reset(session) != LIB_STATUS_OK) goto done;
    composition = test_board_capture_composition(session->board);
    if (!composition.auxiliary_pit_configured || !composition.fdc_configured ||
            composition.fdc.irq != 6u || composition.fdc.dma_channel != 2u ||
            !composition.hdc_configured || composition.hdc.irq != 14u ||
            composition.hdc.protocol !=
                CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB ||
            !composition.rtc_cmos_configured || composition.rtc_irq != 8u ||
            !test_core_port_has_read(session->core_machine,
                0x03f7u) || !test_core_port_has_write(
                session->core_machine, 0x004bu)) goto done;
    stage = 4;
    {
        if (core_machine_bus_write(session->core_machine, 0x03f2u, 0u) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x03f2u, 0x1cu) != LIB_STATUS_OK ||
            !test_board_fdc_advance_due(session->board) ||
            !test_board_fdc_irq_source_is_asserted(session->board)) goto done;
        for (sense_status = 0u; sense_status < CORE_MACHINE_FDC_DRIVE_COUNT;
            ++sense_status) {
            if (core_machine_bus_write(session->core_machine, 0x03f5u, 0x08u) != LIB_STATUS_OK ||
                !test_board_fdc_advance_ticks(session->board, 1u) ||
                core_machine_bus_read(session->core_machine, 0x03f5u, &value) != LIB_STATUS_OK)
                goto done;
            reset_status[sense_status] = (lib_u8)value;
            if (core_machine_bus_read(session->core_machine, 0x03f5u, &value) != LIB_STATUS_OK)
                goto done;
            sense_cylinder = (lib_u8)value;
            if (reset_status[sense_status] !=
                (TEST_FDC_ST0_READY_CHANGE | sense_status) ||
                sense_cylinder != 0u) goto done;
        }
        if (core_machine_bus_write(session->core_machine, 0x03f5u, 0x08u) != LIB_STATUS_OK ||
            !test_board_fdc_advance_ticks(session->board, 1u) ||
            core_machine_bus_read(session->core_machine, 0x03f5u, &value) != LIB_STATUS_OK)
            goto done;
        sense_status = (lib_u8)value;
        if (sense_status != 0x80u ||
            core_machine_bus_read(session->core_machine, 0x03f4u, &value) != LIB_STATUS_OK ||
            (value & (TEST_FDC_MSR_RQM | TEST_FDC_MSR_DIO | TEST_FDC_MSR_CB)) !=
            TEST_FDC_MSR_RQM) goto done;
    }
    stage = 5;
    {
        if (core_machine_bus_write(session->core_machine, 0x01f6u, 0x2au) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x01f7u, 0x90u) != LIB_STATUS_OK) goto done;
        test_board_hdc_service(session->board);
        if (test_board_hdc_observe(session->board).error != 0x01u ||
            !test_board_hdc_irq_pending(session->board) ||
            core_machine_bus_read(session->core_machine, 0x01f7u, &value) != LIB_STATUS_OK ||
            test_board_hdc_irq_pending(session->board) ||
            core_machine_bus_write(session->core_machine, 0x01f7u, 0xecu) != LIB_STATUS_OK) goto done;
        test_board_hdc_service(session->board);
        if (core_machine_bus_read(session->core_machine, 0x01f7u, &value) != LIB_STATUS_OK ||
            (value & X86_HDC_STATUS_ERR) == 0u ||
            core_machine_bus_read(session->core_machine, 0x01f1u, &value) != LIB_STATUS_OK ||
            value != X86_HDC_ERROR_ABORT) goto done;
    }
    stage = 6;
    {
        /* Real selected CECG ports -> snapshot -> the sole Machine frame.
         * Start 12/end 0 means the last two rows of a 14-row cell. */
        const lib_u8 geometry[][2] = {
            {1u, 79u}, {7u, 0x12u}, {0x12u, 0x5du},
            {0x13u, 40u}, {0x17u, 0xe3u}, {0x18u, 0xffu}
        };

        for (lib_size index = 0u; index < sizeof(geometry) / sizeof(geometry[0]); ++index)
            if (core_machine_bus_write(session->core_machine, 0x03d4u,
                    geometry[index][0]) != LIB_STATUS_OK ||
                core_machine_bus_write(session->core_machine, 0x03d5u,
                    geometry[index][1]) != LIB_STATUS_OK) goto done;
        if (core_machine_bus_write(session->core_machine, 0x03d4u, 0x09u) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x03d5u, 13u) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x03d4u, 0x0au) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x03d5u, 12u) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x03d4u, 0x0bu) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x03d5u, 0u) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x03d4u, 0x0eu) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x03d5u, 0u) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x03d4u, 0x0fu) != LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x03d5u, 0u) != LIB_STATUS_OK ||
            vm_machine_publish_display(session, LIB_TRUE) != LIB_STATUS_OK ||
            !session->latest_frame.window.valid || session->latest_frame.window.graphics ||
            !session->latest_frame.window.text.base.cursor_mode ||
            session->latest_frame.window.text.base.font_height != 14u ||
            session->latest_frame.window.text.base.text_columns != 80u ||
            session->latest_frame.window.text.base.text_rows != 25u ||
            session->latest_frame.window.text.base.cursor_top != 12u ||
            session->latest_frame.window.text.base.cursor_bottom != 13u) goto done;
    }
    failed = 0;
done:
    if (failed) {
        lib_c_printf("MODEL40-MACHINE-CONTRACT:FAILED-stage=%u-fdc=%02X/%02X-cmos=%02X-reset=%02X,%02X,%02X,%02X-final=%02X\n",
            (unsigned int)stage,
            (unsigned int)composition.drives.installed_mask,
            (unsigned int)composition.drives.track_zero_active_low_mask,
            (unsigned int)floppy_type, (unsigned int)reset_status[0u],
            (unsigned int)reset_status[1u], (unsigned int)reset_status[2u],
            (unsigned int)reset_status[3u], (unsigned int)sense_status);
    }
    if (!failed) lib_c_printf("MODEL40-MACHINE-CONTRACT:OK\n");
    if (!failed) lib_c_printf("MODEL40-CONTROLS:OK\n");
    vm_machine_destroy(session);
    return failed ? 1 : 0;
}

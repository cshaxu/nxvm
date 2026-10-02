#include "../../../support/hdc.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "../devices/support/fdc_fixture.h"
#include "lib/types/types_interface.h"
#include "support/kbc_fixture.h"
#include <stdio.h>

#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/devices/fdc.h"
#include "app-nxvm/devices/hdc.h"
#include "app-nxvm/devices/kbc.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_board_state.h"
#include "app-nxvm/devices/port.h"
#include "support/rom/model40_session_assets.h"

lib_i32 main(void)
{
    static lib_u8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static lib_u8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_machine_config invalid_config = {
        .profile_kind = VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40
    };
    vm_machine_assets missing_assets = {0};
    vm_machine *session = LIB_NULL;
    core_machine_cpu_profile cpu_profile;
    lib_size memory_bytes;
    core_machine_d4_platform_observation d4;
    lib_u32 value = 0u;
    lib_u8 rom_byte = 0u;
    lib_u8 sense_status = 0u;
    lib_u8 sense_cylinder = 0u;
    lib_u8 reset_status[CORE_MACHINE_FDC_DRIVE_COUNT] = {0};
    lib_i32 failed = 0;
    lib_i32 stage = 0;

    even[0x3ff8u] = 0xa5u;

    failed |= vm_machine_create_from_assets(&invalid_config, &missing_assets, &session) !=
        LIB_STATUS_INVALID_ARGUMENT || session != LIB_NULL;
    if (!failed) failed |= vm_model40_fixture_create_bytes(even, odd, &session) !=
        LIB_STATUS_OK || session == LIB_NULL || !vm_profile_machine_plan_is_model40(session->profile_plan) ||
        core_machine_get_cpu_profile(session->core_machine, &cpu_profile) !=
            LIB_STATUS_OK || cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
            LIB_STATUS_OK || memory_bytes != 2u * 1024u * 1024u ||
        core_machine_get_d4_platform_observation(session->core_machine, &d4) !=
            LIB_STATUS_OK || !d4.configured || d4.iochk_enabled ||
        d4.failsafe_enabled ||
        core_machine_bus_read(session->core_machine, 0x07c6u, &value) !=
            LIB_STATUS_OK || value != 0u ||
        core_machine_bus_read(session->core_machine, 0x0bc6u, &value) !=
            LIB_STATUS_OK || value != 0x30u ||
        core_machine_bus_read(session->core_machine, 0x0fc6u, &value) !=
            LIB_STATUS_OK || value != 0x01u ||
        core_machine_bus_read(session->core_machine, 0x0061u, &value) !=
            LIB_STATUS_OK || value != 0x1fu ||
        core_machine_memory_read(session->core_machine, 0x000ffff0u, &rom_byte,
            sizeof(rom_byte)) != LIB_STATUS_OK || rom_byte != 0xa5u ||
        x86_kbc8042_aux_enabled(session->core_machine->board->shared_kbc.chip) ||
            x86_kbc8042_aux_enabled(session->core_machine->board->shared_kbc.chip) ||
        !kbc_test_command_matches(&session->core_machine->board->shared_kbc,
            &session->core_machine->executor_port, 0x20u,
            CORE_MACHINE_KBC_COMMAND_DISABLE_AUX, CORE_MACHINE_KBC_COMMAND_DISABLE_AUX);
    if (failed) stage = 1;
    if (!failed) {
        core_machine_guest_input_event event = {0};

        event.kind = CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE;
        event.data.relative_mouse.delta_x = 1;
        event.data.relative_mouse.delta_y = 1;
        event.data.relative_mouse.buttons = 1u;
        failed |= (core_machine_port_read(&session->core_machine->executor_port,
            0x64u) & VKBC_STATUS_OBF) != 0u;
        failed |= vm_machine_submit_host_input(session, &event) != LIB_STATUS_OK;
        failed |= (core_machine_port_read(&session->core_machine->executor_port,
            0x64u) & VKBC_STATUS_OBF) != 0u;
        core_machine_port_write(&session->core_machine->executor_port,
            0x0064u, 0xa8u);
        failed |= x86_kbc8042_aux_enabled(session->core_machine->board->shared_kbc.chip) ||
            !kbc_test_command_matches(&session->core_machine->board->shared_kbc,
            &session->core_machine->executor_port, 0x20u,
            CORE_MACHINE_KBC_COMMAND_DISABLE_AUX, CORE_MACHINE_KBC_COMMAND_DISABLE_AUX);
    failed |= !failed && (session->core_machine->board->fdc_topology.drives.installed_mask !=
        0x03u || session->core_machine->board->fdc_topology.drives.double_sided_mask != 0x03u ||
        session->core_machine->board->fdc_topology.drives.cylinder_count[0u] != 80u ||
        session->core_machine->board->fdc_topology.drives.cylinder_count[1u] != 80u ||
        session->core_machine->board->fdc_topology.drives.track_zero_active_low_mask != 0u ||
        x86_rtc_read_register(session->core_machine->board->shared_rtc, CORE_MACHINE_RTC_TYPE_DISK_FLOPPY) !=
            0x22u);
        core_machine_port_write(&session->core_machine->executor_port,
            0x0060u, 0xf5u);
        failed |= kbc_test_read_reply(&session->core_machine->board->shared_kbc,
            &session->core_machine->executor_port) != 0xfau;
        core_machine_port_write(&session->core_machine->executor_port,
            0x0064u, 0xd4u);
        core_machine_port_write(&session->core_machine->executor_port,
            0x0060u, 0xf4u);
        failed |= x86_keyboard_get_signals(session->core_machine->board->shared_kbc.connect.keyboard).scanning ||
            (core_machine_port_read(&session->core_machine->executor_port,
                0x64u) & VKBC_STATUS_OBF) != 0u;
        core_machine_port_write(&session->core_machine->executor_port, 0x60u, 0xeeu);
        failed |= kbc_test_read_reply(&session->core_machine->board->shared_kbc,
            &session->core_machine->executor_port) != 0xeeu;
        if (failed) stage = 2;
    }
    if (!failed) {
        vm_machine_reset(session);
        failed |= !session->core_machine->board->auxiliary_pit_configured ||
            !session->core_machine->board->fdc_configured ||
            session->core_machine->board->fdc_topology.config.irq != 6u ||
            session->core_machine->board->fdc_topology.config.dma_channel != 2u ||
            !session->core_machine->board->hdc_configured ||
            session->core_machine->board->hdc_topology.config.irq != 14u ||
            session->core_machine->board->hdc_topology.config.protocol !=
                CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB ||
            !session->core_machine->board->rtc_cmos_configured ||
            session->core_machine->board->rtc_cmos_config.irq != 8u ||
            !core_machine_port_has_read(&session->core_machine->executor_port,
                0x03f7u) || !core_machine_port_has_write(
                &session->core_machine->executor_port, 0x004bu);
    }
    if (!failed) {
        core_machine_port_write(&session->core_machine->executor_port,
            0x03f2u, 0u);
        core_machine_port_write(&session->core_machine->executor_port,
            0x03f2u, 0x1cu);
        failed |= !test_fdc_advance_due(&session->core_machine->board->fdc);
        failed |= !session->core_machine->board->fdc.connect.irq_source.asserted;
        for (sense_status = 0u; sense_status < CORE_MACHINE_FDC_DRIVE_COUNT;
            ++sense_status) {
            core_machine_port_write(&session->core_machine->executor_port,
                0x03f5u, 0x08u);
            test_fdc_advance(&session->core_machine->board->fdc);
            reset_status[sense_status] = (lib_u8)core_machine_port_read(
                &session->core_machine->executor_port, 0x03f5u);
            sense_cylinder = (lib_u8)core_machine_port_read(
                &session->core_machine->executor_port, 0x03f5u);
            failed |= reset_status[sense_status] !=
                (TEST_FDC_ST0_READY_CHANGE | sense_status) ||
                sense_cylinder != 0u;
        }
        core_machine_port_write(&session->core_machine->executor_port, 0x03f5u, 0x08u);
        test_fdc_advance(&session->core_machine->board->fdc);
        sense_status = (lib_u8)core_machine_port_read(
            &session->core_machine->executor_port, 0x03f5u);
        failed |= sense_status != 0x80u ||
            (core_machine_port_read(&session->core_machine->executor_port, 0x03f4u) &
            (TEST_FDC_MSR_RQM | TEST_FDC_MSR_DIO | TEST_FDC_MSR_CB)) != TEST_FDC_MSR_RQM;
        if (failed) stage = 4;
    }
    if (!failed) {
        core_machine_port_write(&session->core_machine->executor_port,
            0x01f6u, 0x2au);
        core_machine_port_write(&session->core_machine->executor_port,
            0x01f7u, 0x90u);
        hdc_service(&session->core_machine->board->hdc);
        failed |= hdc_observe(&session->core_machine->board->hdc).error != 0x01u ||
            !core_machine_hdc_irq_pending(&session->core_machine->board->hdc);
        (void)core_machine_port_read(&session->core_machine->executor_port,
            0x01f7u);
        failed |= core_machine_hdc_irq_pending(&session->core_machine->board->hdc);
        core_machine_port_write(&session->core_machine->executor_port,
            0x01f7u, 0xecu);
        hdc_service(&session->core_machine->board->hdc);
        failed |= (core_machine_port_read(&session->core_machine->executor_port,
            0x01f7u) & X86_HDC_STATUS_ERR) == 0u ||
            core_machine_port_read(&session->core_machine->executor_port,
                0x01f1u) != X86_HDC_ERROR_ABORT;
        if (failed) stage = 5;
    }
    if (failed && session != LIB_NULL) {
        printf("M5:T386:S8:MODEL40-INTEGRATION:FAILED-stage=%u-fdc=%02X/%02X-cmos=%02X-reset=%02X,%02X,%02X,%02X-final=%02X\n",
            (unsigned int)stage,
            (unsigned int)session->core_machine->board->fdc_topology.drives.installed_mask,
            (unsigned int)session->core_machine->board->fdc_topology.drives.track_zero_active_low_mask,
            (unsigned int)x86_rtc_read_register(session->core_machine->board->shared_rtc, CORE_MACHINE_RTC_TYPE_DISK_FLOPPY), (unsigned int)reset_status[0u],
            (unsigned int)reset_status[1u], (unsigned int)reset_status[2u],
            (unsigned int)reset_status[3u], (unsigned int)sense_status);
    }
    if (!failed) printf("M5:T386:S8:MODEL40-INTEGRATION:OK\n");
    if (!failed) printf("M5:T386:S8:MODEL40-CONTROLS:OK\n");
    vm_machine_destroy(session);
    return failed ? 1 : 0;
}

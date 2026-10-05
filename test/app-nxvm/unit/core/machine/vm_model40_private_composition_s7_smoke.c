#include "app-nxvm/profiles/model40/observation_interface.h"
#include "../../../support/guest_input.h"
#include "../../../support/profile.h"
#include "app-nxvm/profiles/machine_factory_interface.h"
#include "../../../support/model40.h"
#include "../../../../x86/core/composition_fixture.h"
#include "../../../../ibmpc/board-common/composition_fixture.h"
#include "../../../../ibmpc/board-common/cmos_fixture.h"
#include "../../../../ibmpc/board-common/kbc_state_fixture.h"
#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include <stdio.h>

#include "ibmpc/machine/machine_private.h"
#include "ibmpc/machine/machine_interface.h"
#include "ibmpc/board-at/kbc_interface.h"
#include "support/rom/model40_session_assets.h"

lib_i32 main(void)
{
    static lib_u8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static lib_u8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_machine_config invalid_config = {0};
    vm_machine_assets missing_assets = {0};
    vm_machine *session = LIB_NULL;
    core_machine_cpu_profile cpu_profile;
    lib_size memory_bytes;
    core_machine_d4_platform_observation d4;
    core_machine_speaker_observation speaker;
    lib_u32 value = 0u;
    lib_u8 rom_byte = 0u;
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    lib_i32 failed = 0;

    even[0x3ff8u] = 0x26u;
    odd[0x3ff8u] = 0x90u;

    failed = vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        &invalid_config, &missing_assets, &session) !=
        LIB_STATUS_INVALID_ARGUMENT || session != LIB_NULL;
    if (!failed) failed = vm_model40_fixture_create_bytes(even, odd, &session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        const core_machine_transaction_contract transaction =
            test_core_capture_transaction_contract(session->core_machine);
        const test_board_composition_observation composition =
            test_board_capture_composition(session->board);
        const test_board_plan_observation plan =
            test_board_capture_plan(session->core_machine_plan);

        failed = !vm_profile_machine_plan_is_model40(vm_test_profile_plan(session)) ||
        test_core_retirement_contract(session->core_machine) !=
            CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC ||
        transaction.external_cycle_timing.page_bytes != 2048u ||
        transaction.external_cycle_timing.page_miss_ticks != 2u ||
        transaction.external_cycle_timing.page_hit_ticks != 0u ||
        transaction.external_cycle_timing.first_eligible_address != 0u ||
        transaction.external_cycle_timing.last_eligible_address != 0x0009ffffu ||
        transaction.external_access_wait_windows[0].space !=
            CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT ||
        transaction.external_access_wait_windows[0].first_address != 0x03b4u ||
        transaction.external_access_wait_windows[2].last_address != 0x03dcu ||
        transaction.external_access_wait_windows[5].first_address != 0x0fc6u ||
        transaction.external_access_wait_windows[5].wait_ticks != 1u ||
        transaction.external_access_wait_windows[6].space != CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY ||
        transaction.external_access_wait_windows[6].first_address != 0x000a0000u ||
        transaction.external_access_wait_windows[6].last_address != 0x000affffu ||
        transaction.cpu_cycle_bus_ready_gate_enabled != LIB_TRUE ||
        transaction.cpu_prefetch_reservation_enabled != LIB_TRUE ||
        transaction.external_cycle_timing.overlap_policy !=
            CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL ||
        composition.dma_clock.numerator != 1u ||
        composition.dma_clock.denominator != 1u ||
        composition.pit_clock.numerator != 1u ||
        composition.pit_clock.denominator != 1u ||
        composition.auxiliary_pit_clock.numerator != 5u ||
        composition.auxiliary_pit_clock.denominator != 16u ||
        composition.rtc_clock.numerator != 1u ||
        composition.rtc_clock.denominator != 1u ||
        core_machine_get_cpu_profile(session->core_machine, &cpu_profile) !=
            LIB_STATUS_OK || cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        core_machine_get_memory_bytes(session->core_machine, &memory_bytes) !=
            LIB_STATUS_OK || memory_bytes != 2u * 1024u * 1024u ||
        test_board_cmos_read_register(session->board, CORE_MACHINE_RTC_BASEMEM_LSB) != 0x80u ||
        test_board_cmos_read_register(session->board, CORE_MACHINE_RTC_BASEMEM_MSB) != 0x02u ||
        test_board_cmos_read_register(session->board, CORE_MACHINE_RTC_EXTMEM_LSB) != 0u ||
        test_board_cmos_read_register(session->board, CORE_MACHINE_RTC_EXTMEM_MSB) != 0x04u ||
        vm_test_model40_d4_observe(session, &d4) !=
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
            sizeof(rom_byte)) != LIB_STATUS_OK || rom_byte != 0x26u ||
        test_board_kbc_aux_enabled(session->board) ||
        core_machine_bus_write(session->core_machine, 0x0061u, 0x02u) !=
            LIB_STATUS_OK || core_machine_get_speaker_observation(
            session->board, &speaker) != LIB_STATUS_OK ||
        !speaker.configured || speaker.timer_gate || !speaker.data_enabled ||
        !speaker.output || core_machine_bus_write(session->core_machine, 0x0061u,
            0x0fu) != LIB_STATUS_OK ||
        plan.memory_bytes != 2u * 1024u * 1024u ||
        plan.cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        test_board_kbc_aux_enabled(session->board) ||
        !test_board_kbc_command_matches(session->board,
            session->core_machine, 0x20u,
            0x20u, 0x20u);
    }
    if (!failed) {
        /* Model-40 selects the existing generic-AT 2-tick initial prefetch
         * locality miss in addition to the deterministic base instruction tick. */
        failed = core_machine_run(session->core_machine, budget, &result) !=
            LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != 1u || result.ticks != 3u ||
            result.elapsed_ticks != 3u;
    }
    if (!failed) {
        core_machine_guest_input_event event = {0};

        event.kind = CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE;
        event.data.relative_mouse.delta_x = 1;
        event.data.relative_mouse.delta_y = 1;
        event.data.relative_mouse.buttons = 1u;
        failed = core_machine_bus_read(session->core_machine, 0x64u, &value) !=
            LIB_STATUS_OK || (value & 0x01u) != 0u ||
            vm_test_submit_host_input(session, &event) != LIB_STATUS_OK ||
            core_machine_bus_read(session->core_machine, 0x64u, &value) !=
            LIB_STATUS_OK || (value & 0x01u) != 0u ||
            core_machine_bus_write(session->core_machine, 0x0064u, 0xa8u) !=
            LIB_STATUS_OK || test_board_kbc_aux_enabled(session->board) ||
            !test_board_kbc_command_matches(session->board,
                session->core_machine, 0x20u, 0x20u, 0x20u) ||
            core_machine_bus_write(session->core_machine, 0x0060u, 0xf5u) !=
            LIB_STATUS_OK || test_board_kbc_read_reply(session->board,
                session->core_machine) != 0xfau ||
            core_machine_bus_write(session->core_machine, 0x0064u, 0xd4u) !=
            LIB_STATUS_OK ||
            core_machine_bus_write(session->core_machine, 0x0060u, 0xf4u) !=
            LIB_STATUS_OK || test_board_keyboard_scanning(session->board) ||
            core_machine_bus_read(session->core_machine, 0x64u, &value) !=
            LIB_STATUS_OK || (value & 0x01u) != 0u ||
            core_machine_bus_write(session->core_machine, 0x60u, 0xeeu) !=
            LIB_STATUS_OK || test_board_kbc_read_reply(session->board,
            session->core_machine) != 0xeeu;
    }
    if (!failed) printf("M5:T386:S7:MODEL40-PRIVATE-COMPOSITION:OK\n");
    if (!failed) printf("M5:T421:S1:MODEL40-SPEAKER-SELECTION:OK\n");
    if (!failed) printf("M5:T386:S7:EXTERNAL-ROM-GUARD:OK\n");
    if (!failed) printf("M5:T390:S34:MODEL40-DETERMINISTIC-CONTRACT:OK\n");
    if (!failed) printf("M5:T477:S3:DESKPRO-SESSION-CUTOVER:OK\n");
    vm_machine_destroy(session);
    return failed ? 1 : 0;
}

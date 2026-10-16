#include "app-mydeskpro386/machine_observation_interface.h"
#include "../../x86-nxvm/machine/support/guest_input.h"
#include "../support/profile.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "../support/model40.h"
#include "../../x86-nxvm/board-base/board_binding_fixture.h"
#include "../../x86-nxvm/board-base/support/composition_fixture.h"
#include "../../x86-nxvm/board-base/support/cmos_fixture.h"
#include "../../x86-nxvm/board-base/support/kbc_state_fixture.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "lib/types/file.h"

#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "x86-nxvm/board-at/kbc_interface.h"
#include "../support/model40_session_assets.h"
#include "x86-core/machine/clock_interface.h"

static lib_bool model40_clock_inputs_match(void)
{
    x86_nxvm_board_config config;
    x86_core_machine_clock_domain clock;
    x86_pit *pit = LIB_NULL;
    lib_bool saw_low = LIB_FALSE;
    lib_bool valid;

    mydeskpro386_core_config_initialize(&config);
    if (config.kbc_output_pulse_ticks != 32u) return LIB_FALSE;
    if (x86_core_machine_clock_domain_initialize(&clock, &config.clock_plan.pit) !=
            LIB_STATUS_OK || x86_pit_create(X86_PIT_PERSONALITY_8254, &pit) !=
            LIB_STATUS_OK) return LIB_FALSE;
    valid = x86_pit_write_register(pit, 3u, 0x74u) == LIB_STATUS_OK &&
        x86_pit_write_register(pit, 1u, 18u) == LIB_STATUS_OK &&
        x86_pit_write_register(pit, 1u, 0u) == LIB_STATUS_OK;
    x86_pit_set_gate(pit, 1u, LIB_TRUE);
    /* The source-backed ratio must not hide every low pulse from this
     * 32-source-tick poll. The old 1:1 input aliases this observation. */
    for (lib_u32 sample = 0u; valid && sample < 256u; ++sample) {
        x86_pit_advance(pit, x86_core_machine_clock_domain_advance(&clock, 32u));
        if (!x86_pit_get_output(pit, 1u)) saw_low = LIB_TRUE;
    }
    x86_pit_destroy(pit);
    if (!valid || !saw_low ||
        x86_core_machine_clock_domain_initialize(&clock, &config.clock_plan.rtc) !=
            LIB_STATUS_OK ||
        x86_core_machine_clock_domain_advance(&clock, 16000000u) != 32768u ||
        x86_core_machine_clock_domain_initialize(&clock, &config.clock_plan.dma) !=
            LIB_STATUS_OK ||
        x86_core_machine_clock_domain_advance(&clock, 64u) != 16u ||
        x86_core_machine_clock_domain_initialize(&clock,
            &config.clock_plan.auxiliary_pit) != LIB_STATUS_OK ||
        x86_core_machine_clock_domain_advance(&clock, 64u) != 16u) return LIB_FALSE;
    return LIB_TRUE;
}

lib_i32 main(void)
{
    static lib_u8 even[MYDESKPRO386_ROM_CHIP_BYTES];
    static lib_u8 odd[MYDESKPRO386_ROM_CHIP_BYTES];
    x86_nxvm_machine_config invalid_config = {0};
    x86_nxvm_machine_assets missing_assets = {0};
    x86_nxvm_machine *session = LIB_NULL;
    x86_core_cpu_profile cpu_profile;
    lib_size memory_bytes;
    mydeskpro386_d4_platform_observation d4;
    x86_nxvm_board_speaker_observation speaker;
    lib_u32 value = 0u;
    lib_u8 rom_byte = 0u;
    const x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result result;
    x86_core_machine_executor_config executor = {0};
    lib_size constructor_count = 0u;
    lib_i32 failed = !model40_clock_inputs_match();

    even[0x3ff8u] = 0x26u;
    odd[0x3ff8u] = 0x90u;

    if (!failed) failed = vm_test_machine_create_from_assets(VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40,
        &invalid_config, &missing_assets, &session) !=
        LIB_STATUS_INVALID_ARGUMENT || session != LIB_NULL;
    test_board_observe_constructor(&executor, &constructor_count);
    if (!failed) failed = vm_model40_fixture_create_bytes(even, odd, &session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    test_board_observe_constructor(LIB_NULL, LIB_NULL);
    failed |= constructor_count != 1u;
    if (!failed) {
        const x86_core_machine_transaction_contract transaction =
            executor.transaction_contract;
        const test_board_composition_observation composition =
            test_board_capture_composition(session->board);
        const test_board_plan_observation plan =
            test_board_capture_plan(session->x86_nxvm_board_plan);

        failed = !vm_test_profile_is_model40(vm_test_profile_construction(session)) ||
        executor.retirement_time_contract !=
            X86_CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC ||
        transaction.external_cycle_timing.page_bytes != 2048u ||
        transaction.external_cycle_timing.page_miss_ticks != 2u ||
        transaction.external_cycle_timing.page_hit_ticks != 0u ||
        transaction.external_cycle_timing.first_eligible_address != 0u ||
        transaction.external_cycle_timing.last_eligible_address != 0x0009ffffu ||
        transaction.external_access_wait_windows[0].space !=
            X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT ||
        transaction.external_access_wait_windows[0].first_address != 0x03b4u ||
        transaction.external_access_wait_windows[2].last_address != 0x03dcu ||
        transaction.external_access_wait_windows[5].first_address != 0x0fc6u ||
        transaction.external_access_wait_windows[5].wait_ticks != 1u ||
        transaction.external_access_wait_windows[6].space != X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY ||
        transaction.external_access_wait_windows[6].first_address != 0x000a0000u ||
        transaction.external_access_wait_windows[6].last_address != 0x000affffu ||
        transaction.cpu_cycle_bus_ready_gate_enabled != LIB_TRUE ||
        transaction.cpu_prefetch_reservation_enabled != LIB_TRUE ||
        transaction.external_cycle_timing.overlap_policy !=
            X86_CORE_MACHINE_EXTERNAL_CYCLE_OVERLAP_EXPLICIT_SEQUENTIAL ||
        composition.dma_clock.numerator != 1u ||
        composition.dma_clock.denominator != 4u ||
        composition.pit_clock.numerator != 715909u ||
        composition.pit_clock.denominator != 9600000u ||
        composition.auxiliary_pit_clock.numerator != 1u ||
        composition.auxiliary_pit_clock.denominator != 4u ||
        composition.rtc_clock.numerator != 256u ||
        composition.rtc_clock.denominator != 125000u ||
        x86_core_machine_get_cpu_profile(session->x86_core_machine, &cpu_profile) !=
            LIB_STATUS_OK || cpu_profile != X86_CORE_CPU_PROFILE_80386 ||
        x86_core_machine_get_memory_bytes(session->x86_core_machine, &memory_bytes) !=
            LIB_STATUS_OK || memory_bytes != 2u * 1024u * 1024u ||
        test_board_cmos_read_register(session->board, X86_NXVM_BOARD_RTC_BASEMEM_LSB) != 0x80u ||
        test_board_cmos_read_register(session->board, X86_NXVM_BOARD_RTC_BASEMEM_MSB) != 0x02u ||
        test_board_cmos_read_register(session->board, X86_NXVM_BOARD_RTC_EXTMEM_LSB) != 0u ||
        test_board_cmos_read_register(session->board, X86_NXVM_BOARD_RTC_EXTMEM_MSB) != 0x04u ||
        vm_test_model40_d4_observe(session, &d4) !=
            LIB_STATUS_OK || !d4.configured || d4.iochk_enabled ||
        d4.failsafe_enabled ||
        x86_core_machine_bus_read(session->x86_core_machine, 0x07c6u, &value) !=
            LIB_STATUS_OK || value != 0x0du ||
        x86_core_machine_bus_read(session->x86_core_machine, 0x0bc6u, &value) !=
            LIB_STATUS_OK || value != 0x30u ||
        x86_core_machine_bus_read(session->x86_core_machine, 0x0fc6u, &value) !=
            LIB_STATUS_OK || value != 0x01u ||
        x86_core_machine_bus_read(session->x86_core_machine, 0x0061u, &value) !=
            LIB_STATUS_OK || value != 0x1fu ||
        x86_core_machine_memory_read(session->x86_core_machine, 0x000ffff0u, &rom_byte,
            sizeof(rom_byte)) != LIB_STATUS_OK || rom_byte != 0x26u ||
        test_board_kbc_aux_enabled(session->board) ||
        x86_core_machine_bus_write(session->x86_core_machine, 0x0061u, 0x02u) !=
            LIB_STATUS_OK || x86_nxvm_board_get_speaker_observation(
            session->board, &speaker) != LIB_STATUS_OK ||
        !speaker.configured || speaker.timer_gate || !speaker.data_enabled ||
        speaker.output != speaker.timer_output || x86_core_machine_bus_write(session->x86_core_machine, 0x0061u,
            0x0fu) != LIB_STATUS_OK ||
        plan.memory_bytes != 2u * 1024u * 1024u ||
        plan.cpu_profile != X86_CORE_CPU_PROFILE_80386 ||
        test_board_kbc_aux_enabled(session->board) ||
        !test_board_kbc_command_matches(session->board,
            session->x86_core_machine, 0x20u,
            0x20u, 0u);
    }
    if (!failed) {
        /* Model-40 selects the existing generic-AT 2-tick initial prefetch
         * locality miss in addition to the deterministic base instruction tick. */
        failed = x86_core_machine_run(session->x86_core_machine, budget, &result) !=
            LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
            result.executed != 1u || result.ticks != 3u ||
            result.elapsed_ticks != 3u;
    }
    if (!failed) {
        core_machine_guest_input_event event = {0};

        event.kind = CORE_MACHINE_GUEST_INPUT_RELATIVE_MOUSE;
        event.data.relative_mouse.delta_x = 1;
        event.data.relative_mouse.delta_y = 1;
        event.data.relative_mouse.buttons = 1u;
        failed = x86_core_machine_bus_read(session->x86_core_machine, 0x64u, &value) !=
            LIB_STATUS_OK || (value & 0x01u) != 0u ||
            vm_test_submit_host_input(session, &event) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(session->x86_core_machine, 0x64u, &value) !=
            LIB_STATUS_OK || (value & 0x01u) != 0u ||
            x86_core_machine_bus_write(session->x86_core_machine, 0x0064u, 0xa8u) !=
            LIB_STATUS_OK || test_board_kbc_aux_enabled(session->board) ||
            !test_board_kbc_command_matches(session->board,
                session->x86_core_machine, 0x20u, 0x20u, 0u) ||
            x86_core_machine_bus_write(session->x86_core_machine, 0x0060u, 0xf5u) !=
            LIB_STATUS_OK || test_board_kbc_read_reply(session->board,
                session->x86_core_machine) != 0xfau ||
            x86_core_machine_bus_write(session->x86_core_machine, 0x0064u, 0xd4u) !=
            LIB_STATUS_OK ||
            x86_core_machine_bus_write(session->x86_core_machine, 0x0060u, 0xf4u) !=
            LIB_STATUS_OK || test_board_keyboard_scanning(session->board) ||
            x86_core_machine_bus_read(session->x86_core_machine, 0x64u, &value) !=
            LIB_STATUS_OK || (value & 0x01u) != 0u ||
            x86_core_machine_bus_write(session->x86_core_machine, 0x60u, 0xeeu) !=
            LIB_STATUS_OK || test_board_kbc_read_reply(session->board,
            session->x86_core_machine) != 0xeeu;
    }
    if (!failed) lib_c_printf("MODEL40-COMPOSITION:OK\n");
    if (!failed) lib_c_printf("MODEL40-SPEAKER-SELECTION:OK\n");
    if (!failed) lib_c_printf("EXTERNAL-ROM-GUARD:OK\n");
    if (!failed) lib_c_printf("MODEL40-DETERMINISTIC-CONTRACT:OK\n");
    if (!failed) lib_c_printf("DESKPRO-SESSION-CUTOVER:OK\n");
    x86_nxvm_machine_destroy(session);
    return failed ? 1 : 0;
}

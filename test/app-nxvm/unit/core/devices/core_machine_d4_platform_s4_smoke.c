#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86/core/machine.h"
#include "x86/core/debug_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"

#include "support/core_machine_board_fixture.h"

static lib_i32 core_machine_port_b_exclusivity(void)
{
    core_machine_config config = {0};
    core_machine_planar_parity_config planar = {
        .port = CORE_MACHINE_PC_AT_PORT_B,
        .memory_bytes = 512u * 1024u,
        .refresh_status_source = CORE_MACHINE_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1,
        .refresh_status_toggle_ticks = 0u
    };
    core_machine_d4_platform_config d4 = {CORE_MACHINE_PC_AT_PORT_B, 0u};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    config.memory_bytes = 2u * 1024u * 1024u;
    config.auxiliary_pit_present = LIB_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    failed |= core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_configure_planar_parity(board, &planar) != LIB_STATUS_OK ||
        core_machine_configure_d4_platform(board, &d4) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, CORE_MACHINE_PC_AT_PORT_B, &value) !=
            LIB_STATUS_OK || (value & 0x0fu) != 0x04u;
    core_machine_destroy(machine);

    machine = LIB_NULL;
    value = 0u;
    failed |= core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_configure_d4_platform(board, &d4) != LIB_STATUS_OK ||
        core_machine_configure_d4_platform(board, &d4) != LIB_STATUS_INVALID_STATE ||
        core_machine_configure_planar_parity(board, &planar) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, CORE_MACHINE_PC_AT_PORT_B, &value) !=
            LIB_STATUS_OK || (value & 0x0fu) != 0x0fu;
    core_machine_destroy(machine);
    return failed;
}
static lib_u16 read_refresh_count(core_machine *machine)
{
    lib_u16 low;
    core_machine_port_write(&machine->executor_port, 0x0043u, 0x40u);
    low = core_machine_port_read(&machine->executor_port, 0x0041u);
    return (lib_u16)(low | (core_machine_port_read(&machine->executor_port, 0x0041u) << 8u));
}

lib_i32 main(void)
{
    core_machine_config config = {0};
    core_machine_d4_platform_config d4 = {CORE_MACHINE_PC_AT_PORT_B, 0u};
    core_machine_rtc_cmos_config cmos = {0};
    core_machine_d4_platform_observation observation;
    core_machine_speaker_observation speaker;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    config.memory_bytes = 2u * 1024u * 1024u;
    config.auxiliary_pit_present = LIB_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    cmos.index_port = 0x0070u;
    cmos.data_port = 0x0071u;
    cmos.irq = 8u;
    cmos.nmi_mask_bit = 0x80u;
    cmos.ticks_per_second = 1u;
    failed |= core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        core_machine_configure_d4_platform(board, &d4) != LIB_STATUS_OK ||
        core_machine_configure_rtc_cmos(board, &cmos) != LIB_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
        read_refresh_count(machine) != 18u ||
        core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        value != 0x1fu ||
        core_machine_get_d4_platform_observation(board, &observation) !=
            LIB_STATUS_OK || !observation.configured || observation.iochk_enabled ||
        observation.failsafe_enabled || observation.iochk_latched ||
        observation.failsafe_latched || observation.nmi_signaled;
    if (!failed) printf("M5:T386:S4:D4-PLATFORM-PORT:OK\n");
    if (!failed) failed |= core_machine_bus_write(machine, 0x0043u, 0x74u) !=
            LIB_STATUS_OK || core_machine_bus_write(machine, 0x0041u, 2u) !=
            LIB_STATUS_OK || core_machine_bus_write(machine, 0x0041u, 0u) !=
            LIB_STATUS_OK || core_machine_bus_write(machine, 0x0043u, 0xb4u) !=
            LIB_STATUS_OK || core_machine_bus_write(machine, 0x0042u, 2u) !=
            LIB_STATUS_OK || core_machine_bus_write(machine, 0x0042u, 0u) !=
            LIB_STATUS_OK || core_machine_bus_read(machine, 0x0061u, &value) !=
            LIB_STATUS_OK || (value & 0x30u) != 0x30u ||
        core_machine_bus_write(machine, 0x0061u, 0x0au) != LIB_STATUS_OK ||
        core_machine_advance_time(machine, 3u) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        (value & 0x30u) != 0x20u ||
        core_machine_bus_write(machine, 0x0061u, 0x0bu) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        (value & 0x20u) != 0x20u || core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK || value != 0x1fu;
    if (!failed) printf("M5:T386:S25:D4-PORT-B-SYSTEM-PIT:OK\n");

    if (!failed) failed |= core_machine_bus_write(machine, 0x0061u, 0x02u) !=
        LIB_STATUS_OK || core_machine_get_speaker_observation(board, &speaker) !=
        LIB_STATUS_OK || !speaker.configured || speaker.timer_gate ||
        !speaker.data_enabled || !speaker.output ||
        core_machine_bus_write(machine, 0x0061u, 0x00u) != LIB_STATUS_OK ||
        core_machine_get_speaker_observation(board, &speaker) != LIB_STATUS_OK ||
        speaker.output || core_machine_bus_write(machine, 0x0043u, 0xb4u) !=
        LIB_STATUS_OK || core_machine_bus_write(machine, 0x0042u, 2u) !=
        LIB_STATUS_OK || core_machine_bus_write(machine, 0x0042u, 0u) !=
        LIB_STATUS_OK || core_machine_bus_write(machine, 0x0061u, 0x03u) !=
        LIB_STATUS_OK || core_machine_get_speaker_observation(board, &speaker) !=
        LIB_STATUS_OK || !speaker.timer_gate || !speaker.data_enabled ||
        !speaker.timer_output || !speaker.output || core_machine_advance_time(machine,
        3u) != LIB_STATUS_OK || core_machine_get_speaker_observation(board,
        &speaker) != LIB_STATUS_OK || speaker.timer_output || speaker.output ||
        core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
        core_machine_get_speaker_observation(board, &speaker) != LIB_STATUS_OK ||
        !speaker.timer_output || !speaker.output;
    if (!failed) printf("M5:T421:S1:D4-SPEAKER-LINE:OK\n");
    if (!failed) failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_get_speaker_observation(board, &speaker) != LIB_STATUS_OK ||
        !speaker.configured || !speaker.timer_gate || !speaker.data_enabled ||
        speaker.output != speaker.timer_output;

    if (!failed) failed |= test_core_machine_fixture_nmi_prepare(machine);
    if (!failed) failed |= core_machine_bus_write(machine, 0x0061u, 0x03u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0070u, 0x80u) !=
            LIB_STATUS_OK || core_machine_report_d4_iochk_fault(board) !=
            LIB_STATUS_OK || core_machine_get_d4_platform_observation(board,
            &observation) != LIB_STATUS_OK || !observation.iochk_latched ||
        observation.nmi_signaled ||
        core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        value != 0x53u || test_core_machine_fixture_nmi_execute(machine, LIB_FALSE) ||
        core_machine_bus_write(machine, 0x0070u, 0u) !=
            LIB_STATUS_OK || core_machine_get_d4_platform_observation(board,
            &observation) != LIB_STATUS_OK || !observation.nmi_signaled ||
        test_core_machine_fixture_nmi_execute(machine, LIB_TRUE);
    if (!failed) printf("M5:T386:S4:D4-NMI-MASK:OK\n");

    if (!failed) failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
        test_core_machine_fixture_nmi_prepare(machine) ||
        core_machine_bus_write(machine, 0x0061u, 0u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0064u, 0xd0u) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x0060u, &value) != LIB_STATUS_OK || value != 1u ||
        core_machine_bus_write(machine, 0x004bu, 0x30u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0048u, 1u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x0048u, 0u) != LIB_STATUS_OK ||
        core_machine_advance_time(machine, 2u) != LIB_STATUS_OK ||
        core_machine_get_d4_platform_observation(board, &observation) !=
            LIB_STATUS_OK || !observation.failsafe_enabled ||
        !observation.failsafe_latched || !observation.nmi_signaled ||
        core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        value != 0x90u || test_core_machine_fixture_nmi_execute(machine, LIB_TRUE);
    if (!failed) printf("M5:T386:S4:D4-FAILSAFE-ROUTE:OK\n");

    if (!failed) {
        core_machine_run_result result;
        core_machine_cpu_state cpu;
        lib_u64 elapsed_before_shutdown;

        elapsed_before_shutdown = machine->elapsed_ticks;
        failed |= core_machine_debug_write_register(machine,
            CORE_MACHINE_DEBUG_EIP, 0x0000fff0u) != LIB_STATUS_OK ||
            !core_machine_cpu_is_halted(machine->executor_cpu_execution);
        core_machine_cpu_execution_request_shutdown(machine->executor_cpu_execution);
        failed |= core_machine_run(machine, (core_machine_run_budget){1u, 0u},
            &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_RESET_REQUESTED ||
            core_machine_get_cpu_state(machine, &cpu) != LIB_STATUS_OK ||
            cpu.eip != 0x0000fff0u ||
            machine->elapsed_ticks != elapsed_before_shutdown ||
            core_machine_get_d4_platform_observation(board, &observation) !=
                LIB_STATUS_OK || !observation.failsafe_enabled ||
            !observation.failsafe_latched;
    }
    if (!failed) failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_get_d4_platform_observation(board, &observation) !=
            LIB_STATUS_OK || observation.iochk_enabled || observation.failsafe_enabled ||
        observation.iochk_latched || observation.failsafe_latched ||
        observation.nmi_signaled ||
        core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        value != 0x1fu || test_core_machine_fixture_nmi_prepare(machine) ||
        test_core_machine_fixture_nmi_execute(machine, LIB_FALSE);
    if (!failed) failed |= core_machine_port_b_exclusivity();
    core_machine_destroy(machine);
    if (failed) return 1;
    printf("M5:T421:S2:PORT-B-EXCLUSIVITY:OK\n");
    printf("M5:T386:S4:D4-RESET-ISOLATION:OK\n");
    printf("M5:T386:S23:D4-RESET-ARBITRATION:OK\n");
    printf("M5:T399:S2:B3-ACTIVE-LOW-NMI:OK\n");
    return 0;
}

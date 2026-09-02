#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"

#include "../support/core_machine_cpu_fixture.h"

static C_INT core_machine_port_b_exclusivity(C_VOID)
{
    core_machine_config config = {0};
    core_machine_planar_parity_config planar = {CORE_MACHINE_PC_AT_PORT_B,
        512u * 1024u};
    core_machine_d4_platform_config d4 = {CORE_MACHINE_PC_AT_PORT_B, 0u};
    core_machine *machine = STD_NULL;
    type_unsigned_32 value = 0u;
    C_INT failed = 0;

    config.memory_bytes = 2u * 1024u * 1024u;
    config.auxiliary_pit_present = TYPE_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_configure_planar_parity(machine, &planar) != TYPE_STATUS_OK ||
        core_machine_configure_d4_platform(machine, &d4) != TYPE_STATUS_INVALID_ARGUMENT ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_bus_read(machine, CORE_MACHINE_PC_AT_PORT_B, &value) !=
            TYPE_STATUS_OK || (value & 0x0fu) != 0x04u;
    core_machine_destroy(machine);

    machine = STD_NULL;
    value = 0u;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_configure_d4_platform(machine, &d4) != TYPE_STATUS_OK ||
        core_machine_configure_planar_parity(machine, &planar) != TYPE_STATUS_INVALID_ARGUMENT ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_bus_read(machine, CORE_MACHINE_PC_AT_PORT_B, &value) !=
            TYPE_STATUS_OK || (value & 0x0fu) != 0x0fu;
    core_machine_destroy(machine);
    return failed;
}
C_INT main(C_VOID)
{
    core_machine_config config = {0};
    core_machine_d4_platform_config d4 = {CORE_MACHINE_PC_AT_PORT_B, 0u};
    core_machine_rtc_cmos_config cmos = {0};
    core_machine_d4_platform_observation observation;
    core_machine_speaker_observation speaker;
    core_machine *machine = STD_NULL;
    type_unsigned_32 value = 0u;
    C_INT failed = 0;

    config.memory_bytes = 2u * 1024u * 1024u;
    config.auxiliary_pit_present = TYPE_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    cmos.index_port = 0x0070u;
    cmos.data_port = 0x0071u;
    cmos.irq = 8u;
    cmos.nmi_mask_bit = 0x80u;
    cmos.ticks_per_second = 1u;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_configure_d4_platform(machine, &d4) != TYPE_STATUS_OK ||
        core_machine_configure_rtc_cmos(machine, &cmos) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_advance_time(machine, 1u) != TYPE_STATUS_OK ||
        machine->shared_pit.data.reload[1u] != 18u ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK ||
        value != 0x1fu ||
        core_machine_get_d4_platform_observation(machine, &observation) !=
            TYPE_STATUS_OK || !observation.configured || observation.iochk_enabled ||
        observation.failsafe_enabled || observation.iochk_latched ||
        observation.failsafe_latched || observation.nmi_signaled;
    if (!failed) STD_PRINTF("M5:T386:S4:D4-PLATFORM-PORT:OK\n");
    if (!failed) failed |= core_machine_bus_write(machine, 0x0043u, 0x74u) !=
            TYPE_STATUS_OK || core_machine_bus_write(machine, 0x0041u, 2u) !=
            TYPE_STATUS_OK || core_machine_bus_write(machine, 0x0041u, 0u) !=
            TYPE_STATUS_OK || core_machine_bus_write(machine, 0x0043u, 0xb4u) !=
            TYPE_STATUS_OK || core_machine_bus_write(machine, 0x0042u, 2u) !=
            TYPE_STATUS_OK || core_machine_bus_write(machine, 0x0042u, 0u) !=
            TYPE_STATUS_OK || core_machine_bus_read(machine, 0x0061u, &value) !=
            TYPE_STATUS_OK || (value & 0x30u) != 0x30u ||
        core_machine_bus_write(machine, 0x0061u, 0x0au) != TYPE_STATUS_OK ||
        core_machine_advance_time(machine, 3u) != TYPE_STATUS_OK ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK ||
        (value & 0x30u) != 0x20u ||
        core_machine_bus_write(machine, 0x0061u, 0x0bu) != TYPE_STATUS_OK ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK ||
        (value & 0x20u) != 0x20u || core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK || value != 0x1fu;
    if (!failed) STD_PRINTF("M5:T386:S25:D4-PORT-B-SYSTEM-PIT:OK\n");

    if (!failed) failed |= core_machine_bus_write(machine, 0x0061u, 0x02u) !=
        TYPE_STATUS_OK || core_machine_get_speaker_observation(machine, &speaker) !=
        TYPE_STATUS_OK || !speaker.configured || speaker.timer_gate ||
        !speaker.data_enabled || !speaker.output ||
        core_machine_bus_write(machine, 0x0061u, 0x00u) != TYPE_STATUS_OK ||
        core_machine_get_speaker_observation(machine, &speaker) != TYPE_STATUS_OK ||
        speaker.output || core_machine_bus_write(machine, 0x0043u, 0xb4u) !=
        TYPE_STATUS_OK || core_machine_bus_write(machine, 0x0042u, 2u) !=
        TYPE_STATUS_OK || core_machine_bus_write(machine, 0x0042u, 0u) !=
        TYPE_STATUS_OK || core_machine_bus_write(machine, 0x0061u, 0x03u) !=
        TYPE_STATUS_OK || core_machine_get_speaker_observation(machine, &speaker) !=
        TYPE_STATUS_OK || !speaker.timer_gate || !speaker.data_enabled ||
        !speaker.timer_output || !speaker.output || core_machine_advance_time(machine,
        3u) != TYPE_STATUS_OK || core_machine_get_speaker_observation(machine,
        &speaker) != TYPE_STATUS_OK || speaker.timer_output || speaker.output ||
        core_machine_advance_time(machine, 1u) != TYPE_STATUS_OK ||
        core_machine_get_speaker_observation(machine, &speaker) != TYPE_STATUS_OK ||
        !speaker.timer_output || !speaker.output;
    if (!failed) STD_PRINTF("M5:T421:S1:D4-SPEAKER-LINE:OK\n");
    if (!failed) failed |= core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_get_speaker_observation(machine, &speaker) != TYPE_STATUS_OK ||
        !speaker.configured || !speaker.timer_gate || !speaker.data_enabled ||
        speaker.output != speaker.timer_output;

    if (!failed) failed |= core_machine_bus_write(machine, 0x0061u, 0x03u) != TYPE_STATUS_OK ||
        core_machine_bus_write(machine, 0x0070u, 0x80u) !=
            TYPE_STATUS_OK || core_machine_report_d4_iochk_fault(machine) !=
            TYPE_STATUS_OK || core_machine_get_d4_platform_observation(machine,
            &observation) != TYPE_STATUS_OK || !observation.iochk_latched ||
        observation.nmi_signaled || machine->executor_cpu.data.flagNMI ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK ||
        value != 0x53u || core_machine_bus_write(machine, 0x0070u, 0u) !=
            TYPE_STATUS_OK || core_machine_get_d4_platform_observation(machine,
            &observation) != TYPE_STATUS_OK || !observation.nmi_signaled ||
        !machine->executor_cpu.data.flagNMI;
    if (!failed) STD_PRINTF("M5:T386:S4:D4-NMI-MASK:OK\n");

    if (!failed) failed |= core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_bus_write(machine, 0x0061u, 0u) != TYPE_STATUS_OK ||
        machine->shared_kbc.data.output_port != 1u ||
        core_machine_bus_write(machine, 0x004bu, 0x30u) != TYPE_STATUS_OK ||
        core_machine_bus_write(machine, 0x0048u, 1u) != TYPE_STATUS_OK ||
        core_machine_bus_write(machine, 0x0048u, 0u) != TYPE_STATUS_OK ||
        core_machine_advance_time(machine, 2u) != TYPE_STATUS_OK ||
        core_machine_get_d4_platform_observation(machine, &observation) !=
            TYPE_STATUS_OK || !observation.failsafe_enabled ||
        !observation.failsafe_latched || !observation.nmi_signaled ||
        !machine->executor_cpu.data.flagNMI ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK ||
        value != 0x90u;
    if (!failed) STD_PRINTF("M5:T386:S4:D4-FAILSAFE-ROUTE:OK\n");

    if (!failed) {
        core_machine_run_result result;
        type_unsigned_64 elapsed_before_shutdown;

        elapsed_before_shutdown = machine->elapsed_ticks;
        core_machine_cpu_execution_request_shutdown(&machine->executor_cpu_execution);
        machine->executor_cpu.data.flagHalt = TYPE_TRUE;
        failed |= core_machine_run(machine, (core_machine_run_budget){1u, 0u},
            &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_RESET_REQUESTED ||
            machine->executor_cpu.data.eip != 0x0000fff0u ||
            machine->elapsed_ticks != elapsed_before_shutdown ||
            core_machine_get_d4_platform_observation(machine, &observation) !=
                TYPE_STATUS_OK || !observation.failsafe_enabled ||
            !observation.failsafe_latched;
    }
    if (!failed) failed |= core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_get_d4_platform_observation(machine, &observation) !=
            TYPE_STATUS_OK || observation.iochk_enabled || observation.failsafe_enabled ||
        observation.iochk_latched || observation.failsafe_latched ||
        observation.nmi_signaled || machine->executor_cpu.data.flagNMI ||
        core_machine_bus_read(machine, 0x0061u, &value) != TYPE_STATUS_OK ||
        value != 0x1fu;
    if (!failed) failed |= core_machine_port_b_exclusivity();
    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T421:S2:PORT-B-EXCLUSIVITY:OK\n");
    STD_PRINTF("M5:T386:S4:D4-RESET-ISOLATION:OK\n");
    STD_PRINTF("M5:T386:S23:D4-RESET-ARBITRATION:OK\n");
    STD_PRINTF("M5:T399:S2:B3-ACTIVE-LOW-NMI:OK\n");
    return 0;
}

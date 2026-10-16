#include "app-mydeskpro386/d4_platform_interface.h"
#include "app-mydeskpro386/d4_platform.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "../../x86-nxvm/board-base/composition/composition_fixture.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

#include "../../x86-nxvm/board-base/board_board_fixture.h"

static lib_i32 core_machine_port_b_exclusivity(void)
{
    x86_nxvm_board_config config = {0};
    x86_nxvm_board_planar_parity_config planar = {
        .port = X86_NXVM_BOARD_PC_AT_PORT_B,
        .memory_bytes = 512u * 1024u,
        .refresh_status_source = X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_PIT_COUNTER_1,
        .refresh_status_toggle_ticks = 0u
    };
    mydeskpro386_d4_platform_config d4 = {X86_NXVM_BOARD_PC_AT_PORT_B, 0u, 2u, 18u};
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    mydeskpro386_d4_platform *d4_board = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = 0;

    config.memory_bytes = 2u * 1024u * 1024u;
    config.auxiliary_pit_present = LIB_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    failed |= x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK ||
        x86_nxvm_board_configure_planar_parity(board, &planar) != LIB_STATUS_OK ||
        mydeskpro386_d4_platform_attach(board, &d4, &d4_board) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, X86_NXVM_BOARD_PC_AT_PORT_B, &value) !=
            LIB_STATUS_OK || (value & 0x0fu) != 0x04u;
    x86_core_machine_destroy(machine);

    machine = LIB_NULL;
    value = 0u;
    failed |= x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK ||
        mydeskpro386_d4_platform_attach(board, &d4, &d4_board) != LIB_STATUS_OK ||
        mydeskpro386_d4_platform_attach(board, &d4, &d4_board) != LIB_STATUS_INVALID_STATE ||
        x86_nxvm_board_configure_planar_parity(board, &planar) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, X86_NXVM_BOARD_PC_AT_PORT_B, &value) !=
            LIB_STATUS_OK || (value & 0x0fu) != 0x0fu;
    x86_core_machine_destroy(machine);
    return failed;
}
static lib_u16 read_refresh_count(x86_core_machine *machine)
{
    lib_u16 low;
    test_x86_nxvm_write_port_after_run(machine, 0x0043u, 0x40u);
    low = test_x86_nxvm_read_port_after_run(machine, 0x0041u);
    return (lib_u16)(low | (test_x86_nxvm_read_port_after_run(machine, 0x0041u) << 8u));
}

static lib_i32 mydeskpro386_d4_speed_command_contract(
    mydeskpro386_d4_platform *platform)
{
    lib_u8 value = 0u;

    return platform == LIB_NULL ||
        mydeskpro386_d4_platform_kbc_command(platform, 0xa1u, 0xb4u, 0x20u,
            &value) != X86_KBC8042_COMMAND_EXTENSION_UNHANDLED ||
        mydeskpro386_d4_platform_kbc_command(platform, 0xa5u, 0xb4u, 0x20u,
            &value) != X86_KBC8042_COMMAND_EXTENSION_SPECIAL_READ ||
        value != 0xa4u ||
        mydeskpro386_d4_platform_kbc_command(platform, 0xa5u, 0xb4u, 0x21u,
            &value) != X86_KBC8042_COMMAND_EXTENSION_SPECIAL_READ ||
        value != 0xb4u ||
        mydeskpro386_d4_platform_kbc_command(platform, 0xa3u, 0xb4u, 0x20u,
            &value) != X86_KBC8042_COMMAND_EXTENSION_HANDLED ||
        !platform->d4_speed_control_enabled ||
        !platform->d4_speed_switch_selected ||
        mydeskpro386_d4_platform_kbc_command(platform, 0xa4u, 0xb4u, 0x20u,
            &value) != X86_KBC8042_COMMAND_EXTENSION_HANDLED ||
        platform->d4_speed_switch_selected ||
        mydeskpro386_d4_platform_kbc_command(platform, 0xa4u, 0xa4u, 0x20u,
            &value) != X86_KBC8042_COMMAND_EXTENSION_HANDLED ||
        platform->d4_speed_control_enabled ||
        !platform->d4_speed_switch_selected;
}

lib_i32 main(void)
{
    x86_nxvm_board_config config = {0};
    mydeskpro386_d4_platform_config d4 = {X86_NXVM_BOARD_PC_AT_PORT_B, 0u, 2u, 18u};
    x86_nxvm_board_rtc_cmos_config cmos = {0};
    mydeskpro386_d4_platform_observation observation;
    x86_nxvm_board_speaker_observation speaker;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    mydeskpro386_d4_platform *d4_board = LIB_NULL;
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
    failed |= x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK ||
        mydeskpro386_d4_platform_attach(board, &d4, &d4_board) != LIB_STATUS_OK ||
        x86_nxvm_board_configure_rtc_cmos(board, &cmos) != LIB_STATUS_OK ||
        test_x86_nxvm_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
            0x000ffff0u, 16u) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;
    /* Drive the App's actual borrowed PIT inputs. Executor deadline/time
     * wiring is covered separately by d4_refresh_deadline_smoke. */
    if (!failed) x86_pit_advance(d4_board->shared_pit, 1u);
    if (!failed) failed |= read_refresh_count(machine) != 18u ||
        x86_core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        value != 0x1fu ||
        mydeskpro386_d4_platform_observe(d4_board, &observation) !=
            LIB_STATUS_OK || !observation.configured || observation.iochk_enabled ||
        observation.failsafe_enabled || observation.iochk_latched ||
        observation.failsafe_latched || observation.nmi_signaled;
    if (!failed) lib_c_printf("D4-PLATFORM-PORT:OK\n");
    if (!failed) failed |= mydeskpro386_d4_speed_command_contract(d4_board);
    if (!failed) lib_c_printf("D4-KBC-SPEED-COMMANDS:OK\n");
    if (!failed) failed |= x86_core_machine_bus_write(machine, 0x0043u, 0x74u) !=
            LIB_STATUS_OK || x86_core_machine_bus_write(machine, 0x0041u, 2u) !=
            LIB_STATUS_OK || x86_core_machine_bus_write(machine, 0x0041u, 0u) !=
            LIB_STATUS_OK || x86_core_machine_bus_write(machine, 0x0043u, 0xb4u) !=
            LIB_STATUS_OK || x86_core_machine_bus_write(machine, 0x0042u, 2u) !=
            LIB_STATUS_OK || x86_core_machine_bus_write(machine, 0x0042u, 0u) !=
            LIB_STATUS_OK || x86_core_machine_bus_read(machine, 0x0061u, &value) !=
            LIB_STATUS_OK || (value & 0x30u) != 0x30u ||
        x86_core_machine_bus_write(machine, 0x0061u, 0x0au) != LIB_STATUS_OK;
    if (!failed) x86_pit_advance(d4_board->shared_pit, 2u);
    if (!failed) failed |= x86_core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        (value & 0x30u) != 0x20u ||
        x86_core_machine_bus_write(machine, 0x0061u, 0x0bu) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        (value & 0x20u) != 0x20u || x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK || value != 0x1fu;
    if (!failed) lib_c_printf("D4-PORT-B-SYSTEM-PIT:OK\n");

    if (!failed) failed |= x86_core_machine_bus_write(machine, 0x0061u, 0x02u) !=
        LIB_STATUS_OK || x86_nxvm_board_get_speaker_observation(board, &speaker) !=
        LIB_STATUS_OK || !speaker.configured || speaker.timer_gate ||
        !speaker.data_enabled || speaker.output != speaker.timer_output ||
        x86_core_machine_bus_write(machine, 0x0061u, 0x00u) != LIB_STATUS_OK ||
        x86_nxvm_board_get_speaker_observation(board, &speaker) != LIB_STATUS_OK ||
        speaker.output || x86_core_machine_bus_write(machine, 0x0043u, 0xb4u) !=
        LIB_STATUS_OK || x86_core_machine_bus_write(machine, 0x0042u, 2u) !=
        LIB_STATUS_OK || x86_core_machine_bus_write(machine, 0x0042u, 0u) !=
        LIB_STATUS_OK || x86_core_machine_bus_write(machine, 0x0061u, 0x03u) !=
        LIB_STATUS_OK || x86_nxvm_board_get_speaker_observation(board, &speaker) !=
        LIB_STATUS_OK || !speaker.timer_gate || !speaker.data_enabled ||
        !speaker.timer_output || !speaker.output;
    if (!failed) x86_pit_advance(d4_board->shared_pit, 2u);
    if (!failed) failed |= x86_nxvm_board_get_speaker_observation(board,
        &speaker) != LIB_STATUS_OK || speaker.timer_output || speaker.output;
    if (!failed) x86_pit_advance(d4_board->shared_pit, 1u);
    if (!failed) failed |= x86_nxvm_board_get_speaker_observation(board, &speaker) != LIB_STATUS_OK ||
        !speaker.timer_output || !speaker.output;
    if (!failed) lib_c_printf("D4-SPEAKER-LINE:OK\n");
    if (!failed) failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_nxvm_board_get_speaker_observation(board, &speaker) != LIB_STATUS_OK ||
        !speaker.configured || !speaker.timer_gate || !speaker.data_enabled ||
        speaker.output != speaker.timer_output;

    if (!failed) failed |= test_x86_nxvm_machine_fixture_nmi_prepare(machine);
    if (!failed) failed |= x86_core_machine_bus_write(machine, 0x0061u, 0x03u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x0070u, 0x80u) !=
            LIB_STATUS_OK || mydeskpro386_d4_platform_report_iochk(d4_board) !=
            LIB_STATUS_OK || mydeskpro386_d4_platform_observe(d4_board,
            &observation) != LIB_STATUS_OK || !observation.iochk_latched ||
        observation.nmi_signaled ||
        x86_core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        value != 0x53u || test_x86_nxvm_machine_fixture_nmi_execute(machine, LIB_FALSE) ||
        x86_core_machine_bus_write(machine, 0x0070u, 0u) !=
            LIB_STATUS_OK || mydeskpro386_d4_platform_observe(d4_board,
            &observation) != LIB_STATUS_OK || !observation.nmi_signaled ||
        test_x86_nxvm_machine_fixture_nmi_execute(machine, LIB_TRUE);
    if (!failed) lib_c_printf("D4-NMI-MASK:OK\n");

    if (!failed) failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        test_x86_nxvm_machine_fixture_nmi_prepare(machine) ||
        x86_core_machine_bus_write(machine, 0x0061u, 0u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x0064u, 0xd0u) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0x0060u, &value) != LIB_STATUS_OK || value != 1u ||
        x86_core_machine_bus_write(machine, 0x004bu, 0x30u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x0048u, 1u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x0048u, 0u) != LIB_STATUS_OK;
    if (!failed) x86_pit_advance(d4_board->auxiliary_pit, 2u);
    if (!failed) failed |= mydeskpro386_d4_platform_observe(d4_board, &observation) !=
            LIB_STATUS_OK || !observation.failsafe_enabled ||
        !observation.failsafe_latched || !observation.nmi_signaled ||
        x86_core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        value != 0x90u || test_x86_nxvm_machine_fixture_nmi_execute(machine, LIB_TRUE);
    if (!failed) lib_c_printf("D4-FAILSAFE-ROUTE:OK\n");

    if (!failed) {
        /* Real guest LIDT with a zero limit makes INT 3 and its fault
         * delivery fail, exercising the board's shutdown-to-reset wiring. */
        const lib_u8 program[] = {0x0fu, 0x01u, 0x1eu, 0x00u, 0x03u, 0xccu};
        const lib_u8 idtr[6] = {0};
        const x86_core_cpu_debug_register_patch entry = {
            .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
                X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
            .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x0400u}
        };
        x86_core_machine_run_result result;
        x86_core_cpu_state cpu;
        lib_u64 elapsed_before_shutdown;

        /* Processor reset wakes the prior NMI HLT without resetting the
         * board's failsafe latch; no private CPU flags are injected. */
        x86_core_machine_signal_processor_reset(machine);
        failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
            &result) != LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_RESET_REQUESTED ||
            x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x0400u, program, sizeof(program)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x0300u, idtr, sizeof(idtr)) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
                &result) != LIB_STATUS_OK || result.reason != X86_CORE_MACHINE_STOP_BUDGET ||
            result.executed != 1u;
        elapsed_before_shutdown = test_x86_nxvm_elapsed_ticks(machine);
        failed |= x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 0u},
            &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_RESET_REQUESTED ||
            x86_core_machine_get_cpu_state(machine, &cpu) != LIB_STATUS_OK ||
            cpu.eip != 0x0000fff0u ||
            test_x86_nxvm_elapsed_ticks(machine) != elapsed_before_shutdown ||
            mydeskpro386_d4_platform_observe(d4_board, &observation) !=
                LIB_STATUS_OK || !observation.failsafe_enabled ||
            !observation.failsafe_latched;
    }
    if (!failed) failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        mydeskpro386_d4_platform_observe(d4_board, &observation) !=
            LIB_STATUS_OK || observation.iochk_enabled || observation.failsafe_enabled ||
        observation.iochk_latched || observation.failsafe_latched ||
        observation.nmi_signaled ||
        x86_core_machine_bus_read(machine, 0x0061u, &value) != LIB_STATUS_OK ||
        value != 0x1fu || test_x86_nxvm_machine_fixture_nmi_prepare(machine) ||
        test_x86_nxvm_machine_fixture_nmi_execute(machine, LIB_FALSE);
    if (!failed) failed |= core_machine_port_b_exclusivity();
    x86_core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("PORT-B-EXCLUSIVITY:OK\n");
    lib_c_printf("D4-RESET-ISOLATION:OK\n");
    lib_c_printf("D4-RESET-ARBITRATION:OK\n");
    lib_c_printf("B3-ACTIVE-LOW-NMI:OK\n");
    return 0;
}

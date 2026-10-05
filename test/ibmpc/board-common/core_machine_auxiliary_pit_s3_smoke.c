#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include <stdio.h>

#include "../../x86/core/time_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"

static void pit_port_write(core_machine *machine, lib_u16 port, lib_u32 value)
{
    if (core_machine_bus_write(machine, port, value) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
}

static lib_u32 pit_port_read(core_machine *machine, lib_u16 port)
{
    lib_u32 value;
    if (core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
    return value;
}

static void program_counter0(core_machine *machine, lib_u16 base,
    lib_u8 control, lib_u16 count)
{
    pit_port_write(machine,
        (lib_u16)(base + 3u), control);
    pit_port_write(machine, base, count & 0xffu);
    pit_port_write(machine, base, count >> 8u);
}

static lib_u16 read_counter0(core_machine *machine, lib_u16 base)
{
    lib_u16 low;
    pit_port_write(machine, (lib_u16)(base + 3u), 0u);
    low = (lib_u16)pit_port_read(machine, base);
    return (lib_u16)(low | (pit_port_read(machine, base) << 8u));
}

lib_i32 main(void)
{
    core_machine_config config = { 0 };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u32 value;
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    config.clock_plan.auxiliary_pit = (core_machine_clock_ratio) {1u, 4u, 0u};
    config.auxiliary_pit_present = LIB_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    failed |= core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;
    if (!failed) {
        failed |= !board->auxiliary_pit_configured;
        failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        failed |= core_machine_bus_read(machine, 0x0048u, &value) != LIB_STATUS_OK ||
            core_machine_bus_write(machine, 0x004bu, 0u) != LIB_STATUS_OK ||
            core_machine_bus_read(machine, 0x0040u, &value) != LIB_STATUS_OK;
        program_counter0(machine, 0x0040u, 0x30u, 3u);
        program_counter0(machine, 0x0048u, 0x30u, 2u);
        /* A completed count write reaches CE on the following clock. */
        failed |= read_counter0(machine, 0x0040u) != 0u ||
            read_counter0(machine, 0x0048u) != 0u;
        failed |= test_core_machine_advance_time(machine, 1u) != LIB_STATUS_OK;
        failed |= read_counter0(machine, 0x0040u) != 3u ||
            read_counter0(machine, 0x0048u) != 0u;
        failed |= test_core_machine_advance_time(machine, 3u) != LIB_STATUS_OK;
        failed |= read_counter0(machine, 0x0040u) != 0u ||
            read_counter0(machine, 0x0048u) != 2u;
        pit_port_write(machine, 0x004bu, 0x00u);
        failed |= pit_port_read(machine, 0x0048u) != 2u ||
            pit_port_read(machine, 0x0048u) != 0u;
        pit_port_write(machine, 0x004bu, 0x12u);
        pit_port_write(machine, 0x0048u, 0x22u);
        failed |= test_core_machine_advance_time(machine, 4u) != LIB_STATUS_OK;
        pit_port_write(machine, 0x004bu, 0x00u);
        failed |= pit_port_read(machine, 0x0048u) != 0x22u ||
            pit_port_read(machine, 0x0048u) != 0x22u;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        /* Reset cancels both programmed output transitions. */
        {
            lib_u64 deadline;
            failed |= x86_pit_get_output(board->shared_pit, 0u) ||
                x86_pit_get_output(board->auxiliary_pit, 0u) ||
                x86_pit_ticks_until_output(board->shared_pit, 0u, &deadline) != LIB_STATUS_INVALID_STATE ||
                x86_pit_ticks_until_output(board->auxiliary_pit, 0u, &deadline) != LIB_STATUS_INVALID_STATE;
        }
    }
    core_machine_destroy(machine);
    if (failed) return 1;
    printf("M5:T386:S3:SECOND-PIT-OWNER:OK\n");
    printf("M5:T386:S3:SECOND-PIT-ISOLATION:OK\n");
    return 0;
}

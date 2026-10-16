#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"

static void pit_port_write(x86_core_machine *machine, lib_u16 port, lib_u32 value)
{
    if (x86_core_machine_bus_write(machine, port, value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}

static lib_u32 pit_port_read(x86_core_machine *machine, lib_u16 port)
{
    lib_u32 value;
    if (x86_core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return value;
}

static void program_counter0(x86_core_machine *machine, lib_u16 base,
    lib_u8 control, lib_u16 count)
{
    pit_port_write(machine,
        (lib_u16)(base + 3u), control);
    pit_port_write(machine, base, count & 0xffu);
    pit_port_write(machine, base, count >> 8u);
}

static lib_u16 read_counter0(x86_core_machine *machine, lib_u16 base)
{
    lib_u16 low;
    pit_port_write(machine, (lib_u16)(base + 3u), 0u);
    low = (lib_u16)pit_port_read(machine, base);
    return (lib_u16)(low | (pit_port_read(machine, base) << 8u));
}

static lib_i32 auxiliary_pit_deadline_wiring(void)
{
    const x86_nxvm_board_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80286,
        .clock_plan.auxiliary_pit = {1u, 4u, 0u},
        .auxiliary_pit_present = LIB_TRUE,
        .auxiliary_pit_base_port = 0x48u
    };
    const x86_nxvm_board_plan_topology topology = {0};
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_core_machine_time_observation time = {0};
    lib_u8 advanced = LIB_FALSE;
    lib_i32 failed = x86_nxvm_board_plan_create(&config, &plan) != LIB_STATUS_OK ||
        x86_nxvm_board_plan_set_topology(plan, &topology) != LIB_STATUS_OK ||
        x86_nxvm_board_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;

    if (!failed) {
        program_counter0(machine, 0x40u, 0x30u, 3u);
        program_counter0(machine, 0x48u, 0x30u, 2u);
        failed |= x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
            !time.next_deadline_valid || time.next_deadline_tick != 1u ||
            x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
            !advanced || read_counter0(machine, 0x40u) != 3u ||
            read_counter0(machine, 0x48u) != 0u;
        failed |= x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
            time.elapsed_ticks != 1u || !time.next_deadline_valid || time.next_deadline_tick != 4u ||
            x86_core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK ||
            !advanced || read_counter0(machine, 0x40u) != 0u ||
            read_counter0(machine, 0x48u) != 2u ||
            x86_core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
            time.elapsed_ticks != 4u;
    }
    x86_core_machine_destroy(machine);
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}

lib_i32 main(void)
{
    x86_nxvm_board_config config = { 0 };
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_u32 value;
    lib_i32 failed = 0;

    config.cpu_profile = X86_CORE_CPU_PROFILE_80286;
    config.clock_plan.auxiliary_pit = (x86_core_machine_clock_ratio) {1u, 4u, 0u};
    config.auxiliary_pit_present = LIB_TRUE;
    config.auxiliary_pit_base_port = 0x0048u;
    failed |= x86_nxvm_board_create(&config, &machine, &board) != LIB_STATUS_OK;
    if (!failed) {
        failed |= !board->auxiliary_pit_configured;
        failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
        failed |= x86_core_machine_bus_read(machine, 0x0048u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x004bu, 0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0x0040u, &value) != LIB_STATUS_OK;
        program_counter0(machine, 0x0040u, 0x30u, 3u);
        program_counter0(machine, 0x0048u, 0x30u, 2u);
        /* A completed count write reaches CE on the following clock. */
        failed |= read_counter0(machine, 0x0040u) != 0u ||
            read_counter0(machine, 0x0048u) != 0u;
        /* Unit inputs to the Board-owned clock conversion and PIT ordering;
         * the separate deadline case proves their real executor wiring. */
        x86_nxvm_board_before_internal_advance(board, 1u, 1u);
        failed |= read_counter0(machine, 0x0040u) != 3u ||
            read_counter0(machine, 0x0048u) != 0u;
        x86_nxvm_board_before_internal_advance(board, 3u, 4u);
        failed |= read_counter0(machine, 0x0040u) != 0u ||
            read_counter0(machine, 0x0048u) != 2u;
        pit_port_write(machine, 0x004bu, 0x00u);
        failed |= pit_port_read(machine, 0x0048u) != 2u ||
            pit_port_read(machine, 0x0048u) != 0u;
        pit_port_write(machine, 0x004bu, 0x12u);
        pit_port_write(machine, 0x0048u, 0x22u);
        x86_nxvm_board_before_internal_advance(board, 4u, 8u);
        pit_port_write(machine, 0x004bu, 0x00u);
        failed |= pit_port_read(machine, 0x0048u) != 0u ||
            pit_port_read(machine, 0x0048u) != 0u;
        /* Mode 1 loads CE only on the clock following a GATE trigger. */
        x86_pit_set_gate(board->auxiliary_pit, 0u, LIB_FALSE);
        x86_pit_set_gate(board->auxiliary_pit, 0u, LIB_TRUE);
        x86_nxvm_board_before_internal_advance(board, 4u, 12u);
        pit_port_write(machine, 0x004bu, 0x00u);
        failed |= pit_port_read(machine, 0x0048u) != 0x22u ||
            pit_port_read(machine, 0x0048u) != 0x22u;
        lib_u64 elapsed = 99u;
        failed |= x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
            elapsed != 0u || x86_core_machine_reset(machine) != LIB_STATUS_OK;
        /* Reset cancels both programmed output transitions. */
        {
            lib_u64 deadline;
            failed |= x86_pit_get_output(board->shared_pit, 0u) ||
                x86_pit_get_output(board->auxiliary_pit, 0u) ||
                x86_pit_ticks_until_output(board->shared_pit, 0u, &deadline) != LIB_STATUS_INVALID_STATE ||
                x86_pit_ticks_until_output(board->auxiliary_pit, 0u, &deadline) != LIB_STATUS_INVALID_STATE;
        }
    }
    x86_core_machine_destroy(machine);
    failed |= auxiliary_pit_deadline_wiring();
    if (failed) return 1;
    lib_c_printf("SECOND-PIT-OWNER:OK\n");
    lib_c_printf("SECOND-PIT-ISOLATION:OK\n");
    return 0;
}

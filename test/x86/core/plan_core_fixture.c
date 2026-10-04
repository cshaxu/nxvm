#include "plan_core_fixture.h"
#include "construction_fixture.h"

lib_i32 test_plan_core_attachment(core_machine *machine,
    core_machine_board_state *board)
{
    return board == LIB_NULL || (void *)board != machine->attachment.context;
}

lib_i32 test_plan_core_allocation_failures(const core_machine_config *configuration)
{
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_i32 failed = 0;

    for (lib_size index = 1u; index <= 3u; ++index) {
        core_machine_port_test_allocation allocation = { index, 0u };
        machine = (core_machine *)(lib_uptr)1u;
        board = (core_machine_board_state *)(lib_uptr)1u;
        failed |= test_core_machine_create_with_allocation(configuration, &machine,
            LIB_NULL, &allocation, &board) != LIB_STATUS_NO_MEMORY ||
            machine != LIB_NULL || board != LIB_NULL ||
            allocation.attempts != index;
    }
    return failed;
}

lib_i32 test_plan_core_xt_routes(core_machine *machine, lib_bool after_reset)
{
    if (after_reset) {
        return (!core_machine_port_has_read(&machine->executor_port, 0x0020u) ||
            core_machine_port_has_read(&machine->executor_port, 0x00a0u) ||
            core_machine_port_has_read(&machine->executor_port, 0x0070u));
    }
    return (!core_machine_port_has_read(&machine->executor_port, 0x0020u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x0000u) ||
        !core_machine_port_has_read(&machine->executor_port, 0x0081u) ||
        !core_machine_port_has_write(&machine->executor_port, 0x0083u) ||
        core_machine_port_has_read(&machine->executor_port, 0x0087u) ||
        core_machine_port_has_write(&machine->executor_port, 0x0089u) ||
        core_machine_port_has_read(&machine->executor_port, 0x008au) ||
        core_machine_port_has_write(&machine->executor_port, 0x008bu) ||
        core_machine_port_has_read(&machine->executor_port, 0x008fu) ||
        core_machine_port_has_read(&machine->executor_port, 0x00a0u) ||
        core_machine_port_has_write(&machine->executor_port, 0x00d0u) ||
        core_machine_port_has_read(&machine->executor_port, 0x0070u) ||
        core_machine_port_has_write(&machine->executor_port, 0x0071u));
}

lib_i32 test_plan_core_pit_deadline(core_machine *machine)
{
    core_machine_time_observation observation;
    core_machine_timing_disposition disposition;
    lib_u8 advanced = LIB_FALSE;
    lib_i32 failed = 0;

    failed |= !failed && core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_get_timing_disposition(machine,
        CORE_MACHINE_TIMING_CAPABILITY_CTRL_PIT, &disposition) != LIB_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK;
    core_machine_port_write(&machine->executor_port, 0x0043u, 0x34u);
    core_machine_port_write(&machine->executor_port, 0x0040u, 4u);
    core_machine_port_write(&machine->executor_port, 0x0040u, 0u);
    failed |= !failed && core_machine_capture_time_observation(machine, &observation) !=
        LIB_STATUS_OK;
    failed |= !failed && (!observation.next_deadline_valid ||
        observation.next_deadline_tick != 1u ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE);
    failed |= !failed && core_machine_advance_to_next_deadline(machine, &advanced) !=
        LIB_STATUS_OK;
    failed |= !failed && (!advanced || machine->elapsed_ticks != 1u);
    return failed;
}

lib_i32 test_plan_core_dma_deadline(core_machine *machine,
    core_machine_dma_bus *dma, const core_machine_dma_request_binding *binding)
{
    core_machine_time_observation observation;
    lib_u8 advanced = LIB_FALSE;
    lib_i32 failed = 0;

    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    core_machine_port_write(&machine->executor_port, 0x000bu, 0x46u);
    core_machine_port_write(&machine->executor_port, 0x000au, 0x02u);
    core_machine_dma_request_assert(dma, binding);
    failed |= !failed && core_machine_capture_time_observation(machine, &observation) !=
        LIB_STATUS_OK;
    failed |= !failed && (!observation.next_deadline_valid ||
        observation.next_deadline_tick != 3u ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE);
    failed |= !failed && core_machine_advance_to_next_deadline(machine, &advanced) !=
        LIB_STATUS_OK;
    failed |= !failed && (!advanced || machine->elapsed_ticks != 3u);
    core_machine_dma_request_deassert(dma, binding);
    return failed;
}

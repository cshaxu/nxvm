#include "plan_core_fixture.h"
#include "bus_fixture.h"
#include "../board_binding_fixture.h"

lib_i32 test_plan_board_device_failures(const core_machine_config *configuration)
{
    lib_i32 failed = 0;

    for (lib_u32 dependency = 1u; dependency <= 3u; ++dependency)
        failed |= test_board_device_failure(configuration, dependency);
    return failed;
}

lib_i32 test_plan_core_xt_routes(x86_core_machine *machine, lib_bool after_reset)
{
    if (after_reset) {
        lib_u32 value = 0u;
        return x86_core_machine_bus_read(machine, 0x20u, &value) != LIB_STATUS_OK ||
            x86_core_machine_bus_read(machine, 0xa0u, &value) != LIB_STATUS_UNSUPPORTED ||
            x86_core_machine_bus_read(machine, 0x70u, &value) != LIB_STATUS_UNSUPPORTED;
    }
    const struct {lib_u16 port; lib_bool write; lib_bool installed;} routes[] = {
        {0x20u, LIB_FALSE, LIB_TRUE}, {0u, LIB_TRUE, LIB_TRUE},
        {0x81u, LIB_FALSE, LIB_TRUE}, {0x83u, LIB_TRUE, LIB_TRUE},
        {0x87u, LIB_FALSE, LIB_FALSE}, {0x89u, LIB_TRUE, LIB_FALSE},
        {0x8au, LIB_FALSE, LIB_FALSE}, {0x8bu, LIB_TRUE, LIB_FALSE},
        {0x8fu, LIB_FALSE, LIB_FALSE}, {0xa0u, LIB_FALSE, LIB_FALSE},
        {0xd0u, LIB_TRUE, LIB_FALSE}, {0x70u, LIB_FALSE, LIB_FALSE},
        {0x71u, LIB_TRUE, LIB_FALSE}
    };
    lib_i32 failed = 0;

    /* Public construction conflicts prove routing before reset. A probe of an
     * unused endpoint is revoked immediately; no existing route is removed. */
    for (lib_size index = 0u; index < sizeof(routes) / sizeof(routes[0]); ++index) {
        failed |= test_core_machine_fixture_expect_port(machine, routes[index].port,
            routes[index].write, routes[index].installed);
    }
    return failed;
}

lib_i32 test_plan_core_pit_deadline(x86_core_machine *machine,
    core_machine_board_state *board)
{
    x86_core_machine_time_observation observation;
    core_machine_board_timing_disposition disposition;
    lib_u8 advanced = LIB_FALSE;
    lib_u64 elapsed = 0u;
    lib_i32 failed = 0;

    failed |= !failed && x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_board_get_timing_disposition(board,
        CORE_MACHINE_BOARD_TIMING_CAPABILITY_CTRL_PIT, &disposition) != LIB_STATUS_OK;
    failed |= !failed && disposition != CORE_MACHINE_BOARD_TIMING_DISPOSITION_L2_FALLBACK;
    failed |= !failed && (x86_core_machine_bus_write(machine, 0x43u, 0x34u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x40u, 4u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x40u, 0u) != LIB_STATUS_OK);
    failed |= !failed && x86_core_machine_capture_time_observation(machine, &observation) !=
        LIB_STATUS_OK;
    failed |= !failed && (!observation.next_deadline_valid ||
        observation.next_deadline_tick != 1u ||
        observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_DEADLINE);
    failed |= !failed && x86_core_machine_advance_to_next_deadline(machine, &advanced) !=
        LIB_STATUS_OK;
    failed |= !failed && (!advanced ||
        x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK || elapsed != 1u);
    return failed;
}

lib_i32 test_plan_core_dma_deadline(x86_core_machine *machine,
    core_machine_dma_bus *dma, const core_machine_dma_request_binding *binding)
{
    x86_core_machine_time_observation observation;
    lib_u8 advanced = LIB_FALSE;
    lib_u64 elapsed = 0u;
    lib_i32 failed = 0;

    failed |= !failed && x86_core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && (x86_core_machine_bus_write(machine, 0xbu, 0x46u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0xau, 0x02u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0xd4u, 0u) != LIB_STATUS_OK);
    core_machine_dma_request_assert(dma, binding);
    failed |= !failed && x86_core_machine_capture_time_observation(machine, &observation) !=
        LIB_STATUS_OK;
    failed |= !failed && (!observation.next_deadline_valid ||
        observation.next_deadline_tick != 3u ||
        observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_DEADLINE);
    failed |= !failed && x86_core_machine_advance_to_next_deadline(machine, &advanced) !=
        LIB_STATUS_OK;
    failed |= !failed && (!advanced ||
        x86_core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK || elapsed != 3u);
    core_machine_dma_request_deassert(dma, binding);
    return failed;
}

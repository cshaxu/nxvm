#ifndef TEST_CORE_MACHINE_DMA_DEADLINE_FIXTURE_H
#define TEST_CORE_MACHINE_DMA_DEADLINE_FIXTURE_H

#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "core/board-base/dma_bus_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "core/machine/machine_interface.h"
#include "../../board-base/support/composition_fixture.h"

typedef lib_i32 test_core_dma_deadline_configure(core_machine_config *out_configuration);

static inline lib_i32 test_core_dma_deadline_assert(const char *name,
    test_core_dma_deadline_configure *configure)
{
    core_machine_config configuration = {0};
    core_machine_dma_wiring wiring = {0};
    core_machine_dma_request_binding request = {0};
    core_machine_time_observation observation;
    core_machine_plan *plan = LIB_NULL;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_status status;
    lib_i32 failed = 0;

    if (name == LIB_NULL || configure == LIB_NULL || configure(&configuration) ||
        configuration.clock_plan.dma.numerator == 0u ||
        configuration.clock_plan.dma.denominator == 0u) return 1;
    wiring.fdc_channel = 2u;
    wiring.controller_count = configuration.dma_controller_count;
    wiring.cascade_channel = configuration.dma_controller_count == 1u ? 0u :
        CORE_MACHINE_DMA_CASCADE_CHANNEL;
    status = core_machine_plan_create(&configuration, &plan);
    if (status != LIB_STATUS_OK || plan == LIB_NULL) {
        lib_c_printf("%s: plan create status %d\n", name, status);
        failed = 1;
        goto done;
    }
    status = core_machine_create_from_plan(plan, &machine, &board);
    if (status != LIB_STATUS_OK || machine == LIB_NULL || board == LIB_NULL) {
        lib_c_printf("%s: core create status %d\n", name, status);
        failed = 1;
    }
    if (!failed) {
        status = core_machine_configure_dma(board, &wiring, &request);
        if (status != LIB_STATUS_OK || request.core_token == 0u) {
            lib_c_printf("%s: DMA configuration status %d\n", name, status);
            failed = 1;
        }
    }
    if (!failed) {
        status = core_machine_freeze_execution_providers(machine);
        if (status != LIB_STATUS_OK) {
            lib_c_printf("%s: provider freeze status %d\n", name, status);
            failed = 1;
        }
    }
    if (!failed) {
        status = core_machine_reset(machine);
        if (status != LIB_STATUS_OK) {
            lib_c_printf("%s: machine reset status %d\n", name, status);
            failed = 1;
        }
    }
    if (!failed)
        failed = core_machine_bus_write(machine, 0x000au, 0x02u) != LIB_STATUS_OK;
    if (!failed) {
        test_board_dma_request_assert(board, &request);
        status = core_machine_capture_time_observation(machine, &observation);
        if (status != LIB_STATUS_OK || !observation.next_deadline_valid ||
            observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE) {
            lib_c_printf("%s: observation status %d\n", name, status);
            if (status == LIB_STATUS_OK)
                lib_c_printf("deadline %u disposition %d\n", observation.next_deadline_valid,
                    observation.progress_disposition);
            failed = 1;
        }
    }
done:
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

#endif

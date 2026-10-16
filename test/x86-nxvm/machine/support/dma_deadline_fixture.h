#ifndef TEST_X86_NXVM_MACHINE_DMA_DEADLINE_FIXTURE_H
#define TEST_X86_NXVM_MACHINE_DMA_DEADLINE_FIXTURE_H

#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86-nxvm/board-base/dma_bus_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "../../board-base/support/composition_fixture.h"

typedef lib_i32 test_x86_nxvm_dma_deadline_configure(x86_nxvm_board_config *out_configuration);

static inline lib_i32 test_x86_nxvm_dma_deadline_assert(const char *name,
    test_x86_nxvm_dma_deadline_configure *configure)
{
    x86_nxvm_board_config configuration = {0};
    x86_nxvm_board_dma_wiring wiring = {0};
    x86_nxvm_board_dma_request_binding request = {0};
    x86_core_machine_time_observation observation;
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    lib_status status;
    lib_i32 failed = 0;

    if (name == LIB_NULL || configure == LIB_NULL || configure(&configuration) ||
        configuration.clock_plan.dma.numerator == 0u ||
        configuration.clock_plan.dma.denominator == 0u) return 1;
    wiring.fdc_channel = 2u;
    wiring.controller_count = configuration.dma_controller_count;
    wiring.cascade_channel = configuration.dma_controller_count == 1u ? 0u :
        X86_NXVM_BOARD_DMA_CASCADE_CHANNEL;
    status = x86_nxvm_board_plan_create(&configuration, &plan);
    if (status != LIB_STATUS_OK || plan == LIB_NULL) {
        lib_c_printf("%s: plan create status %d\n", name, status);
        failed = 1;
        goto done;
    }
    status = x86_nxvm_board_create_from_plan(plan, &machine, &board);
    if (status != LIB_STATUS_OK || machine == LIB_NULL || board == LIB_NULL) {
        lib_c_printf("%s: core create status %d\n", name, status);
        failed = 1;
    }
    if (!failed) {
        status = x86_nxvm_board_configure_dma(board, &wiring, &request);
        if (status != LIB_STATUS_OK || request.core_token == 0u) {
            lib_c_printf("%s: DMA configuration status %d\n", name, status);
            failed = 1;
        }
    }
    if (!failed) {
        status = x86_core_machine_freeze_execution_providers(machine);
        if (status != LIB_STATUS_OK) {
            lib_c_printf("%s: provider freeze status %d\n", name, status);
            failed = 1;
        }
    }
    if (!failed) {
        status = x86_core_machine_reset(machine);
        if (status != LIB_STATUS_OK) {
            lib_c_printf("%s: machine reset status %d\n", name, status);
            failed = 1;
        }
    }
    if (!failed)
        failed = x86_core_machine_bus_write(machine, 0x000au, 0x02u) != LIB_STATUS_OK;
    if (!failed && wiring.controller_count == 2u)
        failed = x86_core_machine_bus_write(machine, 0x00d4u, 0x00u) != LIB_STATUS_OK;
    if (!failed) {
        test_board_dma_request_assert(board, &request);
        status = x86_core_machine_capture_time_observation(machine, &observation);
        if (status != LIB_STATUS_OK || !observation.next_deadline_valid ||
            observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_DEADLINE) {
            lib_c_printf("%s: observation status %d\n", name, status);
            if (status == LIB_STATUS_OK)
                lib_c_printf("deadline %u disposition %d\n", observation.next_deadline_valid,
                    observation.progress_disposition);
            failed = 1;
        }
    }
done:
    x86_core_machine_destroy(machine);
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}

#endif

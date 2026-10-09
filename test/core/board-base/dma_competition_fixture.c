#include "dma_competition_fixture.h"
#include "core/board-base/machine_board_state.h"

lib_status test_dma_competition_create(const core_machine_config *config,
    const core_machine_dma_channel_provider *provider, void *source,
    core_machine **out_machine, core_machine_dma_bus **out_dma,
    core_machine_dma_request_binding *out_binding)
{
    core_machine_board_state *board = LIB_NULL;
    lib_status status;
    *out_dma = LIB_NULL;
    status = core_machine_create(config, out_machine, &board);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_dma_bind_channel(board->shared_dma, 2u,
        provider, source, out_binding);
    if (status != LIB_STATUS_OK) {
        core_machine_destroy(*out_machine);
        *out_machine = LIB_NULL;
        return status;
    }
    *out_dma = board->shared_dma;
    return LIB_STATUS_OK;
}

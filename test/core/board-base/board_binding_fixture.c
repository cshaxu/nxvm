#include "board_binding_fixture.h"
#include "core/board-base/machine_board_state.h"

lib_status test_board_binding_create(const core_machine_config *config,
    core_machine **out_machine, core_machine_attachment *out_expected)
{
    core_machine_board_state *board = LIB_NULL;
    lib_status status;
    core_machine_board_refresh_nmi(LIB_NULL);
    status = core_machine_create(config, out_machine, &board);
    if (status != LIB_STATUS_OK) return status;
    if (board->core != *out_machine) {
        core_machine_destroy(*out_machine);
        *out_machine = LIB_NULL;
        return LIB_STATUS_INTERNAL_ERROR;
    }
    *out_expected = (core_machine_attachment) {
        .reset_devices = core_machine_board_reset_devices,
        .reset_clocks = core_machine_board_reset_clocks,
        .refresh_nmi = core_machine_board_refresh_nmi,
        .finalize_devices = core_machine_board_finalize_devices,
        .context = board
    };
    return LIB_STATUS_OK;
}

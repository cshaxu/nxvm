#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"

#include "core/board-base/machine_board_state.h"

lib_status core_machine_capture_display_snapshot(const core_machine_board_state *board,
    x86_video_snapshot *out_snapshot)
{
    core_machine_board_state *mutable_board = (core_machine_board_state *)board;
    core_machine_lifecycle lifecycle;

    if (board == LIB_NULL || out_snapshot == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (core_machine_get_lifecycle(board->core, &lifecycle) != LIB_STATUS_OK ||
        (lifecycle != CORE_MACHINE_STOPPED && lifecycle != CORE_MACHINE_PAUSED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    return core_machine_vadp_capture_snapshot(mutable_board->shared_vadp,
        out_snapshot) ? LIB_STATUS_OK :
        LIB_STATUS_UNSUPPORTED;
}

lib_status core_machine_observe_display_snapshot(const core_machine_board_state *board,
    lib_u8 acknowledged_generation_valid,
    lib_u64 acknowledged_generation,
    x86_video_snapshot_observation *out_observation)
{
    core_machine_lifecycle lifecycle;

    if (board == LIB_NULL || out_observation == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (core_machine_get_lifecycle(board->core, &lifecycle) != LIB_STATUS_OK ||
        (lifecycle != CORE_MACHINE_STOPPED && lifecycle != CORE_MACHINE_PAUSED)) {
        return LIB_STATUS_INVALID_STATE;
    }
    core_machine_vadp_observe_snapshot(board->shared_vadp,
        acknowledged_generation_valid, acknowledged_generation, out_observation);
    return LIB_STATUS_OK;
}


lib_status core_machine_configure_display(core_machine_board_state *board,
    const core_machine_display_config *config)
{
    lib_status status;

    if (board == LIB_NULL || !core_machine_configuration_is_open(board->core) ||
            board->display_configured) {
        return LIB_STATUS_INVALID_STATE;
    }

    if (!core_machine_vadp_config_is_valid(config)) return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_vadp_configure(board->shared_vadp, config);
    if (status != LIB_STATUS_OK) return status;
    board->display_ports = config->ports;
    board->display_configured = LIB_TRUE;
    return LIB_STATUS_OK;
}

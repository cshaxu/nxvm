#include "video_topology_fixture.h"
#include "core/board-base/machine_board_state.h"
#include "core/board-base/vadp.h"

lib_bool test_video_aperture_mismatch(core_machine_board_state *board,
    lib_bool expected)
{
    return !!x86_video_ega_aperture_contains(board->shared_vadp->chip,
        0xa0000u, 1u) != expected;
}

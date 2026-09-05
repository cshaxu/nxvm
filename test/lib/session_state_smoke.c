#include "type.h"

#include "lib/session/state.h"

C_INT main(C_VOID)
{
    lib_session_state state;

    lib_session_state_initialize(&state);
    if (lib_session_state_is_active(&state) || lib_session_state_is_paused(&state) ||
        lib_session_state_request_step(&state)) return 1;
    lib_session_state_start(&state);
    if (!lib_session_state_is_active(&state) || !lib_session_state_flip(&state)) return 1;
    lib_session_state_request_reset(&state);
    if (!lib_session_state_take_reset(&state) || lib_session_state_take_reset(&state)) return 1;
    lib_session_state_request_pause(&state, 7u);
    if (!lib_session_state_pause_requested(&state) ||
        lib_session_state_pause_reason(&state) != 7u) return 1;
    lib_session_state_acknowledge_pause(&state);
    if (!lib_session_state_is_paused(&state) || !lib_session_state_request_step(&state) ||
        !lib_session_state_step_requested(&state) || !lib_session_state_take_step(&state)) return 1;
    lib_session_state_fault(&state);
    if (lib_session_state_is_active(&state) || lib_session_state_is_paused(&state) ||
        lib_session_state_pause_reason(&state) != 0u) return 1;
    puts("M5:T522:S8:SESSION-STATE:OK");
    return 0;
}

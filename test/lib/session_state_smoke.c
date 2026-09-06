#include "lib/session/state.h"

typedef enum session_state_command {
    SESSION_STATE_START,
    SESSION_STATE_RESET,
    SESSION_STATE_PAUSE,
    SESSION_STATE_ACKNOWLEDGE_PAUSE,
    SESSION_STATE_STOP
} session_state_command;

typedef struct session_state_row {
    session_state_command command;
    lib_session_lifecycle expected;
} session_state_row;

static int session_state_apply(lib_session_state *state,
    session_state_command command)
{
    switch (command) {
    case SESSION_STATE_START: lib_session_state_start(state); break;
    case SESSION_STATE_RESET:
        lib_session_state_request_reset(state);
        return lib_session_state_take_reset(state) &&
            !lib_session_state_take_reset(state);
    case SESSION_STATE_PAUSE: lib_session_state_request_pause(state); break;
    case SESSION_STATE_ACKNOWLEDGE_PAUSE:
        lib_session_state_acknowledge_pause(state); break;
    case SESSION_STATE_STOP: lib_session_state_stop(state); break;
    }
    return 1;
}

int main(void)
{
    lib_session_state *state = LIB_NULL;
    static const session_state_row rows[] = {
        { SESSION_STATE_START, LIB_SESSION_RUNNING },
        { SESSION_STATE_RESET, LIB_SESSION_RUNNING },
        { SESSION_STATE_PAUSE, LIB_SESSION_RUNNING },
        { SESSION_STATE_ACKNOWLEDGE_PAUSE, LIB_SESSION_PAUSED },
        { SESSION_STATE_STOP, LIB_SESSION_STOPPED },
        { SESSION_STATE_START, LIB_SESSION_RUNNING }
    };
    size_t index;

    if (lib_session_state_create(&state) != LIB_STATUS_OK ||
        lib_session_state_lifecycle(state) != LIB_SESSION_STOPPED ||
        lib_session_state_pause_requested(state)) goto failed;
    for (index = 0u; index < sizeof(rows) / sizeof(rows[0]); ++index) {
        if (!session_state_apply(state, rows[index].command) ||
            lib_session_state_lifecycle(state) != rows[index].expected) goto failed;
    }
    lib_session_state_destroy(state);
    puts("M5:T523:S4:SESSION-STATE:OK");
    return 0;
failed:
    lib_session_state_destroy(state);
    return 1;
}

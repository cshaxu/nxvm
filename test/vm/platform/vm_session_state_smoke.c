#include "vm/platform/session_state.h"

typedef enum session_state_command {
    SESSION_STATE_START,
    SESSION_STATE_RESET,
    SESSION_STATE_PAUSE,
    SESSION_STATE_ACKNOWLEDGE_PAUSE,
    SESSION_STATE_STOP
} session_state_command;

typedef struct session_state_row {
    session_state_command command;
    vm_session_lifecycle expected;
} session_state_row;

static int session_state_apply(vm_session_state *state,
    session_state_command command)
{
    switch (command) {
    case SESSION_STATE_START: vm_session_state_start(state); break;
    case SESSION_STATE_RESET:
        vm_session_state_request_reset(state);
        return vm_session_state_take_reset(state) &&
            !vm_session_state_take_reset(state);
    case SESSION_STATE_PAUSE: vm_session_state_request_pause(state); break;
    case SESSION_STATE_ACKNOWLEDGE_PAUSE:
        vm_session_state_acknowledge_pause(state); break;
    case SESSION_STATE_STOP: vm_session_state_stop(state); break;
    }
    return 1;
}

int main(void)
{
    vm_session_state *state = STD_NULL;
    static const session_state_row rows[] = {
        { SESSION_STATE_START, VM_SESSION_RUNNING },
        { SESSION_STATE_RESET, VM_SESSION_RUNNING },
        { SESSION_STATE_PAUSE, VM_SESSION_RUNNING },
        { SESSION_STATE_ACKNOWLEDGE_PAUSE, VM_SESSION_PAUSED },
        { SESSION_STATE_STOP, VM_SESSION_STOPPED },
        { SESSION_STATE_START, VM_SESSION_RUNNING }
    };
    size_t index;

    if (vm_session_state_create(&state) != TYPE_STATUS_OK ||
        vm_session_state_lifecycle(state) != VM_SESSION_STOPPED ||
        vm_session_state_pause_requested(state)) goto failed;
    vm_session_state_request_pause(state);
    if (vm_session_state_lifecycle(state) != VM_SESSION_STOPPED ||
        vm_session_state_is_paused(state) ||
        vm_session_state_pause_requested(state)) goto failed;
    for (index = 0u; index < sizeof(rows) / sizeof(rows[0]); ++index) {
        if (!session_state_apply(state, rows[index].command) ||
            vm_session_state_lifecycle(state) != rows[index].expected) goto failed;
    }
    vm_session_state_destroy(state);
    puts("M5:T523:S4:SESSION-STATE:OK");
    return 0;
failed:
    vm_session_state_destroy(state);
    return 1;
}

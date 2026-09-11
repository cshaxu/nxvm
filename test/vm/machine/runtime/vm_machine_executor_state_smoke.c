#include "vm/machine/runtime/executor_state.h"

typedef enum session_state_command {
    SESSION_STATE_START,
    SESSION_STATE_RESET,
    SESSION_STATE_PAUSE,
    SESSION_STATE_ACKNOWLEDGE_PAUSE,
    SESSION_STATE_STOP
} session_state_command;

typedef struct session_state_row {
    session_state_command command;
    vm_machine_lifecycle expected;
} session_state_row;

static int session_state_apply(vm_machine_executor_state *state,
    session_state_command command)
{
    switch (command) {
    case SESSION_STATE_START: vm_machine_executor_state_start(state); break;
    case SESSION_STATE_RESET:
        vm_machine_executor_state_request_reset(state);
        return vm_machine_executor_state_take_reset(state) &&
            !vm_machine_executor_state_take_reset(state);
    case SESSION_STATE_PAUSE: vm_machine_executor_state_request_pause(state); break;
    case SESSION_STATE_ACKNOWLEDGE_PAUSE:
        vm_machine_executor_state_acknowledge_pause(state); break;
    case SESSION_STATE_STOP: vm_machine_executor_state_stop(state); break;
    }
    return 1;
}

int main(void)
{
    vm_machine_executor_state *state = STD_NULL;
    static const session_state_row rows[] = {
        { SESSION_STATE_START, VM_MACHINE_RUNNING },
        { SESSION_STATE_RESET, VM_MACHINE_RUNNING },
        { SESSION_STATE_PAUSE, VM_MACHINE_RUNNING },
        { SESSION_STATE_ACKNOWLEDGE_PAUSE, VM_MACHINE_PAUSED },
        { SESSION_STATE_STOP, VM_MACHINE_STOPPED },
        { SESSION_STATE_START, VM_MACHINE_RUNNING }
    };
    size_t index;

    if (vm_machine_executor_state_create(&state) != TYPE_STATUS_OK ||
        vm_machine_executor_state_lifecycle(state) != VM_MACHINE_STOPPED ||
        vm_machine_executor_state_pause_requested(state)) goto failed;
    vm_machine_executor_state_request_pause(state);
    if (vm_machine_executor_state_lifecycle(state) != VM_MACHINE_STOPPED ||
        vm_machine_executor_state_is_paused(state) ||
        vm_machine_executor_state_pause_requested(state)) goto failed;
    for (index = 0u; index < sizeof(rows) / sizeof(rows[0]); ++index) {
        if (!session_state_apply(state, rows[index].command) ||
            vm_machine_executor_state_lifecycle(state) != rows[index].expected) goto failed;
    }
    vm_machine_executor_state_destroy(state);
    puts("M5:T524:S15:SESSION-STATE:OK");
    return 0;
failed:
    vm_machine_executor_state_destroy(state);
    return 1;
}

#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86-nxvm/machine/control.h"

int main(void)
{
    x86_nxvm_machine_control_state control = {0};
    x86_nxvm_machine_executor_state *state = &control.state;

    lib_atomic_i32_initialize(&state->active, LIB_FALSE);
    lib_atomic_i32_initialize(&state->reset_requested, LIB_FALSE);
    if (x86_nxvm_machine_executor_state_is_active(state)) goto failed;
    x86_nxvm_machine_executor_state_request_reset(state);
    if (!x86_nxvm_machine_executor_state_take_reset(state) ||
        x86_nxvm_machine_executor_state_take_reset(state) ||
        x86_nxvm_machine_executor_state_is_active(state)) goto failed;
    x86_nxvm_machine_executor_state_start(state);
    if (!x86_nxvm_machine_executor_state_is_active(state)) goto failed;
    x86_nxvm_machine_executor_state_request_reset(state);
    if (!x86_nxvm_machine_executor_state_take_reset(state) ||
        x86_nxvm_machine_executor_state_take_reset(state)) goto failed;
    x86_nxvm_machine_executor_state_request_reset(state);
    x86_nxvm_machine_executor_state_start(state);
    if (!x86_nxvm_machine_executor_state_take_reset(state)) goto failed;
    x86_nxvm_machine_executor_state_request_reset(state);
    x86_nxvm_machine_executor_state_stop(state);
    if (x86_nxvm_machine_executor_state_is_active(state) ||
        x86_nxvm_machine_executor_state_take_reset(state)) goto failed;
    lib_c_printf("EXECUTOR-STATE:OK\n");
    return 0;

failed:
    return 1;
}

#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "core/machine/control.h"

int main(void)
{
    vm_machine_control_state control = {0};
    vm_machine_executor_state *state = &control.state;

    lib_atomic_i32_initialize(&state->active, LIB_FALSE);
    lib_atomic_i32_initialize(&state->reset_requested, LIB_FALSE);
    if (vm_machine_executor_state_is_active(state)) goto failed;
    vm_machine_executor_state_request_reset(state);
    if (!vm_machine_executor_state_take_reset(state) ||
        vm_machine_executor_state_take_reset(state) ||
        vm_machine_executor_state_is_active(state)) goto failed;
    vm_machine_executor_state_start(state);
    if (!vm_machine_executor_state_is_active(state)) goto failed;
    vm_machine_executor_state_request_reset(state);
    if (!vm_machine_executor_state_take_reset(state) ||
        vm_machine_executor_state_take_reset(state)) goto failed;
    vm_machine_executor_state_request_reset(state);
    vm_machine_executor_state_start(state);
    if (!vm_machine_executor_state_take_reset(state)) goto failed;
    vm_machine_executor_state_request_reset(state);
    vm_machine_executor_state_stop(state);
    if (vm_machine_executor_state_is_active(state) ||
        vm_machine_executor_state_take_reset(state)) goto failed;
    lib_c_printf("EXECUTOR-STATE:OK\n");
    return 0;

failed:
    return 1;
}

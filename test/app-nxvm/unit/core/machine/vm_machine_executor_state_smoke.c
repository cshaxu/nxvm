#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/machine/executor_state.h"

int main(void)
{
    vm_machine_executor_state *state = LIB_NULL;

    if (vm_machine_executor_state_create(&state) != LIB_STATUS_OK ||
        vm_machine_executor_state_is_active(state)) goto failed;
    vm_machine_executor_state_request_reset(state);
    if (!vm_machine_executor_state_take_reset(state) ||
        vm_machine_executor_state_take_reset(state) ||
        vm_machine_executor_state_is_active(state)) goto failed;
    vm_machine_executor_state_start(state);
    if (!vm_machine_executor_state_is_active(state)) goto failed;
    vm_machine_executor_state_request_reset(state);
    if (!vm_machine_executor_state_take_reset(state) ||
        vm_machine_executor_state_take_reset(state)) goto failed;
    vm_machine_executor_state_stop(state);
    if (vm_machine_executor_state_is_active(state)) goto failed;
    vm_machine_executor_state_destroy(state);
    puts("M5:T534:S26:EXECUTOR-STATE:OK");
    return 0;

failed:
    vm_machine_executor_state_destroy(state);
    return 1;
}

#include "type.h"
#include "core/machine/executor_state.h"

struct vm_machine_executor_state {
    STD_ATOMIC_BOOL active;
    STD_ATOMIC_BOOL reset_requested;
};

type_status vm_machine_executor_state_create(vm_machine_executor_state **out_state)
{
    vm_machine_executor_state *state;

    if (out_state == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_state = STD_NULL;
    state = STD_CALLOC(1u, sizeof(*state));
    if (state == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    STD_ATOMIC_INIT(&state->active, TYPE_FALSE);
    STD_ATOMIC_INIT(&state->reset_requested, TYPE_FALSE);
    *out_state = state;
    return TYPE_STATUS_OK;
}

C_VOID vm_machine_executor_state_destroy(vm_machine_executor_state *state)
{
    STD_FREE(state);
}

C_VOID vm_machine_executor_state_start(vm_machine_executor_state *state)
{
    if (state == STD_NULL) return;
    if (STD_ATOMIC_LOAD(&state->active)) return;
    STD_ATOMIC_STORE(&state->reset_requested, TYPE_FALSE);
    STD_ATOMIC_STORE(&state->active, TYPE_TRUE);
}

C_VOID vm_machine_executor_state_stop(vm_machine_executor_state *state)
{
    if (state == STD_NULL) return;
    STD_ATOMIC_STORE(&state->active, TYPE_FALSE);
    STD_ATOMIC_STORE(&state->reset_requested, TYPE_FALSE);
}

C_VOID vm_machine_executor_state_request_reset(vm_machine_executor_state *state)
{
    if (state != STD_NULL) STD_ATOMIC_STORE(&state->reset_requested, TYPE_TRUE);
}

C_INT vm_machine_executor_state_take_reset(vm_machine_executor_state *state)
{
    return state != STD_NULL && STD_ATOMIC_EXCHANGE(&state->reset_requested, TYPE_FALSE);
}

C_INT vm_machine_executor_state_is_active(const vm_machine_executor_state *state)
{ return state != STD_NULL && STD_ATOMIC_LOAD(&state->active); }

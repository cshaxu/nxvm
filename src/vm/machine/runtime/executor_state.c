#include "type.h"
#include "vm/machine/runtime/executor_state.h"

struct vm_machine_executor_state {
    atomic_bool active;
    atomic_bool reset_requested;
    atomic_bool pause_requested;
    atomic_bool paused;
};

type_status vm_machine_executor_state_create(vm_machine_executor_state **out_state)
{
    vm_machine_executor_state *state;

    if (out_state == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_state = STD_NULL;
    state = calloc(1u, sizeof(*state));
    if (state == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    atomic_init(&state->active, TYPE_FALSE);
    atomic_init(&state->reset_requested, TYPE_FALSE);
    atomic_init(&state->pause_requested, TYPE_FALSE);
    atomic_init(&state->paused, TYPE_FALSE);
    *out_state = state;
    return TYPE_STATUS_OK;
}

void vm_machine_executor_state_destroy(vm_machine_executor_state *state)
{
    free(state);
}

void vm_machine_executor_state_start(vm_machine_executor_state *state)
{
    if (state == STD_NULL) return;
    if (atomic_load(&state->active)) return;
    atomic_store(&state->reset_requested, TYPE_FALSE);
    atomic_store(&state->pause_requested, TYPE_FALSE);
    atomic_store(&state->paused, TYPE_FALSE);
    atomic_store(&state->active, TYPE_TRUE);
}

void vm_machine_executor_state_stop(vm_machine_executor_state *state)
{
    if (state == STD_NULL) return;
    atomic_store(&state->active, TYPE_FALSE);
    atomic_store(&state->reset_requested, TYPE_FALSE);
    atomic_store(&state->paused, TYPE_FALSE);
    atomic_store(&state->pause_requested, TYPE_FALSE);
}

void vm_machine_executor_state_request_reset(vm_machine_executor_state *state)
{
    if (state != STD_NULL) atomic_store(&state->reset_requested, TYPE_TRUE);
}

int vm_machine_executor_state_take_reset(vm_machine_executor_state *state)
{
    return state != STD_NULL && atomic_exchange(&state->reset_requested, TYPE_FALSE);
}

void vm_machine_executor_state_request_pause(vm_machine_executor_state *state)
{
    if (state != STD_NULL && atomic_load(&state->active))
        atomic_store(&state->pause_requested, TYPE_TRUE);
}

void vm_machine_executor_state_acknowledge_pause(vm_machine_executor_state *state)
{
    if (state != STD_NULL) atomic_store(&state->paused, TYPE_TRUE);
}

void vm_machine_executor_state_resume(vm_machine_executor_state *state)
{
    if (state == STD_NULL) return;
    atomic_store(&state->pause_requested, TYPE_FALSE);
    atomic_store(&state->paused, TYPE_FALSE);
}

int vm_machine_executor_state_is_active(const vm_machine_executor_state *state)
{ return state != STD_NULL && atomic_load(&state->active); }
int vm_machine_executor_state_is_paused(const vm_machine_executor_state *state)
{ return state != STD_NULL && atomic_load(&state->paused); }
int vm_machine_executor_state_pause_requested(const vm_machine_executor_state *state)
{ return state != STD_NULL && atomic_load(&state->pause_requested); }

vm_machine_lifecycle vm_machine_executor_state_lifecycle(const vm_machine_executor_state *state)
{
    return state == STD_NULL || !atomic_load(&state->active) ? VM_MACHINE_STOPPED :
        atomic_load(&state->paused) ? VM_MACHINE_PAUSED : VM_MACHINE_RUNNING;
}

#include "type.h"
#include "vm/platform/session_state.h"

struct vm_session_state {
    atomic_bool active;
    atomic_bool reset_requested;
    atomic_bool pause_requested;
    atomic_bool paused;
};

type_status vm_session_state_create(vm_session_state **out_state)
{
    vm_session_state *state;

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

void vm_session_state_destroy(vm_session_state *state)
{
    free(state);
}

void vm_session_state_start(vm_session_state *state)
{
    if (state == STD_NULL) return;
    if (atomic_load(&state->active)) return;
    atomic_store(&state->reset_requested, TYPE_FALSE);
    atomic_store(&state->pause_requested, TYPE_FALSE);
    atomic_store(&state->paused, TYPE_FALSE);
    atomic_store(&state->active, TYPE_TRUE);
}

void vm_session_state_stop(vm_session_state *state)
{
    if (state == STD_NULL) return;
    atomic_store(&state->active, TYPE_FALSE);
    atomic_store(&state->reset_requested, TYPE_FALSE);
    atomic_store(&state->paused, TYPE_FALSE);
    atomic_store(&state->pause_requested, TYPE_FALSE);
}

void vm_session_state_request_reset(vm_session_state *state)
{
    if (state != STD_NULL) atomic_store(&state->reset_requested, TYPE_TRUE);
}

int vm_session_state_take_reset(vm_session_state *state)
{
    return state != STD_NULL && atomic_exchange(&state->reset_requested, TYPE_FALSE);
}

void vm_session_state_request_pause(vm_session_state *state)
{
    if (state != STD_NULL && atomic_load(&state->active))
        atomic_store(&state->pause_requested, TYPE_TRUE);
}

void vm_session_state_acknowledge_pause(vm_session_state *state)
{
    if (state != STD_NULL) atomic_store(&state->paused, TYPE_TRUE);
}

void vm_session_state_resume(vm_session_state *state)
{
    if (state == STD_NULL) return;
    atomic_store(&state->pause_requested, TYPE_FALSE);
    atomic_store(&state->paused, TYPE_FALSE);
}

int vm_session_state_is_active(const vm_session_state *state)
{ return state != STD_NULL && atomic_load(&state->active); }
int vm_session_state_is_paused(const vm_session_state *state)
{ return state != STD_NULL && atomic_load(&state->paused); }
int vm_session_state_pause_requested(const vm_session_state *state)
{ return state != STD_NULL && atomic_load(&state->pause_requested); }

vm_session_lifecycle vm_session_state_lifecycle(const vm_session_state *state)
{
    return state == STD_NULL || !atomic_load(&state->active) ? VM_SESSION_STOPPED :
        atomic_load(&state->paused) ? VM_SESSION_PAUSED : VM_SESSION_RUNNING;
}

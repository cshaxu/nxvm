#include "lib/base/base.h"
#include "lib/session/state.h"

struct lib_session_state {
    atomic_bool flip;
    atomic_bool active;
    atomic_bool reset_requested;
    atomic_bool pause_requested;
    atomic_bool paused;
    atomic_bool step_requested;
    atomic_int pause_reason;
};

lib_status lib_session_state_create(lib_session_state **out_state)
{
    lib_session_state *state;

    if (out_state == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_state = LIB_NULL;
    state = calloc(1u, sizeof(*state));
    if (state == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    atomic_init(&state->flip, LIB_FALSE);
    atomic_init(&state->active, LIB_FALSE);
    atomic_init(&state->reset_requested, LIB_FALSE);
    atomic_init(&state->pause_requested, LIB_FALSE);
    atomic_init(&state->paused, LIB_FALSE);
    atomic_init(&state->step_requested, LIB_FALSE);
    atomic_init(&state->pause_reason, 0u);
    *out_state = state;
    return LIB_STATUS_OK;
}

void lib_session_state_destroy(lib_session_state *state)
{
    free(state);
}

void lib_session_state_start(lib_session_state *state)
{
    if (state == LIB_NULL) return;
    if (atomic_load(&state->active)) return;
    atomic_store(&state->reset_requested, LIB_FALSE);
    atomic_store(&state->pause_requested, LIB_FALSE);
    atomic_store(&state->paused, LIB_FALSE);
    atomic_store(&state->step_requested, LIB_FALSE);
    atomic_store(&state->pause_reason, 0u);
    atomic_store(&state->active, LIB_TRUE);
    atomic_store(&state->flip, !atomic_load(&state->flip));
}

void lib_session_state_stop(lib_session_state *state)
{
    if (state == LIB_NULL) return;
    atomic_store(&state->active, LIB_FALSE);
    atomic_store(&state->reset_requested, LIB_FALSE);
    atomic_store(&state->paused, LIB_FALSE);
    atomic_store(&state->pause_requested, LIB_FALSE);
    atomic_store(&state->step_requested, LIB_FALSE);
    atomic_store(&state->pause_reason, 0u);
}

void lib_session_state_fault(lib_session_state *state)
{
    lib_session_state_stop(state);
    if (state != LIB_NULL) {
        atomic_store(&state->step_requested, LIB_FALSE);
        atomic_store(&state->pause_reason, 0u);
    }
}

void lib_session_state_request_reset(lib_session_state *state)
{
    if (state != LIB_NULL) atomic_store(&state->reset_requested, LIB_TRUE);
}

int lib_session_state_take_reset(lib_session_state *state)
{
    return state != LIB_NULL && atomic_exchange(&state->reset_requested, LIB_FALSE);
}

void lib_session_state_request_pause(lib_session_state *state, lib_u32 reason)
{
    if (state == LIB_NULL) return;
    atomic_store(&state->pause_reason, reason);
    if (atomic_load(&state->active)) {
        atomic_store(&state->pause_requested, LIB_TRUE);
    } else {
        atomic_store(&state->paused, LIB_TRUE);
    }
}

void lib_session_state_acknowledge_pause(lib_session_state *state)
{
    if (state != LIB_NULL) atomic_store(&state->paused, LIB_TRUE);
}

void lib_session_state_resume(lib_session_state *state)
{
    if (state == LIB_NULL) return;
    atomic_store(&state->pause_requested, LIB_FALSE);
    atomic_store(&state->paused, LIB_FALSE);
    atomic_store(&state->step_requested, LIB_FALSE);
    atomic_store(&state->pause_reason, 0u);
}

int lib_session_state_request_step(lib_session_state *state)
{
    if (state == LIB_NULL || !atomic_load(&state->paused)) return LIB_FALSE;
    lib_session_state_resume(state);
    atomic_store(&state->step_requested, LIB_TRUE);
    return LIB_TRUE;
}

int lib_session_state_take_step(lib_session_state *state)
{
    return state != LIB_NULL && atomic_exchange(&state->step_requested, LIB_FALSE);
}

int lib_session_state_is_active(const lib_session_state *state)
{ return state != LIB_NULL && atomic_load(&state->active); }
int lib_session_state_is_paused(const lib_session_state *state)
{ return state != LIB_NULL && atomic_load(&state->paused); }
int lib_session_state_pause_requested(const lib_session_state *state)
{ return state != LIB_NULL && atomic_load(&state->pause_requested); }
int lib_session_state_step_requested(const lib_session_state *state)
{ return state != LIB_NULL && atomic_load(&state->step_requested); }
lib_u32 lib_session_state_pause_reason(const lib_session_state *state)
{ return state == LIB_NULL ? 0u : atomic_load(&state->pause_reason); }
int lib_session_state_flip(const lib_session_state *state)
{ return state != LIB_NULL && atomic_load(&state->flip); }

lib_session_lifecycle lib_session_state_lifecycle(const lib_session_state *state)
{
    return state == LIB_NULL || !atomic_load(&state->active) ? LIB_SESSION_STOPPED :
        atomic_load(&state->paused) ? LIB_SESSION_PAUSED : LIB_SESSION_RUNNING;
}

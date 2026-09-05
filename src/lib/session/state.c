#include "lib/session/state.h"

C_VOID lib_session_state_initialize(lib_session_state *state)
{
    if (state == STD_NULL) return;
    STD_ATOMIC_INIT(&state->flip, TYPE_FALSE);
    STD_ATOMIC_INIT(&state->active, TYPE_FALSE);
    STD_ATOMIC_INIT(&state->reset_requested, TYPE_FALSE);
    STD_ATOMIC_INIT(&state->pause_requested, TYPE_FALSE);
    STD_ATOMIC_INIT(&state->paused, TYPE_FALSE);
    STD_ATOMIC_INIT(&state->step_requested, TYPE_FALSE);
    STD_ATOMIC_INIT(&state->pause_reason, 0u);
}

C_VOID lib_session_state_start(lib_session_state *state)
{
    if (state == STD_NULL) return;
    STD_ATOMIC_STORE(&state->active, TYPE_TRUE);
    STD_ATOMIC_STORE(&state->flip, !STD_ATOMIC_LOAD(&state->flip));
}

C_VOID lib_session_state_stop(lib_session_state *state)
{
    if (state == STD_NULL) return;
    STD_ATOMIC_STORE(&state->active, TYPE_FALSE);
    STD_ATOMIC_STORE(&state->paused, TYPE_FALSE);
    STD_ATOMIC_STORE(&state->pause_requested, TYPE_FALSE);
}

C_VOID lib_session_state_fault(lib_session_state *state)
{
    lib_session_state_stop(state);
    if (state != STD_NULL) {
        STD_ATOMIC_STORE(&state->step_requested, TYPE_FALSE);
        STD_ATOMIC_STORE(&state->pause_reason, 0u);
    }
}

C_VOID lib_session_state_request_reset(lib_session_state *state)
{
    if (state != STD_NULL) STD_ATOMIC_STORE(&state->reset_requested, TYPE_TRUE);
}

C_INT lib_session_state_take_reset(lib_session_state *state)
{
    return state != STD_NULL && STD_ATOMIC_EXCHANGE(&state->reset_requested, TYPE_FALSE);
}

C_VOID lib_session_state_request_pause(lib_session_state *state,
    type_unsigned_32 reason)
{
    if (state == STD_NULL) return;
    STD_ATOMIC_STORE(&state->pause_reason, reason);
    if (STD_ATOMIC_LOAD(&state->active)) {
        STD_ATOMIC_STORE(&state->pause_requested, TYPE_TRUE);
    } else {
        STD_ATOMIC_STORE(&state->paused, TYPE_TRUE);
    }
}

C_VOID lib_session_state_acknowledge_pause(lib_session_state *state)
{
    if (state != STD_NULL) STD_ATOMIC_STORE(&state->paused, TYPE_TRUE);
}

C_VOID lib_session_state_resume(lib_session_state *state)
{
    if (state == STD_NULL) return;
    STD_ATOMIC_STORE(&state->pause_requested, TYPE_FALSE);
    STD_ATOMIC_STORE(&state->paused, TYPE_FALSE);
    STD_ATOMIC_STORE(&state->step_requested, TYPE_FALSE);
    STD_ATOMIC_STORE(&state->pause_reason, 0u);
}

C_INT lib_session_state_request_step(lib_session_state *state)
{
    if (state == STD_NULL || !STD_ATOMIC_LOAD(&state->paused)) return TYPE_FALSE;
    lib_session_state_resume(state);
    STD_ATOMIC_STORE(&state->step_requested, TYPE_TRUE);
    return TYPE_TRUE;
}

C_INT lib_session_state_take_step(lib_session_state *state)
{
    return state != STD_NULL && STD_ATOMIC_EXCHANGE(&state->step_requested, TYPE_FALSE);
}

C_INT lib_session_state_is_active(const lib_session_state *state)
{ return state != STD_NULL && STD_ATOMIC_LOAD(&state->active); }
C_INT lib_session_state_is_paused(const lib_session_state *state)
{ return state != STD_NULL && STD_ATOMIC_LOAD(&state->paused); }
C_INT lib_session_state_pause_requested(const lib_session_state *state)
{ return state != STD_NULL && STD_ATOMIC_LOAD(&state->pause_requested); }
C_INT lib_session_state_step_requested(const lib_session_state *state)
{ return state != STD_NULL && STD_ATOMIC_LOAD(&state->step_requested); }
type_unsigned_32 lib_session_state_pause_reason(const lib_session_state *state)
{ return state == STD_NULL ? 0u : STD_ATOMIC_LOAD(&state->pause_reason); }
C_INT lib_session_state_flip(const lib_session_state *state)
{ return state != STD_NULL && STD_ATOMIC_LOAD(&state->flip); }

#ifndef LIB_SESSION_STATE_H
#define LIB_SESSION_STATE_H

#include "lib/base/base.h"

typedef struct lib_session_state {
    atomic_bool flip;
    atomic_bool active;
    atomic_bool reset_requested;
    atomic_bool pause_requested;
    atomic_bool paused;
    atomic_bool step_requested;
    atomic_int pause_reason;
} lib_session_state;

void lib_session_state_initialize(lib_session_state *state);
void lib_session_state_start(lib_session_state *state);
void lib_session_state_stop(lib_session_state *state);
void lib_session_state_fault(lib_session_state *state);
void lib_session_state_request_reset(lib_session_state *state);
int lib_session_state_take_reset(lib_session_state *state);
void lib_session_state_request_pause(lib_session_state *state, lib_u32 reason);
void lib_session_state_acknowledge_pause(lib_session_state *state);
void lib_session_state_resume(lib_session_state *state);
int lib_session_state_request_step(lib_session_state *state);
int lib_session_state_take_step(lib_session_state *state);
int lib_session_state_is_active(const lib_session_state *state);
int lib_session_state_is_paused(const lib_session_state *state);
int lib_session_state_pause_requested(const lib_session_state *state);
int lib_session_state_step_requested(const lib_session_state *state);
lib_u32 lib_session_state_pause_reason(const lib_session_state *state);
int lib_session_state_flip(const lib_session_state *state);

#endif

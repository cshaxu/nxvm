#ifndef LIB_SESSION_STATE_H
#define LIB_SESSION_STATE_H

#include "type.h"

typedef struct lib_session_state {
    STD_ATOMIC_BOOL flip;
    STD_ATOMIC_BOOL active;
    STD_ATOMIC_BOOL reset_requested;
    STD_ATOMIC_BOOL pause_requested;
    STD_ATOMIC_BOOL paused;
    STD_ATOMIC_BOOL step_requested;
    STD_ATOMIC_INT pause_reason;
} lib_session_state;

C_VOID lib_session_state_initialize(lib_session_state *state);
C_VOID lib_session_state_start(lib_session_state *state);
C_VOID lib_session_state_stop(lib_session_state *state);
C_VOID lib_session_state_fault(lib_session_state *state);
C_VOID lib_session_state_request_reset(lib_session_state *state);
C_INT lib_session_state_take_reset(lib_session_state *state);
C_VOID lib_session_state_request_pause(lib_session_state *state,
    type_unsigned_32 reason);
C_VOID lib_session_state_acknowledge_pause(lib_session_state *state);
C_VOID lib_session_state_resume(lib_session_state *state);
C_INT lib_session_state_request_step(lib_session_state *state);
C_INT lib_session_state_take_step(lib_session_state *state);
C_INT lib_session_state_is_active(const lib_session_state *state);
C_INT lib_session_state_is_paused(const lib_session_state *state);
C_INT lib_session_state_pause_requested(const lib_session_state *state);
C_INT lib_session_state_step_requested(const lib_session_state *state);
type_unsigned_32 lib_session_state_pause_reason(const lib_session_state *state);
C_INT lib_session_state_flip(const lib_session_state *state);

#endif

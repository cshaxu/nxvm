#ifndef LIB_SESSION_STATE_H
#define LIB_SESSION_STATE_H

#include "lib/base/base.h"

typedef struct lib_session_state lib_session_state;

typedef enum lib_session_lifecycle {
    LIB_SESSION_STOPPED,
    LIB_SESSION_RUNNING,
    LIB_SESSION_PAUSED
} lib_session_lifecycle;

lib_status lib_session_state_create(lib_session_state **out_state);
void lib_session_state_destroy(lib_session_state *state);
void lib_session_state_start(lib_session_state *state);
void lib_session_state_stop(lib_session_state *state);
void lib_session_state_request_reset(lib_session_state *state);
int lib_session_state_take_reset(lib_session_state *state);
void lib_session_state_request_pause(lib_session_state *state);
void lib_session_state_acknowledge_pause(lib_session_state *state);
void lib_session_state_resume(lib_session_state *state);
int lib_session_state_is_active(const lib_session_state *state);
int lib_session_state_is_paused(const lib_session_state *state);
int lib_session_state_pause_requested(const lib_session_state *state);
lib_session_lifecycle lib_session_state_lifecycle(const lib_session_state *state);

#endif

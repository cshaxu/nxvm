#ifndef VM_SESSION_STATE_H
#define VM_SESSION_STATE_H

#include "type.h"

typedef struct vm_session_state vm_session_state;

typedef enum vm_session_lifecycle {
    VM_SESSION_STOPPED,
    VM_SESSION_RUNNING,
    VM_SESSION_PAUSED
} vm_session_lifecycle;

type_status vm_session_state_create(vm_session_state **out_state);
void vm_session_state_destroy(vm_session_state *state);
void vm_session_state_start(vm_session_state *state);
void vm_session_state_stop(vm_session_state *state);
void vm_session_state_request_reset(vm_session_state *state);
int vm_session_state_take_reset(vm_session_state *state);
void vm_session_state_request_pause(vm_session_state *state);
void vm_session_state_acknowledge_pause(vm_session_state *state);
void vm_session_state_resume(vm_session_state *state);
int vm_session_state_is_active(const vm_session_state *state);
int vm_session_state_is_paused(const vm_session_state *state);
int vm_session_state_pause_requested(const vm_session_state *state);
vm_session_lifecycle vm_session_state_lifecycle(const vm_session_state *state);

#endif

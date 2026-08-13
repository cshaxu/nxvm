#ifndef VM_SESSION_RUNNER_H
#define VM_SESSION_RUNNER_H

#include "type.h"

struct vm_session;

C_VOID vm_session_runner_run(struct vm_session *session);

#endif

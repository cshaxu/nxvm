#ifndef NTVDM64_VM_SESSION_RUNNER_H
#define NTVDM64_VM_SESSION_RUNNER_H

#include "type.h"

struct vm_session;

C_VOID vm_session_runner_run(struct vm_session *session);

#endif

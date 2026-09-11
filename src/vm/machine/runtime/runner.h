#ifndef VM_MACHINE_RUNNER_H
#define VM_MACHINE_RUNNER_H

#include "type.h"

struct vm_machine;

C_VOID vm_machine_runner_run(struct vm_machine *session);

#endif

#ifndef VM_MACHINE_RUNNER_H
#define VM_MACHINE_RUNNER_H
#include "lib/types/types_interface.h"

#include "type.h"

struct vm_machine;

C_VOID vm_machine_runner_run(struct vm_machine *session);

#endif

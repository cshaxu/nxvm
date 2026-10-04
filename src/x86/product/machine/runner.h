#ifndef VM_MACHINE_RUNNER_H
#define VM_MACHINE_RUNNER_H
#include "lib/types/types_interface.h"


struct vm_machine;

void vm_machine_runner_run(struct vm_machine *session);

#endif

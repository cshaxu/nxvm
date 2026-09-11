#ifndef VM_MACHINE_DISPLAY_BRIDGE_H
#define VM_MACHINE_DISPLAY_BRIDGE_H


#include "type.h"

#include "core/machine/display_interface.h"
struct vm_machine;
C_VOID vm_machine_bind_display(struct vm_machine *machine);
core_machine_display_kind vm_machine_publish_display(struct vm_machine *machine,
    C_INT force);

#endif

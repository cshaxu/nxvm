#ifndef NTVDM64_VM_COMPOSITION_DISPLAY_H
#define NTVDM64_VM_COMPOSITION_DISPLAY_H


#include "type.h"
struct vm_composition_live_machine;
C_VOID vm_composition_bind_display(struct vm_composition_live_machine *machine);
C_VOID vm_composition_publish_display(struct vm_composition_live_machine *machine,
    C_INT force);

#endif

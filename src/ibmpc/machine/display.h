#ifndef VM_MACHINE_DISPLAY_BRIDGE_H
#define VM_MACHINE_DISPLAY_BRIDGE_H
#include "lib/types/types_interface.h"



#include "ibmpc/board-common/display_interface.h"
struct vm_machine;
void vm_machine_bind_display(struct vm_machine *machine);
x86_video_kind vm_machine_publish_display(struct vm_machine *machine,
    lib_bool force);

#endif

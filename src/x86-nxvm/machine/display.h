#ifndef X86_NXVM_MACHINE_DISPLAY_BRIDGE_H
#define X86_NXVM_MACHINE_DISPLAY_BRIDGE_H
#include "lib/types/types_interface.h"



#include "x86-nxvm/board-base/display_interface.h"
struct x86_nxvm_machine;
void x86_nxvm_machine_bind_display(struct x86_nxvm_machine *machine);
lib_status x86_nxvm_machine_publish_display(struct x86_nxvm_machine *machine,
    lib_bool force);

#endif

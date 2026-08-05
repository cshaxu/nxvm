#ifndef VM_SESSION_DISPLAY_BRIDGE_H
#define VM_SESSION_DISPLAY_BRIDGE_H


#include "type.h"

#include "core/machine/display_interface.h"
struct vm_session;
C_VOID vm_session_bind_display(struct vm_session *machine);
core_machine_display_kind vm_session_publish_display(struct vm_session *machine,
    C_INT force);

#endif

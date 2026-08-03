#ifndef NTVDM64_VM_SESSION_DISPLAY_BRIDGE_H
#define NTVDM64_VM_SESSION_DISPLAY_BRIDGE_H


#include "type.h"
struct vm_session;
C_VOID vm_session_bind_display(struct vm_session *machine);
C_VOID vm_session_publish_display(struct vm_session *machine,
    C_INT force);

#endif

#ifndef NTVDM64_VM_SESSION_BLOCK_PROVIDER_H
#define NTVDM64_VM_SESSION_BLOCK_PROVIDER_H


#include "type.h"
struct vm_session;
C_VOID vm_session_bind_block(struct vm_session *machine);

#endif

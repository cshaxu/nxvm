#ifndef NTVDM64_VM_SESSION_CONSOLE_TARGET_H
#define NTVDM64_VM_SESSION_CONSOLE_TARGET_H

#include "type.h"

#include "vm/product/console_target.h"

#include "vm/composition/session/session.h"

C_VOID vm_session_console_target_initialize(
    nxvm_product_console_target *target,
    vm_session *machine);

#endif

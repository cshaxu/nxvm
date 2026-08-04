#ifndef VM_SESSION_DEBUG_TARGET_H
#define VM_SESSION_DEBUG_TARGET_H

#include "type.h"

#include "core/product/debug/debug_target.h"

#include "vm/composition/session/session.h"

const core_product_debug_target *vm_session_debug_target(
    vm_session *machine);
C_VOID vm_session_debug_flush_console_input(C_VOID *context);
C_VOID vm_session_debug_target_finalize(vm_session *machine);

#endif

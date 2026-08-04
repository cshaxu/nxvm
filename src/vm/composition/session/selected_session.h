#ifndef VM_SELECTED_SESSION_H
#define VM_SELECTED_SESSION_H

#include "type.h"

#include "core/product/session/session_interface.h"
#include "vm/composition/session/session.h"

vm_session *vm_session_borrow_selected(C_VOID *manager_context);

#endif

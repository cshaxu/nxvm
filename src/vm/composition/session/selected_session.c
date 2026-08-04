#include "type.h"

#include "vm/composition/session/selected_session.h"

vm_session *vm_session_borrow_selected(C_VOID *manager_context)
{
    C_VOID *session = STD_NULL;
    core_product_session_manager *manager =
        (core_product_session_manager *)manager_context;

    if (manager == STD_NULL || core_product_session_manager_borrow_selected(
            manager, &session) != TYPE_STATUS_OK) {
        return STD_NULL;
    }
    return (vm_session *)session;
}

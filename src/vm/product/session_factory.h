#ifndef VM_PRODUCT_SESSION_FACTORY_H
#define VM_PRODUCT_SESSION_FACTORY_H

#include "type.h"

#include "vm/composition/session/session_interface.h"
#include "vm/product/console_machine_provider.h"

typedef struct vm_product_session_request vm_product_session_request;

type_status vm_session_create_from_request(
    const vm_product_session_request *request, vm_session **out_session);

#endif

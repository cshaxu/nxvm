#ifndef VM_PRODUCT_SESSION_FACTORY_H
#define VM_PRODUCT_SESSION_FACTORY_H

#include "type.h"

#include "vm/machine/runtime/machine_interface.h"
#include "vm/session/catalog.h"

typedef struct vm_product_session_request vm_product_session_request;

type_status vm_machine_create_from_request(
    const vm_product_session_request *request, vm_machine **out_session);

#endif

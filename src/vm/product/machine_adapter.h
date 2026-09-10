#ifndef VM_PRODUCT_MACHINE_ADAPTER_H
#define VM_PRODUCT_MACHINE_ADAPTER_H

#include "vm/product/console_machine_provider.h"

typedef struct vm_session vm_session;

C_VOID vm_product_machine_provider_initialize(
    vm_session_machine_provider *machine_provider,
    vm_session **session_slot);

#endif

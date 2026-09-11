#ifndef VM_PRODUCT_MACHINE_ADAPTER_H
#define VM_PRODUCT_MACHINE_ADAPTER_H

#include "vm/product/console_machine_provider.h"

typedef struct vm_machine vm_machine;

C_VOID vm_product_machine_provider_initialize(
    vm_product_machine_provider *machine_provider,
    vm_machine **session_slot);

#endif

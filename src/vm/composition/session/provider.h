#ifndef VM_SESSION_PROVIDER_H
#define VM_SESSION_PROVIDER_H

#include "type.h"

#include "core/product/session/session_provider.h"

#include "vm/product/console_machine_provider.h"

#include "vm/composition/session/session_interface.h"

C_VOID vm_session_provider_initialize(core_product_session_provider *provider);
C_VOID vm_session_machine_provider_initialize(
    vm_product_console_machine_provider *machine_provider,
    core_product_session_manager *manager);

#endif

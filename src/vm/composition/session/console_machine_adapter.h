#ifndef VM_COMPOSITION_SESSION_CONSOLE_MACHINE_ADAPTER_H
#define VM_COMPOSITION_SESSION_CONSOLE_MACHINE_ADAPTER_H

#include "core/product/session/session_interface.h"
#include "vm/product/console_machine_provider.h"

C_VOID vm_composition_console_machine_provider_initialize(
    vm_session_machine_provider *machine_provider,
    core_product_session_manager *manager);

#endif

#ifndef VM_APP_FACTORY_INTERFACE_H
#define VM_APP_FACTORY_INTERFACE_H

#include "ibmpc/product/machine_interface.h"
#include "ibmpc/machine/input_interface.h"

/* Fixed composition values and assets outlive Product. prepare interprets the
 * selected Profile and publishes copied construction plus its context on
 * success; failure releases any partial Profile and leaves no candidate. */
typedef struct vm_app_machine_binding {
    const char *name;
    core_machine_cpu_profile cpu;
    x86_fpu_profile fpu;
    vm_machine_floppy_format floppy_format;
    lib_size bios_count;
    const vm_machine_assets *firmware;
    lib_status (*prepare)(const vm_machine_config *config,
        const vm_machine_assets *assets, vm_machine_construction *out_construction);
} vm_app_machine_binding;

lib_status vm_app_configure_machine(const vm_app_machine_binding *binding,
    const vm_session_request *request, vm_machine_config *out_config);
void vm_app_configure_factory(const vm_app_machine_binding *binding,
    vm_app_factory *out_factory);

#endif

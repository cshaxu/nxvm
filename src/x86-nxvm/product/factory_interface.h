#ifndef X86_NXVM_PRODUCT_FACTORY_INTERFACE_H
#define X86_NXVM_PRODUCT_FACTORY_INTERFACE_H

#include "x86-nxvm/product/request_interface.h"
#include "x86-ibmpc/product/entry_interface.h"
#include "x86-ibmpc/product/command_interface.h"
#include "x86-nxvm/product/machine_interface.h"
#include "x86-nxvm/machine/input_interface.h"
#include "x86-nxvm/machine/machine_interface.h"

/* Fixed composition values and assets outlive Product. prepare interprets the
 * selected Profile and publishes copied construction plus its context on
 * success; failure releases any partial Profile and leaves no candidate. */
typedef struct x86_nxvm_product_machine_binding {
    const char *name;
    x86_core_cpu_profile cpu;
    x86_fpu_profile fpu;
    /* Product-selected runner policy.  Standard remains the zero/default
     * value, so existing bindings retain their established behavior. */
    x86_nxvm_machine_speed initial_speed;
    x86_nxvm_machine_floppy_format floppy_format;
    lib_size bios_count;
    const x86_nxvm_machine_assets *firmware;
    lib_status (*prepare)(const x86_nxvm_machine_config *config,
        const x86_nxvm_machine_assets *assets, x86_nxvm_machine_construction *out_construction);
    /* Called after worker shutdown and unbinding, before Machine destruction. */
    lib_status (*save_configuration)(x86_nxvm_machine *machine);
} x86_nxvm_product_machine_binding;

lib_status x86_nxvm_product_configure_machine(const x86_nxvm_product_machine_binding *binding,
    const x86_nxvm_product_request *request, x86_nxvm_machine_config *out_config);
lib_status x86_nxvm_product_compose_machine(const x86_nxvm_product_machine_binding *binding,
    const x86_nxvm_product_request *request, x86_nxvm_product_machine *out_machine);
lib_status x86_nxvm_product_configure_ui(const x86_nxvm_product_request *request,
    x86_ibmpc_product_ui_options *out_ui);

/* NXVM-family commands are registered by each App; shared Product dispatch
 * has no INFO, SPEED or floppy cases. */
lib_status x86_nxvm_product_configure_standard_extensions(x86_nxvm_product_machine *machine,
    x86_ibmpc_product_command_extensions *out_extensions);

#endif

#ifndef VM_APP_ENTRY_INTERFACE_H
#define VM_APP_ENTRY_INTERFACE_H

#include "ibmpc/product/machine_interface.h"

/* Immutable App identity and factory; all borrowed values outlive run. */
typedef struct vm_app_definition {
    const char *name;
    const char *version;
    const char *copyright;
    const char *build_time;
    vm_app_factory factory;
} vm_app_definition;

/* Sole PC process body. Returns the existing process success/failure code. */
lib_i32 vm_app_run(const vm_app_definition *definition);

#endif

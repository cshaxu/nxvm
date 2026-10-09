#ifndef VM_APP_ENTRY_INTERFACE_H
#define VM_APP_ENTRY_INTERFACE_H

#include "ibmpc/product/composition_interface.h"
#include "ibmpc/product/command_interface.h"

/* Configuration syntax is App policy.  Product receives only one validated
 * request and never assumes an NXVM.ini or SoftPC.ini document shape. */
typedef lib_status (*vm_app_request_loader)(const char *configuration_file,
    vm_session_request *out_request);

/* Immutable App identity and factory; all borrowed values outlive run. */
typedef struct vm_app_definition {
    const char *name;
    const char *version;
    const char *copyright;
    const char *build_time;
    const char *configuration_file;
    vm_app_request_loader load_request;
    vm_app_factory factory;
    lib_status (*configure_extensions)(vm_app *, app_command_extensions *);
} vm_app_definition;

/* Sole PC process body. Returns the existing process success/failure code. */
lib_i32 vm_app_run(const vm_app_definition *definition);

#endif

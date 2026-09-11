#ifndef VM_PRODUCT_CONSOLE_HOST_H
#define VM_PRODUCT_CONSOLE_HOST_H

#include "type.h"
#include "lib/console/console_interface.h"

typedef struct vm_product_console_host vm_product_console_host;

type_status vm_product_console_host_create(vm_product_console_host **out_host,
    C_VOID *line_context, void (*line_sink)(C_VOID *context,
        const C_CHAR *text));
C_VOID vm_product_console_host_destroy(vm_product_console_host *host);
type_status vm_product_console_host_request_line(vm_product_console_host *host);
type_status vm_product_console_host_write(vm_product_console_host *host,
    const C_CHAR *text);
type_status vm_product_console_host_claim_guest(vm_product_console_host *host,
    lib_console *guest_console);
type_status vm_product_console_host_release_guest(vm_product_console_host *host,
    lib_console *guest_console);

#endif

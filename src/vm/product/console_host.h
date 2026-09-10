#ifndef VM_PRODUCT_CONSOLE_HOST_H
#define VM_PRODUCT_CONSOLE_HOST_H

#include "type.h"
#include "vm/platform/platform.h"

typedef struct vm_product_console_host vm_product_console_host;

type_status vm_product_console_host_create(vm_product_console_host **out_host,
    C_VOID *line_context, void (*line_sink)(C_VOID *context,
        const C_CHAR *text));
C_VOID vm_product_console_host_destroy(vm_product_console_host *host);
type_status vm_product_console_host_request_line(vm_product_console_host *host);
type_status vm_product_console_host_write(vm_product_console_host *host,
    const C_CHAR *text);
const vm_platform_console_binding *vm_product_console_host_binding(
    const vm_product_console_host *host);

#endif

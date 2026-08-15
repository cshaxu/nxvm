#ifndef VM_PLATFORM_VIRTUAL_TIME_INTERFACE_H
#define VM_PLATFORM_VIRTUAL_TIME_INTERFACE_H

#include "type.h"

/* Composition selects the source; platform supplies one host-backed source. */
typedef type_status (*vm_virtual_time_source_next)(C_VOID *context,
    type_unsigned_64 *out_source_ticks);
typedef C_VOID (*vm_virtual_time_source_reset)(C_VOID *context);

typedef struct vm_virtual_time_source {
    vm_virtual_time_source_next next;
    vm_virtual_time_source_reset reset;
    C_VOID *context;
} vm_virtual_time_source;

#endif

#ifndef VM_PLATFORM_VIRTUAL_TIME_H
#define VM_PLATFORM_VIRTUAL_TIME_H

#include "type.h"

#include "vm/platform/virtual_time_interface.h"

typedef struct vm_platform_virtual_time_source vm_platform_virtual_time_source;

type_status vm_platform_host_milliseconds(type_unsigned_64 *out_milliseconds);
type_status vm_platform_virtual_time_source_create(
    type_unsigned_64 source_ticks_per_second, vm_virtual_time_source *out_source,
    vm_platform_virtual_time_source **out_source_owner);
C_VOID vm_platform_virtual_time_source_destroy(
    vm_platform_virtual_time_source *source);

#endif

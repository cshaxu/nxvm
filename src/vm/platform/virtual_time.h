#ifndef VM_PLATFORM_VIRTUAL_TIME_H
#define VM_PLATFORM_VIRTUAL_TIME_H

#include "type.h"

#include "vm/composition/session/session_interface.h"

typedef struct vm_platform_virtual_time_source {
    type_unsigned_64 last_units;
    type_unsigned_64 remainder;
    type_unsigned_64 units_per_second;
    type_unsigned_64 source_ticks_per_second;
    C_INT initialized;
} vm_platform_virtual_time_source;

type_status vm_platform_virtual_time_source_initialize(
    vm_platform_virtual_time_source *source, type_unsigned_64 source_ticks_per_second,
    vm_session_virtual_time_source *out_source);

#endif

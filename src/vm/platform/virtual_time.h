#ifndef VM_PLATFORM_VIRTUAL_TIME_H
#define VM_PLATFORM_VIRTUAL_TIME_H

#include "type.h"

type_status vm_platform_host_milliseconds(type_unsigned_64 *out_milliseconds);
/* A monotonic host counter for pacing comparisons. It never represents or
 * advances guest time. */
type_status vm_platform_host_monotonic_counter(type_unsigned_64 *out_units,
    type_unsigned_64 *out_units_per_second);

#endif

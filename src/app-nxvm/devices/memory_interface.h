#ifndef CORE_MACHINE_MEMORY_INTERFACE_H
#define CORE_MACHINE_MEMORY_INTERFACE_H
#include "lib/types/types_interface.h"




#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef void (*core_machine_memory_parity_fault_observer)(void *owner,
    lib_u32 physical);

typedef void (*core_machine_memory_write_observer)(void *owner,
    lib_u32 physical, lib_uptr bytes);

typedef enum core_machine_a20_wrap_policy {
    CORE_MACHINE_A20_WRAP_GLOBAL_MASK = 0,
    CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB = 1
} core_machine_a20_wrap_policy;
typedef enum core_machine_memory_access {
    CORE_MACHINE_MEMORY_ACCESS_READ = 0,
    CORE_MACHINE_MEMORY_ACCESS_WRITE
} core_machine_memory_access;

typedef enum core_machine_memory_route {
    CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM = 0,
    CORE_MACHINE_MEMORY_ROUTE_PROVIDER
} core_machine_memory_route;

/* A composition-owned device may claim a configured physical range.  Returning
 * LIB_STATUS_UNSUPPORTED from query leaves the range available to the next
 * registered device or ordinary RAM; other failures are terminal. */
typedef lib_status (*core_machine_memory_device_read)(void *owner,
    lib_u32 physical, lib_uptr destination,
    lib_uptr bytes);
typedef lib_status (*core_machine_memory_device_write)(void *owner,
    lib_u32 physical, lib_uptr source,
    lib_uptr bytes);
typedef lib_status (*core_machine_memory_device_query)(void *owner,
    lib_u32 physical, lib_uptr bytes,
    core_machine_memory_access access);

typedef struct core_machine_memory_device_callbacks {
    core_machine_memory_device_read read;
    core_machine_memory_device_write write;
    core_machine_memory_device_query query;
} core_machine_memory_device_callbacks;

/* Configuration-only generic device registration.  Core owns checked routing;
 * the composition-owned callback context supplies all device semantics. */
lib_status core_machine_enable_memory_parity(core_machine *machine,
    lib_size bytes, core_machine_memory_parity_fault_observer fault, void *owner);

lib_status core_machine_register_memory_write_observer(core_machine *machine,
    core_machine_memory_write_observer callback, void *owner);
lib_status core_machine_register_memory_device(core_machine *machine,
    lib_u32 physical_start, lib_size bytes,
    const core_machine_memory_device_callbacks *callbacks, void *owner);
/* A board-owned selected decode replaces any lower ROM/RAM provider while its
 * query accepts the access; on decline the ordinary route remains intact. */
lib_status core_machine_register_memory_replacement_device(core_machine *machine,
    lib_u32 physical_start, lib_size bytes,
    const core_machine_memory_device_callbacks *callbacks, void *owner);
lib_status core_machine_memory_read(
    const core_machine *machine,
    lib_u32 physical,
    void *out_data,
    lib_size size);

lib_status core_machine_memory_write(
    core_machine *machine,
    lib_u32 physical,
    const void *data,
    lib_size size);

/* Queries one complete physical range without exposing storage or invoking a
 * provider data callback. It is valid only at a stopped or paused boundary. */
lib_status core_machine_memory_query(
    const core_machine *machine,
    lib_u32 physical,
    lib_size size,
    core_machine_memory_access access,
    core_machine_memory_route *out_route);

lib_status core_machine_set_a20(
    core_machine *machine,
    lib_i32 enabled);

#ifdef __cplusplus
}
#endif

#endif

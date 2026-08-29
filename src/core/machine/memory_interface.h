#ifndef CORE_MACHINE_MEMORY_INTERFACE_H
#define CORE_MACHINE_MEMORY_INTERFACE_H


#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef C_VOID (*core_machine_memory_parity_fault_observer)(C_VOID *owner,
    type_unsigned_32 physical);

typedef C_VOID (*core_machine_memory_write_observer)(C_VOID *owner,
    type_unsigned_32 physical, type_native_unsigned bytes);

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
 * TYPE_STATUS_UNSUPPORTED from query leaves the range available to the next
 * registered device or ordinary RAM; other failures are terminal. */
typedef type_status (*core_machine_memory_device_read)(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address destination,
    type_native_unsigned bytes);
typedef type_status (*core_machine_memory_device_write)(C_VOID *owner,
    type_unsigned_32 physical, type_virtual_address source,
    type_native_unsigned bytes);
typedef type_status (*core_machine_memory_device_query)(C_VOID *owner,
    type_unsigned_32 physical, type_native_unsigned bytes,
    core_machine_memory_access access);

typedef struct core_machine_memory_device_callbacks {
    core_machine_memory_device_read read;
    core_machine_memory_device_write write;
    core_machine_memory_device_query query;
} core_machine_memory_device_callbacks;

/* Configuration-only generic device registration.  Core owns checked routing;
 * the composition-owned callback context supplies all device semantics. */
type_status core_machine_enable_memory_parity(core_machine *machine,
    STD_SIZE_T bytes, core_machine_memory_parity_fault_observer fault, C_VOID *owner);

type_status core_machine_register_memory_write_observer(core_machine *machine,
    core_machine_memory_write_observer callback, C_VOID *owner);
type_status core_machine_register_memory_device(core_machine *machine,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    const core_machine_memory_device_callbacks *callbacks, C_VOID *owner);
/* A board-owned selected decode replaces any lower ROM/RAM provider while its
 * query accepts the access; on decline the ordinary route remains intact. */
type_status core_machine_register_memory_replacement_device(core_machine *machine,
    type_unsigned_32 physical_start, STD_SIZE_T bytes,
    const core_machine_memory_device_callbacks *callbacks, C_VOID *owner);
type_status core_machine_memory_read(
    const core_machine *machine,
    type_unsigned_32 physical,
    C_VOID *out_data,
    STD_SIZE_T size);

type_status core_machine_memory_write(
    core_machine *machine,
    type_unsigned_32 physical,
    const C_VOID *data,
    STD_SIZE_T size);

/* Queries one complete physical range without exposing storage or invoking a
 * provider data callback. It is valid only at a stopped or paused boundary. */
type_status core_machine_memory_query(
    const core_machine *machine,
    type_unsigned_32 physical,
    STD_SIZE_T size,
    core_machine_memory_access access,
    core_machine_memory_route *out_route);

type_status core_machine_set_a20(
    core_machine *machine,
    C_INT enabled);

#ifdef __cplusplus
}
#endif

#endif

#ifndef CORE_MACHINE_MEMORY_INTERFACE_H
#define CORE_MACHINE_MEMORY_INTERFACE_H


#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef enum core_machine_memory_access {
    CORE_MACHINE_MEMORY_ACCESS_READ = 0,
    CORE_MACHINE_MEMORY_ACCESS_WRITE
} core_machine_memory_access;

typedef enum core_machine_memory_route {
    CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM = 0,
    CORE_MACHINE_MEMORY_ROUTE_PROVIDER
} core_machine_memory_route;

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

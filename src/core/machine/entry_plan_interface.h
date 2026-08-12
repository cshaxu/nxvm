#ifndef CORE_MACHINE_ENTRY_PLAN_INTERFACE_H
#define CORE_MACHINE_ENTRY_PLAN_INTERFACE_H

#include "type.h"

#include "core/machine/memory_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

#define CORE_MACHINE_ENTRY_PLAN_PRELOAD_CAPACITY 16u

typedef struct core_machine_entry_plan_preload {
    /* Preload ranges must be nonempty and pairwise non-overlapping. */
    type_unsigned_32 physical;
    const type_unsigned_8 *bytes;
    STD_SIZE_T byte_count;
} core_machine_entry_plan_preload;

typedef struct core_machine_entry_plan_state {
    type_unsigned_16 cs;
    type_unsigned_16 ds;
    type_unsigned_16 es;
    type_unsigned_16 ss;
    type_unsigned_16 ip;
    type_unsigned_16 sp;
    type_unsigned_32 eax;
    type_unsigned_32 ebx;
    type_unsigned_32 ecx;
    type_unsigned_32 edx;
    type_unsigned_32 esi;
    type_unsigned_32 edi;
    type_unsigned_32 ebp;
    type_unsigned_32 eflags;
} core_machine_entry_plan_state;

typedef struct core_machine_entry_plan {
    core_machine_entry_plan_state state;
    type_unsigned_32 entry_physical;
    core_machine_memory_route entry_route;
    const core_machine_entry_plan_preload *preloads;
    STD_SIZE_T preload_count;
} core_machine_entry_plan;

type_status core_machine_apply_entry_plan(core_machine *machine,
    const core_machine_entry_plan *plan);

#ifdef __cplusplus
}
#endif

#endif

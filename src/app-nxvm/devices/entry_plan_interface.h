#ifndef CORE_MACHINE_ENTRY_PLAN_INTERFACE_H
#define CORE_MACHINE_ENTRY_PLAN_INTERFACE_H
#include "lib/types/types_interface.h"

#include "type.h"

#include "app-nxvm/devices/memory_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

#define CORE_MACHINE_ENTRY_PLAN_PRELOAD_CAPACITY 16u

typedef struct core_machine_entry_plan_preload {
    /* Preload ranges must be nonempty and pairwise non-overlapping. */
    lib_u32 physical;
    const lib_u8 *bytes;
    lib_size byte_count;
} core_machine_entry_plan_preload;

typedef struct core_machine_entry_plan_state {
    lib_u16 cs;
    lib_u16 ds;
    lib_u16 es;
    lib_u16 ss;
    lib_u16 ip;
    lib_u16 sp;
    lib_u32 eax;
    lib_u32 ebx;
    lib_u32 ecx;
    lib_u32 edx;
    lib_u32 esi;
    lib_u32 edi;
    lib_u32 ebp;
    lib_u32 eflags;
} core_machine_entry_plan_state;

typedef struct core_machine_entry_plan {
    core_machine_entry_plan_state state;
    lib_u32 entry_physical;
    core_machine_memory_route entry_route;
    const core_machine_entry_plan_preload *preloads;
    lib_size preload_count;
} core_machine_entry_plan;

type_status core_machine_apply_entry_plan(core_machine *machine,
    const core_machine_entry_plan *plan);

#ifdef __cplusplus
}
#endif

#endif

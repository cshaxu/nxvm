#ifndef CORE_MACHINE_ENTRY_PLAN_INTERFACE_H
#define CORE_MACHINE_ENTRY_PLAN_INTERFACE_H
#include "lib/types/types_interface.h"
#include "x86/chips/cpu/cpu_interface.h"


#include "x86/core/memory_interface.h"

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


typedef struct core_machine_entry_plan {
    core_machine_entry_plan_state state;
    lib_u32 entry_physical;
    core_machine_memory_route entry_route;
    const core_machine_entry_plan_preload *preloads;
    lib_size preload_count;
} core_machine_entry_plan;

lib_status core_machine_apply_entry_plan(core_machine *machine,
    const core_machine_entry_plan *plan);

#ifdef __cplusplus
}
#endif

#endif

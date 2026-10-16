#ifndef X86_CORE_MACHINE_ENTRY_PLAN_INTERFACE_H
#define X86_CORE_MACHINE_ENTRY_PLAN_INTERFACE_H
#include "lib/types/types_interface.h"
#include "x86-core/cpu/cpu_interface.h"


#include "x86-core/machine/memory_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct x86_core_machine x86_core_machine;

#define X86_CORE_MACHINE_ENTRY_PLAN_PRELOAD_CAPACITY 16u

typedef struct x86_core_machine_entry_plan_preload {
    /* Preload ranges must be nonempty and pairwise non-overlapping. */
    lib_u32 physical;
    const lib_u8 *bytes;
    lib_size byte_count;
} x86_core_machine_entry_plan_preload;


typedef struct x86_core_machine_entry_plan {
    x86_core_cpu_entry_plan_state state;
    lib_u32 entry_physical;
    x86_core_machine_memory_route entry_route;
    const x86_core_machine_entry_plan_preload *preloads;
    lib_size preload_count;
} x86_core_machine_entry_plan;

lib_status x86_core_machine_apply_entry_plan(x86_core_machine *machine,
    const x86_core_machine_entry_plan *plan);

#ifdef __cplusplus
}
#endif

#endif

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
    uint32_t physical;
    const uint8_t *bytes;
    STD_SIZE_T byte_count;
} core_machine_entry_plan_preload;

typedef struct core_machine_entry_plan_state {
    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t ss;
    uint16_t ip;
    uint16_t sp;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t eflags;
} core_machine_entry_plan_state;

typedef struct core_machine_entry_plan {
    core_machine_entry_plan_state state;
    uint32_t entry_physical;
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

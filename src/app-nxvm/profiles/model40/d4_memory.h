#ifndef CORE_MACHINE_D4_MEMORY_H
#define CORE_MACHINE_D4_MEMORY_H
#include "lib/types/types_interface.h"
#include "app-nxvm/profiles/model40/d4_memory_interface.h"
#include "x86/core/memory_interface.h"

typedef void (*core_machine_d4_iochk_output)(void *context, lib_bool asserted);

typedef struct core_machine_d4_memory {
    lib_u8 control;
    lib_u8 diagnostic_low;
    lib_u8 diagnostic_high;
    lib_u16 reset_ram_setup;
    lib_u16 ram_setup;
    lib_u8 parity_fault_mask;
    lib_u8 configured;
    core_machine_d4_iochk_output iochk_output;
    void *iochk_context;
} core_machine_d4_memory;

lib_status core_machine_d4_memory_configure(core_machine *core,
    core_machine_d4_memory *memory, const core_machine_d4_memory_config *config,
    core_machine_d4_iochk_output output, void *context);
void core_machine_d4_memory_reset(core_machine_d4_memory *memory);

#endif

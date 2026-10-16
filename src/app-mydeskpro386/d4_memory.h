#ifndef MYDESKPRO386_D4_MEMORY_H
#define MYDESKPRO386_D4_MEMORY_H
#include "lib/types/types_interface.h"
#include "app-mydeskpro386/d4_memory_interface.h"
#include "x86-core/machine/memory_interface.h"

typedef void (*mydeskpro386_d4_iochk_output)(void *context, lib_bool asserted);

typedef struct mydeskpro386_d4_memory {
    x86_core_machine *core;
    lib_u8 control;
    lib_u8 diagnostic_low;
    lib_u8 diagnostic_high;
    lib_u16 reset_ram_setup;
    lib_u16 ram_setup;
    lib_u8 parity_fault_mask;
    lib_bool configured;
    mydeskpro386_d4_iochk_output iochk_output;
    void *iochk_context;
} mydeskpro386_d4_memory;

lib_status mydeskpro386_d4_memory_configure(x86_core_machine *core,
    mydeskpro386_d4_memory *memory, const mydeskpro386_d4_memory_config *config,
    mydeskpro386_d4_iochk_output output, void *context);
void mydeskpro386_d4_memory_reset(mydeskpro386_d4_memory *memory);

#endif

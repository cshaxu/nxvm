#ifndef NXVM_VM_PROFILE_DEFAULT_CONTEXT_H
#define NXVM_VM_PROFILE_DEFAULT_CONTEXT_H

#include "core/machine/cpu_instructions.h"

typedef struct t_bios t_bios;
typedef struct t_qdx t_qdx;
typedef struct t_ram t_ram;
typedef struct t_vadp t_vadp;
typedef struct core_machine_block_provider_slot core_machine_block_provider_slot;
typedef struct core_machine_keyboard_provider_slot core_machine_keyboard_provider_slot;

typedef struct vm_profile_default_context {
    t_bios *bios;
    t_qdx *qdx;
    t_ram *ram;
    t_vadp *vadp;
    core_machine_block_provider_slot *block_provider;
    core_machine_keyboard_provider_slot *keyboard_provider;
    core_machine_cpu_execution_context *execution;
} vm_profile_default_context;

static inline void vm_profile_default_context_initialize(
    vm_profile_default_context *context, t_bios *bios, t_qdx *qdx, t_ram *ram,
    t_vadp *vadp, core_machine_block_provider_slot *block_provider,
    core_machine_keyboard_provider_slot *keyboard_provider)
{
    if (context == NULL) return;
    context->bios = bios;
    context->qdx = qdx;
    context->ram = ram;
    context->vadp = vadp;
    context->block_provider = block_provider;
    context->keyboard_provider = keyboard_provider;
    context->execution = NULL;
}

static inline vm_profile_default_context *
vm_profile_default_context_from_execution(core_machine_cpu_execution_context *execution)
{
    return (vm_profile_default_context *)
        core_machine_cpu_execution_context_extension(execution);
}

#endif

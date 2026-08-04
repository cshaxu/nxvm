#ifndef VM_PROFILE_DEFAULT_CONTEXT_H
#define VM_PROFILE_DEFAULT_CONTEXT_H

#include "type.h"

#include "core/machine/cpu_instructions.h"
#include "core/machine/machine_interface.h"
#include "core/product/wait_provider.h"

typedef struct t_bios t_bios;
typedef struct t_qdx t_qdx;
typedef struct t_vadp t_vadp;
typedef struct core_machine_block_provider_slot core_machine_block_provider_slot;
typedef struct core_machine_keyboard_provider_slot core_machine_keyboard_provider_slot;
typedef struct core_machine_display_provider_slot core_machine_display_provider_slot;

typedef struct vm_profile_default_context {
    t_bios *bios;
    t_qdx *qdx;
    core_machine_profile_binding binding;
    t_vadp *vadp;
    core_machine_block_provider_slot *block_provider;
    core_machine_keyboard_provider_slot *keyboard_provider;
    core_machine_display_provider_slot *display_provider;
    const core_product_wait_scope *wait_scope;
} vm_profile_default_context;

static inline C_VOID vm_profile_default_context_initialize(
    vm_profile_default_context *context, t_bios *bios, t_qdx *qdx,
    core_machine_profile_binding binding,
    t_vadp *vadp, core_machine_block_provider_slot *block_provider,
    core_machine_keyboard_provider_slot *keyboard_provider)
{
    if (context == STD_NULL) return;
    context->bios = bios;
    context->qdx = qdx;
    context->binding = binding;
    context->vadp = vadp;
    context->block_provider = block_provider;
    context->keyboard_provider = keyboard_provider;
    context->display_provider = STD_NULL;
    context->wait_scope = STD_NULL;
}

static inline t_ram *vm_profile_default_context_memory(
    const vm_profile_default_context *context)
{ return context == STD_NULL ? STD_NULL :
    core_machine_profile_binding_memory(&context->binding); }

static inline core_machine_cpu_execution_context *
vm_profile_default_context_execution(const vm_profile_default_context *context)
{ return context == STD_NULL ? STD_NULL :
    core_machine_profile_binding_execution(&context->binding); }

static inline vm_profile_default_context *
vm_profile_default_context_from_execution(core_machine_cpu_execution_context *execution)
{
    return (vm_profile_default_context *)
        core_machine_cpu_execution_context_extension(execution);
}

#endif

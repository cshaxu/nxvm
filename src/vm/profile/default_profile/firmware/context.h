#ifndef VM_PROFILE_DEFAULT_CONTEXT_H
#define VM_PROFILE_DEFAULT_CONTEXT_H

#include "type.h"

#include "core/machine/cpu_instructions.h"
#include "core/machine/media_interface.h"
#include "core/machine/machine_interface.h"

typedef struct t_bios t_bios;
typedef struct core_machine_display_provider_slot core_machine_display_provider_slot;

typedef struct vm_profile_default_context {
    t_bios *bios;
    core_machine_profile_binding binding;
    core_machine_media_registry *media_registry;
    core_machine_media_id hdd_media_id;
    core_machine_display_provider_slot *display_provider;
} vm_profile_default_context;

static inline C_VOID vm_profile_default_context_initialize(
    vm_profile_default_context *context, t_bios *bios,
    core_machine_profile_binding binding,
    core_machine_media_registry *media_registry, core_machine_media_id hdd_media_id)
{
    if (context == STD_NULL) return;
    context->bios = bios;
    context->binding = binding;
    context->media_registry = media_registry;
    context->hdd_media_id = hdd_media_id;
    context->display_provider = STD_NULL;
}

static inline t_ram *vm_profile_default_context_memory(
    const vm_profile_default_context *context)
{ return context == STD_NULL ? STD_NULL :
    core_machine_profile_binding_memory(&context->binding); }

static inline core_machine_cpu_execution_context *
vm_profile_default_context_execution(const vm_profile_default_context *context)
{ return context == STD_NULL ? STD_NULL :
    core_machine_profile_binding_execution(&context->binding); }

#endif

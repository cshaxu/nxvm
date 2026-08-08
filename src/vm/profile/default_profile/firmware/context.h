#ifndef VM_PROFILE_DEFAULT_CONTEXT_H
#define VM_PROFILE_DEFAULT_CONTEXT_H

#include "type.h"

#include "core/machine/firmware_interface.h"
#include "core/machine/media_interface.h"

typedef struct t_bios t_bios;

typedef struct vm_profile_default_context {
    t_bios *bios;
    core_machine_media_registry *media_registry;
    core_machine_media_id hdd_media_id;
} vm_profile_default_context;

static inline C_VOID vm_profile_default_context_initialize(
    vm_profile_default_context *context, t_bios *bios,
    core_machine_media_registry *media_registry, core_machine_media_id hdd_media_id)
{
    if (context == STD_NULL) return;
    context->bios = bios;
    context->media_registry = media_registry;
    context->hdd_media_id = hdd_media_id;
}

#endif

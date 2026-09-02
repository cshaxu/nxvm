#ifndef VM_PROFILE_DEFAULT_CONTEXT_H
#define VM_PROFILE_DEFAULT_CONTEXT_H

#include "type.h"

#include "core/machine/firmware_interface.h"
#include "core/machine/media_interface.h"
#include "vm/profile/default_profile/pc_at_profile_private.h"

typedef struct t_bios t_bios;

typedef struct vm_profile_default_context {
    t_bios *bios;
    core_machine_media_registry *media_registry;
    core_machine_media_id fdd_media_id;
    core_machine_media_id hdd_media_id;
    vm_profile_default_pc_at_firmware_slot firmware_slot;
    /* Optional session-owned ROM selected before Core construction.  It
       replaces, rather than supplements, the synthetic default-ROM route. */
    const type_unsigned_8 *external_rom;
} vm_profile_default_context;

static inline C_VOID vm_profile_default_context_initialize(
    vm_profile_default_context *context, t_bios *bios,
    core_machine_media_registry *media_registry, core_machine_media_id fdd_media_id,
    core_machine_media_id hdd_media_id,
    vm_profile_default_pc_at_firmware_slot firmware_slot,
    const type_unsigned_8 *external_rom)
{
    if (context == STD_NULL) return;
    context->bios = bios;
    context->media_registry = media_registry;
    context->fdd_media_id = fdd_media_id;
    context->hdd_media_id = hdd_media_id;
    context->firmware_slot = firmware_slot;
    context->external_rom = external_rom;
}

#endif

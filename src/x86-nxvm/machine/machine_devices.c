/* Copyright 2012-2014 Neko. */
#include "lib/types/types_interface.h"


#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/machine/media/media_interface.h"
#include "x86-nxvm/machine/machine_devices.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/media/fdd_interface.h"
#include "x86-nxvm/machine/media/hdd_interface.h"

lib_status x86_nxvm_machine_devices_initialize_media(x86_nxvm_machine *session)
{
    lib_status status;

    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_machine_fdd_allocate(x86_nxvm_board_floppy_geometry_get(
        session->construction.media_kind), &session->floppy[0u]);
    if (status != LIB_STATUS_OK) return status;
    if (session->construction.floppy_slot_count > 1u) {
        status = x86_nxvm_machine_fdd_allocate(x86_nxvm_board_floppy_geometry_get(
            session->construction.media_kind), &session->floppy[1u]);
        if (status != LIB_STATUS_OK) return status;
    }
    if (session->construction.hdc_present) {
        status = x86_nxvm_machine_hdd_allocate(&session->fixed_disk[0u]);
        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

lib_status x86_nxvm_machine_devices_bind_media(x86_nxvm_machine *session)
{
    lib_status status;

    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_media_registry_bind(session->media_registry,
        X86_NXVM_MACHINE_MEDIA_FDD_ID, session->floppy[0u], x86_nxvm_machine_fdd_media_provider());
    if (status != LIB_STATUS_OK) return status;
    if (session->construction.floppy_slot_count > 1u) {
        status = x86_nxvm_board_media_registry_bind(session->media_registry,
            X86_NXVM_MACHINE_MEDIA_FDD_SECONDARY_ID, session->floppy[1u],
            x86_nxvm_machine_fdd_media_provider());
        if (status != LIB_STATUS_OK) return status;
    }
    if (session->construction.hdc_present) {
        status = x86_nxvm_board_media_registry_bind(session->media_registry,
            X86_NXVM_MACHINE_MEDIA_HDD_ID, session->fixed_disk[0u], x86_nxvm_machine_hdd_media_provider());
        if (status != LIB_STATUS_OK) return status;
    }
    return x86_nxvm_board_media_registry_freeze(session->media_registry);
}

void x86_nxvm_machine_devices_reset(x86_nxvm_machine *session)
{
    if (session == LIB_NULL) return;
    x86_nxvm_machine_fdd_reset(session->floppy[0u]);
    if (session->construction.floppy_slot_count > 1u)
        x86_nxvm_machine_fdd_reset(session->floppy[1u]);
    if (session->construction.hdc_present) {
        x86_nxvm_machine_hdd_reset(session->fixed_disk[0u]);
    }
}

void x86_nxvm_machine_devices_finalize(x86_nxvm_machine *session)
{
    if (session == LIB_NULL) return;
    /* Empty slots also participate in partial-construction rollback. */
    for (lib_size slot = 0u; slot < X86_NXVM_MACHINE_FLOPPY_SLOT_COUNT; ++slot)
        x86_nxvm_machine_fdd_destroy(&session->floppy[slot]);
    for (lib_size slot = 0u; slot < X86_NXVM_MACHINE_FIXED_DISK_SLOT_COUNT; ++slot)
        x86_nxvm_machine_hdd_destroy(&session->fixed_disk[slot]);
}

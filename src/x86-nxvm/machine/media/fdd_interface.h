/* Copyright 2012-2014 Neko. */

#ifndef X86_NXVM_MACHINE_FDD_H
#define X86_NXVM_MACHINE_FDD_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "x86-nxvm/board-base/media_interface.h"
#include "lib/storage/medium_interface.h"

typedef struct t_fdd t_fdd;

#define X86_NXVM_MACHINE_DEVICE_FDD "Floppy Disk Drive"
#define VFDD_BYTE_PER_MB ((1 << 10) * 1000)

lib_size x86_nxvm_machine_fdd_image_size(const t_fdd *fdd);
lib_bool x86_nxvm_machine_fdd_has_media(const t_fdd *fdd);
/* Caller owns the opaque object; provider contexts borrow it until destruction.
 * Allocate into an empty slot. Failure leaves that slot unchanged.
 * Calls are serialized by the stopped/paused Machine owner. */
lib_status x86_nxvm_machine_fdd_allocate(const x86_nxvm_board_media_geometry *geometry,
    t_fdd **out_fdd);
void x86_nxvm_machine_fdd_destroy(t_fdd **fdd);
void x86_nxvm_machine_fdd_reset(t_fdd *fdd);
void x86_nxvm_machine_fdd_create_for(t_fdd *fdd);
lib_status x86_nxvm_machine_fdd_replace_bytes(t_fdd *fdd, const void *bytes,
    lib_size byte_count);
lib_status x86_nxvm_machine_fdd_insert_for(t_fdd *fdd, const char *file_name,
    lib_storage_medium_mode mode);
lib_status x86_nxvm_machine_fdd_remove_for(t_fdd *fdd);
const x86_nxvm_board_media_provider *x86_nxvm_machine_fdd_media_provider(void);

#ifdef __cplusplus
}
#endif

#endif

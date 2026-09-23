/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_FDD_H
#define VM_MACHINE_FDD_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "type.h"
#include "app-nxvm/devices/media_interface.h"
#include "lib/storage/medium_interface.h"

typedef struct t_fdd t_fdd;

#define VM_MACHINE_DEVICE_FDD "Floppy Disk Drive"
#define VFDD_BYTE_PER_MB ((1 << 10) * 1000)

lib_size vm_machine_fdd_image_size(const t_fdd *fdd);
C_INT vm_machine_fdd_has_media(const t_fdd *fdd);
C_INT vm_machine_fdd_chs_valid(const t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 bytes);
C_INT vm_machine_fdd_read_byte(const t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 offset,
    lib_u8 *out_byte);
C_INT vm_machine_fdd_write_byte(t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 offset,
    lib_u8 value);
C_INT vm_machine_fdd_format_sector(t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u8 fill_byte);
C_INT vm_machine_fdd_initialize_with_geometry(t_fdd *fdd,
    const core_machine_media_geometry *geometry);
C_VOID vm_machine_fdd_reset(t_fdd *fdd);
C_VOID vm_machine_fdd_finalize(t_fdd *fdd);
C_VOID vm_machine_fdd_create_for(t_fdd *fdd);
C_INT vm_machine_fdd_replace_bytes(t_fdd *fdd, const C_VOID *bytes,
    lib_size byte_count);
C_INT vm_machine_fdd_insert_for(t_fdd *fdd, const C_CHAR *file_name,
    lib_storage_medium_mode mode);
C_INT vm_machine_fdd_remove_for(t_fdd *fdd);
const core_machine_media_provider *vm_machine_fdd_media_provider(C_VOID);

#ifdef __cplusplus
}
#endif

#endif

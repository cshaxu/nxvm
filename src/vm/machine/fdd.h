/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_FDD_H
#define VM_MACHINE_FDD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/media_interface.h"

typedef struct t_fdd t_fdd;

#define VM_MACHINE_DEVICE_FDD "Floppy Disk Drive"
#define VFDD_BYTE_PER_MB ((1 << 10) * 1000)

STD_SIZE_T vm_machine_fdd_image_size(const t_fdd *fdd);
C_INT vm_machine_fdd_has_media(const t_fdd *fdd);
C_INT vm_machine_fdd_chs_valid(const t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_16 bytes);
C_INT vm_machine_fdd_read_byte(const t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_16 offset,
    type_unsigned_8 *out_byte);
C_INT vm_machine_fdd_write_byte(t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_16 offset,
    type_unsigned_8 value);
C_INT vm_machine_fdd_format_sector(t_fdd *fdd, type_unsigned_16 cylinder,
    type_unsigned_16 head, type_unsigned_16 sector, type_unsigned_8 fill_byte);
C_VOID vm_machine_fdd_initialize(t_fdd *fdd);
C_INT vm_machine_fdd_initialize_with_geometry(t_fdd *fdd,
    const core_machine_media_geometry *geometry);
C_VOID vm_machine_fdd_reset(t_fdd *fdd);
C_VOID vm_machine_fdd_finalize(t_fdd *fdd);
C_VOID vm_machine_fdd_create_for(t_fdd *fdd);
C_INT vm_machine_fdd_replace_bytes(t_fdd *fdd, const C_VOID *bytes,
    STD_SIZE_T byte_count);
C_INT vm_machine_fdd_insert_for(t_fdd *fdd, const C_CHAR *file_name);
C_INT vm_machine_fdd_remove_for(t_fdd *fdd, const C_CHAR *file_name);
C_VOID vm_machine_fdd_print(const t_fdd *fdd);
const core_machine_media_provider *vm_machine_fdd_media_provider(C_VOID);

#ifdef __cplusplus
}
#endif

#endif

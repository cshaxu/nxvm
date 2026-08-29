/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_FDD_PRIVATE_H
#define VM_MACHINE_FDD_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/media_interface.h"
#include "vm/machine/fdd.h"

typedef struct t_latch t_latch;


typedef struct {
    type_unsigned_16 cyl;     /* vfdc.C; cylinder id */
    type_unsigned_16 head;    /* vfdc.H; head id */
    type_unsigned_16 sector;  /* vfdc.R; sector id */
    type_unsigned_8  gpl;     /* vfdc.GPL; gap length of sector */
    type_unsigned_16 ncyl;    /* configured number of cylinders */
    type_unsigned_16 nhead;   /* configured number of heads */
    type_unsigned_16 nsector; /* configured sectors per track */
    type_unsigned_16 nbyte;   /* configured bytes per sector */
} t_fdd_data;

typedef struct {
    type_bool flagReadOnly;  /* write protect status */
    type_bool flagDiskExist; /* flag of floppy disk existance */

    type_virtual_address pImgBase;   /* pointer to disk in ram */
    type_virtual_address pAddressMarks; /* one Deleted-Data flag per logical sector */
    type_unsigned_32 media_generation; /* advances on every insert/remove/create */
} t_fdd_connect;

struct t_fdd {
    core_machine_media_geometry geometry;
    t_fdd_data data;
    t_fdd_connect connect;
};

STD_SIZE_T vm_machine_fdd_image_size(const t_fdd *fdd);
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
}/*_EOCD_*/
#endif

#endif

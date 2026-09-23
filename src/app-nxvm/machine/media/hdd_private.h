/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_HDD_PRIVATE_H
#define VM_MACHINE_HDD_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "type.h"
#include "app-nxvm/devices/media_interface.h"
#include "lib/storage/medium_interface.h"
#include "app-nxvm/machine/media/hdd.h"


typedef struct {
    lib_u16 cyl;     /* vfdc.C; cylinder id (0 to 79) */
    lib_u16 head;    /* vfdc.H; head id (0 or 1) */
    lib_u16 sector;  /* vfdc.R; sector id (1 to 18) */
    lib_u8  gpl;     /* vfdc.GPL; gap length of sector (default is 3) */
    lib_u32 ncyl;            /* compatibility CHS cylinders; LBA capacity is authoritative */
    lib_u16 nhead;   /* number of heads, should be 16 here */
    lib_u16 nsector; /* vfdc.EOT; should be 63 here */
    lib_u16 nbyte;   /* vfdc.N; bytes per sector (default is 512) */
} t_hdd_data;

typedef struct {
    type_bool flagReadOnly;  /* write protect status */
    type_bool flagDiskExist; /* flag of floppy disk existance */

    lib_storage_medium *medium;      /* sole owner of file or overlay bytes */
    lib_size raw_byte_count; /* exact bytes read from the backing image */
    lib_size virtual_byte_count; /* guest-visible rounded sector capacity */
    lib_u32 media_generation; /* advances on create, insert, remove, format */
    lib_u32 geometry_cylinders;
    lib_u16 geometry_heads;
    lib_u16 geometry_sectors_per_track;
} t_hdd_connect;

struct t_hdd {
    t_hdd_data data;
    t_hdd_connect connect;
};

lib_size vm_machine_hdd_image_size(const t_hdd *hdd);
C_VOID vm_machine_hdd_initialize(t_hdd *hdd);
C_VOID vm_machine_hdd_reset(t_hdd *hdd);
C_VOID vm_machine_hdd_finalize(t_hdd *hdd);
C_INT vm_machine_hdd_create(t_hdd *hdd, lib_u16 cylinders);
C_INT vm_machine_hdd_replace_bytes(t_hdd *hdd, const C_VOID *bytes,
    lib_size raw_byte_count);
C_INT vm_machine_hdd_insert(t_hdd *hdd, const C_CHAR *file_name,
    lib_storage_medium_mode mode);
C_INT vm_machine_hdd_set_geometry(t_hdd *hdd, lib_u32 cylinders,
    lib_u16 heads, lib_u16 sectors_per_track);
C_INT vm_machine_hdd_remove(t_hdd *hdd);
const core_machine_media_provider *vm_machine_hdd_media_provider(C_VOID);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif

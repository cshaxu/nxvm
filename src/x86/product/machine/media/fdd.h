/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_FDD_PRIVATE_H
#define VM_MACHINE_FDD_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "x86/ibmpc-common/media_interface.h"
#include "lib/storage/medium_interface.h"
#include "x86/product/machine/media/fdd_interface.h"

typedef struct {
    lib_u16 cyl;     /* vfdc.C; cylinder id */
    lib_u16 head;    /* vfdc.H; head id */
    lib_u16 sector;  /* vfdc.R; sector id */
    lib_u8  gpl;     /* vfdc.GPL; gap length of sector */
    lib_u16 ncyl;    /* configured number of cylinders */
    lib_u16 nhead;   /* configured number of heads */
    lib_u16 nsector; /* configured sectors per track */
    lib_u16 nbyte;   /* configured bytes per sector */
} t_fdd_data;

typedef struct {
    lib_u8 flagReadOnly;  /* write protect status */
    lib_u8 flagDiskExist; /* flag of floppy disk existance */

    lib_storage_medium *medium;      /* sole owner of file or overlay bytes */
    lib_uptr pAddressMarks; /* one Deleted-Data flag per logical sector */
    lib_u32 media_generation; /* advances on every insert/remove/create */
} t_fdd_connect;

struct t_fdd {
    core_machine_media_geometry geometry;
    t_fdd_data data;
    t_fdd_connect connect;
};

lib_i32 vm_machine_fdd_initialize_with_geometry(t_fdd *fdd,
    const core_machine_media_geometry *geometry);
void vm_machine_fdd_finalize(t_fdd *fdd);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif

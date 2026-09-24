/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_FDD_PRIVATE_H
#define VM_MACHINE_FDD_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "app-nxvm/devices/media_interface.h"
#include "lib/storage/medium_interface.h"
#include "app-nxvm/machine/media/fdd.h"

typedef struct t_latch t_latch;


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

lib_size vm_machine_fdd_image_size(const t_fdd *fdd);
lib_i32 vm_machine_fdd_chs_valid(const t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 bytes);
lib_i32 vm_machine_fdd_read_byte(const t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 offset,
    lib_u8 *out_byte);
lib_i32 vm_machine_fdd_write_byte(t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u16 offset,
    lib_u8 value);
lib_i32 vm_machine_fdd_format_sector(t_fdd *fdd, lib_u16 cylinder,
    lib_u16 head, lib_u16 sector, lib_u8 fill_byte);
lib_i32 vm_machine_fdd_initialize_with_geometry(t_fdd *fdd,
    const core_machine_media_geometry *geometry);
void vm_machine_fdd_reset(t_fdd *fdd);
void vm_machine_fdd_finalize(t_fdd *fdd);
void vm_machine_fdd_create_for(t_fdd *fdd);
lib_i32 vm_machine_fdd_replace_bytes(t_fdd *fdd, const void *bytes,
    lib_size byte_count);
lib_i32 vm_machine_fdd_insert_for(t_fdd *fdd, const char *file_name,
    lib_storage_medium_mode mode);
lib_i32 vm_machine_fdd_remove_for(t_fdd *fdd);

const core_machine_media_provider *vm_machine_fdd_media_provider(void);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif

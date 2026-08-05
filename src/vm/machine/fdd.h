/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_FDD_H
#define VM_MACHINE_FDD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

typedef struct t_latch t_latch;

#define VM_MACHINE_DEVICE_FDD "3.5\" Floppy Disk Drive"

typedef struct {
    type_unsigned_16 cyl;     /* vfdc.C; cylinder id (0 to 79) */
    type_unsigned_16 head;    /* vfdc.H; head id (0 or 1) */
    type_unsigned_16 sector;  /* vfdc.R; sector id (1 to 18) */
    type_unsigned_8  gpl;     /* vfdc.GPL; gap length of sector (default is 3) */
    type_unsigned_16 ncyl;    /* number of cylinders */
    type_unsigned_16 nhead;   /* number of heads */
    type_unsigned_16 nsector; /* vfdc.EOT; end sector id (default is 18) */
    type_unsigned_16 nbyte;   /* vfdc.N; bytes per sector (default is 512) */
} t_fdd_data;

typedef struct {
    type_bool flagReadOnly;  /* write protect status */
    type_bool flagDiskExist; /* flag of floppy disk existance */

    type_virtual_address pImgBase;   /* pointer to disk in ram */
    uint32_t media_generation; /* advances on every insert/remove/create */
} t_fdd_connect;

typedef struct {
    t_fdd_data data;
    t_fdd_connect connect;
} t_fdd;

#define VFDD_BYTE_PER_MB ((1 << 10) * 1000)

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
C_VOID vm_machine_fdd_reset(t_fdd *fdd);
C_VOID vm_machine_fdd_refresh(t_fdd *fdd);
C_VOID vm_machine_fdd_finalize(t_fdd *fdd);
C_VOID vm_machine_fdd_create_for(t_fdd *fdd);
C_INT vm_machine_fdd_insert_for(t_fdd *fdd, const C_CHAR *file_name);
C_INT vm_machine_fdd_remove_for(t_fdd *fdd, const C_CHAR *file_name);

C_VOID vm_machine_fdd_print(const t_fdd *fdd);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif

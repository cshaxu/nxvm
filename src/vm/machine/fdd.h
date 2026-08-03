/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VM_MACHINE_FDD_H
#define NXVM_VM_MACHINE_FDD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

typedef struct t_latch t_latch;

#define NXVM_DEVICE_FDD "3.5\" Floppy Disk Drive"

typedef struct {
    ntvdm64_type_unsigned_16 cyl;     /* vfdc.C; cylinder id (0 to 79) */
    ntvdm64_type_unsigned_16 head;    /* vfdc.H; head id (0 or 1) */
    ntvdm64_type_unsigned_16 sector;  /* vfdc.R; sector id (1 to 18) */
    ntvdm64_type_unsigned_8  gpl;     /* vfdc.GPL; gap length of sector (default is 3) */
    ntvdm64_type_unsigned_16 ncyl;    /* number of cylinders */
    ntvdm64_type_unsigned_16 nhead;   /* number of heads */
    ntvdm64_type_unsigned_16 nsector; /* vfdc.EOT; end sector id (default is 18) */
    ntvdm64_type_unsigned_16 nbyte;   /* vfdc.N; bytes per sector (default is 512) */
} t_fdd_data;

typedef struct {
    ntvdm64_type_bool flagReadOnly;  /* write protect status */
    ntvdm64_type_bool flagDiskExist; /* flag of floppy disk existance */

    ntvdm64_type_virtual_address pImgBase;   /* pointer to disk in ram */
    ntvdm64_type_virtual_address pCurrByte;  /* pointer to current byte */
    ntvdm64_type_unsigned_16 transCount; /* number of transfer bytes */
} t_fdd_connect;

typedef struct {
    t_fdd_data data;
    t_fdd_connect connect;
} t_fdd;

#define VFDD_BYTE_PER_MB ((1 << 10) * 1000)

STD_SIZE_T vm_machine_fdd_image_size(const t_fdd *fdd);
C_VOID vm_machine_fdd_set_pointer(t_fdd *fdd);
C_VOID vm_machine_fdd_transfer_read(t_fdd *fdd, t_latch *latch);
C_VOID vm_machine_fdd_transfer_write(t_fdd *fdd, t_latch *latch);
C_VOID vm_machine_fdd_format_track(t_fdd *fdd, ntvdm64_type_unsigned_8 fill_byte);
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

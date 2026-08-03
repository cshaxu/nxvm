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
    t_nubit16 cyl;     /* vfdc.C; cylinder id (0 to 79) */
    t_nubit16 head;    /* vfdc.H; head id (0 or 1) */
    t_nubit16 sector;  /* vfdc.R; sector id (1 to 18) */
    t_nubit8  gpl;     /* vfdc.GPL; gap length of sector (default is 3) */
    t_nubit16 ncyl;    /* number of cylinders */
    t_nubit16 nhead;   /* number of heads */
    t_nubit16 nsector; /* vfdc.EOT; end sector id (default is 18) */
    t_nubit16 nbyte;   /* vfdc.N; bytes per sector (default is 512) */
} t_fdd_data;

typedef struct {
    t_bool flagReadOnly;  /* write protect status */
    t_bool flagDiskExist; /* flag of floppy disk existance */

    t_vaddrcc pImgBase;   /* pointer to disk in ram */
    t_vaddrcc pCurrByte;  /* pointer to current byte */
    t_nubit16 transCount; /* number of transfer bytes */
} t_fdd_connect;

typedef struct {
    t_fdd_data data;
    t_fdd_connect connect;
} t_fdd;

#define VFDD_BYTE_PER_MB ((1 << 10) * 1000)

size_t vm_machine_fdd_image_size(const t_fdd *fdd);
void vm_machine_fdd_set_pointer(t_fdd *fdd);
void vm_machine_fdd_transfer_read(t_fdd *fdd, t_latch *latch);
void vm_machine_fdd_transfer_write(t_fdd *fdd, t_latch *latch);
void vm_machine_fdd_format_track(t_fdd *fdd, t_nubit8 fill_byte);
void vm_machine_fdd_initialize(t_fdd *fdd);
void vm_machine_fdd_reset(t_fdd *fdd);
void vm_machine_fdd_refresh(t_fdd *fdd);
void vm_machine_fdd_finalize(t_fdd *fdd);
void vm_machine_fdd_create_for(t_fdd *fdd);
int vm_machine_fdd_insert_for(t_fdd *fdd, const char *file_name);
int vm_machine_fdd_remove_for(t_fdd *fdd, const char *file_name);

void vm_machine_fdd_print(const t_fdd *fdd);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif

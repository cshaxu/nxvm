/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VM_MACHINE_HDD_H
#define NXVM_VM_MACHINE_HDD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

typedef struct t_latch t_latch;

#define NXVM_DEVICE_HDD "Unknown Hard Disk Drive"

typedef struct {
    ntvdm64_type_unsigned_16 cyl;     /* vfdc.C; cylinder id (0 to 79) */
    ntvdm64_type_unsigned_16 head;    /* vfdc.H; head id (0 or 1) */
    ntvdm64_type_unsigned_16 sector;  /* vfdc.R; sector id (1 to 18) */
    ntvdm64_type_unsigned_8  gpl;     /* vfdc.GPL; gap length of sector (default is 3) */
    ntvdm64_type_unsigned_16 ncyl;    /* number of cylinders, should be 20 here */
    ntvdm64_type_unsigned_16 nhead;   /* number of heads, should be 16 here */
    ntvdm64_type_unsigned_16 nsector; /* vfdc.EOT; should be 63 here */
    ntvdm64_type_unsigned_16 nbyte;   /* vfdc.N; bytes per sector (default is 512) */
} t_hdd_data;

typedef struct {
    ntvdm64_type_bool flagReadOnly;  /* write protect status */
    ntvdm64_type_bool flagDiskExist; /* flag of floppy disk existance */

    ntvdm64_type_virtual_address pImgBase;   /* pointer to disk in ram */
    ntvdm64_type_virtual_address pCurrByte;  /* pointer to current byte */
    ntvdm64_type_unsigned_16 transCount; /* number of transfer bytes */
} t_hdd_connect;

typedef struct {
    t_hdd_data data;
    t_hdd_connect connect;
} t_hdd;

#define VHDD_BYTE_PER_MB (1 << 20)

size_t vm_machine_hdd_image_size(const t_hdd *hdd);
void vm_machine_hdd_set_pointer(t_hdd *hdd);
void vm_machine_hdd_transfer_read(t_hdd *hdd, t_latch *latch);
void vm_machine_hdd_transfer_write(t_hdd *hdd, t_latch *latch);
void vm_machine_hdd_format_track(t_hdd *hdd, ntvdm64_type_unsigned_8 fill_byte);
void vm_machine_hdd_initialize(t_hdd *hdd);
void vm_machine_hdd_reset(t_hdd *hdd);
void vm_machine_hdd_refresh(t_hdd *hdd);
void vm_machine_hdd_finalize(t_hdd *hdd);
void vm_machine_hdd_create(t_hdd *hdd, uint16_t cylinders);
int vm_machine_hdd_insert(t_hdd *hdd, const char *file_name);
int vm_machine_hdd_remove(t_hdd *hdd, const char *file_name);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif

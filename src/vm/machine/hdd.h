/* Copyright 2012-2014 Neko. */

#ifndef VM_MACHINE_HDD_H
#define VM_MACHINE_HDD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/media_interface.h"

#define VM_MACHINE_DEVICE_HDD "Unknown Hard Disk Drive"

typedef struct {
    type_unsigned_16 cyl;     /* vfdc.C; cylinder id (0 to 79) */
    type_unsigned_16 head;    /* vfdc.H; head id (0 or 1) */
    type_unsigned_16 sector;  /* vfdc.R; sector id (1 to 18) */
    type_unsigned_8  gpl;     /* vfdc.GPL; gap length of sector (default is 3) */
    uint32_t ncyl;            /* compatibility CHS cylinders; LBA capacity is authoritative */
    type_unsigned_16 nhead;   /* number of heads, should be 16 here */
    type_unsigned_16 nsector; /* vfdc.EOT; should be 63 here */
    type_unsigned_16 nbyte;   /* vfdc.N; bytes per sector (default is 512) */
} t_hdd_data;

typedef struct {
    type_bool flagReadOnly;  /* write protect status */
    type_bool flagDiskExist; /* flag of floppy disk existance */

    type_virtual_address pImgBase;   /* pointer to disk in ram */
    STD_SIZE_T raw_byte_count; /* exact bytes read from the backing image */
    STD_SIZE_T virtual_byte_count; /* guest-visible rounded sector capacity */
    type_bool flagPaddingWritten; /* persistence must materialize tail padding */
    uint32_t media_generation; /* advances on create, insert, remove, format */
} t_hdd_connect;

typedef struct {
    t_hdd_data data;
    t_hdd_connect connect;
} t_hdd;

#define VHDD_BYTE_PER_MB (1 << 20)

STD_SIZE_T vm_machine_hdd_image_size(const t_hdd *hdd);
C_VOID vm_machine_hdd_initialize(t_hdd *hdd);
C_VOID vm_machine_hdd_reset(t_hdd *hdd);
C_VOID vm_machine_hdd_refresh(t_hdd *hdd);
C_VOID vm_machine_hdd_finalize(t_hdd *hdd);
C_VOID vm_machine_hdd_create(t_hdd *hdd, uint16_t cylinders);
C_INT vm_machine_hdd_replace_bytes(t_hdd *hdd, const C_VOID *bytes,
    STD_SIZE_T raw_byte_count);
C_INT vm_machine_hdd_insert(t_hdd *hdd, const C_CHAR *file_name);
C_INT vm_machine_hdd_remove(t_hdd *hdd, const C_CHAR *file_name);
const core_machine_media_provider *vm_machine_hdd_media_provider(C_VOID);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif

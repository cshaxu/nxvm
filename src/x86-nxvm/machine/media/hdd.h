/* Copyright 2012-2014 Neko. */

#ifndef X86_NXVM_MACHINE_HDD_PRIVATE_H
#define X86_NXVM_MACHINE_HDD_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "x86-nxvm/board-base/media_interface.h"
#include "lib/storage/medium_interface.h"
#include "x86-nxvm/machine/media/hdd_interface.h"


typedef struct {
    lib_u32 ncyl;            /* compatibility CHS cylinders; LBA capacity is authoritative */
    lib_u16 nhead;   /* heads per cylinder */
    lib_u16 nsector; /* sectors per track */
    lib_u16 nbyte;   /* bytes per sector */
} t_hdd_data;

typedef struct x86_nxvm_machine_hdd_ecc_record {
    lib_u64 sector;
    lib_u8 ecc[4];
    struct x86_nxvm_machine_hdd_ecc_record *next;
} x86_nxvm_machine_hdd_ecc_record;

typedef struct x86_nxvm_machine_hdd_track {
    x86_nxvm_board_media_track format;
    struct x86_nxvm_machine_hdd_track *next;
} x86_nxvm_machine_hdd_track;

typedef struct {
    lib_bool flagReadOnly;  /* write protect status */
    lib_bool flagDiskExist; /* medium presence */

    lib_storage_medium *medium;      /* sole owner of file or overlay bytes */
    x86_nxvm_machine_hdd_ecc_record *records; /* supplied ECC, installed-lease lifetime */
    x86_nxvm_machine_hdd_track *tracks; /* formatted IDs and bad flags, same lease */
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

void x86_nxvm_machine_hdd_initialize(t_hdd *hdd);
void x86_nxvm_machine_hdd_finalize(t_hdd *hdd);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif

#ifndef X86_NXVM_PRODUCT_REQUEST_INTERFACE_H
#define X86_NXVM_PRODUCT_REQUEST_INTERFACE_H

#include "lib/types/types_interface.h"
#include "lib/storage/medium_interface.h"

/* Immutable copied runtime request. The fixed App binding supplies machine
 * facts; this NXVM-family request contains only user-selectable values. */
#define X86_NXVM_PRODUCT_REQUEST_PATH_MAX 1024u
#define X86_NXVM_PRODUCT_REQUEST_MEDIA_SLOT_COUNT 2u

typedef struct x86_nxvm_product_request {
    lib_u8 file_name[X86_NXVM_PRODUCT_REQUEST_PATH_MAX];
    lib_u8 display[16];
    lib_i32 console_control;
    lib_u8 floppy[X86_NXVM_PRODUCT_REQUEST_MEDIA_SLOT_COUNT][X86_NXVM_PRODUCT_REQUEST_PATH_MAX];
    lib_u8 fixed_disk[X86_NXVM_PRODUCT_REQUEST_MEDIA_SLOT_COUNT][X86_NXVM_PRODUCT_REQUEST_PATH_MAX];
    lib_storage_medium_mode floppy_mode[X86_NXVM_PRODUCT_REQUEST_MEDIA_SLOT_COUNT];
    lib_storage_medium_mode fixed_disk_mode[X86_NXVM_PRODUCT_REQUEST_MEDIA_SLOT_COUNT];
    lib_size floppy_count;
    lib_size fixed_disk_count;
    lib_size memory_bytes;
} x86_nxvm_product_request;

#endif

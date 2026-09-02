#ifndef VM_PRODUCT_SESSION_CATALOG_H
#define VM_PRODUCT_SESSION_CATALOG_H

#include "type.h"

#define VM_PRODUCT_SESSION_CATALOG_MAX 64u
#define VM_PRODUCT_SESSION_CATALOG_PATH_MAX 1024u
#define VM_PRODUCT_SESSION_MEDIA_SLOT_COUNT 2u
#define VM_PRODUCT_SESSION_BIOS_SLOT_COUNT 2u

typedef struct vm_product_session_request {
    C_CHAR file_name[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR profile[64];
    C_CHAR cpu[32];
    C_CHAR fpu[32];
    C_CHAR display[16];
    C_CHAR boot[16];
    C_CHAR floppy_format[16];
    C_CHAR floppy[VM_PRODUCT_SESSION_MEDIA_SLOT_COUNT][VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR fixed_disk[VM_PRODUCT_SESSION_MEDIA_SLOT_COUNT][VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    STD_SIZE_T floppy_count;
    STD_SIZE_T fixed_disk_count;
    C_CHAR bios[VM_PRODUCT_SESSION_BIOS_SLOT_COUNT][VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    STD_SIZE_T bios_count;
    C_CHAR video[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR cmos[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR font[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    STD_SIZE_T memory_bytes;
} vm_product_session_request;

typedef struct vm_product_session_catalog vm_product_session_catalog;

type_status vm_product_session_catalog_create(const C_CHAR *directory,
    vm_product_session_catalog **out_catalog);
C_VOID vm_product_session_catalog_destroy(vm_product_session_catalog *catalog);
STD_SIZE_T vm_product_session_catalog_count(const vm_product_session_catalog *catalog);
STD_SIZE_T vm_product_session_catalog_rejected(const vm_product_session_catalog *catalog);
type_status vm_product_session_catalog_get_request(const vm_product_session_catalog *catalog,
    STD_SIZE_T index, vm_product_session_request *out_request);

#endif

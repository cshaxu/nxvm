#ifndef VM_PRODUCT_SESSION_CATALOG_H
#define VM_PRODUCT_SESSION_CATALOG_H

#include "type.h"

#define VM_PRODUCT_SESSION_CATALOG_MAX 64u
#define VM_PRODUCT_SESSION_CATALOG_PATH_MAX 1024u

typedef struct vm_product_session_catalog_entry {
    C_CHAR file_name[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR profile[64];
    C_CHAR cpu[32];
    C_CHAR fpu[32];
    C_CHAR display[16];
    C_CHAR boot[16];
    C_CHAR floppy[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR hard_disk[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    STD_SIZE_T memory_bytes;
} vm_product_session_catalog_entry;

typedef struct vm_product_session_catalog {
    vm_product_session_catalog_entry entries[VM_PRODUCT_SESSION_CATALOG_MAX];
    STD_SIZE_T count;
    STD_SIZE_T rejected;
} vm_product_session_catalog;

C_VOID vm_product_session_catalog_initialize(vm_product_session_catalog *catalog,
    const C_CHAR *directory);
const vm_product_session_catalog_entry *vm_product_session_catalog_get(
    const vm_product_session_catalog *catalog, STD_SIZE_T index);

#endif

#ifndef VM_PRODUCT_SESSION_CATALOG_H
#define VM_PRODUCT_SESSION_CATALOG_H

#include "type.h"

#define VM_PRODUCT_SESSION_CATALOG_MAX 64u
#define VM_PRODUCT_SESSION_CATALOG_PATH_MAX 1024u

typedef struct vm_product_session_request {
    C_CHAR file_name[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR profile[64];
    C_CHAR cpu[32];
    C_CHAR fpu[32];
    C_CHAR display[16];
    C_CHAR boot[16];
    C_CHAR floppy[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR hard_disk[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR model40_even_path[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR model40_even_sha256[65];
    C_CHAR model40_odd_path[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR model40_odd_sha256[65];
    C_CHAR model40_provenance[256];
    C_CHAR xt_system_path[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR xt_system_sha256[65];
    C_CHAR xt_xebec_path[VM_PRODUCT_SESSION_CATALOG_PATH_MAX];
    C_CHAR xt_xebec_sha256[65];
    C_CHAR xt_provenance[256];
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

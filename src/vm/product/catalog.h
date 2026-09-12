#ifndef VM_PRODUCT_SESSION_CATALOG_H
#define VM_PRODUCT_SESSION_CATALOG_H

#include "type.h"
#include "vm/machine/request_interface.h"

#define VM_PRODUCT_SESSION_CATALOG_MAX 64u

typedef struct vm_product_session_catalog vm_product_session_catalog;

/* Parses one mutable YAML document without opening a host file.  The caller
 * supplies the source directory/name solely for relative asset resolution. */
type_status vm_session_request_parse(const C_CHAR *directory,
    const C_CHAR *name, C_CHAR *document, vm_session_request *out_request);
type_status vm_product_session_catalog_create(const C_CHAR *directory,
    vm_product_session_catalog **out_catalog);
C_VOID vm_product_session_catalog_destroy(vm_product_session_catalog *catalog);
STD_SIZE_T vm_product_session_catalog_count(const vm_product_session_catalog *catalog);
STD_SIZE_T vm_product_session_catalog_rejected(const vm_product_session_catalog *catalog);
type_status vm_product_session_catalog_get_request(const vm_product_session_catalog *catalog,
    STD_SIZE_T index, vm_session_request *out_request);

#endif

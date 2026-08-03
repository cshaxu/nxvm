#ifndef VM_PRODUCT_MEDIA_H
#define VM_PRODUCT_MEDIA_H

#include <stdint.h>

#include "type.h"

#define VM_PRODUCT_MEDIA_PATH_CAPACITY 512u
#define VM_PRODUCT_MEDIA_NAME_CAPACITY 32u
#define VM_PRODUCT_MEDIA_SHA256_CAPACITY 65u

typedef enum vm_product_boot_target {
    VM_PRODUCT_BOOT_FDD = 1,
    VM_PRODUCT_BOOT_HDD = 2
} vm_product_boot_target;

typedef struct vm_product_media_identity {
    const C_CHAR *logical_name;
    uint64_t expected_bytes;
    const C_CHAR *expected_sha256;
} vm_product_media_identity;

typedef struct vm_product_block_provider {
    C_CHAR path[VM_PRODUCT_MEDIA_PATH_CAPACITY];
    C_CHAR logical_name[VM_PRODUCT_MEDIA_NAME_CAPACITY];
    C_CHAR expected_sha256[VM_PRODUCT_MEDIA_SHA256_CAPACITY];
    uint64_t expected_bytes;
    C_INT configured;
    C_INT created;
    uint16_t cylinders;
} vm_product_block_provider;

typedef struct vm_product_media_policy {
    vm_product_block_provider fdd;
    vm_product_block_provider hdd;
    vm_product_boot_target boot_target;
    C_INT frozen;
} vm_product_media_policy;

C_VOID vm_product_media_policy_initialize(
    vm_product_media_policy *policy);
ntvdm64_status vm_product_media_configure(
    vm_product_media_policy *policy,
    vm_product_boot_target target,
    const C_CHAR *path,
    const vm_product_media_identity *identity);
ntvdm64_status vm_product_media_configure_created(
    vm_product_media_policy *policy,
    vm_product_boot_target target,
    uint16_t cylinders);
ntvdm64_status vm_product_media_set_boot_target(
    vm_product_media_policy *policy,
    vm_product_boot_target target);
ntvdm64_status vm_product_media_freeze(
    vm_product_media_policy *policy);
const vm_product_block_provider *vm_product_media_provider(
    const vm_product_media_policy *policy,
    vm_product_boot_target target);

#endif

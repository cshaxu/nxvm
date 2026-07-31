#ifndef NXVM_PRODUCT_NXVM_MEDIA_H
#define NXVM_PRODUCT_NXVM_MEDIA_H

#include <stdint.h>

#include "core/status.h"

#define NXVM_PRODUCT_NXVM_MEDIA_PATH_CAPACITY 512u
#define NXVM_PRODUCT_NXVM_MEDIA_NAME_CAPACITY 32u
#define NXVM_PRODUCT_NXVM_MEDIA_SHA256_CAPACITY 65u

typedef enum nxvm_product_nxvm_boot_target {
    NXVM_PRODUCT_NXVM_BOOT_FDD = 1,
    NXVM_PRODUCT_NXVM_BOOT_HDD = 2
} nxvm_product_nxvm_boot_target;

typedef struct nxvm_product_nxvm_media_identity {
    const char *logical_name;
    uint64_t expected_bytes;
    const char *expected_sha256;
} nxvm_product_nxvm_media_identity;

typedef struct nxvm_product_nxvm_block_provider {
    char path[NXVM_PRODUCT_NXVM_MEDIA_PATH_CAPACITY];
    char logical_name[NXVM_PRODUCT_NXVM_MEDIA_NAME_CAPACITY];
    char expected_sha256[NXVM_PRODUCT_NXVM_MEDIA_SHA256_CAPACITY];
    uint64_t expected_bytes;
    int configured;
    int created;
    uint16_t cylinders;
} nxvm_product_nxvm_block_provider;

typedef struct nxvm_product_nxvm_media_policy {
    nxvm_product_nxvm_block_provider fdd;
    nxvm_product_nxvm_block_provider hdd;
    nxvm_product_nxvm_boot_target boot_target;
    int frozen;
} nxvm_product_nxvm_media_policy;

void nxvm_product_nxvm_media_policy_initialize(
    nxvm_product_nxvm_media_policy *policy);
nxvm_core_status nxvm_product_nxvm_media_configure(
    nxvm_product_nxvm_media_policy *policy,
    nxvm_product_nxvm_boot_target target,
    const char *path,
    const nxvm_product_nxvm_media_identity *identity);
nxvm_core_status nxvm_product_nxvm_media_configure_created(
    nxvm_product_nxvm_media_policy *policy,
    nxvm_product_nxvm_boot_target target,
    uint16_t cylinders);
nxvm_core_status nxvm_product_nxvm_media_set_boot_target(
    nxvm_product_nxvm_media_policy *policy,
    nxvm_product_nxvm_boot_target target);
nxvm_core_status nxvm_product_nxvm_media_freeze(
    nxvm_product_nxvm_media_policy *policy);
const nxvm_product_nxvm_block_provider *nxvm_product_nxvm_media_provider(
    const nxvm_product_nxvm_media_policy *policy,
    nxvm_product_nxvm_boot_target target);

#endif

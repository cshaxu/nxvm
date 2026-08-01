#include "vm/product/media.h"

#include <string.h>

static int nxvm_product_nxvm_media_valid_target(
    nxvm_product_nxvm_boot_target target)
{
    return target == NXVM_PRODUCT_NXVM_BOOT_FDD ||
           target == NXVM_PRODUCT_NXVM_BOOT_HDD;
}

static nxvm_product_nxvm_block_provider *nxvm_product_nxvm_media_mutable_provider(
    nxvm_product_nxvm_media_policy *policy, nxvm_product_nxvm_boot_target target)
{
    if (target == NXVM_PRODUCT_NXVM_BOOT_FDD) return &policy->fdd;
    if (target == NXVM_PRODUCT_NXVM_BOOT_HDD) return &policy->hdd;
    return NULL;
}

static int nxvm_product_nxvm_media_copy(char *destination, size_t capacity,
                                        const char *source)
{
    size_t length;

    if (source == NULL || source[0] == '\0') return 0;
    length = strlen(source);
    if (length >= capacity) return 0;
    memcpy(destination, source, length + 1u);
    return 1;
}

void nxvm_product_nxvm_media_policy_initialize(
    nxvm_product_nxvm_media_policy *policy)
{
    if (policy != NULL) memset(policy, 0, sizeof(*policy));
}

nxvm_core_status nxvm_product_nxvm_media_configure(
    nxvm_product_nxvm_media_policy *policy,
    nxvm_product_nxvm_boot_target target,
    const char *path,
    const nxvm_product_nxvm_media_identity *identity)
{
    nxvm_product_nxvm_block_provider *provider;

    if (policy == NULL || identity == NULL || !nxvm_product_nxvm_media_valid_target(target) ||
        identity->expected_bytes == 0u || policy->frozen) {
        return policy != NULL && policy->frozen ? NXVM_CORE_STATUS_INVALID_STATE :
                                                  NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    provider = nxvm_product_nxvm_media_mutable_provider(policy, target);
    memset(provider, 0, sizeof(*provider));
    if (!nxvm_product_nxvm_media_copy(provider->path, sizeof(provider->path), path) ||
        !nxvm_product_nxvm_media_copy(provider->logical_name,
                                      sizeof(provider->logical_name), identity->logical_name) ||
        !nxvm_product_nxvm_media_copy(provider->expected_sha256,
                                      sizeof(provider->expected_sha256), identity->expected_sha256)) {
        memset(provider, 0, sizeof(*provider));
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    provider->expected_bytes = identity->expected_bytes;
    provider->configured = 1;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_media_configure_created(
    nxvm_product_nxvm_media_policy *policy,
    nxvm_product_nxvm_boot_target target,
    uint16_t cylinders)
{
    nxvm_product_nxvm_block_provider *provider;

    if (policy == NULL || !nxvm_product_nxvm_media_valid_target(target) ||
        policy->frozen ||
        (target == NXVM_PRODUCT_NXVM_BOOT_HDD && cylinders == 0u)) {
        return policy != NULL && policy->frozen ? NXVM_CORE_STATUS_INVALID_STATE :
                                                  NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    provider = nxvm_product_nxvm_media_mutable_provider(policy, target);
    memset(provider, 0, sizeof(*provider));
    provider->configured = 1;
    provider->created = 1;
    provider->cylinders = cylinders;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_media_set_boot_target(
    nxvm_product_nxvm_media_policy *policy,
    nxvm_product_nxvm_boot_target target)
{
    if (policy == NULL || !nxvm_product_nxvm_media_valid_target(target)) {
        return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    }
    if (policy->frozen) return NXVM_CORE_STATUS_INVALID_STATE;
    policy->boot_target = target;
    return NXVM_CORE_STATUS_OK;
}

nxvm_core_status nxvm_product_nxvm_media_freeze(
    nxvm_product_nxvm_media_policy *policy)
{
    const nxvm_product_nxvm_block_provider *provider;

    if (policy == NULL) return NXVM_CORE_STATUS_INVALID_ARGUMENT;
    if (policy->frozen) return NXVM_CORE_STATUS_INVALID_STATE;
    provider = nxvm_product_nxvm_media_provider(policy, policy->boot_target);
    if (provider == NULL || !provider->configured) return NXVM_CORE_STATUS_INVALID_STATE;
    policy->frozen = 1;
    return NXVM_CORE_STATUS_OK;
}

const nxvm_product_nxvm_block_provider *nxvm_product_nxvm_media_provider(
    const nxvm_product_nxvm_media_policy *policy,
    nxvm_product_nxvm_boot_target target)
{
    if (policy == NULL) return NULL;
    if (target == NXVM_PRODUCT_NXVM_BOOT_FDD) return &policy->fdd;
    if (target == NXVM_PRODUCT_NXVM_BOOT_HDD) return &policy->hdd;
    return NULL;
}

#include "vm/product/media.h"

#include <string.h>

static int vm_product_media_valid_target(
    vm_product_boot_target target)
{
    return target == VM_PRODUCT_BOOT_FDD ||
           target == VM_PRODUCT_BOOT_HDD;
}

static vm_product_block_provider *vm_product_media_mutable_provider(
    vm_product_media_policy *policy, vm_product_boot_target target)
{
    if (target == VM_PRODUCT_BOOT_FDD) return &policy->fdd;
    if (target == VM_PRODUCT_BOOT_HDD) return &policy->hdd;
    return NULL;
}

static int vm_product_media_copy(char *destination, size_t capacity,
                                        const char *source)
{
    size_t length;

    if (source == NULL || source[0] == '\0') return 0;
    length = STD_STRLEN(source);
    if (length >= capacity) return 0;
    STD_MEMCPY(destination, source, length + 1u);
    return 1;
}

void vm_product_media_policy_initialize(
    vm_product_media_policy *policy)
{
    if (policy != NULL) STD_MEMSET(policy, 0, sizeof(*policy));
}

ntvdm64_status vm_product_media_configure(
    vm_product_media_policy *policy,
    vm_product_boot_target target,
    const char *path,
    const vm_product_media_identity *identity)
{
    vm_product_block_provider *provider;

    if (policy == NULL || identity == NULL || !vm_product_media_valid_target(target) ||
        identity->expected_bytes == 0u || policy->frozen) {
        return policy != NULL && policy->frozen ? NTVDM64_STATUS_INVALID_STATE :
                                                  NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    provider = vm_product_media_mutable_provider(policy, target);
    STD_MEMSET(provider, 0, sizeof(*provider));
    if (!vm_product_media_copy(provider->path, sizeof(provider->path), path) ||
        !vm_product_media_copy(provider->logical_name,
                                      sizeof(provider->logical_name), identity->logical_name) ||
        !vm_product_media_copy(provider->expected_sha256,
                                      sizeof(provider->expected_sha256), identity->expected_sha256)) {
        STD_MEMSET(provider, 0, sizeof(*provider));
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    provider->expected_bytes = identity->expected_bytes;
    provider->configured = 1;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_product_media_configure_created(
    vm_product_media_policy *policy,
    vm_product_boot_target target,
    uint16_t cylinders)
{
    vm_product_block_provider *provider;

    if (policy == NULL || !vm_product_media_valid_target(target) ||
        policy->frozen ||
        (target == VM_PRODUCT_BOOT_HDD && cylinders == 0u)) {
        return policy != NULL && policy->frozen ? NTVDM64_STATUS_INVALID_STATE :
                                                  NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    provider = vm_product_media_mutable_provider(policy, target);
    STD_MEMSET(provider, 0, sizeof(*provider));
    provider->configured = 1;
    provider->created = 1;
    provider->cylinders = cylinders;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_product_media_set_boot_target(
    vm_product_media_policy *policy,
    vm_product_boot_target target)
{
    if (policy == NULL || !vm_product_media_valid_target(target)) {
        return NTVDM64_STATUS_INVALID_ARGUMENT;
    }
    if (policy->frozen) return NTVDM64_STATUS_INVALID_STATE;
    policy->boot_target = target;
    return NTVDM64_STATUS_OK;
}

ntvdm64_status vm_product_media_freeze(
    vm_product_media_policy *policy)
{
    const vm_product_block_provider *provider;

    if (policy == NULL) return NTVDM64_STATUS_INVALID_ARGUMENT;
    if (policy->frozen) return NTVDM64_STATUS_INVALID_STATE;
    provider = vm_product_media_provider(policy, policy->boot_target);
    if (provider == NULL || !provider->configured) return NTVDM64_STATUS_INVALID_STATE;
    policy->frozen = 1;
    return NTVDM64_STATUS_OK;
}

const vm_product_block_provider *vm_product_media_provider(
    const vm_product_media_policy *policy,
    vm_product_boot_target target)
{
    if (policy == NULL) return NULL;
    if (target == VM_PRODUCT_BOOT_FDD) return &policy->fdd;
    if (target == VM_PRODUCT_BOOT_HDD) return &policy->hdd;
    return NULL;
}

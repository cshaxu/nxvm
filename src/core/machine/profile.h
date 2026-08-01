#ifndef NXVM_CORE_PROFILE_H
#define NXVM_CORE_PROFILE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nxvm_core_profile {
    NXVM_CORE_PROFILE_CUSTOM = 1,
    NXVM_CORE_PROFILE_TEST_MINIMAL = 2
} nxvm_core_profile;

#define NXVM_CORE_DEFAULT_MEMORY_BYTES (16u * 1024u * 1024u)
#define NXVM_CORE_MINIMUM_MEMORY_BYTES (2u * 1024u * 1024u)
#define NXVM_CORE_MAXIMUM_MEMORY_BYTES (64u * 1024u * 1024u)

typedef struct nxvm_core_machine_config {
    unsigned abi_version;
    nxvm_core_profile profile;
    size_t memory_bytes;
} nxvm_core_machine_config;

#ifdef __cplusplus
}
#endif

#endif

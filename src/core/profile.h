#ifndef NXVM_CORE_PROFILE_H
#define NXVM_CORE_PROFILE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nxvm_core_profile {
    NXVM_CORE_PROFILE_CUSTOM = 1,
    NXVM_CORE_PROFILE_TEST_MINIMAL = 2
} nxvm_core_profile;

typedef struct nxvm_core_machine_config {
    unsigned abi_version;
    nxvm_core_profile profile;
} nxvm_core_machine_config;

#ifdef __cplusplus
}
#endif

#endif

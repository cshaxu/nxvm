#ifndef NTVDM64_CORE_MACHINE_PROFILE_INTERFACE_H
#define NTVDM64_CORE_MACHINE_PROFILE_INTERFACE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum core_machine_profile {
    CORE_MACHINE_PROFILE_CUSTOM = 1,
    CORE_MACHINE_PROFILE_TEST_MINIMAL = 2
} core_machine_profile;

#define CORE_MACHINE_DEFAULT_MEMORY_BYTES (16u * 1024u * 1024u)
#define CORE_MACHINE_MINIMUM_MEMORY_BYTES (2u * 1024u * 1024u)
#define CORE_MACHINE_MAXIMUM_MEMORY_BYTES (64u * 1024u * 1024u)

typedef struct core_machine_config {
    core_machine_profile profile;
    STD_SIZE_T memory_bytes;
} core_machine_config;

#ifdef __cplusplus
}
#endif

#endif

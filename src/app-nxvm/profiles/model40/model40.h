#ifndef VM_PROFILE_MODEL40_H
#define VM_PROFILE_MODEL40_H
#include "lib/types/types_interface.h"


#define VM_PROFILE_MODEL40_ROM_CHIP_BYTES (16u * 1024u)
#define VM_PROFILE_MODEL40_VIDEO_ROM_BYTES (16u * 1024u)

/* A local owner-supplied manifest is transient composition input only. */
typedef struct vm_profile_model40_byob_manifest {
    const char *even_path;
    const char *even_sha256;
    const char *odd_path;
    const char *odd_sha256;
    const char *video_path;
    const char *video_sha256;
    const char *provenance;
} vm_profile_model40_byob_manifest;

lib_i32 vm_profile_model40_byob_manifest_is_valid(
    const vm_profile_model40_byob_manifest *manifest);

#endif

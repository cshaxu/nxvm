#ifndef VM_PROFILE_MODEL40_H
#define VM_PROFILE_MODEL40_H

#include "type.h"

#define VM_PROFILE_MODEL40_ROM_CHIP_BYTES (16u * 1024u)
#define VM_PROFILE_MODEL40_VIDEO_ROM_BYTES (16u * 1024u)

/* A local owner-supplied manifest is transient composition input only. */
typedef struct vm_profile_model40_byob_manifest {
    const C_CHAR *even_path;
    const C_CHAR *even_sha256;
    const C_CHAR *odd_path;
    const C_CHAR *odd_sha256;
    const C_CHAR *video_path;
    const C_CHAR *video_sha256;
    const C_CHAR *provenance;
} vm_profile_model40_byob_manifest;

C_INT vm_profile_model40_byob_manifest_is_valid(
    const vm_profile_model40_byob_manifest *manifest);

#endif

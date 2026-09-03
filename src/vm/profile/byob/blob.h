#ifndef VM_PROFILE_BYOB_BLOB_H
#define VM_PROFILE_BYOB_BLOB_H

#include "type.h"

#define VM_PROFILE_BYOB_OPTION_ROM_MAX_BYTES (32u * 1024u)

/* One local, transient file assertion.  Profile manifests own their slot,
 * mapping and provenance rules; this neutral helper owns only byte integrity. */
typedef struct vm_profile_byob_blob {
    const C_CHAR *path;
    const C_CHAR *sha256;
    STD_SIZE_T bytes;
} vm_profile_byob_blob;

C_INT vm_profile_byob_blob_is_valid(const vm_profile_byob_blob *blob);
type_status vm_profile_byob_blob_load(const vm_profile_byob_blob *blob,
    type_unsigned_8 *out_bytes);
C_INT vm_profile_byob_option_rom_is_valid(const type_unsigned_8 *bytes,
    STD_SIZE_T byte_count, STD_SIZE_T maximum_bytes);

#endif

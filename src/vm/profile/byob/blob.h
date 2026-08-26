#ifndef VM_PROFILE_BYOB_BLOB_H
#define VM_PROFILE_BYOB_BLOB_H

#include "type.h"

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

#endif

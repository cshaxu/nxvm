#ifndef VM_PROFILE_BYOB_BLOB_H
#define VM_PROFILE_BYOB_BLOB_H
#include "lib/types/types_interface.h"


#define VM_PROFILE_BYOB_OPTION_ROM_MAX_BYTES (32u * 1024u)

/* One local, transient file assertion.  Profile manifests own their slot,
 * mapping and provenance rules; this neutral helper owns only byte integrity. */
typedef struct vm_profile_byob_blob {
    const char *path;
    const char *sha256;
    lib_size bytes;
} vm_profile_byob_blob;

lib_i32 vm_profile_byob_blob_is_valid(const vm_profile_byob_blob *blob);
lib_status vm_profile_byob_blob_load(const vm_profile_byob_blob *blob,
    lib_u8 *out_bytes);
lib_i32 vm_profile_byob_option_rom_is_valid(const lib_u8 *bytes,
    lib_size byte_count, lib_size maximum_bytes);

#endif

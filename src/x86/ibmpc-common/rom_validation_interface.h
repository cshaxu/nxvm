#ifndef VM_PROFILE_BYOB_BLOB_H
#define VM_PROFILE_BYOB_BLOB_H
#include "lib/types/types_interface.h"


#define VM_PROFILE_BYOB_OPTION_ROM_MAX_BYTES (32u * 1024u)

lib_i32 vm_profile_byob_option_rom_is_valid(const lib_u8 *bytes,
    lib_size byte_count, lib_size maximum_bytes);

#endif

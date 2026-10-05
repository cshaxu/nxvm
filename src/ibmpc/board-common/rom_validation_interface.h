#ifndef VM_PROFILE_BYOB_BLOB_H
#define VM_PROFILE_BYOB_BLOB_H
#include "lib/types/types_interface.h"


#define VM_PROFILE_BYOB_OPTION_ROM_MAX_BYTES (32u * 1024u)

lib_i32 vm_profile_byob_option_rom_is_valid(const lib_u8 *bytes,
    lib_size byte_count, lib_size maximum_bytes);

/* One image from equal even/odd chips. Destination must be twice one chip's
 * size and distinct from both inputs. Invalid inputs leave it unchanged.
 * Chip sizes, guest addresses and aliases are composition-owned. */
lib_status vm_profile_rom_interleave(lib_u8 *destination, lib_size destination_bytes,
    const lib_u8 *even, lib_size even_bytes, const lib_u8 *odd, lib_size odd_bytes);

#endif

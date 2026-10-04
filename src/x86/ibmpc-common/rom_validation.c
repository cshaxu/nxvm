#include "lib/types/types_interface.h"
#include "x86/ibmpc-common/rom_validation_interface.h"

lib_i32 vm_profile_byob_option_rom_is_valid(const lib_u8 *bytes,
    lib_size byte_count, lib_size maximum_bytes)
{
    lib_u8 checksum = 0u;
    lib_size index;

    if (bytes == LIB_NULL || byte_count < 3u || byte_count > maximum_bytes ||
        bytes[0u] != 0x55u || bytes[1u] != 0xaau ||
        (lib_size)bytes[2u] * 512u != byte_count) return 0;
    for (index = 0u; index < byte_count; ++index) {
        checksum = (lib_u8)(checksum + bytes[index]);
    }
    return checksum == 0u;
}

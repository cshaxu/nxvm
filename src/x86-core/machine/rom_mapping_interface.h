#ifndef X86_CORE_MACHINE_ROM_MAPPING_INTERFACE_H
#define X86_CORE_MACHINE_ROM_MAPPING_INTERFACE_H
#include "lib/types/types_interface.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct x86_core_machine x86_core_machine;

/* ROM backing is immutable. In-range physical writes are accepted and
 * discarded, matching a non-writable ROM bus target. */
lib_status x86_core_machine_register_immutable_rom_mapping(
    x86_core_machine *machine,
    lib_u32 physical_start,
    const lib_u8 *image,
    lib_size bytes);

/* Configuration-only alias of an existing core-owned immutable ROM subrange.
 * The source bytes remain owned by their original mapping. Ordinary routes
 * retain their provider order; reset-only fetches select an alias first. */
lib_status x86_core_machine_register_immutable_rom_mapping_alias(
    x86_core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes);
/* CPU reset aliases alone decode before ordinary address-line decoding. */
lib_status x86_core_machine_register_immutable_rom_mapping_reset_alias(
    x86_core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes);

/* Copied coverage answer only; no immutable backing or registry is exposed. */
lib_bool x86_core_machine_immutable_rom_mapping_contains(
    const x86_core_machine *machine, lib_u32 physical_start, lib_size bytes);

/* Configuration-only, all-or-none reset aliases of every existing ROM segment
 * intersecting a supplied window. Holes remain holes; backing is not copied. */
lib_status x86_core_machine_register_immutable_rom_mapping_reset_window(
    x86_core_machine *machine, lib_u32 source_start,
    lib_u32 physical_start, lib_size bytes);

#ifdef __cplusplus
}
#endif

#endif

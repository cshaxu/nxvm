#ifndef X86_NXVM_MACHINE_PREPARATION_INTERFACE_H
#define X86_NXVM_MACHINE_PREPARATION_INTERFACE_H

#include "x86-nxvm/machine/input_interface.h"

/* Call begin before allocating a candidate; out_construction has no owned
 * context on failure. finish consumes the candidate lifetime: it releases on
 * failure or publishes one copied construction for x86_nxvm_machine_create to own.
 * Inputs and output are distinct values; callbacks/context remain caller-owned. */
lib_status x86_nxvm_machine_construction_begin(const x86_nxvm_machine_config *config,
    const x86_nxvm_machine_assets *assets, x86_nxvm_machine_construction *out_construction);
lib_status x86_nxvm_machine_construction_finish(x86_nxvm_machine_construction *candidate,
    const x86_nxvm_machine_config *config, const x86_nxvm_machine_assets *assets,
    lib_status status, x86_nxvm_machine_construction *out_construction);

/* Exact-size byte preparation, without interpreting ROM layout or model. */
lib_status x86_nxvm_machine_asset_copy(lib_u8 *destination, lib_size expected,
    x86_nxvm_machine_asset_bytes source);

/* Eligibility is a copied App policy, not inferred from a default kind.
 * Each bit is 1u << x86_nxvm_board_floppy_kind. PROFILE_DEFAULT uses default_kind. */
lib_status x86_nxvm_machine_floppy_select(const x86_nxvm_machine_config *config,
    x86_nxvm_board_floppy_kind default_kind, lib_u32 allowed_kinds,
    x86_nxvm_board_floppy_kind *out_media);

#endif

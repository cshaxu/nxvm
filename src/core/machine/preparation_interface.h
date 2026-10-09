#ifndef VM_MACHINE_PREPARATION_INTERFACE_H
#define VM_MACHINE_PREPARATION_INTERFACE_H

#include "core/machine/input_interface.h"

/* Call begin before allocating a candidate; out_construction has no owned
 * context on failure. finish consumes the candidate lifetime: it releases on
 * failure or publishes one copied construction for vm_machine_create to own.
 * Inputs and output are distinct values; callbacks/context remain App-owned. */
lib_status vm_machine_construction_begin(const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *out_construction);
lib_status vm_machine_construction_finish(vm_machine_construction *candidate,
    const vm_machine_config *config, const vm_machine_assets *assets,
    lib_status status, vm_machine_construction *out_construction);

/* Exact-size byte preparation, without interpreting ROM layout or model. */
lib_status vm_machine_asset_copy(lib_u8 *destination, lib_size expected,
    vm_machine_asset_bytes source);

/* Eligibility is a copied App policy, not inferred from a default kind.
 * Each bit is 1u << vm_profile_floppy_kind. PROFILE_DEFAULT uses default_kind. */
lib_status vm_machine_floppy_select(const vm_machine_config *config,
    vm_profile_floppy_kind default_kind, lib_u32 allowed_kinds,
    vm_profile_floppy_kind *out_media);

#endif

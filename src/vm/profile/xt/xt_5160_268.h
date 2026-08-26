#ifndef VM_PROFILE_XT_5160_268_H
#define VM_PROFILE_XT_5160_268_H

#include "vm/profile/profile_resolver_interface.h"

/* This is a construction-only snapshot. It deliberately has no board
 * descriptor: B2 owns the 5160 ports, devices, firmware and media binding. */
typedef struct vm_profile_xt_5160_268_resolved_profile {
    vm_resolved_profile resolved;
} vm_profile_xt_5160_268_resolved_profile;

type_status vm_profile_xt_5160_268_declaration_create(
    vm_profile_resolver_declaration *out_declaration);
type_status vm_profile_xt_5160_268_resolve(
    vm_profile_xt_5160_268_resolved_profile *out_profile);

#endif

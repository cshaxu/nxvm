#ifndef VM_PROFILE_XT_5160_268_H
#define VM_PROFILE_XT_5160_268_H

#include "vm/profile/profile_resolver_interface.h"

#define VM_PROFILE_XT_5160_268_FDD_MEDIA_ID 1u

/* This is a construction-only snapshot.  The copied Core topology records
 * only B2-selected shared board facts; later device batches own their own
 * source-qualified additions. */
typedef struct vm_profile_xt_5160_268_resolved_profile {
    vm_resolved_profile resolved;
    core_machine_plan_topology topology;
} vm_profile_xt_5160_268_resolved_profile;

type_status vm_profile_xt_5160_268_declaration_create(
    vm_profile_resolver_declaration *out_declaration);
type_status vm_profile_xt_5160_268_resolve(
    vm_profile_xt_5160_268_resolved_profile *out_profile);

#endif

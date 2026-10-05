#ifndef NXVM_TEST_PROFILE_H
#define NXVM_TEST_PROFILE_H

#include "app-nxvm/profiles/machine_factory_interface.h"
#include "app-nxvm/profiles/selection_interface.h"
#include "app-nxvm/profiles/default_profile/construction_interface.h"
#include "app-nxvm/profiles/xt/construction_interface.h"
#include "app-nxvm/profiles/model40/construction_interface.h"
#include "ibmpc/machine/machine_private.h"

/* App fixtures prepare this actual Profile context; the Shared adapter owns
 * its lifetime. This borrowed test view adds no registry or mirrored state. */
static inline vm_profile_machine_plan *vm_test_profile_plan(
    const vm_machine *machine)
{
    return machine == LIB_NULL ? LIB_NULL :
        (vm_profile_machine_plan *)machine->construction.profile.context;
}

/* Multi-profile selection belongs to fixtures, never to a product EXE. */
static inline lib_status vm_test_profile_plan_create(vm_machine_profile_kind kind,
    const vm_machine_config *config,
    const vm_machine_assets *assets, vm_profile_machine_plan **out_plan)
{
    if (out_plan == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_plan = LIB_NULL;
    if (config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    switch (kind) {
    case VM_MACHINE_PROFILE_DEFAULT_PC_AT:
        return vm_profile_machine_plan_create_default(config, assets, out_plan);
    case VM_MACHINE_PROFILE_IBM_5170_MODEL_339:
        return vm_profile_machine_plan_create_5170(config, assets, out_plan);
    case VM_MACHINE_PROFILE_IBM_5160_MODEL_268:
        return vm_profile_machine_plan_create_xt(config, assets, out_plan);
    case VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40:
        return vm_profile_machine_plan_create_model40(config, assets, out_plan);
    default:
        return LIB_STATUS_INVALID_ARGUMENT;
    }
}

static inline lib_status vm_test_machine_create_from_assets(vm_machine_profile_kind kind,
    const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine **out_machine)
{
    vm_profile_machine_plan *plan = LIB_NULL;
    lib_status status;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    status = vm_test_profile_plan_create(kind, config, assets, &plan);
    return status == LIB_STATUS_OK ?
        vm_machine_create_from_plan(config, plan, out_machine) : status;
}

#endif

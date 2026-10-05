#ifndef NXVM_TEST_PROFILE_H
#define NXVM_TEST_PROFILE_H

#include "ibmpc/machine/machine_interface.h"
#include "selection.h"
#include "app-nxvm/profiles/default_profile/construction_interface.h"
#include "app-nxvm/profiles/xt/construction_interface.h"
#include "app-nxvm/profiles/model40/construction_interface.h"
#include "app-nxvm/profiles/model40/model40_private.h"
#include "ibmpc/machine/machine_private.h"

/* Borrow the actual Machine-owned construction. This test view adds no
 * registry, mirrored state or production getter. */
static inline const vm_machine_construction *vm_test_profile_construction(
    const vm_machine *machine)
{
    return machine == LIB_NULL ? LIB_NULL :
        &machine->construction;
}

static inline lib_bool vm_test_profile_is_model40(const vm_machine_construction *construction)
{
    return construction != LIB_NULL &&
        construction->firmware_provider == vm_profile_model40_firmware_provider();
}

static inline const vm_profile_model40_external_rom *vm_test_profile_model40_rom(
    const vm_machine_construction *construction)
{
    return !vm_test_profile_is_model40(construction) ? LIB_NULL : construction->firmware_context;
}

/* Multi-profile selection belongs to fixtures, never to a product EXE. */
static inline lib_status vm_test_profile_construction_create(vm_machine_profile_kind kind,
    const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine_construction *out_construction)
{
    if (out_construction == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_construction = (vm_machine_construction){0};
    if (config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    switch (kind) {
    case VM_MACHINE_PROFILE_DEFAULT_PC_AT:
        return vm_profile_machine_plan_create_default(config, assets, out_construction);
    case VM_MACHINE_PROFILE_IBM_5170_MODEL_339:
        return vm_profile_machine_plan_create_5170(config, assets, out_construction);
    case VM_MACHINE_PROFILE_IBM_5160_MODEL_268:
        return vm_profile_machine_plan_create_xt(config, assets, out_construction);
    case VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40:
        return vm_profile_machine_plan_create_model40(config, assets, out_construction);
    default:
        return LIB_STATUS_INVALID_ARGUMENT;
    }
}

static inline lib_status vm_test_machine_create_from_assets(vm_machine_profile_kind kind,
    const vm_machine_config *config,
    const vm_machine_assets *assets, vm_machine **out_machine)
{
    vm_machine_construction construction;
    lib_status status;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    status = vm_test_profile_construction_create(kind, config, assets, &construction);
    if (status != LIB_STATUS_OK) return status;
    return vm_machine_create(config, &construction, out_machine);
}

#endif

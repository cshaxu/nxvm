#ifndef TEST_MYDESKPRO386_PROFILE_H
#define TEST_MYDESKPRO386_PROFILE_H

#include "app-mydeskpro386/profiles/construction_interface.h"
#include "app-mydeskpro386/profiles/model40_private.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine_private.h"
#include "../../core/machine/support/selection.h"

static inline const vm_machine_construction *vm_test_profile_construction(
    const vm_machine *machine)
{
    return machine == LIB_NULL ? LIB_NULL : &machine->construction;
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

static inline lib_status vm_test_machine_create_from_assets(vm_machine_profile_kind kind,
    const vm_machine_config *config, const vm_machine_assets *assets, vm_machine **out_machine)
{
    vm_machine_construction construction;
    lib_status status;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    if (kind != VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_profile_machine_plan_create_model40(config, assets, &construction);
    if (status != LIB_STATUS_OK) return status;
    return vm_machine_create(config, &construction, out_machine);
}

#endif

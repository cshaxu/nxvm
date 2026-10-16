#ifndef TEST_MYDESKPRO386_PROFILE_H
#define TEST_MYDESKPRO386_PROFILE_H

#include "app-mydeskpro386/machine_plan_interface.h"
#include "app-mydeskpro386/machine_config.h"
#include "x86-nxvm/machine/machine_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "../../x86-nxvm/machine/support/selection.h"

static inline const x86_nxvm_machine_construction *vm_test_profile_construction(
    const x86_nxvm_machine *machine)
{
    return machine == LIB_NULL ? LIB_NULL : &machine->construction;
}

static inline lib_bool vm_test_profile_is_model40(const x86_nxvm_machine_construction *construction)
{
    return construction != LIB_NULL &&
        construction->firmware_provider == mydeskpro386_firmware_provider();
}

static inline const mydeskpro386_external_rom *vm_test_profile_model40_rom(
    const x86_nxvm_machine_construction *construction)
{
    return !vm_test_profile_is_model40(construction) ? LIB_NULL : construction->firmware_context;
}

static inline lib_status vm_test_machine_create_from_assets(vm_machine_profile_kind kind,
    const x86_nxvm_machine_config *config, const x86_nxvm_machine_assets *assets, x86_nxvm_machine **out_machine)
{
    x86_nxvm_machine_construction construction;
    lib_status status;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    if (kind != VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = mydeskpro386_machine_plan_create(config, assets, &construction);
    if (status != LIB_STATUS_OK) return status;
    return x86_nxvm_machine_create(config, &construction, out_machine);
}

#endif

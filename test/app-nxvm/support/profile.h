#ifndef TEST_APP_NXVM_PROFILE_H
#define TEST_APP_NXVM_PROFILE_H

#include "app-nxvm/profiles/construction_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine_private.h"
#include "../../core/machine/support/selection.h"

/* Default-PC test construction is local to this App.  Cross-profile test
 * construction lives in Core Machine qualification support. */
static inline const vm_machine_construction *vm_test_profile_construction(
    const vm_machine *machine)
{
    return machine == LIB_NULL ? LIB_NULL : &machine->construction;
}

static inline lib_status vm_test_profile_construction_create(vm_machine_profile_kind kind,
    const vm_machine_config *config, const vm_machine_assets *assets,
    vm_machine_construction *out_construction)
{
    if (kind != VM_MACHINE_PROFILE_DEFAULT_PC_AT) return LIB_STATUS_INVALID_ARGUMENT;
    return vm_profile_machine_plan_create_default(config, assets, out_construction);
}

static inline lib_status vm_test_machine_create_from_assets(vm_machine_profile_kind kind,
    const vm_machine_config *config, const vm_machine_assets *assets, vm_machine **out_machine)
{
    vm_machine_construction construction;
    lib_status status;

    if (out_machine == LIB_NULL || kind != VM_MACHINE_PROFILE_DEFAULT_PC_AT)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    status = vm_test_profile_construction_create(kind, config, assets, &construction);
    if (status != LIB_STATUS_OK) return status;
    return vm_machine_create(config, &construction, out_machine);
}

#endif

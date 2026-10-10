#include "app-my5170/profiles/profile_interface.h"
#include "../../../core/machine/support/dma_deadline_fixture.h"

static lib_i32 configure(core_machine_config *out_configuration)
{
    const vm_profile_default_pc_at_descriptor *profile =
        vm_profile_ibm_5170_model_339_descriptor_get();
    vm_profile_default_pc_at_cpu_contract contract;
    core_machine_controller_timing_rules rules;

    return out_configuration == LIB_NULL || profile == LIB_NULL ||
        !vm_profile_default_pc_at_cpu_contract_select(profile, profile->cpu_profile,
            profile->fpu_profile, &contract) ||
        !vm_profile_default_pc_at_core_config_materialize(profile, &contract,
            out_configuration, &rules);
}

lib_i32 main(void)
{
    if (test_core_dma_deadline_assert("ibm-5170-model-339", configure)) return 1;
    lib_c_printf("IBM-5170-DMA-DEADLINE:OK\n");
    return 0;
}

#include "app-nxvm/machine_config_interface.h"
#include "../../x86-nxvm/machine/support/dma_deadline_fixture.h"

static lib_i32 configure(x86_nxvm_board_config *out_configuration)
{
    const x86_nxvm_board_pc_at_descriptor *profile =
        vm_profile_default_pc_at_descriptor_get();
    x86_nxvm_board_pc_at_cpu_contract contract;
    x86_nxvm_board_controller_timing_rules rules;

    return out_configuration == LIB_NULL || profile == LIB_NULL ||
        !x86_nxvm_board_pc_at_cpu_contract_select(profile, profile->cpu_profile,
            profile->fpu_profile, &contract) ||
        !x86_nxvm_board_pc_at_core_config_materialize(profile, &contract,
            out_configuration, &rules);
}

lib_i32 main(void)
{
    if (test_x86_nxvm_dma_deadline_assert("default-pc-at", configure)) return 1;
    lib_c_printf("DEFAULT-PC-AT-DMA-DEADLINE:OK\n");
    return 0;
}

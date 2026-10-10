#include "app-my5160/profiles/xt_5160_268.h"
#include "../../../core/machine/support/dma_deadline_fixture.h"

static lib_i32 configure(core_machine_config *out_configuration)
{
    vm_profile_xt_5160_268_plan_snapshot profile;

    if (out_configuration == LIB_NULL ||
        vm_profile_xt_5160_268_plan_create(&profile, LIB_FALSE) != LIB_STATUS_OK) return 1;
    *out_configuration = profile.values.core.configuration;
    return 0;
}

lib_i32 main(void)
{
    if (test_core_dma_deadline_assert("ibm-5160-model-268", configure)) return 1;
    lib_c_printf("IBM-5160-DMA-DEADLINE:OK\n");
    return 0;
}

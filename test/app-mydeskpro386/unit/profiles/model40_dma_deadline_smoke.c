#include "app-mydeskpro386/profiles/model40_private.h"
#include "../../../core/machine/support/dma_deadline_fixture.h"

static lib_i32 configure(core_machine_config *out_configuration)
{
    if (out_configuration == LIB_NULL) return 1;
    vm_profile_model40_core_config_initialize(out_configuration);
    return 0;
}

lib_i32 main(void)
{
    if (test_core_dma_deadline_assert("compaq-deskpro-386-model-40", configure)) return 1;
    lib_c_printf("MODEL40-DMA-DEADLINE:OK\n");
    return 0;
}

#include "app-mydeskpro386/machine_config.h"
#include "../../x86-nxvm/machine/support/dma_deadline_fixture.h"

static lib_i32 configure(x86_nxvm_board_config *out_configuration)
{
    if (out_configuration == LIB_NULL) return 1;
    mydeskpro386_core_config_initialize(out_configuration);
    return 0;
}

lib_i32 main(void)
{
    if (test_x86_nxvm_dma_deadline_assert("compaq-deskpro-386-model-40", configure)) return 1;
    lib_c_printf("MODEL40-DMA-DEADLINE:OK\n");
    return 0;
}

#ifndef TEST_IBMPC_COMMON_DMA_COMPETITION_FIXTURE_H
#define TEST_IBMPC_COMMON_DMA_COMPETITION_FIXTURE_H
#include "x86/ibmpc-common/machine_board_interface.h"
#include "x86/ibmpc-common/dma_bus_interface.h"

/* Board owns DMA and finalizes it with Core's attachment. The integration
 * fixture publishes only existing opaque handles, never either private layout. */
lib_status test_dma_competition_create(const core_machine_config *config,
    const core_machine_dma_channel_provider *provider, void *source,
    core_machine **out_machine, core_machine_dma_bus **out_dma,
    core_machine_dma_request_binding *out_binding);
#endif

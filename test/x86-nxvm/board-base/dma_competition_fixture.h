#ifndef TEST_X86_NXVM_EMULATOR_DMA_COMPETITION_FIXTURE_H
#define TEST_X86_NXVM_EMULATOR_DMA_COMPETITION_FIXTURE_H
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/dma_bus_interface.h"

/* Board owns DMA and finalizes it with Core's attachment. The integration
 * fixture publishes only existing opaque handles, never either private layout. */
lib_status test_dma_competition_create(const x86_nxvm_board_config *config,
    const x86_nxvm_board_dma_channel_provider *provider, void *source,
    x86_core_machine **out_machine, x86_nxvm_board_dma_bus **out_dma,
    x86_nxvm_board_dma_request_binding *out_binding);
lib_status test_dma_competition_program_channel2(x86_core_machine *machine);
#endif

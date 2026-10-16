#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"
#include "x86-nxvm/board-base/media_interface.h"

static const x86_nxvm_board_config x86_nxvm_test_dma_binding_token_config = {
    .memory_bytes = (2u * 1024u * 1024u),
    .cpu_profile = X86_CORE_CPU_PROFILE_8086,
    .fpu_profile = X86_FPU_PROFILE_NONE,
    .ticks_per_instruction = 1u
};

static const x86_nxvm_board_dma_wiring x86_nxvm_test_dma_binding_token_wiring = {
    .fdc_channel = 2u,
    .controller_count = X86_NXVM_BOARD_DMA_CONTROLLER_COUNT,
    .cascade_channel = X86_NXVM_BOARD_DMA_CASCADE_CHANNEL
};

static const x86_nxvm_board_fdc_config x86_nxvm_test_dma_binding_token_fdc_config = {
    .dor_port = 0x03f2u, .status_port = 0x03f4u, .data_port = 0x03f5u,
    .direction_port = 0x03f7u, .control_port = 0x03f7u,
    .irq = 6u, .dma_channel = 2u
};

static x86_nxvm_board_fdc_topology x86_nxvm_test_dma_binding_token_topology(
    x86_nxvm_board_media_registry *media,
    x86_nxvm_board_dma_request_binding request)
{
    x86_nxvm_board_fdc_topology topology = {0};

    topology.media_registry = media;
    topology.drives.media_id[0] = X86_NXVM_BOARD_MEDIA_ID_INVALID;
    topology.drives.media_id[1] = X86_NXVM_BOARD_MEDIA_ID_INVALID;
    topology.drives.media_id[2] = X86_NXVM_BOARD_MEDIA_ID_INVALID;
    topology.drives.media_id[3] = X86_NXVM_BOARD_MEDIA_ID_INVALID;
    topology.dma_request = request;
    topology.config = x86_nxvm_test_dma_binding_token_fdc_config;
    return topology;
}

lib_i32 main(void)
{
    x86_nxvm_board_media_registry *media = LIB_NULL;
    x86_nxvm_board_dma_request_binding first_request = {0};
    x86_nxvm_board_dma_request_binding second_request = {0};
    x86_nxvm_board_fdc_topology first_topology;
    x86_nxvm_board_fdc_topology second_topology;
    x86_core_machine *first = LIB_NULL;
    x86_nxvm_board_state *first_board = LIB_NULL;
    x86_core_machine *second = LIB_NULL;
    x86_nxvm_board_state *second_board = LIB_NULL;
    lib_i32 failed = 0;

    if (x86_nxvm_board_media_registry_create(&media) != LIB_STATUS_OK ||
        x86_nxvm_board_create(&x86_nxvm_test_dma_binding_token_config, &first, &first_board) !=
            LIB_STATUS_OK ||
        x86_nxvm_board_create(&x86_nxvm_test_dma_binding_token_config, &second, &second_board) !=
            LIB_STATUS_OK ||
        x86_nxvm_board_configure_dma(first_board, &x86_nxvm_test_dma_binding_token_wiring,
            &first_request) != LIB_STATUS_OK ||
        x86_nxvm_board_configure_dma(second_board, &x86_nxvm_test_dma_binding_token_wiring,
            &second_request) != LIB_STATUS_OK ||
        first_request.core_token == 0u || second_request.core_token == 0u ||
        first_request.core_token == second_request.core_token) {
        failed = 1;
        goto done;
    }

    first_topology = x86_nxvm_test_dma_binding_token_topology(media, first_request);
    second_topology = x86_nxvm_test_dma_binding_token_topology(media, second_request);
    if (x86_nxvm_board_configure_fdc(first_board, &second_topology) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_board_configure_fdc(second_board, &first_topology) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        x86_nxvm_board_configure_fdc(first_board, &first_topology) != LIB_STATUS_OK ||
        x86_nxvm_board_configure_fdc(second_board, &second_topology) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(first) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(second) != LIB_STATUS_OK ||
        x86_core_machine_reset(first) != LIB_STATUS_OK ||
        x86_core_machine_reset(second) != LIB_STATUS_OK) {
        failed = 1;
    }

done:
    x86_core_machine_destroy(second);
    x86_core_machine_destroy(first);
    x86_nxvm_board_media_registry_destroy(media);
    if (failed) return 1;
    lib_c_printf("%s\n", "DMA-BINDING-TOKEN:OK");
    return 0;
}

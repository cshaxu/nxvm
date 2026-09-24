#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/media_interface.h"

static const core_machine_config core_machine_dma_binding_token_config = {
    .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
    .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
    .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
    .ticks_per_instruction = 1u
};

static const core_machine_dma_wiring core_machine_dma_binding_token_wiring = {
    .fdc_channel = 2u,
    .controller_count = CORE_MACHINE_DMA_CONTROLLER_COUNT,
    .cascade_channel = CORE_MACHINE_DMA_CASCADE_CHANNEL
};

static const core_machine_fdc_config core_machine_dma_binding_token_fdc_config = {
    .dor_port = 0x03f2u, .status_port = 0x03f4u, .data_port = 0x03f5u,
    .direction_port = 0x03f7u, .control_port = 0x03f7u,
    .irq = 6u, .dma_channel = 2u
};

static core_machine_fdc_topology core_machine_dma_binding_token_topology(
    core_machine_media_registry *media,
    core_machine_dma_request_binding request)
{
    core_machine_fdc_topology topology = {0};

    topology.media_registry = media;
    topology.drives.media_id[0] = CORE_MACHINE_MEDIA_ID_INVALID;
    topology.drives.media_id[1] = CORE_MACHINE_MEDIA_ID_INVALID;
    topology.drives.media_id[2] = CORE_MACHINE_MEDIA_ID_INVALID;
    topology.drives.media_id[3] = CORE_MACHINE_MEDIA_ID_INVALID;
    topology.dma_request = request;
    topology.config = core_machine_dma_binding_token_fdc_config;
    return topology;
}

lib_i32 main(void)
{
    core_machine_media_registry *media = LIB_NULL;
    core_machine_dma_request_binding first_request = {0};
    core_machine_dma_request_binding second_request = {0};
    core_machine_fdc_topology first_topology;
    core_machine_fdc_topology second_topology;
    core_machine *first = LIB_NULL;
    core_machine *second = LIB_NULL;
    lib_i32 failed = 0;

    if (core_machine_media_registry_create(&media) != LIB_STATUS_OK ||
        core_machine_create(&core_machine_dma_binding_token_config, &first) !=
            LIB_STATUS_OK ||
        core_machine_create(&core_machine_dma_binding_token_config, &second) !=
            LIB_STATUS_OK ||
        core_machine_configure_dma(first, &core_machine_dma_binding_token_wiring,
            &first_request) != LIB_STATUS_OK ||
        core_machine_configure_dma(second, &core_machine_dma_binding_token_wiring,
            &second_request) != LIB_STATUS_OK ||
        first_request.core_token == 0u || second_request.core_token == 0u ||
        first_request.core_token == second_request.core_token) {
        failed = 1;
        goto done;
    }

    first_topology = core_machine_dma_binding_token_topology(media, first_request);
    second_topology = core_machine_dma_binding_token_topology(media, second_request);
    if (core_machine_configure_fdc(first, &second_topology) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_configure_fdc(second, &first_topology) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_configure_fdc(first, &first_topology) != LIB_STATUS_OK ||
        core_machine_configure_fdc(second, &second_topology) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(first) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(second) != LIB_STATUS_OK ||
        core_machine_reset(first) != LIB_STATUS_OK ||
        core_machine_reset(second) != LIB_STATUS_OK) {
        failed = 1;
    }

done:
    core_machine_destroy(second);
    core_machine_destroy(first);
    core_machine_media_registry_destroy(media);
    if (failed) return 1;
    puts("M5:T300:S4:DMA-BINDING-TOKEN:OK");
    return 0;
}

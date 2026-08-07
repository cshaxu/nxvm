#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/fdc.h"
#include "core/machine/machine.h"
#include "core/machine/media_interface.h"
#include "core/machine/port.h"

static core_machine_media_result core_machine_fdc_fixture_query(C_VOID *context,
    core_machine_media_info *out_info)
{
    (C_VOID)context;
    if (out_info == STD_NULL) return CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE;
    STD_MEMSET(out_info, 0, sizeof(*out_info));
    out_info->present = TYPE_TRUE;
    out_info->geometry.cylinders = 1u;
    out_info->geometry.heads = 1u;
    out_info->geometry.sectors_per_track = 1u;
    out_info->geometry.bytes_per_sector = 512u;
    out_info->geometry.logical_sector_count = 1u;
    return CORE_MACHINE_MEDIA_RESULT_OK;
}

static const core_machine_media_provider core_machine_fdc_fixture_media = {
    core_machine_fdc_fixture_query, STD_NULL, STD_NULL, STD_NULL, STD_NULL
};

C_INT main(C_VOID)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_fdc_config fdc_config = {
        .dor_port = 0x03f2u,
        .status_port = 0x03f4u,
        .data_port = 0x03f5u,
        .direction_port = 0x03f7u,
        .control_port = 0x03f7u,
        .irq = 6u,
        .dma_channel = 2u
    };
    core_machine_media_registry media = {0};
    core_machine_dma_request_binding dma_request = {0};
    core_machine *machine = STD_NULL;
    core_machine_fdc *fdc;
    t_port *port;
    C_INT failed = 0;

    core_machine_media_registry_initialize(&media);
    if (core_machine_create(&config, &machine) != TYPE_STATUS_OK) failed |= 0x01;
    if (!failed) {
        fdc = core_machine_configuration_shared_fdc_borrow(machine);
        port = core_machine_configuration_port_borrow(machine);
        if (fdc == STD_NULL || port == STD_NULL ||
            core_machine_media_registry_bind(&media, 1u, &media,
                &core_machine_fdc_fixture_media) != TYPE_STATUS_OK ||
            core_machine_media_registry_freeze(&media) != TYPE_STATUS_OK ||
            core_machine_dma_bind_channel(
                core_machine_configuration_shared_dma_latch_borrow(machine),
                core_machine_configuration_shared_dma_primary_borrow(machine),
                core_machine_configuration_shared_dma_secondary_borrow(machine),
                fdc_config.dma_channel, core_machine_fdc_dma_provider(), fdc,
                &dma_request) != TYPE_STATUS_OK) {
            failed |= 0x02;
        } else {
            core_machine_fdc_connect(fdc, &media, 1u, &dma_request,
                core_machine_configuration_shared_pic_master_borrow(machine),
                core_machine_configuration_shared_pic_slave_borrow(machine), port,
                &fdc_config);
            core_machine_fdc_initialize(fdc);
            if (core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
                core_machine_reset(machine) != TYPE_STATUS_OK) {
                failed |= 0x04;
            } else {
                core_machine_port_write(port, fdc_config.dor_port, 0x1cu);
                core_machine_port_write(port, fdc_config.data_port, 0x10u);
                failed |= core_machine_port_read(port, fdc_config.data_port) != 0x90u;
            }
        }
    }
    core_machine_destroy(machine);
    core_machine_media_registry_finalize(&media);
    if (failed) return 1;
    puts("M5:T276:S3:CORE-FDC:OK");
    return 0;
}

#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/memory_interface.h"
#include "core/machine/vadp.h"

static C_VOID core_machine_display_authority_mode_changed(C_VOID *context)
{
    (C_VOID)context;
}

static C_INT core_machine_display_authority_snapshot(C_VOID *context,
    core_machine_display_snapshot *snapshot)
{
    (C_VOID)context;
    (C_VOID)snapshot;
    return TYPE_FALSE;
}

int main(C_VOID)
{
    core_machine_config machine_config = {0};
    core_machine_display_config display_config = {0};
    core_machine_display_provider_slot provider;
    core_machine *machine = STD_NULL;
    uint8_t pixel = 0x5au;
    C_INT failed = 0;

    machine_config.memory_bytes = CORE_MACHINE_DEFAULT_MEMORY_BYTES;
    core_machine_display_provider_slot_initialize(&provider);
    core_machine_display_provider_slot_bind(&provider, STD_NULL,
        core_machine_display_authority_mode_changed, STD_NULL,
        core_machine_display_authority_snapshot);
    display_config.text_timing.active_display_ticks = 48u;
    display_config.text_timing.horizontal_blank_ticks = 8u;
    display_config.text_timing.vertical_retrace_ticks = 8u;
    display_config.ega_sequencer.aperture_base = CORE_MACHINE_VADP_EGA_APERTURE_BASE;
    display_config.ega_sequencer.aperture_bytes = CORE_MACHINE_VADP_EGA_APERTURE_BYTES;
    display_config.ega_sequencer.reset = 0x03u;
    display_config.ega_sequencer.clocking_mode = 0x00u;
    display_config.ega_sequencer.map_mask = 0x0fu;
    display_config.ega_sequencer.memory_mode = 0x02u;
    display_config.ega_sequencer.planar_ega = TYPE_TRUE;
    display_config.ports.attribute_first = CORE_MACHINE_VADP_PORT_ATTRIBUTE;
    display_config.ports.attribute_last = CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ;
    display_config.ports.sequencer_first = CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX;
    display_config.ports.sequencer_last = CORE_MACHINE_VADP_PORT_SEQUENCER_DATA;
    display_config.ports.graphics_first = CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX;
    display_config.ports.graphics_last = CORE_MACHINE_VADP_PORT_GRAPHICS_DATA;
    display_config.ports.crtc_first = CORE_MACHINE_VADP_PORT_CRTC_INDEX;
    display_config.ports.crtc_last = CORE_MACHINE_VADP_PORT_STATUS;
    display_config.provider = &provider;

    if (core_machine_create(&machine_config, &machine) != TYPE_STATUS_OK ||
        core_machine_configure_display(machine, &display_config) != TYPE_STATUS_OK ||
        !provider.frozen ||
        core_machine_configure_display(machine, &display_config) !=
            TYPE_STATUS_INVALID_STATE ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_configure_display(machine, &display_config) !=
            TYPE_STATUS_INVALID_STATE ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, CORE_MACHINE_VADP_EGA_APERTURE_BASE,
            &pixel, sizeof(pixel)) != TYPE_STATUS_OK) {
        failed = 1;
    }
    core_machine_destroy(machine);
    core_machine_display_provider_slot_finalize(&provider);
    if (!failed) STD_PRINTF("M5:T296:S2:DISPLAY-AUTHORITY:OK\n");
    return failed;
}

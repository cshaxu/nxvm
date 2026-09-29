#include "lib/types/types_interface.h"

#include "app-nxvm/devices/machine.h"

lib_status core_machine_capture_display_snapshot(const core_machine *machine,
    x86_video_snapshot *out_snapshot)
{
    core_machine *mutable_machine = (core_machine *)machine;

    if (machine == LIB_NULL || out_snapshot == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }
    return core_machine_vadp_capture_snapshot(&mutable_machine->shared_vadp,
        &mutable_machine->executor_memory, out_snapshot) ? LIB_STATUS_OK :
        LIB_STATUS_UNSUPPORTED;
}

lib_status core_machine_observe_display_snapshot(const core_machine *machine,
    lib_u8 acknowledged_generation_valid,
    lib_u64 acknowledged_generation,
    x86_video_snapshot_observation *out_observation)
{
    if (machine == LIB_NULL || out_observation == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }
    x86_video_observe_snapshot(machine->shared_vadp.chip,
        acknowledged_generation_valid, acknowledged_generation, out_observation);
    return LIB_STATUS_OK;
}

static lib_i32 core_machine_display_ports_are_vadp(
    const core_machine_display_config *config)
{
    const core_machine_display_port_topology *ports;

    if (config == LIB_NULL) return LIB_FALSE;
    ports = &config->ports;
    if (ports->crtc_first != CORE_MACHINE_VADP_PORT_CRTC_INDEX ||
        ports->crtc_last != CORE_MACHINE_VADP_PORT_STATUS) return LIB_FALSE;
    return !config->ega_present ||
        (ports->attribute_first == CORE_MACHINE_VADP_PORT_ATTRIBUTE &&
        ports->attribute_last == CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ &&
        ports->sequencer_first == CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX &&
        ports->sequencer_last == CORE_MACHINE_VADP_PORT_SEQUENCER_DATA &&
        ports->graphics_first == CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX &&
        ports->graphics_last == CORE_MACHINE_VADP_PORT_GRAPHICS_DATA);
}

lib_status core_machine_configure_display(core_machine *machine,
    const core_machine_display_config *config)
{
    lib_status status;

    if (!core_machine_configuration_is_open(machine) || machine->display_configured) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (config == LIB_NULL || !core_machine_display_ports_are_vadp(config) ||
        (config->ega_present && config->ega_personality ==
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR &&
        !x86_video_cecg_config_is_valid(&config->cecg))) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_vadp_configure(&machine->shared_vadp,
        &machine->executor_memory, config);
    if (status != LIB_STATUS_OK) return status;
    machine->display_ports = config->ports;
    machine->display_configured = LIB_TRUE;
    return LIB_STATUS_OK;
}

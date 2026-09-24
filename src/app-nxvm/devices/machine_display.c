#include "lib/types/types_interface.h"

#include "app-nxvm/devices/machine.h"

lib_status core_machine_capture_display_snapshot(const core_machine *machine,
    core_machine_display_snapshot *out_snapshot)
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
    core_machine_display_snapshot_observation *out_observation)
{
    if (machine == LIB_NULL || out_observation == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (machine->lifecycle != CORE_MACHINE_STOPPED &&
        machine->lifecycle != CORE_MACHINE_PAUSED) {
        return LIB_STATUS_INVALID_STATE;
    }
    core_machine_vadp_observe_snapshot(&machine->shared_vadp,
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
    core_machine_port_provider_entry *port_checkpoint;
    lib_status status;

    if (!core_machine_configuration_is_open(machine) || machine->display_configured) {
        return LIB_STATUS_INVALID_STATE;
    }
    if (config == LIB_NULL || !core_machine_display_ports_are_vadp(config) ||
        (config->ega_present && config->ega_personality ==
        CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR &&
        !core_machine_vadp_cecg_config_is_valid(&config->cecg))) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    status = core_machine_vadp_configure_text_timing(&machine->shared_vadp,
        &config->text_timing);
    if (status != LIB_STATUS_OK) return status;
    status = core_machine_vadp_configure_text_glyphs(&machine->shared_vadp,
        &config->text_glyphs);
    if (status != LIB_STATUS_OK) return status;
    if (config->cga_vram_present) {
        status = core_machine_vadp_configure_cga_memory(&machine->shared_vadp,
            &machine->executor_memory);
        if (status != LIB_STATUS_OK) return status;
    }
    if (config->ega_present) {
        port_checkpoint = core_machine_port_registration_begin(
            &machine->executor_port);
        core_machine_vadp_configure_ega_ports(&machine->shared_vadp,
            &machine->executor_port);
        status = core_machine_port_registration_status(&machine->executor_port);
        if (status != LIB_STATUS_OK) {
            core_machine_port_rollback_registration(&machine->executor_port,
                port_checkpoint);
            return status;
        }
        status = core_machine_vadp_configure_ega_sequencer(&machine->shared_vadp,
            &machine->executor_memory, &config->ega_sequencer);
        if (status != LIB_STATUS_OK) return status;
        status = core_machine_vadp_configure_ega_controllers(&machine->shared_vadp,
            &config->ega_controllers);
        if (status != LIB_STATUS_OK) return status;
        status = core_machine_vadp_configure_ega_personality(
            &machine->shared_vadp, &machine->executor_port,
            config->ega_personality);
        if (status == LIB_STATUS_OK && config->ega_personality ==
            CORE_MACHINE_VADP_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) {
            status = core_machine_vadp_configure_cecg(&machine->shared_vadp,
                &config->cecg);
        }
        if (status != LIB_STATUS_OK) {
            core_machine_port_rollback_registration(&machine->executor_port,
                port_checkpoint);
            return status;
        }
    }
    machine->display_ports = config->ports;
    machine->display_configured = LIB_TRUE;
    return LIB_STATUS_OK;
}

/* Copyright 2012-2026 Neko. */
#include "app-nxvm/devices/vadp.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/port.h"

typedef struct video_port_route {
    lib_u16 address;
    lib_bool write;
} video_port_route;

static const video_port_route cga_ports[] = {
    { CORE_MACHINE_VADP_PORT_CRTC_INDEX, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_CRTC_DATA, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_CRTC_DATA, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_MODE, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_COLOR, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_STATUS, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_RESET, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET, LIB_TRUE },
};

static const video_port_route ega_ports[] = {
    { CORE_MACHINE_VADP_PORT_MODE, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_COLOR, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_ATTRIBUTE, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_ATTRIBUTE_DATA_READ, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_SEQUENCER_DATA, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_SEQUENCER_DATA, LIB_TRUE },
};

static const video_port_route generic_ports[] = {
    { CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_MONO, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_COLOR, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_MONO_STATUS, LIB_FALSE },
};

static const video_port_route vga_ports[] = {
    { CORE_MACHINE_VADP_PORT_VGA_DAC_MASK, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_VGA_DAC_MASK, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_VGA_DAC_READ_INDEX, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_VGA_DAC_READ_INDEX, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_VGA_DAC_WRITE_INDEX, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_VGA_DAC_DATA, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_VGA_DAC_DATA, LIB_TRUE },
};

static const video_port_route compaq_ports[] = {
    { CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_COMPAQ_FEATURE_CONTROL, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_MONO_CRTC_DATA, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_MONO_STATUS, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_MONO_STATUS, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_MONO_LIGHTPEN_LATCH_RESET, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_MONO_LIGHTPEN_LATCH_SET, LIB_TRUE },
    { CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_COMPAQ_DISPLAY_TYPE, LIB_FALSE },
    { CORE_MACHINE_VADP_PORT_COMPAQ_INITIAL_MODE, LIB_FALSE },
};

static x86_video_register video_register(lib_u16 address)
{
    switch (address) {
    case 0x03b4u: return X86_VIDEO_REGISTER_MONO_CRTC_INDEX;
    case 0x03b5u: return X86_VIDEO_REGISTER_MONO_CRTC_DATA;
    case 0x03bau: return X86_VIDEO_REGISTER_MONO_STATUS;
    case 0x03bbu: return X86_VIDEO_REGISTER_MONO_LIGHTPEN_RESET;
    case 0x03bcu: return X86_VIDEO_REGISTER_MONO_LIGHTPEN_SET;
    case 0x03c0u: return X86_VIDEO_REGISTER_ATTRIBUTE;
    case 0x03c1u: return X86_VIDEO_REGISTER_ATTRIBUTE_DATA;
    case 0x03c2u: return X86_VIDEO_REGISTER_EXTERNAL_CONTROL;
    case 0x03c4u: return X86_VIDEO_REGISTER_SEQUENCER_INDEX;
    case 0x03c5u: return X86_VIDEO_REGISTER_SEQUENCER_DATA;
    case 0x03c6u: return X86_VIDEO_REGISTER_AUXILIARY_CONTROL;
    case 0x03c7u: return X86_VIDEO_REGISTER_DAC_READ_INDEX;
    case 0x03c8u: return X86_VIDEO_REGISTER_DAC_WRITE_INDEX;
    case 0x03c9u: return X86_VIDEO_REGISTER_DAC_DATA;
    case 0x03ceu: return X86_VIDEO_REGISTER_GRAPHICS_INDEX;
    case 0x03cfu: return X86_VIDEO_REGISTER_GRAPHICS_DATA;
    case 0x03d4u: return X86_VIDEO_REGISTER_COLOR_CRTC_INDEX;
    case 0x03d5u: return X86_VIDEO_REGISTER_COLOR_CRTC_DATA;
    case 0x03d8u: return X86_VIDEO_REGISTER_MODE;
    case 0x03d9u: return X86_VIDEO_REGISTER_COLOR;
    case 0x03dau: return X86_VIDEO_REGISTER_COLOR_STATUS;
    case 0x03dbu: return X86_VIDEO_REGISTER_COLOR_LIGHTPEN_RESET;
    case 0x03dcu: return X86_VIDEO_REGISTER_COLOR_LIGHTPEN_SET;
    case 0x07c6u: return X86_VIDEO_REGISTER_ENVIRONMENT;
    case 0x0bc6u: return X86_VIDEO_REGISTER_DISPLAY_TYPE;
    case 0x0fc6u: return X86_VIDEO_REGISTER_INITIAL_MODE;
    default: return (x86_video_register)-1;
    }
}

static void video_read(t_port *port, lib_u16 address, void *context)
{
    t_vadp *adapter = context;

    (void)x86_video_register_read(adapter->chip, video_register(address),
        &port->data.ioByte);
}

static void video_write(t_port *port, lib_u16 address, void *context)
{
    t_vadp *adapter = context;

    (void)x86_video_register_write(adapter->chip, video_register(address),
        port->data.ioByte);
}

static lib_status register_ports(t_vadp *adapter,
    const video_port_route *routes, lib_size count)
{
    for (lib_size index = 0u; index < count; ++index) {
        lib_status status = routes[index].write ?
            core_machine_port_add_write(adapter->port, routes[index].address,
                video_write, adapter) :
            core_machine_port_add_read(adapter->port, routes[index].address,
                video_read, adapter);

        if (status != LIB_STATUS_OK) return status;
    }
    return LIB_STATUS_OK;
}

static lib_status cga_read(void *context, lib_u32 address,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    return (observe_only ? x86_video_memory_inspect : x86_video_memory_read)(
        context, X86_VIDEO_MEMORY_CGA, address,
        (lib_u8 *)destination, bytes);
}

static lib_status cga_write(void *context, lib_u32 address,
    lib_uptr source, lib_uptr bytes)
{
    return x86_video_memory_write(context, X86_VIDEO_MEMORY_CGA, address,
        (const lib_u8 *)source, bytes);
}

static lib_status cga_query(void *context, lib_u32 address,
    lib_uptr bytes, core_machine_memory_access access)
{
    if (access != CORE_MACHINE_MEMORY_ACCESS_READ &&
        access != CORE_MACHINE_MEMORY_ACCESS_WRITE) return LIB_STATUS_UNSUPPORTED;
    return x86_video_memory_query(context, X86_VIDEO_MEMORY_CGA, address,
        bytes, access == CORE_MACHINE_MEMORY_ACCESS_WRITE);
}

static lib_status planar_read(void *context, lib_u32 address,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    return (observe_only ? x86_video_memory_inspect : x86_video_memory_read)(
        context, X86_VIDEO_MEMORY_PLANAR, address,
        (lib_u8 *)destination, bytes);
}

static lib_status planar_write(void *context, lib_u32 address,
    lib_uptr source, lib_uptr bytes)
{
    return x86_video_memory_write(context, X86_VIDEO_MEMORY_PLANAR, address,
        (const lib_u8 *)source, bytes);
}

static lib_status planar_query(void *context, lib_u32 address,
    lib_uptr bytes, core_machine_memory_access access)
{
    if (access != CORE_MACHINE_MEMORY_ACCESS_READ &&
        access != CORE_MACHINE_MEMORY_ACCESS_WRITE) return LIB_STATUS_UNSUPPORTED;
    return x86_video_memory_query(context, X86_VIDEO_MEMORY_PLANAR, address,
        bytes, access == CORE_MACHINE_MEMORY_ACCESS_WRITE);
}

static void notify_write(void *context, lib_u32 address, lib_uptr bytes)
{
    x86_video_notify_memory_write(context, address, bytes);
}

lib_status core_machine_vadp_initialize(t_vadp *adapter, t_port *port)
{
    core_machine_port_provider_entry *checkpoint;
    lib_status status;

    if (adapter == LIB_NULL || port == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(adapter, 0u, sizeof(*adapter));
    status = core_machine_port_registration_status(port);
    if (status != LIB_STATUS_OK) return status;
    status = x86_video_create(&adapter->chip);
    if (status != LIB_STATUS_OK) return status;
    adapter->port = port;
    checkpoint = core_machine_port_registration_begin(port);
    status = register_ports(adapter, cga_ports, sizeof(cga_ports) / sizeof(cga_ports[0]));
    if (status != LIB_STATUS_OK) {
        core_machine_port_rollback_registration(port, checkpoint);
        x86_video_destroy(adapter->chip);
        adapter->chip = LIB_NULL;
        adapter->port = LIB_NULL;
    }
    return status;
}

lib_status core_machine_vadp_configure(t_vadp *adapter, t_ram *memory,
    const core_machine_display_config *config)
{
    x86_video *candidate = LIB_NULL;
    core_machine_port_provider_entry *checkpoint;
    lib_status status;

    if (adapter == LIB_NULL || adapter->chip == LIB_NULL || memory == LIB_NULL ||
        config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (adapter->memory != LIB_NULL) return LIB_STATUS_INVALID_STATE;
    if (config->vga_present && (!config->ega_present ||
        config->ega_personality != X86_VIDEO_EGA_PERSONALITY_GENERIC))
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_video_create(&candidate);
    if (status != LIB_STATUS_OK) return status;
    checkpoint = core_machine_port_registration_begin(adapter->port);
    status = x86_video_configure_text_timing(candidate, &config->text_timing);
    if (status == LIB_STATUS_OK)
        status = x86_video_configure_text_glyphs(candidate, &config->text_glyphs);
    if (status == LIB_STATUS_OK && config->cga_vram_present)
        status = x86_video_configure_cga_memory(candidate);
    if (status == LIB_STATUS_OK && config->ega_present) {
        status = x86_video_configure_ega_sequencer(candidate, &config->ega_sequencer);
        if (status == LIB_STATUS_OK)
            status = x86_video_configure_ega_controllers(candidate, &config->ega_controllers);
        if (status == LIB_STATUS_OK)
            status = x86_video_configure_ega_personality(candidate, config->ega_personality);
        if (status == LIB_STATUS_OK && config->ega_personality ==
                X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR)
            status = x86_video_configure_cecg(candidate, &config->cecg);
    }
    if (status == LIB_STATUS_OK && config->vga_present)
        status = x86_video_configure_vga(candidate);
    /* Prepare the complete chip before publishing any memory route. Port
     * callbacks still address the old instance until successful publication. */
    if (status == LIB_STATUS_OK && config->cga_vram_present)
        status = core_machine_memory_register_device_provider(memory,
            CORE_MACHINE_VADP_VIDEO_BASE, CORE_MACHINE_VADP_VIDEO_BYTES,
            cga_read, cga_write, cga_query, candidate);
    if (status == LIB_STATUS_OK && config->ega_present) {
        status = config->ega_sequencer.planar_ega ?
            core_machine_memory_register_device_provider_and_write_observer(memory,
                CORE_MACHINE_VADP_EGA_APERTURE_BASE,
                CORE_MACHINE_VADP_EGA_CPU_DECODE_BYTES, planar_read, planar_write,
                planar_query, candidate, notify_write) :
            core_machine_memory_register_write_observer(memory, notify_write, candidate);
        if (status == LIB_STATUS_OK)
            status = register_ports(adapter, ega_ports,
                sizeof(ega_ports) / sizeof(ega_ports[0]));
        if (status == LIB_STATUS_OK) {
            const lib_bool compaq = config->ega_personality ==
                X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
            status = register_ports(adapter, compaq ? compaq_ports : generic_ports,
                compaq ? sizeof(compaq_ports) / sizeof(compaq_ports[0]) :
                sizeof(generic_ports) / sizeof(generic_ports[0]));
        }
    }
    if (status == LIB_STATUS_OK && config->vga_present)
        status = register_ports(adapter, vga_ports,
            sizeof(vga_ports) / sizeof(vga_ports[0]));
    if (status != LIB_STATUS_OK) {
        core_machine_memory_unregister_owner(memory, candidate);
        core_machine_port_rollback_registration(adapter->port, checkpoint);
        x86_video_destroy(candidate);
        return status;
    }
    x86_video_destroy(adapter->chip);
    adapter->chip = candidate;
    adapter->memory = memory;
    return LIB_STATUS_OK;
}

void core_machine_vadp_finalize(t_vadp *adapter)
{
    if (adapter == LIB_NULL) return;
    core_machine_port_unregister_owner(adapter->port, adapter);
    core_machine_memory_unregister_owner(adapter->memory, adapter->chip);
    x86_video_destroy(adapter->chip);
    lib_memory_set(adapter, 0u, sizeof(*adapter));
}

static lib_status read_backing(void *context, lib_u32 address,
    lib_u8 *destination, lib_size bytes)
{
    return core_machine_memory_inspect_physical(context, address,
        (lib_uptr)destination, bytes, LIB_FALSE);
}

lib_i32 core_machine_vadp_capture_snapshot(t_vadp *adapter, t_ram *memory,
    x86_video_snapshot *out_snapshot)
{
    const x86_video_memory_reader reader = { read_backing, memory };

    return adapter != LIB_NULL && x86_video_capture_snapshot_from(adapter->chip,
        memory == LIB_NULL ? LIB_NULL : &reader, out_snapshot);
}

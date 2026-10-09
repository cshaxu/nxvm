/* Copyright 2012-2026 Neko. */
#include "core/board-base/vadp.h"
#include "core/x86/port_interface.h"
#include "core/x86/memory_interface.h"

lib_status core_machine_vadp_create(core_machine *machine, t_vadp **out_adapter)
{
    t_vadp *adapter;
    lib_status status;
    if (out_adapter == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_adapter = LIB_NULL;
    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    adapter = lib_allocate_zero(1u, sizeof(*adapter));
    if (adapter == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = core_machine_vadp_initialize(adapter, machine);
    if (status != LIB_STATUS_OK) {
        lib_release(adapter);
        return status;
    }
    *out_adapter = adapter;
    return LIB_STATUS_OK;
}

void core_machine_vadp_destroy(t_vadp *adapter)
{
    core_machine_vadp_finalize(adapter);
    lib_release(adapter);
}

void core_machine_vadp_reset(t_vadp *adapter)
{
    if (adapter != LIB_NULL) x86_video_reset(adapter->chip);
}

void core_machine_vadp_advance(t_vadp *adapter, lib_u64 ticks)
{
    if (adapter != LIB_NULL) x86_video_advance(adapter->chip, ticks);
}

void core_machine_vadp_observe_snapshot(const t_vadp *adapter,
    lib_u8 acknowledged_generation_valid, lib_u64 acknowledged_generation,
    x86_video_snapshot_observation *out_observation)
{
    x86_video_observe_snapshot(adapter == LIB_NULL ? LIB_NULL : adapter->chip,
        acknowledged_generation_valid, acknowledged_generation, out_observation);
}

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

static lib_status video_read(void *context, lib_u16 address, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    t_vadp *adapter = context;
    lib_u8 value = (lib_u8)*out_value;
    lib_status status = x86_video_register_read(adapter->chip,
        video_register(address), &value);

    if (status == LIB_STATUS_OK) *out_value = (*out_value & ~0xffu) | value;
    return status;
}

static lib_status video_write(void *context, lib_u16 address, lib_u32 value)
{
    t_vadp *adapter = context;

    return x86_video_register_write(adapter->chip, video_register(address),
        (lib_u8)value);
}

static lib_size append_ports(core_machine_port_route *routes, lib_size offset,
    const video_port_route *ports, lib_size count, t_vadp *adapter)
{
    for (lib_size index = 0u; index < count; ++index) {
        routes[offset + index] = (core_machine_port_route) {
            .address = ports[index].address,
            .read = ports[index].write ? LIB_NULL : video_read,
            .write = ports[index].write ? video_write : LIB_NULL,
            .owner = adapter};
    }
    return offset + count;
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

lib_status core_machine_vadp_initialize(t_vadp *adapter, core_machine *machine)
{
    core_machine_port_route routes[sizeof(cga_ports) / sizeof(cga_ports[0])];
    lib_status status;

    if (adapter == LIB_NULL || machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(adapter, 0u, sizeof(*adapter));
    status = x86_video_create(&adapter->chip);
    if (status != LIB_STATUS_OK) return status;
    adapter->machine = machine;
    (void)append_ports(routes, 0u, cga_ports,
        sizeof(cga_ports) / sizeof(cga_ports[0]), adapter);
    status = core_machine_install_port_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]));
    if (status != LIB_STATUS_OK) {
        x86_video_destroy(adapter->chip);
        adapter->chip = LIB_NULL;
        adapter->machine = LIB_NULL;
    }
    return status;
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

lib_bool core_machine_vadp_config_is_valid(const core_machine_display_config *config)
{
    return core_machine_display_ports_are_vadp(config) &&
        (!config->ega_present || config->ega_personality !=
        X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR ||
        x86_video_cecg_config_is_valid(&config->cecg));
}

lib_status core_machine_vadp_configure(t_vadp *adapter,
    const core_machine_display_config *config)
{
    x86_video *candidate = LIB_NULL;
    core_machine_memory_device_route memory_routes[2];
    lib_size memory_route_count = 0u;
    core_machine_port_route routes[
        sizeof(ega_ports) / sizeof(ega_ports[0]) +
        sizeof(compaq_ports) / sizeof(compaq_ports[0]) +
        sizeof(vga_ports) / sizeof(vga_ports[0])];
    lib_size route_count = 0u;
    lib_status status;

    if (adapter == LIB_NULL || adapter->chip == LIB_NULL ||
        config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (adapter->configured) return LIB_STATUS_INVALID_STATE;
    if (config->vga_present && (!config->ega_present ||
        config->ega_personality != X86_VIDEO_EGA_PERSONALITY_GENERIC))
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_video_create(&candidate);
    if (status != LIB_STATUS_OK) return status;
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
    /* Prepare the complete chip before publishing either memory or ports. */
    if (status == LIB_STATUS_OK && config->cga_vram_present)
        memory_routes[memory_route_count++] = (core_machine_memory_device_route) {
            CORE_MACHINE_VADP_VIDEO_BASE, CORE_MACHINE_VADP_VIDEO_BYTES,
            { cga_read, cga_write, cga_query },
            CORE_MACHINE_MEMORY_PROVIDER_STANDARD };
    if (status == LIB_STATUS_OK && config->ega_present) {
        if (config->ega_sequencer.planar_ega)
            memory_routes[memory_route_count++] = (core_machine_memory_device_route) {
                CORE_MACHINE_VADP_EGA_APERTURE_BASE,
                CORE_MACHINE_VADP_EGA_CPU_DECODE_BYTES,
                { planar_read, planar_write, planar_query },
                CORE_MACHINE_MEMORY_PROVIDER_STANDARD };
        {
            const lib_bool compaq = config->ega_personality ==
                X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
            route_count = append_ports(routes, route_count, ega_ports,
                sizeof(ega_ports) / sizeof(ega_ports[0]), adapter);
            route_count = append_ports(routes, route_count,
                compaq ? compaq_ports : generic_ports,
                compaq ? sizeof(compaq_ports) / sizeof(compaq_ports[0]) :
                sizeof(generic_ports) / sizeof(generic_ports[0]), adapter);
        }
    }
    if (status == LIB_STATUS_OK && config->vga_present)
        route_count = append_ports(routes, route_count, vga_ports,
            sizeof(vga_ports) / sizeof(vga_ports[0]), adapter);
    if (status == LIB_STATUS_OK && (memory_route_count != 0u || config->ega_present))
        status = core_machine_install_memory_device_routes(adapter->machine,
            memory_routes, memory_route_count,
            config->ega_present ? notify_write : LIB_NULL, LIB_NULL, candidate);
    if (status == LIB_STATUS_OK && route_count != 0u)
        status = core_machine_install_port_routes(adapter->machine, routes, route_count);
    if (status != LIB_STATUS_OK) {
        (void)core_machine_remove_memory_device_routes(adapter->machine, candidate);
        x86_video_destroy(candidate);
        return status;
    }
    x86_video_destroy(adapter->chip);
    adapter->chip = candidate;
    adapter->configured = LIB_TRUE;
    return LIB_STATUS_OK;
}

void core_machine_vadp_finalize(t_vadp *adapter)
{
    if (adapter == LIB_NULL) return;
    if (adapter->machine != LIB_NULL)
        (void)core_machine_remove_port_routes(adapter->machine, adapter);
    if (adapter->configured)
        (void)core_machine_remove_memory_device_routes(adapter->machine, adapter->chip);
    x86_video_destroy(adapter->chip);
    lib_memory_set(adapter, 0u, sizeof(*adapter));
}

static lib_status read_backing(void *context, lib_u32 address,
    lib_u8 *destination, lib_size bytes)
{
    return core_machine_memory_inspect(context, address, destination, bytes);
}

lib_status core_machine_vadp_observe_bus(const t_vadp *adapter,
    x86_video_bus_observation *out_observation)
{
    return adapter == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        x86_video_observe_bus(adapter->chip, out_observation);
}

lib_i32 core_machine_vadp_capture_snapshot(t_vadp *adapter,
    x86_video_snapshot *out_snapshot)
{
    const x86_video_memory_reader reader = { read_backing,
        adapter == LIB_NULL ? LIB_NULL : adapter->machine };

    return adapter != LIB_NULL && x86_video_capture_snapshot_from(adapter->chip,
        adapter->machine == LIB_NULL ? LIB_NULL : &reader, out_snapshot);
}

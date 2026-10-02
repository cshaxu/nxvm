#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/vadp.h"
#include "app-nxvm/devices/machine_interface.h"

static void ega_write_crtc(t_port *port, lib_u16 index_port,
    lib_u8 index, lib_u8 value)
{
    core_machine_port_write(port, index_port, index);
    core_machine_port_write(port, index_port + 1u, value);
}

static lib_u8 ega_read_crtc(t_port *port, lib_u16 index_port,
    lib_u8 index)
{
    core_machine_port_write(port, index_port, index);
    return core_machine_port_read(port, index_port + 1u);
}

lib_i32 main(void)
{
    const x86_video_ega_sequencer_config sequencer = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE
    };
    const x86_video_ega_controller_config controllers = {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    core_machine machine = {.lifecycle = CORE_MACHINE_INITIALIZED};
    t_port *port = &machine.executor_port;
    t_ram memory;
    t_vadp vadp;
    x86_video_snapshot snapshot;
    x86_video_snapshot_observation observation;
    const lib_u8 chain4_bytes[] = { 0x10u, 0x11u, 0x12u, 0x13u };
    lib_u64 vga_generation;
    lib_i32 failed = 0;
    core_machine_display_config config = {
        .text_timing = {48u, 8u, 8u}, .ega_present = LIB_TRUE,
        .vga_present = LIB_TRUE
    };

    lib_memory_set(&memory, 0, sizeof(memory));
    core_machine_port_initialize(port);
    failed |= core_machine_memory_initialize_for(&memory, 16u * 1024u * 1024u, LIB_NULL) != LIB_STATUS_OK;
    failed |= core_machine_vadp_initialize(&vadp, &machine) != LIB_STATUS_OK;
    config.ega_sequencer = sequencer;
    config.ega_controllers = controllers;
    failed |= core_machine_vadp_configure(&vadp, &memory, &config) != LIB_STATUS_OK;
    x86_video_reset(vadp.chip);
    failed |= !core_machine_port_has_read(port,
        CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0) ||
        !core_machine_port_has_read(port, CORE_MACHINE_VADP_PORT_VGA_DAC_MASK) ||
        !core_machine_port_has_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA) ||
        !core_machine_port_has_write(port,
        CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT) ||
        !core_machine_port_has_read(port, CORE_MACHINE_VADP_PORT_MONO_STATUS) ||
        !core_machine_port_has_write(port,
        CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_MONO) ||
        !core_machine_port_has_write(port,
        CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_COLOR) ||
        (core_machine_port_read(port,
        CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0) & 0x7fu) != 0u;

    ega_write_crtc(port, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX, 0x0eu, 0x12u);
    failed |= ega_read_crtc(port, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX, 0x0eu) !=
        0x12u || ega_read_crtc(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x0eu) !=
        0u;
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT,
        0x01u);
    ega_write_crtc(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x0eu, 0x34u);
    failed |= ega_read_crtc(port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x0eu) !=
        0x34u || ega_read_crtc(port, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX,
        0x0eu) != 0u;
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_MONO,
        0x03u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_COLOR,
        0x02u);
    /* Feature-control storage is verified by the independent chip case. */

    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_WRITE_INDEX, 2u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA, 0x7fu);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA, 0x15u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA, 0x2au);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_READ_INDEX, 2u);
    failed |= core_machine_port_read(port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA) !=
        0x3fu || core_machine_port_read(port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA) !=
        0x15u || core_machine_port_read(port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA) !=
        0x2au;
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_MASK, 0xa5u);
    failed |= core_machine_port_read(port, CORE_MACHINE_VADP_PORT_VGA_DAC_MASK) !=
        0xa5u;

    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX, 4u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_SEQUENCER_DATA, 0x0eu);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 5u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x40u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
    (void)core_machine_port_read(port, CORE_MACHINE_VADP_PORT_STATUS);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x30u);
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x01u);
    failed |= core_machine_memory_write_physical(&memory,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, (lib_uptr)chain4_bytes,
        sizeof(chain4_bytes)) != LIB_STATUS_OK;
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_VGA_320X200X256 ||
        snapshot.pixel_width != 320u || snapshot.pixel_height != 200u ||
        snapshot.pixels[0] != 0x10u || snapshot.pixels[1] != 0x11u ||
        snapshot.pixels[2] != 0x12u || snapshot.pixels[3] != 0x13u;
    x86_video_observe_snapshot(vadp.chip, LIB_FALSE, 0u, &observation);
    vga_generation = observation.generation;
    x86_video_observe_snapshot(vadp.chip, LIB_TRUE, vga_generation, &observation);
    failed |= !observation.generation_reliable || observation.capture_required;
    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA, 0x1fu);
    x86_video_observe_snapshot(vadp.chip, LIB_TRUE, vga_generation, &observation);
    failed |= !observation.generation_reliable || !observation.capture_required ||
        observation.generation == vga_generation;

    core_machine_port_write(port, CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x00u);
    (void)core_machine_port_read(port, CORE_MACHINE_VADP_PORT_STATUS);
    x86_video_reset(vadp.chip);
    failed |= core_machine_port_read(port, CORE_MACHINE_VADP_PORT_VGA_DAC_MASK) != 0xffu ||
        ega_read_crtc(port, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX, 0x0eu) !=
        0u;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(port);
    if (failed) {
        fprintf(stderr, "M5:T466:S2:EGA-EXTERNAL-PORT:FAIL\n");
        return 1;
    }
    printf("M5:T466:S2:EGA-EXTERNAL-PORT:OK\n");
    printf("M5:T480:S4:DAC:OK\n");
    printf("M5:T480:S4:CHAIN4:OK\n");
    printf("M5:T480:S4:SNAPSHOT:OK\n");
    return 0;
}

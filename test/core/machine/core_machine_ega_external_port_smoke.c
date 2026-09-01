#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_VOID ega_write_crtc(t_port *port, type_unsigned_16 index_port,
    type_unsigned_8 index, type_unsigned_8 value)
{
    core_machine_port_write(port, index_port, index);
    core_machine_port_write(port, index_port + 1u, value);
}

static type_unsigned_8 ega_read_crtc(t_port *port, type_unsigned_16 index_port,
    type_unsigned_8 index)
{
    core_machine_port_write(port, index_port, index);
    return core_machine_port_read(port, index_port + 1u);
}

C_INT main(C_VOID)
{
    const core_machine_vadp_ega_sequencer_config sequencer = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, TYPE_TRUE
    };
    const core_machine_vadp_ega_controller_config controllers = {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    t_port port;
    t_ram memory;
    t_vadp vadp;
    core_machine_display_snapshot snapshot;
    core_machine_display_snapshot_observation observation;
    const type_unsigned_8 chain4_bytes[] = { 0x10u, 0x11u, 0x12u, 0x13u };
    type_unsigned_64 vga_generation;
    C_INT failed = 0;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    failed |= core_machine_vadp_configure_ega_sequencer(&vadp, &memory,
        &sequencer) != TYPE_STATUS_OK ||
        core_machine_vadp_configure_ega_controllers(&vadp, &controllers) !=
        TYPE_STATUS_OK || core_machine_vadp_configure_ega_personality(&vadp,
        &port, CORE_MACHINE_VADP_EGA_PERSONALITY_GENERIC) != TYPE_STATUS_OK ||
        core_machine_vadp_configure_vga(&vadp, &port) != TYPE_STATUS_OK;
    core_machine_vadp_reset(&vadp);
    failed |= !core_machine_port_has_read(&port,
        CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0) ||
        !core_machine_port_has_read(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_MASK) ||
        !core_machine_port_has_write(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA) ||
        !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT) ||
        !core_machine_port_has_read(&port, CORE_MACHINE_VADP_PORT_MONO_STATUS) ||
        !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_MONO) ||
        !core_machine_port_has_write(&port,
        CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_COLOR) ||
        (core_machine_port_read(&port,
        CORE_MACHINE_VADP_PORT_EGA_INPUT_STATUS_0) & 0x7fu) != 0u;

    ega_write_crtc(&port, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX, 0x0eu, 0x12u);
    failed |= ega_read_crtc(&port, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX, 0x0eu) !=
        0x12u || ega_read_crtc(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x0eu) !=
        0u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_EGA_MISCELLANEOUS_OUTPUT,
        0x01u);
    ega_write_crtc(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x0eu, 0x34u);
    failed |= ega_read_crtc(&port, CORE_MACHINE_VADP_PORT_CRTC_INDEX, 0x0eu) !=
        0x34u || ega_read_crtc(&port, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX,
        0x0eu) != 0u;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_MONO,
        0x03u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_EGA_FEATURE_CONTROL_COLOR,
        0x02u);
    failed |= vadp.data.ega_feature_control != 0x02u;

    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_WRITE_INDEX, 2u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA, 0x7fu);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA, 0x15u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA, 0x2au);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_READ_INDEX, 2u);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA) !=
        0x3fu || core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA) !=
        0x15u || core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA) !=
        0x2au;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_MASK, 0xa5u);
    failed |= core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_MASK) !=
        0xa5u;

    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_SEQUENCER_INDEX, 4u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_SEQUENCER_DATA, 0x0eu);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 5u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x40u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_INDEX, 6u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_GRAPHICS_DATA, 0x05u);
    (C_VOID)core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x30u);
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x01u);
    failed |= core_machine_memory_write_physical(&memory,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, (type_virtual_address)chain4_bytes,
        sizeof(chain4_bytes)) != TYPE_STATUS_OK;
    STD_MEMSET(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_VGA_320X200X256 ||
        snapshot.pixel_width != 320u || snapshot.pixel_height != 200u ||
        snapshot.pixels[0] != 0x10u || snapshot.pixels[1] != 0x11u ||
        snapshot.pixels[2] != 0x12u || snapshot.pixels[3] != 0x13u;
    core_machine_vadp_observe_snapshot(&vadp, TYPE_FALSE, 0u, &observation);
    vga_generation = observation.generation;
    core_machine_vadp_observe_snapshot(&vadp, TYPE_TRUE, vga_generation, &observation);
    failed |= !observation.generation_reliable || observation.capture_required;
    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_DATA, 0x1fu);
    core_machine_vadp_observe_snapshot(&vadp, TYPE_TRUE, vga_generation, &observation);
    failed |= !observation.generation_reliable || !observation.capture_required ||
        observation.generation == vga_generation;

    core_machine_port_write(&port, CORE_MACHINE_VADP_PORT_ATTRIBUTE, 0x00u);
    failed |= !vadp.data.attribute_data_phase;
    (C_VOID)core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_STATUS);
    failed |= vadp.data.attribute_data_phase;
    core_machine_vadp_reset(&vadp);
    failed |= vadp.data.ega_miscellaneous_output != 0u ||
        vadp.data.ega_feature_control != 0u ||
        core_machine_port_read(&port, CORE_MACHINE_VADP_PORT_VGA_DAC_MASK) != 0xffu ||
        ega_read_crtc(&port, CORE_MACHINE_VADP_PORT_MONO_CRTC_INDEX, 0x0eu) !=
        0u;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T466:S2:EGA-EXTERNAL-PORT:FAIL\n");
        return 1;
    }
    STD_PRINTF("M5:T466:S2:EGA-EXTERNAL-PORT:OK\n");
    STD_PRINTF("M5:T480:S4:DAC:OK\n");
    STD_PRINTF("M5:T480:S4:CHAIN4:OK\n");
    STD_PRINTF("M5:T480:S4:SNAPSHOT:OK\n");
    return 0;
}

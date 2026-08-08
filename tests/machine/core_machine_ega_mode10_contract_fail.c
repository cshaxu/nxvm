#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

#define T284_EGA_MODE10_WIDTH 640u
#define T284_EGA_MODE10_HEIGHT 350u
#define T284_EGA_MODE10_ROW_BYTES 80u
#define T284_EGA_MODE10_LAST_ROW_OFFSET \
    ((T284_EGA_MODE10_HEIGHT - 1u) * T284_EGA_MODE10_ROW_BYTES)

static C_INT t284_write_byte(t_ram *memory, uint32_t physical, uint8_t value)
{
    return core_machine_memory_write_physical(memory, physical,
        (type_virtual_address)&value, sizeof(value)) == TYPE_STATUS_OK;
}

static C_INT t284_read_byte(t_ram *memory, uint32_t physical, uint8_t *value)
{
    return core_machine_memory_read_physical(memory, physical,
        (type_virtual_address)value, sizeof(*value)) == TYPE_STATUS_OK;
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
    uint8_t value = 0u;
    C_INT has_640x350_frame;
    C_INT has_crtc_offset;
    C_INT has_tail_row_plane;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    if (core_machine_vadp_configure_ega_sequencer(&vadp, &memory, &sequencer) !=
            TYPE_STATUS_OK ||
        core_machine_vadp_configure_ega_controllers(&vadp, &controllers) !=
            TYPE_STATUS_OK) {
        STD_FPRINTF(STD_STDERR,
            "M5:T284:S2:EGA-MODE10:EXPECTED-FAIL setup\n");
        return 1;
    }

    core_machine_port_write(&port, 0x03d4u, 0x13u);
    core_machine_port_write(&port, 0x03d5u, 0x28u);
    core_machine_port_write(&port, 0x03d4u, 0x13u);
    has_crtc_offset = core_machine_port_read(&port, 0x03d5u) == 0x28u;

    core_machine_port_write(&port, 0x03c4u, 2u);
    core_machine_port_write(&port, 0x03c5u, 0x0fu);
    core_machine_port_write(&port, 0x03ceu, 6u);
    core_machine_port_write(&port, 0x03cfu, 0x05u);
    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x30u);
    core_machine_port_write(&port, 0x03c0u, 0x01u);
    has_tail_row_plane = t284_write_byte(&memory,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE + T284_EGA_MODE10_LAST_ROW_OFFSET,
        0xf0u) &&
        t284_read_byte(&memory,
            CORE_MACHINE_VADP_EGA_APERTURE_BASE + T284_EGA_MODE10_LAST_ROW_OFFSET,
            &value) && value == 0xf0u;

    STD_MEMSET(&snapshot, 0, sizeof(snapshot));
    has_640x350_frame = core_machine_vadp_capture_snapshot(&vadp, &memory,
        &snapshot) && snapshot.pixel_width == T284_EGA_MODE10_WIDTH &&
        snapshot.pixel_height == T284_EGA_MODE10_HEIGHT &&
        CORE_MACHINE_DISPLAY_MAX_PIXELS >=
            T284_EGA_MODE10_WIDTH * T284_EGA_MODE10_HEIGHT;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);

    if (has_640x350_frame && has_crtc_offset && has_tail_row_plane) {
        STD_PRINTF("M5:T284:S2:EGA-MODE10:CONTRACT:READY\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR,
        "M5:T284:S2:EGA-MODE10:EXPECTED-FAIL frame=%ux%u max=%u crtc-offset=%d tail-row=%d\n",
        (C_UINT)snapshot.pixel_width, (C_UINT)snapshot.pixel_height,
        (C_UINT)CORE_MACHINE_DISPLAY_MAX_PIXELS, has_crtc_offset,
        has_tail_row_plane);
    return 1;
}

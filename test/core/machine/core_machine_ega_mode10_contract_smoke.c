#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

#define T285_EGA_MODE10_WIDTH 640u
#define T285_EGA_MODE10_HEIGHT 350u
#define T285_EGA_MODE10_ROW_BYTES 80u
#define T285_EGA_MODE10_START_WORD 1u
#define T285_EGA_MODE10_START_BYTE (T285_EGA_MODE10_START_WORD * 2u)
#define T285_EGA_MODE10_LAST_ROW_OFFSET \
    (T285_EGA_MODE10_START_BYTE + (T285_EGA_MODE10_HEIGHT - 1u) * \
        T285_EGA_MODE10_ROW_BYTES)

static C_INT t285_write_byte(t_ram *memory, type_unsigned_32 physical, type_unsigned_8 value)
{
    return core_machine_memory_write_physical(memory, physical,
        (type_virtual_address)&value, sizeof(value)) == TYPE_STATUS_OK;
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
    type_unsigned_8 plane;
    C_INT failed = 0;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    if (core_machine_vadp_configure_ega_sequencer(&vadp, &memory, &sequencer) !=
            TYPE_STATUS_OK ||
        core_machine_vadp_configure_ega_controllers(&vadp, &controllers) !=
            TYPE_STATUS_OK) {
        failed = 1;
    }

    core_machine_port_write(&port, 0x03d4u, 0x13u);
    core_machine_port_write(&port, 0x03d5u, 0x28u);
    core_machine_port_write(&port, 0x03d4u, 0x13u);
    failed |= core_machine_port_read(&port, 0x03d5u) != 0u ||
        vadp.data.crtc[0x13u] != 0x28u;
    core_machine_port_write(&port, 0x03d4u, 0x01u);
    core_machine_port_write(&port, 0x03d5u, 0x4fu);
    core_machine_port_write(&port, 0x03d4u, 0x07u);
    core_machine_port_write(&port, 0x03d5u, 0x02u);
    core_machine_port_write(&port, 0x03d4u, 0x12u);
    core_machine_port_write(&port, 0x03d5u, 0x5du);
    core_machine_port_write(&port, 0x03d4u, 0x0cu);
    core_machine_port_write(&port, 0x03d5u, 0x00u);
    core_machine_port_write(&port, 0x03d4u, 0x0du);
    core_machine_port_write(&port, 0x03d5u, T285_EGA_MODE10_START_WORD);

    core_machine_port_write(&port, 0x03c4u, 2u);
    core_machine_port_write(&port, 0x03c5u, 0x0fu);
    core_machine_port_write(&port, 0x03ceu, 6u);
    core_machine_port_write(&port, 0x03cfu, 0x05u);
    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x30u);
    core_machine_port_write(&port, 0x03c0u, 0x01u);
    for (plane = 0u; plane < CORE_MACHINE_VADP_EGA_PLANES; ++plane) {
        core_machine_port_write(&port, 0x03c4u, 2u);
        core_machine_port_write(&port, 0x03c5u, (type_unsigned_8)(1u << plane));
        failed |= !t285_write_byte(&memory,
            CORE_MACHINE_VADP_EGA_APERTURE_BASE + T285_EGA_MODE10_START_BYTE,
            0x80u);
        failed |= !t285_write_byte(&memory,
            CORE_MACHINE_VADP_EGA_APERTURE_BASE + T285_EGA_MODE10_LAST_ROW_OFFSET,
            0x80u);
    }
    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x2fu);
    core_machine_port_write(&port, 0x03c0u, 0x0cu);

    STD_MEMSET(&snapshot, 0, sizeof(snapshot));
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot);
    failed |= snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16 ||
        snapshot.pixel_width != T285_EGA_MODE10_WIDTH ||
        snapshot.pixel_height != T285_EGA_MODE10_HEIGHT ||
        CORE_MACHINE_DISPLAY_MAX_PIXELS <
            T285_EGA_MODE10_WIDTH * T285_EGA_MODE10_HEIGHT ||
        snapshot.pixels[0] != 15u ||
        snapshot.pixels[(T285_EGA_MODE10_HEIGHT - 1u) * T285_EGA_MODE10_WIDTH] != 15u ||
        snapshot.palette_rgb[15] != 0xff5555u;

    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x0fu);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16 ||
        snapshot.pixel_width != T285_EGA_MODE10_WIDTH ||
        snapshot.pixel_height != T285_EGA_MODE10_HEIGHT || !snapshot.buffer_changed ||
        snapshot.pixels[0] != 0u ||
        snapshot.pixels[(T285_EGA_MODE10_HEIGHT - 1u) * T285_EGA_MODE10_WIDTH] != 0u;

    core_machine_port_write(&port, 0x03d4u, 0x01u);
    core_machine_port_write(&port, 0x03d5u, 0x00u);
    failed |= !core_machine_vadp_capture_snapshot(&vadp, &memory, &snapshot) ||
        snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT || !snapshot.buffer_changed ||
        snapshot.characters[0] != 0x20u || snapshot.attributes[0] != 0u;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);

    if (!failed) {
        STD_PRINTF("M5:T285:S3:EGA-MODE10:CONTRACT:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T285:S3:EGA-MODE10:CONTRACT:FAIL\n");
    return 1;
}

#include "type.h"

#include "core/machine/machine.h"
#include "core/platform/presentation_mailbox_interface.h"
#include "vm/composition/session/display.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

static C_VOID vm_display_s5_port_write(vm_session *session, type_unsigned_16 port,
    type_unsigned_8 value)
{
    core_machine_port_write(&session->core_machine->executor_port, port, value);
}

static C_INT vm_display_s5_capture(vm_session *session,
    core_platform_display_frame *frame, core_machine_display_kind expected_kind)
{
    return vm_session_publish_display(session, TYPE_TRUE) == expected_kind &&
        core_platform_presentation_mailbox_capture(session->presentation_mailbox,
            frame) == TYPE_STATUS_OK;
}

static C_INT vm_display_s5_enable_planar(vm_session *session)
{
    vm_display_s5_port_write(session, 0x03c4u, 2u);
    vm_display_s5_port_write(session, 0x03c5u, 0x0fu);
    vm_display_s5_port_write(session, 0x03ceu, 5u);
    vm_display_s5_port_write(session, 0x03cfu, 0x00u);
    vm_display_s5_port_write(session, 0x03ceu, 6u);
    vm_display_s5_port_write(session, 0x03cfu, 0x05u);
    vm_display_s5_port_write(session, 0x03c2u, 0x01u);
    (C_VOID)core_machine_port_read(&session->core_machine->executor_port, 0x03dau);
    vm_display_s5_port_write(session, 0x03c0u, 0x30u);
    vm_display_s5_port_write(session, 0x03c0u, 0x01u);
    vm_display_s5_port_write(session, 0x03d4u, 0x01u);
    vm_display_s5_port_write(session, 0x03d5u, 0x27u);
    vm_display_s5_port_write(session, 0x03d4u, 0x07u);
    vm_display_s5_port_write(session, 0x03d5u, 0x00u);
    vm_display_s5_port_write(session, 0x03d4u, 0x12u);
    vm_display_s5_port_write(session, 0x03d5u, 0xc7u);
    vm_display_s5_port_write(session, 0x03d4u, 0x13u);
    vm_display_s5_port_write(session, 0x03d5u, 0x14u);
    return 1;
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    core_platform_display_frame frame;
    core_machine_display_snapshot snapshot;
    core_machine_timeline_observation timeline;
    type_unsigned_8 cga_even = 0x1bu;
    type_unsigned_8 cga_odd = 0xe4u;
    type_unsigned_8 ega_pixel = 0xa5u;
    type_unsigned_64 text_generation;
    type_unsigned_64 cga_generation;
    C_INT failed = 0;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) {
        return 1;
    }
    STD_MEMSET(&frame, 0, sizeof(frame));
    failed |= !vm_display_s5_capture(session, &frame, CORE_MACHINE_DISPLAY_KIND_TEXT) ||
        frame.kind != CORE_PLATFORM_DISPLAY_KIND_TEXT || frame.columns != 80u ||
        frame.rows != 25u;
    text_generation = frame.generation;

    vm_display_s5_port_write(session, 0x03d8u, 0x0au);
    vm_display_s5_port_write(session, 0x03d9u, 0x00u);
    failed |= core_machine_memory_write(session->core_machine,
        CORE_MACHINE_VADP_VIDEO_BASE, &cga_even, sizeof(cga_even)) != TYPE_STATUS_OK ||
        core_machine_memory_write(session->core_machine,
            CORE_MACHINE_VADP_VIDEO_BASE + 0x2000u, &cga_odd,
            sizeof(cga_odd)) != TYPE_STATUS_OK;
    failed |= !vm_display_s5_capture(session, &frame,
        CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4) ||
        frame.kind != CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS ||
        frame.pixel_width != 320u || frame.pixel_height != 200u ||
        frame.pixels[0] != 0u || frame.pixels[1] != 1u ||
        frame.pixels[2] != 2u || frame.pixels[3] != 3u ||
        frame.pixels[320u] != 3u || frame.generation <= text_generation;
    cga_generation = frame.generation;
    cga_even = 0xffu;
    failed |= core_machine_memory_write(session->core_machine,
        CORE_MACHINE_VADP_VIDEO_BASE, &cga_even, sizeof(cga_even)) != TYPE_STATUS_OK ||
        core_platform_presentation_mailbox_capture(session->presentation_mailbox,
            &frame) != TYPE_STATUS_OK || frame.generation != cga_generation ||
        frame.pixels[0] != 0u || frame.pixels[1] != 1u || frame.pixels[2] != 2u;

    failed |= !vm_display_s5_enable_planar(session) ||
        core_machine_memory_write(session->core_machine,
            CORE_MACHINE_VADP_EGA_APERTURE_BASE, &ega_pixel,
            sizeof(ega_pixel)) != TYPE_STATUS_OK ||
        !vm_display_s5_capture(session, &frame,
            CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16) ||
        frame.kind != CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS ||
        frame.pixel_width != 320u || frame.pixel_height != 200u ||
        frame.pixels[0] != 15u || frame.pixels[1] != 0u ||
        frame.pixels[2] != 15u || frame.palette_rgb[15] != 0xffffffu ||
        frame.generation <= cga_generation;

    vm_session_reset(session);
    failed |= core_machine_get_timeline_observation(session->core_machine,
        &timeline) != TYPE_STATUS_OK || timeline.now != 0u ||
        timeline.pending_events != 0u ||
        core_platform_presentation_mailbox_capture(session->presentation_mailbox,
            &frame) != TYPE_STATUS_OK ||
        frame.kind != CORE_PLATFORM_DISPLAY_KIND_TEXT || frame.columns != 80u ||
        frame.rows != 25u || frame.pixel_width != 0u || frame.pixel_height != 0u ||
        frame.pixels[0] != 0u || frame.palette_rgb[15] != 0u ||
        core_machine_capture_display_snapshot(session->core_machine, &snapshot) !=
            TYPE_STATUS_OK || snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT;

    failed |= !vm_display_s5_enable_planar(session) ||
        !vm_display_s5_capture(session, &frame,
            CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16) ||
        frame.pixels[0] != 0u || frame.pixel_width != 320u ||
        frame.pixel_height != 200u;

    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T352:S5:DISPLAY-COMPOSITION:OK\n");
    return 0;
}

#include "app-nxvm/profiles/machine_factory_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "x86/ibmpc-common/machine_board_interface.h"
#include "x86/product/machine/machine_private.h"
#include "x86/product/machine/waiting.h"
#include "test/app-nxvm/integration/support/session_ini.h"

#define VM_CGA254_BOOT_BUDGET 500000u

static lib_status vm_cga254_boot_fixture(integration_ini_session *session,
    void *opaque)
{
    static const lib_u8 boot_code[] = {
        0x31u, 0xc0u, 0x8eu, 0xd8u,
        0xb8u, 0x06u, 0x00u, 0xcdu, 0x10u,
        0xb8u, 0x00u, 0xb8u, 0x8eu, 0xc0u, 0x31u, 0xffu,
        0xb0u, 0xa0u, 0xaau, 0xbfu, 0x00u, 0x20u, 0xb0u, 0x40u, 0xaau,
        0xb8u, 0x03u, 0x00u, 0xcdu, 0x10u,
        0xebu, 0xfeu
    };
    lib_u8 *image = LIB_NULL;
    lib_size image_size = 0u;
    lib_status status;

    (void)opaque;
    status = integration_ini_session_overlay_read(session, VM_MACHINE_MEDIA_FDD_ID,
        (void **)&image, &image_size);
    if (status == LIB_STATUS_OK && image_size >= 512u) {
        lib_memory_set(image, 0, 512u);
        lib_memory_copy(image, boot_code, sizeof(boot_code));
        image[510u] = 0x55u;
        image[511u] = 0xaau;
        status = integration_ini_session_overlay_write(session,
            VM_MACHINE_MEDIA_FDD_ID, image, image_size);
    } else if (status == LIB_STATUS_OK) {
        status = LIB_STATUS_INVALID_ARGUMENT;
    }
    lib_release(image);
    return status;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    integration_ini_session ini_session;
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    x86_video_snapshot snapshot;
    vm_machine *session;
    lib_u8 mode = 0u;
    lib_u32 instruction;
    lib_i32 saw_cga = 0;
    lib_i32 saw_text = 0;

    if (argc != 3 || integration_ini_session_open_with_overlay_transform(
            argv[1], argv[2], vm_cga254_boot_fixture, LIB_NULL,
            &ini_session) != LIB_STATUS_OK) return 77;
    session = ini_session.session;
    for (instruction = 0u; instruction < VM_CGA254_BOOT_BUDGET; ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) != LIB_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT ||
            core_machine_capture_display_snapshot(session->board,
                &snapshot) != LIB_STATUS_OK) goto done;
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            lib_i32 advanced = 0;

            if (vm_machine_waiting_advance(session, &result, &advanced) !=
                    LIB_STATUS_OK || !advanced) goto done;
        }
        if (snapshot.kind == X86_VIDEO_KIND_CGA_640X200X2 &&
            snapshot.pixels[0] == 1u && snapshot.pixels[1] == 0u &&
            snapshot.pixels[2] == 1u && snapshot.pixels[640u] == 0u &&
            snapshot.pixels[641u] == 1u && snapshot.palette_rgb[0] == 0u &&
            snapshot.palette_rgb[1] == 0xffffffu &&
            core_machine_memory_read(session->core_machine, 0x0449u, &mode,
                sizeof(mode)) == LIB_STATUS_OK && mode == 0x06u) {
            saw_cga = 1;
        }
        if (saw_cga && snapshot.kind == X86_VIDEO_KIND_TEXT &&
            core_machine_memory_read(session->core_machine, 0x0449u, &mode,
                sizeof(mode)) == LIB_STATUS_OK && mode == 0x03u) {
            saw_text = 1;
            break;
        }
    }

done:
    integration_ini_session_close(&ini_session);
    if (!saw_cga || !saw_text) return 1;
    printf("M5:T254:S3:CGA-640:SYSTEM:OK\n");
    return 0;
}

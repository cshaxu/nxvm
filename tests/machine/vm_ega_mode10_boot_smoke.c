#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define T285_MODE10_IMAGE_BYTES (1440u * 1024u)
#define T285_MODE10_BOOT_BUDGET 2000u

static type_unsigned_8 t285_mode10_image[T285_MODE10_IMAGE_BYTES];

static C_INT t285_mode10_write_fixture(C_CHAR path[MAX_PATH])
{
    static const type_unsigned_8 boot_code[] = {
        0x31u, 0xc0u, 0x8eu, 0xd8u,             /* xor ax,ax; mov ds,ax */
        0xb8u, 0x0du, 0x00u, 0xcdu, 0x10u,      /* mov ax,000dh; int 10h */
        0xb4u, 0x0fu, 0xcdu, 0x10u,             /* mov ah,0fh; int 10h */
        0xa3u, 0x00u, 0x05u,                    /* mov [0500h],ax */
        0xb8u, 0x0eu, 0x00u, 0xcdu, 0x10u,      /* mov ax,000eh; int 10h */
        0xb4u, 0x0fu, 0xcdu, 0x10u,             /* mov ah,0fh; int 10h */
        0xa3u, 0x02u, 0x05u,                    /* mov [0502h],ax */
        0xb8u, 0x10u, 0x00u, 0xcdu, 0x10u,      /* mov ax,0010h; int 10h */
        0xb4u, 0x0fu, 0xcdu, 0x10u,             /* mov ah,0fh; int 10h */
        0xa3u, 0x04u, 0x05u,                    /* mov [0504h],ax */
        0xb8u, 0x00u, 0xa0u, 0x8eu, 0xc0u,      /* mov ax,a000h; mov es,ax */
        0xbau, 0xc4u, 0x03u, 0xb0u, 0x02u, 0xeeu, 0x42u,
        0xb0u, 0x01u, 0xeeu, 0x31u, 0xffu, 0xb0u, 0x80u, 0xaau,
        0xbau, 0xc4u, 0x03u, 0xb0u, 0x02u, 0xeeu, 0x42u,
        0xb0u, 0x02u, 0xeeu, 0x31u, 0xffu, 0xb0u, 0x40u, 0xaau,
        0xbau, 0xc4u, 0x03u, 0xb0u, 0x02u, 0xeeu, 0x42u,
        0xb0u, 0x04u, 0xeeu, 0x31u, 0xffu, 0xb0u, 0x20u, 0xaau,
        0xbau, 0xc4u, 0x03u, 0xb0u, 0x02u, 0xeeu, 0x42u,
        0xb0u, 0x08u, 0xeeu, 0x31u, 0xffu, 0xb0u, 0x10u, 0xaau,
        0xb0u, 0x01u, 0xa2u, 0x06u, 0x05u,     /* mov al,1; mov [0506h],al */
        0xf4u, 0xebu, 0xfeu                      /* hlt; jmp $ */
    };
    STD_FILE *file;
    DWORD length;

    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) return 0;
    STD_MEMSET(t285_mode10_image, 0, sizeof(t285_mode10_image));
    STD_MEMCPY(t285_mode10_image, boot_code, sizeof(boot_code));
    t285_mode10_image[510u] = 0x55u;
    t285_mode10_image[511u] = 0xaau;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return 0;
    if (STD_FWRITE(t285_mode10_image, 1u, sizeof(t285_mode10_image), file) !=
            sizeof(t285_mode10_image)) {
        STD_FCLOSE(file);
        DeleteFileA(path);
        return 0;
    }
    STD_FCLOSE(file);
    return 1;
}

C_INT main(C_VOID)
{
    const vm_session_config config = {
        .fdd_image = "",
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_display_snapshot snapshot;
    vm_session *session = STD_NULL;
    C_CHAR path[MAX_PATH] = {0};
    type_unsigned_16 mode0d_query = 0u;
    type_unsigned_16 mode0e_query = 0u;
    type_unsigned_16 mode10_query = 0u;
    type_unsigned_8 bda_mode = 0u;
    type_unsigned_8 graphics_ready = 0u;
    type_unsigned_8 observed_pixels[4] = {0u};
    type_unsigned_32 instruction;
    C_INT saw_mode0d = 0;
    C_INT saw_mode0e = 0;
    C_INT saw_mode10 = 0;
    C_INT saw_write_frame = 0;
    C_INT saw_pixels = 0;

    if (!t285_mode10_write_fixture(path)) goto done;
    {
        vm_session_config fixture_config = config;
        fixture_config.fdd_image = path;
        if (vm_session_create(&fixture_config, &session) != TYPE_STATUS_OK ||
            session == STD_NULL) goto done;
    }
    for (instruction = 0u; instruction < T285_MODE10_BOOT_BUDGET; ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) !=
                TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) goto done;
        if (!saw_mode0d &&
            core_machine_memory_read(session->core_machine, 0x0500u, &mode0d_query,
                sizeof(mode0d_query)) == TYPE_STATUS_OK && mode0d_query == 0x500du &&
            core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) == TYPE_STATUS_OK &&
                snapshot.kind == CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16 &&
                snapshot.pixel_width == 320u && snapshot.pixel_height == 200u) {
            saw_mode0d = 1;
        }
        if (saw_mode0d && !saw_mode0e &&
            core_machine_memory_read(session->core_machine, 0x0502u, &mode0e_query,
                sizeof(mode0e_query)) == TYPE_STATUS_OK && mode0e_query == 0x500eu &&
            core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) == TYPE_STATUS_OK &&
                snapshot.kind == CORE_MACHINE_DISPLAY_KIND_EGA_640X200X16 &&
                snapshot.pixel_width == 640u && snapshot.pixel_height == 200u) {
            saw_mode0e = 1;
        }
        if (saw_mode0e && !saw_mode10 &&
            core_machine_memory_read(session->core_machine, 0x0449u, &bda_mode,
                sizeof(bda_mode)) == TYPE_STATUS_OK && bda_mode == 0x10u &&
            core_machine_memory_read(session->core_machine, 0x0504u, &mode10_query,
                sizeof(mode10_query)) == TYPE_STATUS_OK && mode10_query == 0x5010u &&
            core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) == TYPE_STATUS_OK &&
                snapshot.kind == CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16 &&
                snapshot.pixel_width == 640u && snapshot.pixel_height == 350u) {
            saw_mode10 = 1;
        }
        if (saw_mode10 && !saw_pixels &&
            core_machine_memory_read(session->core_machine, 0x0506u,
                &graphics_ready, sizeof(graphics_ready)) == TYPE_STATUS_OK &&
            graphics_ready == 1u &&
            core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) == TYPE_STATUS_OK &&
            snapshot.kind == CORE_MACHINE_DISPLAY_KIND_EGA_640X350X16) {
            saw_write_frame = 1;
            STD_MEMCPY(observed_pixels, snapshot.pixels, sizeof(observed_pixels));
            if (snapshot.pixels[0] == 1u && snapshot.pixels[1] == 2u &&
                snapshot.pixels[2] == 4u && snapshot.pixels[3] == 8u &&
                snapshot.palette_rgb[15] == 0xffffffu) {
                saw_pixels = 1;
            }
        }
        if (saw_pixels) break;
    }

done:
    if (session != STD_NULL && session->core_machine != STD_NULL) {
        (C_VOID)core_machine_memory_read(session->core_machine, 0x0449u, &bda_mode,
            sizeof(bda_mode));
        (C_VOID)core_machine_memory_read(session->core_machine, 0x0500u, &mode0d_query,
            sizeof(mode0d_query));
        (C_VOID)core_machine_memory_read(session->core_machine, 0x0502u, &mode0e_query,
            sizeof(mode0e_query));
        (C_VOID)core_machine_memory_read(session->core_machine, 0x0504u, &mode10_query,
            sizeof(mode10_query));
    }
    vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    if (saw_mode0d && saw_mode0e && saw_mode10 && saw_pixels) {
        STD_PRINTF("M5:T285:S3:EGA-MODE10:BOOT:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR,
        "M5:T285:S3:EGA-MODE10:BOOT:FAIL mode0d=%04x mode0e=%04x mode10=%04x bda=%02x ready=%u frame=%d pixels=%u/%u/%u/%u palette=%06x saw=%d/%d/%d/%d\n",
        mode0d_query, mode0e_query, mode10_query, bda_mode, graphics_ready, (C_INT)snapshot.kind,
        observed_pixels[0], observed_pixels[1], observed_pixels[2], observed_pixels[3],
        snapshot.palette_rgb[15], saw_mode0d, saw_mode0e, saw_mode10, saw_write_frame);
    return 1;
}

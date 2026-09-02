#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_CGA_GRAPHICS_IMAGE_BYTES (1440u * 1024u)
#define VM_CGA_GRAPHICS_BOOT_BUDGET 500000u

static type_unsigned_8 vm_cga_graphics_image[VM_CGA_GRAPHICS_IMAGE_BYTES];

static C_INT vm_cga_graphics_write_fixture(C_CHAR path[MAX_PATH])
{
    static const type_unsigned_8 boot_code[] = {
        0xb8u, 0x00u, 0xb8u,       /* mov ax, b800h */
        0x8eu, 0xc0u,              /* mov es, ax */
        0xbau, 0xd8u, 0x03u,       /* mov dx, 3d8h */
        0xb0u, 0x0au,              /* mov al, 0ah */
        0xeeu,                     /* out dx, al */
        0xbau, 0xd9u, 0x03u,       /* mov dx, 3d9h */
        0xb0u, 0x20u,              /* mov al, palette 1 */
        0xeeu,                     /* out dx, al */
        0x31u, 0xffu,              /* xor di, di */
        0xb0u, 0x1bu,              /* mov al, 00,01,10,11 */
        0xaau,                     /* stosb (even row) */
        0xbfu, 0x00u, 0x20u,       /* mov di, 2000h */
        0xb0u, 0xe4u,              /* mov al, 11,10,01,00 */
        0xaau,                     /* stosb (odd row) */
        0xebu, 0xfeu               /* jmp $ */
    };
    STD_FILE *file;
    DWORD length;

    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) return 0;
    STD_MEMSET(vm_cga_graphics_image, 0, sizeof(vm_cga_graphics_image));
    STD_MEMCPY(vm_cga_graphics_image, boot_code, sizeof(boot_code));
    vm_cga_graphics_image[510u] = 0x55u;
    vm_cga_graphics_image[511u] = 0xaau;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return 0;
    if (STD_FWRITE(vm_cga_graphics_image, 1u, sizeof(vm_cga_graphics_image),
            file) != sizeof(vm_cga_graphics_image)) {
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
        .floppy_image = { "" },
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_display_snapshot snapshot;
    vm_session *session = STD_NULL;
    C_CHAR path[MAX_PATH] = {0};
    type_unsigned_32 instruction;
    C_INT passed = 0;

    if (!vm_cga_graphics_write_fixture(path)) goto done;
    {
        vm_session_config fixture_config = config;
        fixture_config.floppy_image[0u] = path;
        if (vm_session_create(&fixture_config, &session) != TYPE_STATUS_OK ||
            session == STD_NULL) goto done;
    }
    for (instruction = 0u; instruction < VM_CGA_GRAPHICS_BOOT_BUDGET;
         ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) !=
                TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) {
            goto done;
        }
        if (core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) != TYPE_STATUS_OK ||
            snapshot.kind != CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4) {
            continue;
        }
        if (snapshot.pixels[0] == 0u && snapshot.pixels[1] == 1u &&
            snapshot.pixels[2] == 2u && snapshot.pixels[3] == 3u &&
            snapshot.pixels[320u] == 3u && snapshot.pixels[321u] == 2u &&
            snapshot.pixels[322u] == 1u && snapshot.pixels[323u] == 0u &&
            snapshot.palette_rgb[1] == 0x00aaaau) {
            passed = 1;
            break;
        }
    }

done:
    vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    if (!passed) return 1;
    STD_PRINTF("M5:T228:S3:CGA:SYSTEM:OK\n");
    return 0;
}

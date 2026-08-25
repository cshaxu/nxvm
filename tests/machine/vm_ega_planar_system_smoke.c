#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_EGA_PLANAR_IMAGE_BYTES (1440u * 1024u)
#define VM_EGA_PLANAR_BOOT_BUDGET 500000u

static type_unsigned_8 vm_ega_planar_image[VM_EGA_PLANAR_IMAGE_BYTES];

static C_INT vm_ega_planar_write_fixture(C_CHAR path[MAX_PATH])
{
    static const type_unsigned_8 boot_code[] = {
        0xb8u, 0x0du, 0x00u,       /* mov ax, 000dh */
        0xcdu, 0x10u,              /* int 10h */
        0xb8u, 0x00u, 0xa0u,       /* mov ax, a000h */
        0x8eu, 0xc0u,              /* mov es, ax */
        0xbau, 0xc4u, 0x03u,       /* mov dx, 3c4h */
        0xb0u, 0x02u,              /* mov al, 2 */
        0xeeu,                     /* out dx, al */
        0x42u,                     /* inc dx */
        0xb0u, 0x0fu,              /* mov al, 0fh */
        0xeeu,                     /* out dx, al */
        0xbau, 0xceu, 0x03u,       /* mov dx, 3ceh */
        0xb0u, 0x05u,              /* mov al, 5 */
        0xeeu,                     /* out dx, al */
        0x42u,                     /* inc dx */
        0x30u, 0xc0u,              /* xor al, al */
        0xeeu,                     /* out dx, al */
        0x4au,                     /* dec dx */
        0xb0u, 0x06u,              /* mov al, 6 */
        0xeeu,                     /* out dx, al */
        0x42u,                     /* inc dx */
        0xb0u, 0x05u,              /* mov al, 5 */
        0xeeu,                     /* out dx, al */
        0xbau, 0xdau, 0x03u,       /* mov dx, 3dah */
        0xecu,                     /* in al, dx */
        0xbau, 0xc0u, 0x03u,       /* mov dx, 3c0h */
        0xb0u, 0x30u,              /* mov al, index 10h, enable */
        0xeeu,                     /* out dx, al */
        0xb0u, 0x01u,              /* mov al, graphics enable */
        0xeeu,                     /* out dx, al */
        0x31u, 0xffu,              /* xor di, di */
        0xb0u, 0xa5u,              /* mov al, 10100101b */
        0xaau,                     /* stosb */
        0xebu, 0xfeu               /* jmp $ */
    };
    STD_FILE *file;
    DWORD length;

    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) return 0;
    STD_MEMSET(vm_ega_planar_image, 0, sizeof(vm_ega_planar_image));
    STD_MEMCPY(vm_ega_planar_image, boot_code, sizeof(boot_code));
    vm_ega_planar_image[510u] = 0x55u;
    vm_ega_planar_image[511u] = 0xaau;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return 0;
    if (STD_FWRITE(vm_ega_planar_image, 1u, sizeof(vm_ega_planar_image), file) !=
        sizeof(vm_ega_planar_image)) {
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
    type_unsigned_32 instruction;
    C_INT passed = 0;

    if (!vm_ega_planar_write_fixture(path)) goto done;
    {
        vm_session_config fixture_config = config;
        fixture_config.fdd_image = path;
        if (vm_session_create(&fixture_config, &session) != TYPE_STATUS_OK ||
            session == STD_NULL) goto done;
    }
    for (instruction = 0u; instruction < VM_EGA_PLANAR_BOOT_BUDGET;
        ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) !=
                TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) {
            goto done;
        }
        if (core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) != TYPE_STATUS_OK ||
            snapshot.kind != CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16) {
            continue;
        }
        if (snapshot.pixels[0] == 15u && snapshot.pixels[1] == 0u &&
            snapshot.pixels[2] == 15u && snapshot.pixels[3] == 0u &&
            snapshot.pixels[4] == 0u && snapshot.pixels[5] == 15u &&
            snapshot.palette_rgb[15] == 0xffffffu) {
            passed = 1;
            break;
        }
    }

done:
    vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    if (!passed) return 1;
    STD_PRINTF("M5:T238:S3:EGA-PLANAR:SYSTEM:OK\n");
    return 0;
}

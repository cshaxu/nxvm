#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_ROM_EGA_IMAGE_BYTES (1440u * 1024u)
#define VM_ROM_EGA_BOOT_BUDGET 500000u

static type_unsigned_8 vm_rom_ega_image[VM_ROM_EGA_IMAGE_BYTES];

static C_INT vm_rom_ega_write_fixture(C_CHAR path[MAX_PATH])
{
    static const type_unsigned_8 boot_code[] = {
        0x31u, 0xc0u, 0x8eu, 0xd8u,             /* xor ax,ax; mov ds,ax */
        0xb8u, 0x0du, 0x00u, 0xcdu, 0x10u,      /* mov ax,000dh; int 10h */
        0xb4u, 0x0fu, 0xcdu, 0x10u,             /* mov ah,0fh; int 10h */
        0xa3u, 0x00u, 0x05u,                    /* mov [0500h],ax */
        0xb8u, 0x00u, 0xa0u, 0x8eu, 0xc0u,      /* mov ax,a000h; mov es,ax */
        0x31u, 0xffu, 0xb0u, 0xa5u, 0xaau,      /* xor di,di; mov al,a5; stosb */
        0xb8u, 0x03u, 0x00u, 0xcdu, 0x10u,      /* mov ax,0003h; int 10h */
        0xb4u, 0x0fu, 0xcdu, 0x10u,             /* mov ah,0fh; int 10h */
        0xa3u, 0x02u, 0x05u, 0xf4u, 0xebu, 0xfeu /* mov [0502h],ax; hlt; jmp $ */
    };
    STD_FILE *file;
    DWORD length;

    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) return 0;
    STD_MEMSET(vm_rom_ega_image, 0, sizeof(vm_rom_ega_image));
    STD_MEMCPY(vm_rom_ega_image, boot_code, sizeof(boot_code));
    vm_rom_ega_image[510u] = 0x55u;
    vm_rom_ega_image[511u] = 0xaau;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return 0;
    if (STD_FWRITE(vm_rom_ega_image, 1u, sizeof(vm_rom_ega_image), file) !=
        sizeof(vm_rom_ega_image)) {
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
    type_unsigned_16 ega_query = 0u;
    type_unsigned_16 text_query = 0u;
    type_unsigned_8 mode = 0u;
    type_unsigned_32 instruction;
    C_INT saw_ega = 0;
    C_INT saw_text = 0;

    if (!vm_rom_ega_write_fixture(path)) goto done;
    {
        vm_session_config fixture_config = config;
        fixture_config.fdd_image = path;
        if (vm_session_create(&fixture_config, &session) != TYPE_STATUS_OK ||
            session == STD_NULL) goto done;
    }
    for (instruction = 0u; instruction < VM_ROM_EGA_BOOT_BUDGET; ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) !=
                TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) goto done;
        if (core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) != TYPE_STATUS_OK) continue;
        if (snapshot.kind == CORE_MACHINE_DISPLAY_KIND_EGA_320X200X16 &&
            snapshot.pixels[0] == 15u && snapshot.pixels[1] == 0u &&
            snapshot.pixels[2] == 15u && snapshot.palette_rgb[15] == 0xffffffu &&
            core_machine_memory_read(session->core_machine, 0x0449u, &mode,
                sizeof(mode)) == TYPE_STATUS_OK && mode == 0x0du &&
            core_machine_memory_read(session->core_machine, 0x0500u, &ega_query,
                sizeof(ega_query)) == TYPE_STATUS_OK && ega_query == 0x500du) {
            saw_ega = 1;
        }
        if (saw_ega && snapshot.kind == CORE_MACHINE_DISPLAY_KIND_TEXT &&
            core_machine_memory_read(session->core_machine, 0x0449u, &mode,
                sizeof(mode)) == TYPE_STATUS_OK && mode == 0x03u &&
            core_machine_memory_read(session->core_machine, 0x0502u, &text_query,
                sizeof(text_query)) == TYPE_STATUS_OK && text_query == 0x5003u) {
            saw_text = 1;
            break;
        }
    }

done:
    vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    if (!saw_ega || !saw_text) return 1;
    STD_PRINTF("M5:T239:S3:ROM-EGA-INT10:SYSTEM:OK\n");
    return 0;
}

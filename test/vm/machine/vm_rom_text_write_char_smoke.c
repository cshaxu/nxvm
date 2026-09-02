#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#define VM_ROM_TEXT_IMAGE_BYTES (1440u * 1024u)
#define VM_ROM_TEXT_BOOT_BUDGET 300000u

static type_unsigned_8 vm_rom_text_image[VM_ROM_TEXT_IMAGE_BYTES];

static C_INT vm_rom_text_write_fixture(C_CHAR path[MAX_PATH])
{
    static const type_unsigned_8 boot_code[] = {
        0x31u, 0xc0u, 0x8eu, 0xd8u,             /* xor ax,ax; mov ds,ax */
        0xb8u, 0x03u, 0x00u, 0xcdu, 0x10u,      /* mov ax,0003h; int 10h */
        0xb4u, 0x02u, 0xb7u, 0x00u, 0xb6u, 0x01u,
        0xb2u, 0x02u, 0xcdu, 0x10u,             /* AH=02h, page 0, (1,2) */
        0xb4u, 0x09u, 0xb0u, 'X', 0xb7u, 0x00u,
        0xb3u, 0x1eu, 0xb9u, 0x03u, 0x00u, 0xcdu, 0x10u,
        0xb4u, 0x08u, 0xb7u, 0x00u, 0xcdu, 0x10u,
        0xa3u, 0x00u, 0x05u,                    /* store AX from AH=08h */
        0xb4u, 0x05u, 0xb7u, 0x00u, 0xcdu, 0x10u,
        0xa0u, 0x62u, 0x04u, 0xa2u, 0x02u, 0x05u,
        0xb4u, 0x06u, 0xb0u, 0x00u, 0xb7u, 0x2du,
        0xb5u, 0x01u, 0xb1u, 0x03u, 0xb6u, 0x01u,
        0xb2u, 0x03u, 0xcdu, 0x10u,
        0xf4u, 0xebu, 0xfeu
    };
    STD_FILE *file;
    DWORD length;

    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) return 0;
    STD_MEMSET(vm_rom_text_image, 0, sizeof(vm_rom_text_image));
    STD_MEMCPY(vm_rom_text_image, boot_code, sizeof(boot_code));
    vm_rom_text_image[510u] = 0x55u;
    vm_rom_text_image[511u] = 0xaau;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return 0;
    if (STD_FWRITE(vm_rom_text_image, 1u, sizeof(vm_rom_text_image), file) !=
            sizeof(vm_rom_text_image)) {
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
    core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    core_machine_display_snapshot snapshot;
    vm_session *session = STD_NULL;
    C_CHAR path[MAX_PATH] = {0};
    type_unsigned_16 character = 0u;
    type_unsigned_8 page = 0xffu;
    type_unsigned_32 instruction;
    C_INT passed = 0;

    if (!vm_rom_text_write_fixture(path)) goto done;
    {
        vm_session_config fixture_config = config;

        fixture_config.floppy_image[0u] = path;
        if (vm_session_create(&fixture_config, &session) != TYPE_STATUS_OK ||
            session == STD_NULL) goto done;
    }
    for (instruction = 0u; instruction < VM_ROM_TEXT_BOOT_BUDGET; ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) !=
                TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) goto done;
        if (core_machine_memory_read(session->core_machine, 0x0500u, &character,
                sizeof(character)) != TYPE_STATUS_OK || character != 0x1e58u ||
            core_machine_memory_read(session->core_machine, 0x0502u, &page,
                sizeof(page)) != TYPE_STATUS_OK || page != 0u ||
            core_machine_capture_display_snapshot(session->core_machine,
                &snapshot) != TYPE_STATUS_OK) continue;
        if (snapshot.kind == CORE_MACHINE_DISPLAY_KIND_TEXT &&
            snapshot.characters[82u] == 'X' && snapshot.characters[83u] == 0u &&
            snapshot.characters[84u] == 'X' && snapshot.attributes[82u] == 0x1eu &&
            snapshot.attributes[83u] == 0x2du && snapshot.attributes[84u] == 0x1eu &&
            snapshot.cursor_x == 2u && snapshot.cursor_y == 1u) {
            passed = 1;
            break;
        }
    }

done:
    vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    if (!passed) return 1;
    STD_PRINTF("M5:T287:S19:ROM-TEXT-WRITE:OK\n");
    return 0;
}

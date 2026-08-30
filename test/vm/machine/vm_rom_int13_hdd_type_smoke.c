#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/waiting.h"
#include "vm/profile/default_profile/firmware/bios.h"

#define VM_INT13_HDD_FDD_BYTES (1440u * 1024u)
#define VM_INT13_HDD_BOOT_BUDGET 500000u

static type_unsigned_8 vm_int13_hdd_fdd[VM_INT13_HDD_FDD_BYTES];

static C_INT vm_int13_hdd_write_fixture(C_CHAR fdd_path[MAX_PATH],
    C_CHAR hdd_path[MAX_PATH])
{
    static const type_unsigned_8 boot_code[] = {
        0x31u, 0xc0u,                         /* xor ax,ax */
        0x8eu, 0xd8u,                         /* mov ds,ax */
        0xb4u, 0x01u,                         /* mov ah,01h */
        0xb2u, 0x80u,                         /* mov dl,80h */
        0xcdu, 0x13u,                         /* int 13h */
        0xa3u, 0x14u, 0x05u,                  /* mov [0514h],ax */
        0x9cu, 0x58u,                         /* pushf; pop ax */
        0xa3u, 0x16u, 0x05u,                  /* mov [0516h],ax */
        0xb0u, 0x12u,                         /* mov al,12h */
        0xe6u, 0x70u,                         /* out 70h,al */
        0xe4u, 0x71u,                         /* in al,71h */
        0xa2u, 0x0cu, 0x05u,                  /* mov [050Ch],al */
        0xb0u, 0x19u,                         /* mov al,19h */
        0xe6u, 0x70u,                         /* out 70h,al */
        0xe4u, 0x71u,                         /* in al,71h */
        0xa2u, 0x18u, 0x05u,                  /* mov [0518h],al */
        0xb4u, 0x41u,                         /* mov ah,41h */
        0xbbu, 0xaau, 0x55u,                  /* mov bx,55AAh */
        0xb2u, 0x80u,                         /* mov dl,80h */
        0xcdu, 0x13u,                         /* int 13h */
        0xa3u, 0x0eu, 0x05u,                  /* mov [050Eh],ax */
        0x89u, 0x1eu, 0x10u, 0x05u,            /* mov [0510h],bx */
        0x9cu, 0x58u,                         /* pushf; pop ax */
        0xa3u, 0x12u, 0x05u,                  /* mov [0512h],ax */
        0xb4u, 0x08u,                         /* mov ah,08h */
        0xb2u, 0x80u,                         /* mov dl,80h */
        0xb8u, 0x34u, 0x12u,                  /* mov ax,1234h */
        0x8eu, 0xc0u,                         /* mov es,ax */
        0xbfu, 0x78u, 0x56u,                  /* mov di,5678h */
        0xb4u, 0x08u,                         /* restore ah=08h */
        0xcdu, 0x13u,                         /* int 13h */
        0xa3u, 0x1au, 0x05u,                  /* mov [051Ah],ax */
        0x88u, 0x1eu, 0x0du, 0x05u,            /* mov [050Dh],bl */
        0x8cu, 0xc0u,                         /* mov ax,es */
        0xa3u, 0x08u, 0x05u,                  /* mov [0508h],ax */
        0x89u, 0x3eu, 0x0au, 0x05u,            /* mov [050Ah],di */
        0xb4u, 0x15u,                         /* mov ah,15h */
        0xb2u, 0x80u,                         /* mov dl,80h */
        0xcdu, 0x13u,                         /* int 13h */
        0xa3u, 0x00u, 0x05u,                  /* mov [0500h],ax */
        0x9cu, 0x58u,                         /* pushf; pop ax */
        0xa3u, 0x02u, 0x05u,                  /* mov [0502h],ax */
        0x89u, 0x0eu, 0x04u, 0x05u,            /* mov [0504h],cx */
        0x89u, 0x16u, 0x06u, 0x05u,            /* mov [0506h],dx */
        0xb4u, 0x01u,                         /* mov ah,01h */
        0xb2u, 0x80u,                         /* mov dl,80h */
        0xcdu, 0x13u,                         /* int 13h */
        0xa3u, 0x1cu, 0x05u,                  /* mov [051Ch],ax */
        0x9cu, 0x58u,                         /* pushf; pop ax */
        0xa3u, 0x1eu, 0x05u,                  /* mov [051Eh],ax */
        0xb8u, 0x00u, 0x00u, 0x8eu, 0xc0u,    /* ES=0000 */
        0xc7u, 0x06u, 0x00u, 0x06u, 0xa5u, 0x5au, /* [0600]=5AA5 */
        0xc7u, 0x06u, 0x00u, 0x08u, 0x5au, 0xa5u, /* [0800]=A55A */
        0xbbu, 0x00u, 0x06u,                  /* BX=0600 */
        0xb8u, 0x02u, 0x03u,                  /* AH=03, AL=02 */
        0xb9u, 0x01u, 0x00u,                  /* CHS 0/0/1 */
        0xbau, 0x80u, 0x00u,                  /* DH=0, DL=80 */
        0xcdu, 0x13u,                         /* int 13h */
        0xa3u, 0x22u, 0x05u,                  /* mov [0522h],ax */
        0x9cu, 0x58u, 0xa3u, 0x24u, 0x05u,      /* pushf; pop [0524h] */
        0xbbu, 0x00u, 0x0au,                  /* BX=0A00 */
        0xb8u, 0x02u, 0x02u,                  /* AH=02, AL=02 */
        0xb9u, 0x01u, 0x00u,                  /* CHS 0/0/1 */
        0xbau, 0x80u, 0x00u,                  /* DH=0, DL=80 */
        0xfdu,                                 /* std: INT 13h must not depend on DF */
        0xcdu, 0x13u,                         /* int 13h */
        0xfcu,                                 /* cld */
        0xa3u, 0x26u, 0x05u,                  /* mov [0526h],ax */
        0x9cu, 0x58u, 0xa3u, 0x28u, 0x05u,      /* pushf; pop [0528h] */
        0xa1u, 0x00u, 0x0au,                  /* mov ax,[0A00h] */
        0xa3u, 0x2au, 0x05u,                  /* mov [052Ah],ax */
        0xa1u, 0x00u, 0x0cu,                  /* mov ax,[0C00h] */
        0xa3u, 0x2cu, 0x05u,                  /* mov [052Ch],ax */
        0xc7u, 0x06u, 0x20u, 0x05u, 0x5au, 0xa5u, /* mov word [0520h],A55Ah */
        0xf4u, 0xebu, 0xfeu                   /* hlt; jmp $ */
    };
    type_unsigned_8 hdd_sector[1024] = {0};
    STD_FILE *file;
    DWORD length;

    length = GetTempPathA(MAX_PATH, fdd_path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(fdd_path, "n64", 0u, fdd_path) == 0u) return 0;
    length = GetTempPathA(MAX_PATH, hdd_path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(hdd_path, "n64", 0u, hdd_path) == 0u) {
        DeleteFileA(fdd_path);
        return 0;
    }
    STD_MEMSET(vm_int13_hdd_fdd, 0, sizeof(vm_int13_hdd_fdd));
    STD_MEMCPY(vm_int13_hdd_fdd, boot_code, sizeof(boot_code));
    vm_int13_hdd_fdd[510u] = 0x55u;
    vm_int13_hdd_fdd[511u] = 0xaau;
    file = STD_FOPEN(fdd_path, "wb");
    if (file == STD_NULL || STD_FWRITE(vm_int13_hdd_fdd, 1u,
            sizeof(vm_int13_hdd_fdd), file) != sizeof(vm_int13_hdd_fdd)) {
        if (file != STD_NULL) STD_FCLOSE(file);
        DeleteFileA(fdd_path);
        DeleteFileA(hdd_path);
        return 0;
    }
    STD_FCLOSE(file);
    hdd_sector[512u] = 0x34u;
    hdd_sector[513u] = 0x12u;
    file = STD_FOPEN(hdd_path, "wb");
    if (file == STD_NULL || STD_FWRITE(hdd_sector, 1u, sizeof(hdd_sector), file) !=
            sizeof(hdd_sector)) {
        if (file != STD_NULL) STD_FCLOSE(file);
        DeleteFileA(fdd_path);
        DeleteFileA(hdd_path);
        return 0;
    }
    STD_FCLOSE(file);
    return 1;
}

C_INT main(C_VOID)
{
    const core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    const vm_session_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    vm_session *session = STD_NULL;
    C_CHAR fdd_path[MAX_PATH] = {0};
    C_CHAR hdd_path[MAX_PATH] = {0};
    vm_session_config fixture_config = config;
    type_unsigned_16 result_ax = 0u;
    type_unsigned_16 flags = 0u;
    type_unsigned_16 sectors_high = 0u;
    type_unsigned_16 sectors_low = 0u;
    type_unsigned_16 parameter_segment = 0u;
    type_unsigned_16 parameter_offset = 0u;
    type_unsigned_8 sectors_per_track = 0u;
    type_unsigned_8 hdd_count = 0u;
    type_unsigned_8 cmos_fixed_disk_type = 0u;
    type_unsigned_8 drive_type = 0u;
    type_unsigned_8 cmos_extended_disk_type = 0u;
    type_unsigned_16 extension_ax = 0u;
    type_unsigned_16 extension_bx = 0u;
    type_unsigned_16 extension_flags = 0u;
    type_unsigned_16 initial_status_ax = 0u;
    type_unsigned_16 initial_status_flags = 0u;
    type_unsigned_16 parameters_ax = 0u;
    type_unsigned_16 post_type_status_ax = 0u;
    type_unsigned_16 post_type_status_flags = 0u;
    type_unsigned_16 write_ax = 0u;
    type_unsigned_16 write_flags = 0u;
    type_unsigned_16 read_ax = 0u;
    type_unsigned_16 read_flags = 0u;
    type_unsigned_16 read_word = 0u;
    type_unsigned_16 read_second_word = 0u;
    type_unsigned_16 completed = 0u;
    type_unsigned_32 instruction;
    C_INT passed = 0;

    if (!vm_int13_hdd_write_fixture(fdd_path, hdd_path)) goto done;
    fixture_config.fdd_image = fdd_path;
    fixture_config.hdd_image = hdd_path;
    if (vm_session_create(&fixture_config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) goto done;
    for (instruction = 0u; instruction < VM_INT13_HDD_BOOT_BUDGET; ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT) goto done;
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            C_INT advanced = 0;

            if (vm_session_waiting_advance(session, &result, &advanced) != TYPE_STATUS_OK ||
                !advanced) goto done;
        }
        if (core_machine_memory_read(session->core_machine, 0x0500u, &result_ax,
                sizeof(result_ax)) != TYPE_STATUS_OK || result_ax == 0u) continue;
        if (core_machine_memory_read(session->core_machine, 0x0502u, &flags,
                sizeof(flags)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x0504u, &sectors_high,
                sizeof(sectors_high)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x0506u, &sectors_low,
                sizeof(sectors_low)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x0508u, &parameter_segment,
                sizeof(parameter_segment)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x050au, &parameter_offset,
                sizeof(parameter_offset)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, (type_unsigned_32)VBIOS_ADDR_START_SEG * 16u +
                VBIOS_ADDR_HDD_PARAM + 14u, &sectors_per_track,
                sizeof(sectors_per_track)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x050cu, &cmos_fixed_disk_type,
                sizeof(cmos_fixed_disk_type)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x050du, &drive_type,
                sizeof(drive_type)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x050eu, &extension_ax,
                sizeof(extension_ax)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x0510u, &extension_bx,
                sizeof(extension_bx)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x0512u, &extension_flags,
                sizeof(extension_flags)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x0514u, &initial_status_ax,
                sizeof(initial_status_ax)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x0516u, &initial_status_flags,
                sizeof(initial_status_flags)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x0518u, &cmos_extended_disk_type,
                sizeof(cmos_extended_disk_type)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x051au, &parameters_ax,
                sizeof(parameters_ax)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x051cu, &post_type_status_ax,
                sizeof(post_type_status_ax)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x051eu, &post_type_status_flags,
                sizeof(post_type_status_flags)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x0522u, &write_ax, sizeof(write_ax)) !=
                TYPE_STATUS_OK || core_machine_memory_read(session->core_machine,
                0x0524u, &write_flags, sizeof(write_flags)) != TYPE_STATUS_OK ||
            core_machine_memory_read(session->core_machine, 0x0526u, &read_ax,
                sizeof(read_ax)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x0528u, &read_flags, sizeof(read_flags)) !=
                TYPE_STATUS_OK || core_machine_memory_read(session->core_machine,
                0x052au, &read_word, sizeof(read_word)) != TYPE_STATUS_OK ||
            core_machine_memory_read(session->core_machine, 0x052cu,
                &read_second_word, sizeof(read_second_word)) != TYPE_STATUS_OK ||
            core_machine_memory_read(session->core_machine, 0x0520u, &completed,
                sizeof(completed)) != TYPE_STATUS_OK || core_machine_memory_read(
                session->core_machine, 0x0475u, &hdd_count, sizeof(hdd_count)) !=
                TYPE_STATUS_OK) goto done;
        if (completed != 0xa55au) continue;
        passed = (result_ax & 0xff00u) == 0x0300u && (flags & 1u) == 0u &&
            sectors_high == 0u &&
            sectors_low == 1008u &&
            parameter_segment == 0x1234u && parameter_offset == 0x5678u &&
            sectors_per_track == 63u &&
            cmos_fixed_disk_type == 0xf0u && drive_type == 0x2fu &&
            cmos_extended_disk_type == 0x2fu &&
            parameters_ax == 0x003fu &&
            (post_type_status_ax & 0xff00u) == 0u &&
            (post_type_status_flags & 1u) == 0u &&
            (extension_ax & 0xff00u) == 0x0100u && extension_bx == 0x55aau &&
            (extension_flags & 1u) != 0u &&
            (initial_status_ax & 0xff00u) == 0u &&
            (initial_status_flags & 1u) == 0u &&
            (write_ax & 0xff00u) == 0u && (write_flags & 1u) == 0u &&
            (read_ax & 0xff00u) == 0u && (read_flags & 1u) == 0u &&
            read_word == 0x5aa5u && read_second_word == 0xa55au &&
            hdd_count == 1u;
        break;
    }

done:
    vm_session_destroy(session);
    if (fdd_path[0] != '\0') DeleteFileA(fdd_path);
    if (hdd_path[0] != '\0') DeleteFileA(hdd_path);
    if (!passed) {
        STD_FPRINTF(STD_STDERR,
            "M5:T287:S16:ROM-INT13-HDD-TYPE47:FAIL complete=%04X ax=%04X flags=%04X sectors=%04X:%04X dpt=%04X:%04X spt=%02X cmos=%02X/%02X type=%02X params=%04X ext=%04X/%04X/%04X status=%04X/%04X post=%04X/%04X rw=%04X/%04X/%04X/%04X/%04X hdd=%u\n",
            completed, result_ax, flags, sectors_high, sectors_low, parameter_segment,
            parameter_offset, sectors_per_track, cmos_fixed_disk_type, cmos_extended_disk_type,
            drive_type, parameters_ax, extension_ax,
            extension_bx, extension_flags, initial_status_ax,
            initial_status_flags, post_type_status_ax, post_type_status_flags, write_ax,
            write_flags, read_ax, read_flags, read_word,
            hdd_count);
        return 1;
    }
    STD_PRINTF("M5:T287:S16:ROM-INT13-HDD-TYPE47:OK\n");
    return 0;
}

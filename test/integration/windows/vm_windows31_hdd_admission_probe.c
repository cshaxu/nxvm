#include "type.h"

#include <windows.h>

#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/waiting.h"

#define VM_T287_PROBE_FDD_BYTES (1440u * 1024u)
#define VM_T287_PROBE_BUDGET 500000u
#define VM_T287_PROBE_MBR 0x1000u
#define VM_T287_PROBE_VBR 0x1200u
#define VM_T287_PROBE_RESULT 0x0500u

static type_unsigned_8 vm_t287_probe_fdd[VM_T287_PROBE_FDD_BYTES];

static C_INT vm_t287_probe_write_fdd(C_CHAR path[MAX_PATH])
{
    static const type_unsigned_8 boot_code[] = {
        0x31u, 0xc0u,                         /* xor ax,ax */
        0x8eu, 0xd0u,                         /* mov ss,ax */
        0xbcu, 0x00u, 0x7cu,                  /* mov sp,7c00h */
        0x8eu, 0xd8u,                         /* mov ds,ax */
        0x8eu, 0xc0u,                         /* mov es,ax */
        0xfcu,                                 /* cld */
        0xb4u, 0x08u, 0xb2u, 0x80u, 0xcdu, 0x13u, /* AH=08, DL=80 */
        0xa3u, 0x00u, 0x05u,                  /* [0500]=AX */
        0x89u, 0x0eu, 0x02u, 0x05u,            /* [0502]=CX */
        0x89u, 0x16u, 0x04u, 0x05u,            /* [0504]=DX */
        0x9cu, 0x58u, 0xa3u, 0x06u, 0x05u,      /* [0506]=FLAGS */
        0xb8u, 0x00u, 0x01u, 0x8eu, 0xc0u,    /* ES=0100, retain DS=0 */
        0xb8u, 0x01u, 0x02u,                  /* AH=02, AL=01 */
        0x31u, 0xdbu,                         /* BX=0000 (ES:BX=1000) */
        0xb9u, 0x01u, 0x00u,                  /* CHS 0/0/1 */
        0xbau, 0x80u, 0x00u,                  /* DH=0, DL=80 */
        0xcdu, 0x13u,
        0xa3u, 0x08u, 0x05u,                  /* [0508]=AX */
        0x9cu, 0x58u, 0xa3u, 0x0au, 0x05u,      /* [050A]=FLAGS */
        0xa1u, 0x00u, 0x10u,                  /* AX=[MBR+0] */
        0xa3u, 0x14u, 0x05u,                  /* [0514]=AX */
        0xa1u, 0x02u, 0x10u,                  /* AX=[MBR+2] */
        0xa3u, 0x16u, 0x05u,                  /* [0516]=AX */
        0x8au, 0x36u, 0xbfu, 0x11u,            /* DH=[MBR+1BF] */
        0x8au, 0x0eu, 0xc0u, 0x11u,            /* CL=[MBR+1C0] */
        0x8au, 0x2eu, 0xc1u, 0x11u,            /* CH=[MBR+1C1] */
        0xb2u, 0x80u,                         /* DL=80 */
        0xb8u, 0x20u, 0x01u, 0x8eu, 0xc0u,    /* ES=0120 (VBR=1200) */
        0xb8u, 0x01u, 0x02u,                  /* AH=02, AL=01 */
        0x31u, 0xdbu,                         /* BX=0000 */
        0xcdu, 0x13u,
        0xa3u, 0x0cu, 0x05u,                  /* [050C]=AX */
        0x9cu, 0x58u, 0xa3u, 0x0eu, 0x05u,      /* [050E]=FLAGS */
        0xc7u, 0x06u, 0x10u, 0x05u, 0x5au, 0xa5u, /* [0510]=A55A */
        0xf4u, 0xebu, 0xfeu                   /* hlt; jmp $ */
    };
    STD_FILE *file;
    DWORD length;

    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) return 0;
    STD_MEMSET(vm_t287_probe_fdd, 0, sizeof(vm_t287_probe_fdd));
    STD_MEMCPY(vm_t287_probe_fdd, boot_code, sizeof(boot_code));
    vm_t287_probe_fdd[510u] = 0x55u;
    vm_t287_probe_fdd[511u] = 0xaau;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL || STD_FWRITE(vm_t287_probe_fdd, 1u,
            sizeof(vm_t287_probe_fdd), file) != sizeof(vm_t287_probe_fdd)) {
        if (file != STD_NULL) STD_FCLOSE(file);
        DeleteFileA(path);
        return 0;
    }
    STD_FCLOSE(file);
    return 1;
}

static C_INT vm_t287_probe_read_file(const C_CHAR *path, type_unsigned_8 *bytes,
    DWORD count)
{
    HANDLE file;
    DWORD read = 0u;

    if (path == STD_NULL || bytes == STD_NULL || count == 0u) return 0;
    file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, STD_NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, STD_NULL);
    if (file == INVALID_HANDLE_VALUE) return 0;
    if (!ReadFile(file, bytes, count, &read, STD_NULL) || read != count) {
        CloseHandle(file);
        return 0;
    }
    CloseHandle(file);
    return 1;
}

static type_unsigned_32 vm_t287_probe_lba(const type_unsigned_8 *entry)
{
    return (type_unsigned_32)entry[8] | ((type_unsigned_32)entry[9] << 8u) |
        ((type_unsigned_32)entry[10] << 16u) | ((type_unsigned_32)entry[11] << 24u);
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    const core_machine_run_budget budget = {1u, 0u};
    const vm_session_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    vm_session_config probe_config = config;
    core_machine_run_result result;
    core_machine_cpu_state cpu = {0};
    vm_session *session = STD_NULL;
    C_CHAR fdd_path[MAX_PATH] = {0};
    type_unsigned_8 host_mbr[512] = {0};
    type_unsigned_8 host_vbr[512] = {0};
    type_unsigned_8 guest_mbr[512] = {0};
    type_unsigned_8 guest_vbr[512] = {0};
    type_unsigned_16 values[12] = {0};
    type_unsigned_16 int13_vector[2] = {0};
    const type_unsigned_8 *entry = STD_NULL;
    type_unsigned_32 lba = 0u;
    type_unsigned_32 sectors_per_track = 0u;
    type_unsigned_32 heads = 0u;
    type_unsigned_32 cylinder = 0u;
    type_unsigned_32 head = 0u;
    type_unsigned_32 sector = 0u;
    type_unsigned_32 instruction;
    type_unsigned_8 first_sector_number = 0u;
    type_unsigned_8 first_cylinder_low = 0u;
    type_unsigned_8 first_cylinder_high = 0u;
    type_unsigned_8 first_drive_head = 0u;
    type_unsigned_32 first_command_count = 0u;
    C_INT passed = 0;
    STD_SIZE_T mbr_mismatch = sizeof(guest_mbr);
    STD_SIZE_T vbr_mismatch = sizeof(guest_vbr);

    if (argc != 2 || !vm_t287_probe_write_fdd(fdd_path) ||
        !vm_t287_probe_read_file(argv[1], host_mbr, sizeof(host_mbr))) goto done;
    entry = host_mbr + 446u;
    lba = vm_t287_probe_lba(entry);
    if (lba == 0u || lba > (MAXDWORD / 512u) || !vm_t287_probe_read_file(argv[1],
            host_vbr, sizeof(host_vbr))) goto done;
    {
        HANDLE file = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ, STD_NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, STD_NULL);
        DWORD read = 0u;
        LARGE_INTEGER offset;

        if (file == INVALID_HANDLE_VALUE) goto done;
        offset.QuadPart = (LONGLONG)lba * 512ll;
        if (!SetFilePointerEx(file, offset, STD_NULL, FILE_BEGIN) ||
            !ReadFile(file, host_vbr, sizeof(host_vbr), &read, STD_NULL) ||
            read != sizeof(host_vbr)) {
            CloseHandle(file);
            goto done;
        }
        CloseHandle(file);
    }
    probe_config.floppy_image[0u] = fdd_path;
    probe_config.fixed_disk_image[0u] = argv[1];
    if (vm_session_create(&probe_config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) goto done;
    if (core_machine_debug_read_memory(session->core_machine, 0x004cu, int13_vector,
            sizeof(int13_vector)) != TYPE_STATUS_OK) goto done;
    for (instruction = 0u; instruction < VM_T287_PROBE_BUDGET; ++instruction) {
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT || core_machine_debug_read_memory(
                session->core_machine, VM_T287_PROBE_RESULT, values,
                sizeof(values)) != TYPE_STATUS_OK) goto done;
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            C_INT advanced = 0;

            if (vm_session_waiting_advance(session, &result, &advanced) != TYPE_STATUS_OK ||
                !advanced) goto done;
        }
        if (values[10] != 0u && first_command_count == 0u) {
            first_sector_number = session->core_machine->hdc.data.sector_number;
            first_cylinder_low = session->core_machine->hdc.data.cylinder_low;
            first_cylinder_high = session->core_machine->hdc.data.cylinder_high;
            first_drive_head = session->core_machine->hdc.data.drive_head;
            first_command_count = session->core_machine->hdc.data.command_count;
        }
        if (values[8] == 0xa55au) break;
    }
    if (values[8] != 0xa55au || core_machine_debug_read_memory(session->core_machine,
            VM_T287_PROBE_MBR, guest_mbr, sizeof(guest_mbr)) != TYPE_STATUS_OK ||
        core_machine_debug_read_memory(session->core_machine, VM_T287_PROBE_VBR,
            guest_vbr, sizeof(guest_vbr)) != TYPE_STATUS_OK) goto done;
    sectors_per_track = values[1] & 0x3fu;
    heads = (values[2] >> 8u) + 1u;
    if ((values[3] & 1u) != 0u || sectors_per_track == 0u || heads == 0u) goto done;
    cylinder = lba / (heads * sectors_per_track);
    head = (lba / sectors_per_track) % heads;
    sector = lba % sectors_per_track + 1u;
    while (mbr_mismatch > 0u && guest_mbr[mbr_mismatch - 1u] ==
        host_mbr[mbr_mismatch - 1u]) --mbr_mismatch;
    while (vbr_mismatch > 0u && guest_vbr[vbr_mismatch - 1u] ==
        host_vbr[vbr_mismatch - 1u]) --vbr_mismatch;
    passed = (values[5] & 1u) == 0u && (values[7] & 1u) == 0u &&
        STD_MEMCMP(guest_mbr, host_mbr, sizeof(guest_mbr)) == 0 &&
        guest_mbr[510] == 0x55u && guest_mbr[511] == 0xaau &&
        (entry[4] == 1u || entry[4] == 4u || entry[4] == 6u) &&
        cylinder < 1024u && entry[1] == head && entry[2] ==
            (type_unsigned_8)(sector | ((cylinder >> 2u) & 0xc0u)) && entry[3] ==
            (type_unsigned_8)cylinder && STD_MEMCMP(guest_vbr, host_vbr,
                sizeof(guest_vbr)) == 0;

done:
    if (session != STD_NULL) (C_VOID)core_machine_get_cpu_state(session->core_machine,
        &cpu);
    if (passed) {
        STD_PRINTF("M5:T287:S16:HDD-ADMISSION:OK lba=%u chs=%u/%u/%u spt=%u heads=%u "
            "ata_commands=%u\n", lba, cylinder, head, sector, sectors_per_track,
            heads, session->core_machine->hdc.data.command_count);
    } else {
        STD_FPRINTF(STD_STDERR,
            "M5:T287:S16:HDD-ADMISSION:FAIL done=%04X ah08=%04X/%04X/%04X/%04X "
            "mbr=%04X/%04X vbr=%04X/%04X lba=%u chs=%u/%u/%u type=%02X "
            "mismatch=%u/%u int13=%04X:%04X rom_end=%04X bx=%04X first=%04X/%04X task=%02X/%02X%02X/%02X reads=%u bytes=%02X%02X%02X%02X/%02X%02X%02X%02X/%02X%02X%02X%02X cpu=%04X:%08X halt=%u reason=%u hdc=%u/%u/%02X reads=%u\n", values[8], values[0], values[1], values[2],
            values[3], values[4], values[5], values[6], values[7], lba, cylinder,
            head, sector, entry == STD_NULL ? 0u : entry[4], (C_UINT)mbr_mismatch,
            (C_UINT)vbr_mismatch, int13_vector[1], int13_vector[0],
            session == STD_NULL ? 0u : session->default_bios.data.buildIP, values[9],
            values[10], values[11], first_sector_number,
            first_cylinder_low, first_cylinder_high, first_drive_head, first_command_count,
            guest_mbr[0], guest_mbr[1], guest_mbr[2], guest_mbr[3],
            host_mbr[0], host_mbr[1], host_mbr[2], host_mbr[3], guest_vbr[0], guest_vbr[1],
            guest_vbr[2], guest_vbr[3], cpu.cs, cpu.eip, cpu.halted, (C_UINT)result.reason,
            (C_UINT)(session == STD_NULL ? 0u : session->core_machine->hdc.data.phase),
            (C_UINT)(session == STD_NULL ? 0u : session->core_machine->hdc.data.data_index),
            session == STD_NULL ? 0u : session->core_machine->hdc.data.status,
            session == STD_NULL ? 0u : session->core_machine->hdc.data.command_count);
    }
    vm_session_destroy(session);
    if (fdd_path[0] != '\0') DeleteFileA(fdd_path);
    return passed ? 0 : 1;
}

#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/waiting.h"
#include "app-nxvm/machine/machine_private.h"
#include "test/app-nxvm/integration/support/session_ini.h"

#define VM_HDC_HDD_BOOT_ADDRESS 0x00007c00u
#define VM_HDC_HDD_BOOT_BYTES 512u
#define VM_HDC_HDD_PARTITION_TABLE_OFFSET 446u
#define VM_HDC_HDD_PARTITION_LBA_OFFSET 8u
#define VM_HDC_HDD_BOOT_INSTRUCTION_BUDGET 6000000u
#define VM_HDC_HDD_BOOT_QUANTUM 128u

static lib_u32 vm_hdc_hdd_boot_partition_lba(const vm_machine *session)
{
    lib_u8 entry[4];
    core_machine_media_result result;

    if (session == LIB_NULL || core_machine_media_read_bytes(session->media_registry,
        VM_MACHINE_MEDIA_HDD_ID, VM_HDC_HDD_PARTITION_TABLE_OFFSET +
        VM_HDC_HDD_PARTITION_LBA_OFFSET, entry, sizeof(entry), &result) !=
        LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK) return 0u;
    return (lib_u32)entry[0u] | ((lib_u32)entry[1u] << 8u) |
        ((lib_u32)entry[2u] << 16u) | ((lib_u32)entry[3u] << 24u);
}

static lib_i32 vm_hdc_hdd_boot_matches_partition_vbr(const vm_machine *session)
{
    lib_u8 boot_sector[VM_HDC_HDD_BOOT_BYTES];
    lib_u32 partition_lba;
    lib_u8 image[11];
    core_machine_media_result result;
    lib_size index;

    if (session == LIB_NULL || session->core_machine == LIB_NULL ||
        core_machine_debug_read_memory(session->core_machine,
            VM_HDC_HDD_BOOT_ADDRESS, boot_sector, sizeof(boot_sector)) !=
            LIB_STATUS_OK) {
        return 0;
    }
    partition_lba = vm_hdc_hdd_boot_partition_lba(session);
    if (partition_lba == 0u) return 0;
    if (core_machine_media_read_bytes(session->media_registry, VM_MACHINE_MEDIA_HDD_ID,
        (lib_size)partition_lba * VM_HDC_HDD_BOOT_BYTES, image, sizeof(image),
        &result) != LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK) return 0;
    /* The VBR is already executing when this boundary is observed. Its BPB
       contains boot-time writable fields, so compare its stable identity. */
    for (index = 0u; index < 11u; ++index) {
        if (boot_sector[index] != image[index]) {
            return 0;
        }
    }
    return boot_sector[510] == 0x55u && boot_sector[511] == 0xaau;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    integration_ini_session ini_session;
    const core_machine_run_budget budget = {
        VM_HDC_HDD_BOOT_QUANTUM, 0u
    };
    vm_machine *session = LIB_NULL;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_status run_status;
    lib_u32 executed = 0u;
    lib_i32 loaded = 0;

    if (argc != 3 || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != LIB_STATUS_OK) return 77;
    session = ini_session.session;
    if (!session->hdd.connect.flagDiskExist) goto fail;
    /* The canonical product INI intentionally supplies its normal floppy.
     * This probe's distinct subject is HDD firmware handoff, so remove that
     * declared removable medium through the production owner and reset before
     * executing.  It is not a second profile or boot-order configuration. */
    if (vm_machine_set_common_media(session, LIB_NULL,
            LIB_STORAGE_MEDIUM_OVERLAY) != LIB_STATUS_OK ||
        vm_machine_reset(session) != LIB_STATUS_OK) goto fail;
    while (executed < VM_HDC_HDD_BOOT_INSTRUCTION_BUDGET) {
        run_status = core_machine_run(session->core_machine, budget, &result);
        if (run_status != LIB_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT) {
            if (core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) ==
                LIB_STATUS_OK && diagnostic.first_fault.valid) {
                fprintf(stderr,
                    "M5:T213:S3:HDC:SYSTEM-FAULT reason=%u cs=%04X ip=%08X opcode=%02X\n",
                    (lib_u32)result.reason, diagnostic.first_fault.point.cs,
                    diagnostic.first_fault.point.eip,
                    diagnostic.first_fault.point.bytes[0]);
            } else {
                fprintf(stderr,
                    "M5:T213:S3:HDC:SYSTEM-STOP reason=%u status=%u count=%u\n",
                    (lib_u32)result.reason, (lib_u32)run_status,
                    session->core_machine->hdc.data.command_count);
            }
            goto fail;
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            lib_i32 advanced = 0;

            if (vm_machine_waiting_advance(session, &result, &advanced) != LIB_STATUS_OK ||
                !advanced) goto fail;
        }
        executed += result.executed;
        if (session->core_machine->hdc.data.command_count >= 2u &&
            session->core_machine->hdc.data.last_command == 0x20u &&
            vm_hdc_hdd_boot_matches_partition_vbr(session)) {
            loaded = 1;
            break;
        }
    }
    if (!loaded) {
        lib_u8 bytes[16] = {0};
        lib_u8 image_bytes[4] = {0};
        core_machine_media_result image_result;

        (void)core_machine_debug_read_memory(session->core_machine,
            VM_HDC_HDD_BOOT_ADDRESS, bytes, sizeof(bytes));
        (void)core_machine_media_read_bytes(session->media_registry,
            VM_MACHINE_MEDIA_HDD_ID, (lib_size)vm_hdc_hdd_boot_partition_lba(session) *
            VM_HDC_HDD_BOOT_BYTES, image_bytes, sizeof(image_bytes), &image_result);
        fprintf(stderr,
            "M5:T213:S3:HDC:SYSTEM-NO-HANDOFF count=%u command=%02X memory=%02X%02X%02X%02X expected=%02X%02X%02X%02X\n",
            session->core_machine->hdc.data.command_count,
            session->core_machine->hdc.data.last_command,
            bytes[0], bytes[1], bytes[2], bytes[3],
            image_bytes[0], image_bytes[1], image_bytes[2], image_bytes[3]);
        goto fail;
    }
    printf("M5:T287:S22:HDD-ONLY-BOOT:OK command=20 reads=%u instructions=%u\n",
        session->core_machine->hdc.data.command_count, executed);
    integration_ini_session_close(&ini_session);
    return 0;

fail:
    integration_ini_session_close(&ini_session);
    return 1;
}

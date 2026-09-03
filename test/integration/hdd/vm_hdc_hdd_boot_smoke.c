#include "type.h"

#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/waiting.h"
#include "vm/composition/session/session_private.h"
#include "test/integration/support/session_yaml.h"

#define VM_HDC_HDD_BOOT_ADDRESS 0x00007c00u
#define VM_HDC_HDD_BOOT_BYTES 512u
#define VM_HDC_HDD_PARTITION_TABLE_OFFSET 446u
#define VM_HDC_HDD_PARTITION_LBA_OFFSET 8u
#define VM_HDC_HDD_BOOT_INSTRUCTION_BUDGET 6000000u
#define VM_HDC_HDD_BOOT_QUANTUM 128u

static type_unsigned_32 vm_hdc_hdd_boot_partition_lba(const vm_session *session)
{
    const type_unsigned_8 *image;
    const type_unsigned_8 *entry;

    if (session == STD_NULL || session->hdd.connect.pImgBase == 0u) return 0u;
    image = (const type_unsigned_8 *)session->hdd.connect.pImgBase;
    entry = image + VM_HDC_HDD_PARTITION_TABLE_OFFSET;
    return (type_unsigned_32)entry[VM_HDC_HDD_PARTITION_LBA_OFFSET] |
        ((type_unsigned_32)entry[VM_HDC_HDD_PARTITION_LBA_OFFSET + 1u] << 8u) |
        ((type_unsigned_32)entry[VM_HDC_HDD_PARTITION_LBA_OFFSET + 2u] << 16u) |
        ((type_unsigned_32)entry[VM_HDC_HDD_PARTITION_LBA_OFFSET + 3u] << 24u);
}

static C_INT vm_hdc_hdd_boot_matches_partition_vbr(const vm_session *session)
{
    type_unsigned_8 boot_sector[VM_HDC_HDD_BOOT_BYTES];
    type_unsigned_32 partition_lba;
    const type_unsigned_8 *image;
    STD_SIZE_T index;

    if (session == STD_NULL || session->core_machine == STD_NULL ||
        session->hdd.connect.pImgBase == 0u ||
        core_machine_debug_read_memory(session->core_machine,
            VM_HDC_HDD_BOOT_ADDRESS, boot_sector, sizeof(boot_sector)) !=
            TYPE_STATUS_OK) {
        return 0;
    }
    partition_lba = vm_hdc_hdd_boot_partition_lba(session);
    if (partition_lba == 0u) return 0;
    image = (const type_unsigned_8 *)session->hdd.connect.pImgBase +
        (STD_SIZE_T)partition_lba * VM_HDC_HDD_BOOT_BYTES;
    /* The VBR is already executing when this boundary is observed. Its BPB
       contains boot-time writable fields, so compare its stable identity. */
    for (index = 0u; index < 11u; ++index) {
        if (boot_sector[index] != image[index]) {
            return 0;
        }
    }
    return boot_sector[510] == 0x55u && boot_sector[511] == 0xaau;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    integration_yaml_session yaml_session;
    const core_machine_run_budget budget = {
        VM_HDC_HDD_BOOT_QUANTUM, 0u
    };
    vm_session *session = STD_NULL;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_status run_status;
    type_unsigned_32 executed = 0u;
    C_INT loaded = 0;

    if (argc != 3 || integration_yaml_session_open(argv[1], argv[2],
            &yaml_session) != TYPE_STATUS_OK) return 77;
    session = yaml_session.session;
    if (!session->hdd.connect.flagDiskExist) goto fail;
    while (executed < VM_HDC_HDD_BOOT_INSTRUCTION_BUDGET) {
        run_status = core_machine_run(session->core_machine, budget, &result);
        if (run_status != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT) {
            if (core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) ==
                TYPE_STATUS_OK && diagnostic.first_fault.valid) {
                STD_FPRINTF(STD_STDERR,
                    "M5:T213:S3:HDC:SYSTEM-FAULT reason=%u cs=%04X ip=%08X opcode=%02X\n",
                    (C_UINT)result.reason, diagnostic.first_fault.point.cs,
                    diagnostic.first_fault.point.eip,
                    diagnostic.first_fault.point.bytes[0]);
            } else {
                STD_FPRINTF(STD_STDERR,
                    "M5:T213:S3:HDC:SYSTEM-STOP reason=%u status=%u count=%u\n",
                    (C_UINT)result.reason, (C_UINT)run_status,
                    session->core_machine->hdc.data.command_count);
            }
            goto fail;
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            C_INT advanced = 0;

            if (vm_session_waiting_advance(session, &result, &advanced) != TYPE_STATUS_OK ||
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
        type_unsigned_8 bytes[16] = {0};

        (C_VOID)core_machine_debug_read_memory(session->core_machine,
            VM_HDC_HDD_BOOT_ADDRESS, bytes, sizeof(bytes));
        STD_FPRINTF(STD_STDERR,
            "M5:T213:S3:HDC:SYSTEM-NO-HANDOFF count=%u command=%02X memory=%02X%02X%02X%02X expected=%02X%02X%02X%02X\n",
            session->core_machine->hdc.data.command_count,
            session->core_machine->hdc.data.last_command,
            bytes[0], bytes[1], bytes[2], bytes[3],
            TYPE_DEREFERENCE_UNSIGNED_8(session->hdd.connect.pImgBase +
                (STD_SIZE_T)vm_hdc_hdd_boot_partition_lba(session) * VM_HDC_HDD_BOOT_BYTES),
            TYPE_DEREFERENCE_UNSIGNED_8(session->hdd.connect.pImgBase +
                (STD_SIZE_T)vm_hdc_hdd_boot_partition_lba(session) * VM_HDC_HDD_BOOT_BYTES + 1u),
            TYPE_DEREFERENCE_UNSIGNED_8(session->hdd.connect.pImgBase +
                (STD_SIZE_T)vm_hdc_hdd_boot_partition_lba(session) * VM_HDC_HDD_BOOT_BYTES + 2u),
            TYPE_DEREFERENCE_UNSIGNED_8(session->hdd.connect.pImgBase +
                (STD_SIZE_T)vm_hdc_hdd_boot_partition_lba(session) * VM_HDC_HDD_BOOT_BYTES + 3u));
        goto fail;
    }
    STD_PRINTF("M5:T287:S22:HDD-ONLY-BOOT:OK command=20 reads=%u instructions=%u\n",
        session->core_machine->hdc.data.command_count, executed);
    integration_yaml_session_close(&yaml_session);
    return 0;

fail:
    integration_yaml_session_close(&yaml_session);
    return 1;
}

#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"
#include "core/machine/memory_interface.h"
#include "core/machine/port.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "core/machine/fdc.h"
#include "vm/machine/fdd.h"

#define VM_FDC_T242_IMAGE_BYTES (1440u * 1024u)

static type_unsigned_8 vm_fdc_t242_image[VM_FDC_T242_IMAGE_BYTES];

static C_INT vm_fdc_t242_write_boot_loop(C_CHAR path[MAX_PATH])
{
    STD_FILE *file;
    DWORD length;

    length = GetTempPathA(MAX_PATH, path);
    if (length == 0u || length >= MAX_PATH ||
        GetTempFileNameA(path, "n64", 0u, path) == 0u) return 0;
    STD_MEMSET(vm_fdc_t242_image, 0, sizeof(vm_fdc_t242_image));
    vm_fdc_t242_image[0] = 0xebu;
    vm_fdc_t242_image[1] = 0xfeu;
    vm_fdc_t242_image[510u] = 0x55u;
    vm_fdc_t242_image[511u] = 0xaau;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return 0;
    if (STD_FWRITE(vm_fdc_t242_image, 1u, sizeof(vm_fdc_t242_image), file) !=
        sizeof(vm_fdc_t242_image)) {
        STD_FCLOSE(file);
        DeleteFileA(path);
        return 0;
    }
    STD_FCLOSE(file);
    return 1;
}

static C_VOID vm_fdc_t242_write_dma2(t_port *port)
{
    core_machine_port_write(port, 0x000cu, 0u);
    core_machine_port_write(port, 0x0004u, 0x00u);
    core_machine_port_write(port, 0x0004u, 0x05u);
    core_machine_port_write(port, 0x0005u, 0xffu);
    core_machine_port_write(port, 0x0005u, 0x23u);
    core_machine_port_write(port, 0x0081u, 0u);
    core_machine_port_write(port, 0x000bu, 0x86u);
    core_machine_port_write(port, 0x000au, 0x02u);
}

static C_VOID vm_fdc_t242_command(core_machine_fdc *fdc, t_port *port,
    const type_unsigned_8 *bytes, STD_SIZE_T count)
{
    STD_SIZE_T index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    core_machine_fdc_advance(fdc);
    core_machine_fdc_advance(fdc);
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 specify_dma[] = {0x03u, 0xdfu, 0x02u};
    static const type_unsigned_8 read_track[] = {
        0x42u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x12u, 0x1bu, 0xffu
    };
    const vm_session_config config = {
        .floppy_image = { "" },
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    vm_session *session = STD_NULL;
    t_port *port;
    core_machine_run_budget boot_budget = {100000u, 0u};
    core_machine_run_budget transfer_budget = {12000u, 0u};
    core_machine_run_result run = {0};
    type_unsigned_8 expected[512u * 18u];
    type_unsigned_8 actual[sizeof(expected)] = {0};
    type_unsigned_8 untouched[sizeof(actual)] = {0};
    type_unsigned_8 result[7] = {0};
    C_CHAR path[MAX_PATH] = {0};
    STD_SIZE_T index;
    C_CHAR stage = '0';
    C_INT final_intr = 0;
    C_INT final_phase = 0;
    C_INT failed = 0;

    stage = '1';
    if (!vm_fdc_t242_write_boot_loop(path)) goto done;
    {
        vm_session_config fixture_config = config;
        fixture_config.floppy_image[0u] = path;
        if (vm_session_create(&fixture_config, &session) != TYPE_STATUS_OK ||
            session == STD_NULL) goto done;
    }
    stage = '2';
    if (core_machine_run(session->core_machine, boot_budget, &run) !=
            TYPE_STATUS_OK || run.reason == CORE_MACHINE_STOP_FAULT) {
        goto done;
    }
    port = session->core_machine->fdc.connect.port;
    stage = '3';
    if (port == STD_NULL) goto done;
    for (index = 0u; index < sizeof(expected); ++index) {
        expected[index] = (type_unsigned_8)((index * 17u) ^ (index >> 4u));
        if (vm_machine_fdd_write_byte(&session->fdd, 0u, 0u,
                (type_unsigned_16)(index / 512u + 1u),
                (type_unsigned_16)(index % 512u), expected[index])) {
            goto done;
        }
    }
    /* DOR NRS/ENRQ without selected-drive ME0 must fail before DMA can touch
     * RAM. ME1 does not make selected drive 0 ready either. */
    core_machine_port_write(port, 0x03f2u, 0x0cu);
    stage = '4';
    vm_fdc_t242_write_dma2(port);
    vm_fdc_t242_command(&session->core_machine->fdc, port, specify_dma, sizeof(specify_dma));
    vm_fdc_t242_command(&session->core_machine->fdc, port, read_track, sizeof(read_track));
    failed |= !session->core_machine->fdc.data.flagINTR;
    for (index = 0u; index < sizeof(result); ++index) {
        result[index] = (type_unsigned_8)core_machine_port_read(port, 0x03f5u);
    }
    failed |= result[0] != core_machine_fdc_ST0_ABNORMAL || result[1] != 0x04u;
    failed |= core_machine_memory_read(session->core_machine, 0x0500u, actual,
        sizeof(actual)) != TYPE_STATUS_OK || STD_MEMCMP(actual, untouched,
        sizeof(actual)) != 0;
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const type_unsigned_8[]){0x08u}, 1u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    failed |= session->core_machine->fdc.data.flagINTR;
    core_machine_port_write(port, 0x03f2u, 0x2cu);
    vm_fdc_t242_command(&session->core_machine->fdc, port, read_track, sizeof(read_track));
    failed |= !session->core_machine->fdc.data.flagINTR;
    for (index = 0u; index < sizeof(result); ++index) {
        result[index] = (type_unsigned_8)core_machine_port_read(port, 0x03f5u);
    }
    failed |= result[0] != core_machine_fdc_ST0_ABNORMAL || result[1] != 0x04u;
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const type_unsigned_8[]){0x08u}, 1u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    failed |= session->core_machine->fdc.data.flagINTR;
    core_machine_port_write(port, 0x03f2u, 0x1cu);
    vm_fdc_t242_write_dma2(port);
    vm_fdc_t242_command(&session->core_machine->fdc, port, read_track, sizeof(read_track));
    stage = '5';
    if (core_machine_run(session->core_machine, transfer_budget, &run) !=
            TYPE_STATUS_OK || run.reason == CORE_MACHINE_STOP_FAULT ||
        core_machine_memory_read(session->core_machine, 0x0500u, actual,
            sizeof(actual)) != TYPE_STATUS_OK) {
        goto done;
    }
    failed |= STD_MEMCMP(expected, actual, sizeof(expected)) != 0;
    stage = '6';
    if (session->core_machine->fdc.data.flagINTR) {
        for (index = 0u; index < sizeof(result); ++index) {
            result[index] = (type_unsigned_8)core_machine_port_read(port, 0x03f5u);
        }
        failed |= result[0] != core_machine_fdc_ST0_NORMAL || result[1] != 0u ||
            result[2] != 0u || result[3] != 0u || result[4] != 0u ||
            result[5] != 0x13u || result[6] != 0x02u;
        vm_fdc_t242_command(&session->core_machine->fdc, port,
            (const type_unsigned_8[]){0x08u}, 1u);
        (C_VOID)core_machine_port_read(port, 0x03f5u);
        (C_VOID)core_machine_port_read(port, 0x03f5u);
        failed |= session->core_machine->fdc.data.flagINTR;
    } else {
        failed |= session->core_machine->fdc.data.phase != core_machine_fdc_PHASE_COMMAND;
    }
    stage = '7';

    /* Non-MFM stays an owner-local no-data result, not a second command form. */
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const type_unsigned_8[]){
        0x02u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x12u, 0x1bu, 0xffu
    }, 9u);
    for (index = 0u; index < sizeof(result); ++index) {
        result[index] = (type_unsigned_8)core_machine_port_read(port, 0x03f5u);
    }
    failed |= result[0] != core_machine_fdc_ST0_ABNORMAL ||
        result[1] != 0x04u;
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const type_unsigned_8[]){0x08u}, 1u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    failed |= session->core_machine->fdc.data.flagINTR;

done:
    if (session != STD_NULL) {
        final_intr = session->core_machine->fdc.data.flagINTR;
        final_phase = session->core_machine->fdc.data.phase;
    }
    vm_session_destroy(session);
    if (path[0] != '\0') DeleteFileA(path);
    if (failed || session == STD_NULL) {
        STD_FPRINTF(STD_STDERR,
            "T242 read-track failed at %c, reason=%d, executed=%llu data=%02x/%02x result=%02x %02x %02x %02x %02x %02x %02x intr=%d phase=%d\n",
            stage, run.reason, (unsigned long long)run.executed, actual[512],
            expected[512], result[0], result[1], result[2], result[3], result[4],
            result[5], result[6], final_intr, final_phase);
        return 1;
    }
    STD_PRINTF("M5:T268:S1:FDC-MOTOR:PORT:OK\n");
    STD_PRINTF("M5:T242:S2:FDC:READ-TRACK:OK\n");
    return 0;
}

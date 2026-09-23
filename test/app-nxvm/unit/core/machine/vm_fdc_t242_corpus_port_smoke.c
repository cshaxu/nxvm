#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/memory_interface.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/devices/fdc.h"
#include "app-nxvm/machine/media/fdd.h"
#include "support/rom/session_assets.h"

#define VM_FDC_T242_IMAGE_BYTES (1440u * 1024u)

static lib_u8 vm_fdc_t242_image[VM_FDC_T242_IMAGE_BYTES];

static C_VOID vm_fdc_t242_boot_loop(C_VOID)
{
    lib_memory_set(vm_fdc_t242_image, 0, sizeof(vm_fdc_t242_image));
    vm_fdc_t242_image[0] = 0xebu;
    vm_fdc_t242_image[1] = 0xfeu;
    vm_fdc_t242_image[510u] = 0x55u;
    vm_fdc_t242_image[511u] = 0xaau;
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
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    core_machine_fdc_advance(fdc);
    core_machine_fdc_advance(fdc);
}

C_INT main(C_VOID)
{
    static const lib_u8 specify_dma[] = {0x03u, 0xdfu, 0x02u};
    static const lib_u8 read_track[] = {
        0x42u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x12u, 0x1bu, 0xffu
    };
    const vm_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    vm_machine *session = LIB_NULL;
    t_port *port;
    core_machine_run_result run = {0};
    lib_u8 expected[512u * 18u];
    lib_u8 actual[sizeof(expected)] = {0};
    lib_u8 untouched[sizeof(actual)] = {0};
    lib_u8 result[7] = {0};
    lib_size index;
    C_CHAR stage = '0';
    C_INT final_intr = 0;
    C_INT final_phase = 0;
    C_INT failed = 0;
    type_bool advanced = LIB_FALSE;

    stage = '1';
    vm_fdc_t242_boot_loop();
    {
        vm_machine_config fixture_config = config;
        if (vm_test_default_pc_at_session_create(&fixture_config, &session) != TYPE_STATUS_OK ||
            session == LIB_NULL) goto done;
        if (vm_machine_fdd_replace_bytes(&session->fdd, vm_fdc_t242_image,
                sizeof(vm_fdc_t242_image)) != 0) goto done;
    }
    port = session->core_machine->fdc.connect.port;
    stage = '3';
    if (port == LIB_NULL) goto done;
    for (index = 0u; index < sizeof(expected); ++index) {
        expected[index] = (lib_u8)((index * 17u) ^ (index >> 4u));
        if (vm_machine_fdd_write_byte(&session->fdd, 0u, 0u,
                (lib_u16)(index / 512u + 1u),
                (lib_u16)(index % 512u), expected[index])) {
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
        result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
    }
    failed |= result[0] != core_machine_fdc_ST0_ABNORMAL || result[1] != 0x04u;
    failed |= core_machine_memory_read(session->core_machine, 0x0500u, actual,
        sizeof(actual)) != TYPE_STATUS_OK || lib_memory_compare(actual, untouched,
        sizeof(actual)) != 0;
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const lib_u8[]){0x08u}, 1u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    failed |= session->core_machine->fdc.data.flagINTR;
    core_machine_port_write(port, 0x03f2u, 0x2cu);
    vm_fdc_t242_command(&session->core_machine->fdc, port, read_track, sizeof(read_track));
    failed |= !session->core_machine->fdc.data.flagINTR;
    for (index = 0u; index < sizeof(result); ++index) {
        result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
    }
    failed |= result[0] != core_machine_fdc_ST0_ABNORMAL || result[1] != 0x04u;
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const lib_u8[]){0x08u}, 1u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    failed |= session->core_machine->fdc.data.flagINTR;
    core_machine_port_write(port, 0x03f2u, 0x1cu);
    vm_fdc_t242_write_dma2(port);
    vm_fdc_t242_command(&session->core_machine->fdc, port, read_track, sizeof(read_track));
    stage = '5';
    for (index = 0u; index < 1024u && !session->core_machine->fdc.data.flagINTR;
            ++index) {
        if (core_machine_advance_to_next_deadline(session->core_machine,
                &advanced) != TYPE_STATUS_OK || !advanced) goto done;
    }
    if (!session->core_machine->fdc.data.flagINTR ||
        core_machine_memory_read(session->core_machine, 0x0500u, actual,
            sizeof(actual)) != TYPE_STATUS_OK) {
        goto done;
    }
    failed |= lib_memory_compare(expected, actual, sizeof(expected)) != 0;
    stage = '6';
    if (session->core_machine->fdc.data.flagINTR) {
        for (index = 0u; index < sizeof(result); ++index) {
            result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
        }
        failed |= result[0] != core_machine_fdc_ST0_NORMAL || result[1] != 0u ||
            result[2] != 0u || result[3] != 0u || result[4] != 0u ||
            result[5] != 0x13u || result[6] != 0x02u;
        vm_fdc_t242_command(&session->core_machine->fdc, port,
            (const lib_u8[]){0x08u}, 1u);
        (C_VOID)core_machine_port_read(port, 0x03f5u);
        (C_VOID)core_machine_port_read(port, 0x03f5u);
        failed |= session->core_machine->fdc.data.flagINTR;
    } else {
        failed |= session->core_machine->fdc.data.phase != core_machine_fdc_PHASE_COMMAND;
    }
    stage = '7';

    /* Non-MFM stays an owner-local no-data result, not a second command form. */
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const lib_u8[]){
        0x02u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x12u, 0x1bu, 0xffu
    }, 9u);
    for (index = 0u; index < sizeof(result); ++index) {
        result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
    }
    failed |= result[0] != core_machine_fdc_ST0_ABNORMAL ||
        result[1] != 0x04u;
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const lib_u8[]){0x08u}, 1u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    (C_VOID)core_machine_port_read(port, 0x03f5u);
    failed |= session->core_machine->fdc.data.flagINTR;

done:
    if (session != LIB_NULL) {
        final_intr = session->core_machine->fdc.data.flagINTR;
        final_phase = session->core_machine->fdc.data.phase;
    }
    vm_machine_destroy(session);
    if (failed || session == LIB_NULL) {
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

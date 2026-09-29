#include "../devices/support/fdc_fixture.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

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

static void vm_fdc_t242_boot_loop(void)
{
    lib_memory_set(vm_fdc_t242_image, 0, sizeof(vm_fdc_t242_image));
    vm_fdc_t242_image[0] = 0xebu;
    vm_fdc_t242_image[1] = 0xfeu;
    vm_fdc_t242_image[510u] = 0x55u;
    vm_fdc_t242_image[511u] = 0xaau;
}

static void vm_fdc_t242_write_dma2(t_port *port)
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

static void vm_fdc_t242_command(core_machine_fdc *fdc, t_port *port,
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;

    for (index = 0u; index < count; ++index) {
        core_machine_port_write(port, 0x03f5u, bytes[index]);
    }
    test_fdc_advance(fdc);
    test_fdc_advance(fdc);
}

lib_i32 main(void)
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
    char stage = '0';
    lib_i32 final_intr = 0;
    lib_i32 final_phase = 0;
    lib_i32 failed = 0;
    lib_u8 advanced = LIB_FALSE;

    stage = '1';
    vm_fdc_t242_boot_loop();
    {
        vm_machine_config fixture_config = config;
        if (vm_test_default_pc_at_session_create(&fixture_config, &session) != LIB_STATUS_OK ||
            session == LIB_NULL) goto done;
        if (vm_machine_fdd_replace_bytes(&session->fdd, vm_fdc_t242_image,
                sizeof(vm_fdc_t242_image)) != 0) goto done;
    }
    port = session->core_machine->fdc.connect.port;
    stage = '3';
    if (port == LIB_NULL) goto done;
    core_machine_port_write(port, 0x03f7u, 0x00u); /* 1.44MB: 500 kbps. */
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
    failed |= !test_fdc_interrupt_matches(&session->core_machine->fdc, LIB_TRUE);
    for (index = 0u; index < sizeof(result); ++index) {
        result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
    }
    failed |= result[0] != TEST_FDC_ST0_ABNORMAL || result[1] != 0x04u;
    failed |= core_machine_memory_read(session->core_machine, 0x0500u, actual,
        sizeof(actual)) != LIB_STATUS_OK || lib_memory_compare(actual, untouched,
        sizeof(actual)) != 0;
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const lib_u8[]){0x08u}, 1u);
    (void)core_machine_port_read(port, 0x03f5u);
    (void)core_machine_port_read(port, 0x03f5u);
    failed |= !test_fdc_interrupt_matches(&session->core_machine->fdc, LIB_FALSE);
    core_machine_port_write(port, 0x03f2u, 0x2cu);
    vm_fdc_t242_command(&session->core_machine->fdc, port, read_track, sizeof(read_track));
    failed |= !test_fdc_interrupt_matches(&session->core_machine->fdc, LIB_TRUE);
    for (index = 0u; index < sizeof(result); ++index) {
        result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
    }
    failed |= result[0] != TEST_FDC_ST0_ABNORMAL || result[1] != 0x04u;
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const lib_u8[]){0x08u}, 1u);
    (void)core_machine_port_read(port, 0x03f5u);
    (void)core_machine_port_read(port, 0x03f5u);
    failed |= !test_fdc_interrupt_matches(&session->core_machine->fdc, LIB_FALSE);
    core_machine_port_write(port, 0x03f2u, 0x1cu);
    vm_fdc_t242_write_dma2(port);
    vm_fdc_t242_command(&session->core_machine->fdc, port, read_track, sizeof(read_track));
    stage = '5';
    /* Bound phase-level progression for the entire 18-sector transfer,
     * not merely 1024 events. Reaching this bound is never success. */
    for (index = 0u; index < sizeof(expected) * 16u + 1024u &&
            !test_fdc_interrupt_matches(&session->core_machine->fdc, LIB_TRUE);
            ++index) {
        if (core_machine_advance_to_next_deadline(session->core_machine,
                &advanced) != LIB_STATUS_OK || !advanced) goto done;
    }
    if (!test_fdc_interrupt_matches(&session->core_machine->fdc, LIB_TRUE) ||
        core_machine_memory_read(session->core_machine, 0x0500u, actual,
            sizeof(actual)) != LIB_STATUS_OK) {
        goto done;
    }
    failed |= lib_memory_compare(expected, actual, sizeof(expected)) != 0;
    stage = '6';
    if (test_fdc_interrupt_matches(&session->core_machine->fdc, LIB_TRUE)) {
        for (index = 0u; index < sizeof(result); ++index) {
            result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
        }
        failed |= result[0] != TEST_FDC_ST0_NORMAL || result[1] != 0u ||
            result[2] != 0u || result[3] != 0u || result[4] != 0u ||
            result[5] != 0x13u || result[6] != 0x02u;
        vm_fdc_t242_command(&session->core_machine->fdc, port,
            (const lib_u8[]){0x08u}, 1u);
        (void)core_machine_port_read(port, 0x03f5u);
        (void)core_machine_port_read(port, 0x03f5u);
        failed |= !test_fdc_interrupt_matches(&session->core_machine->fdc, LIB_FALSE);
    } else {
        failed |= core_machine_port_read(port, 0x03f4u) != TEST_FDC_MSR_RQM;
    }
    stage = '7';

    /* Non-MFM stays an owner-local no-data result, not a second command form. */
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const lib_u8[]){
        0x02u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x12u, 0x1bu, 0xffu
    }, 9u);
    for (index = 0u; index < sizeof(result); ++index) {
        result[index] = (lib_u8)core_machine_port_read(port, 0x03f5u);
    }
    failed |= result[0] != TEST_FDC_ST0_ABNORMAL ||
        result[1] != 0x04u;
    vm_fdc_t242_command(&session->core_machine->fdc, port, (const lib_u8[]){0x08u}, 1u);
    (void)core_machine_port_read(port, 0x03f5u);
    (void)core_machine_port_read(port, 0x03f5u);
    failed |= !test_fdc_interrupt_matches(&session->core_machine->fdc, LIB_FALSE);

    stage = '8';
done:
    if (session != LIB_NULL) {
        x86_fdc_observation observation = {0};
        if (x86_fdc_capture(session->core_machine->fdc.chip, &observation) == LIB_STATUS_OK) {
            final_intr = observation.interrupt_pending;
            final_phase = observation.phase;
        }
        if (stage != '8') {
            core_machine_time_observation time;
            if (core_machine_capture_time_observation(session->core_machine, &time) == LIB_STATUS_OK) {
                fprintf(stderr, "T242 progress: tick=%llu deadline=%llu valid=%u disposition=%u remaining=%u\n",
                    (unsigned long long)time.elapsed_ticks,
                    (unsigned long long)time.next_deadline_tick,
                    time.next_deadline_valid, time.progress_disposition,
                    observation.transfer_remaining);
            }
        }
    }
    vm_machine_destroy(session);
    if (failed || stage != '8') {
        fprintf(stderr,
            "T242 read-track failed at %c, reason=%d, executed=%llu data=%02x/%02x result=%02x %02x %02x %02x %02x %02x %02x intr=%d phase=%d\n",
            stage, run.reason, (unsigned long long)run.executed, actual[512],
            expected[512], result[0], result[1], result[2], result[3], result[4],
            result[5], result[6], final_intr, final_phase);
        return 1;
    }
    printf("M5:T268:S1:FDC-MOTOR:PORT:OK\n");
    printf("M5:T242:S2:FDC:READ-TRACK:OK\n");
    return 0;
}

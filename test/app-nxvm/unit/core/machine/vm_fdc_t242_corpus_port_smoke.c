#include "app-nxvm/profiles/machine_factory_interface.h"
#include "lib/types/types_interface.h"
#include "../../../../x86/ibmpc-common/controller_fixture.h"
#include <stdio.h>

#include "x86/core/machine_interface.h"
#include "x86/core/memory_interface.h"
#include "x86/product/machine/machine_interface.h"
#include "x86/product/machine/machine_private.h"
#include "x86/product/machine/media/fdd_interface.h"
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

static lib_bool vm_fdc_t242_write_dma2(core_machine *machine)
{
    return core_machine_bus_write(machine, 0x000cu, 0u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x0004u, 0x00u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x0004u, 0x05u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x0005u, 0xffu) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x0005u, 0x23u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x0081u, 0u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x000bu, 0x86u) == LIB_STATUS_OK &&
        core_machine_bus_write(machine, 0x000au, 0x02u) == LIB_STATUS_OK;
}

static lib_bool vm_fdc_t242_command(core_machine_board_state *board, core_machine *machine,
    const lib_u8 *bytes, lib_size count)
{
    lib_size index;

    for (index = 0u; index < count; ++index)
        if (core_machine_bus_write(machine, 0x03f5u, bytes[index]) != LIB_STATUS_OK)
            return LIB_FALSE;
    return test_board_fdc_advance_ticks(board, 1u) &&
        test_board_fdc_advance_ticks(board, 1u);
}

static lib_bool vm_fdc_t242_read(core_machine *machine, lib_u8 *bytes, lib_size count)
{
    lib_u32 value;
    for (lib_size index = 0u; index < count; ++index) {
        if (core_machine_bus_read(machine, 0x03f5u, &value) != LIB_STATUS_OK)
            return LIB_FALSE;
        bytes[index] = (lib_u8)value;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    static const lib_u8 specify_dma[] = {0x03u, 0xdfu, 0x02u};
    static const lib_u8 read_track[] = {
        0x42u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x12u, 0x1bu, 0xffu
    };
    const vm_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    vm_machine *session = LIB_NULL;
    core_machine *machine;
    lib_u32 value;
    lib_u8 expected[512u * 18u] = {0};
    lib_u8 actual[sizeof(expected)] = {0};
    lib_u8 untouched[sizeof(actual)] = {0};
    lib_u8 result[7] = {0};
    lib_size index;
    char stage = '0';
    lib_i32 failed = 1;
    lib_bool advanced = LIB_FALSE;

    stage = '1';
    vm_fdc_t242_boot_loop();
    {
        vm_machine_config fixture_config = config;
        if (vm_test_default_pc_at_session_create(&fixture_config, &session) != LIB_STATUS_OK ||
            session == LIB_NULL || session->core_machine == LIB_NULL) goto done;
        if (vm_machine_fdd_replace_bytes(session->fdd, vm_fdc_t242_image,
                sizeof(vm_fdc_t242_image)) != 0) goto done;
    }
    machine = session->core_machine;
    stage = '3';
    /* 1.44MB: 500 kbps. */
    if (core_machine_bus_write(machine, 0x03f7u, 0x00u) != LIB_STATUS_OK) goto done;
    for (index = 0u; index < sizeof(expected); ++index) {
        expected[index] = (lib_u8)((index * 17u) ^ (index >> 4u));
        if (vm_machine_fdd_write_byte(session->fdd, 0u, 0u,
                (lib_u16)(index / 512u + 1u),
                (lib_u16)(index % 512u), expected[index])) {
            goto done;
        }
    }
    /* DOR NRS/ENRQ without selected-drive ME0 must fail before DMA can touch
     * RAM. ME1 does not make selected drive 0 ready either. */
    if (core_machine_bus_write(machine, 0x03f2u, 0x0cu) != LIB_STATUS_OK) goto done;
    stage = '4';
    if (!vm_fdc_t242_write_dma2(machine) ||
        !vm_fdc_t242_command(session->board, machine, specify_dma, sizeof(specify_dma)) ||
        !vm_fdc_t242_command(session->board, machine, read_track, sizeof(read_track)) ||
        !test_board_fdc_interrupt_matches(session->board, LIB_TRUE) ||
        !vm_fdc_t242_read(machine, result, sizeof(result)) ||
        result[0] != TEST_FDC_ST0_ABNORMAL || result[1] != 0x04u ||
        core_machine_memory_read(session->core_machine, 0x0500u, actual,
        sizeof(actual)) != LIB_STATUS_OK || lib_memory_compare(actual, untouched,
        sizeof(actual)) != 0 ||
        !vm_fdc_t242_command(session->board, machine, (const lib_u8[]){0x08u}, 1u) ||
        !vm_fdc_t242_read(machine, result, 2u) ||
        !test_board_fdc_interrupt_matches(session->board, LIB_FALSE) ||
        core_machine_bus_write(machine, 0x03f2u, 0x2cu) != LIB_STATUS_OK ||
        !vm_fdc_t242_command(session->board, machine, read_track, sizeof(read_track)) ||
        !test_board_fdc_interrupt_matches(session->board, LIB_TRUE) ||
        !vm_fdc_t242_read(machine, result, sizeof(result)) ||
        result[0] != TEST_FDC_ST0_ABNORMAL || result[1] != 0x04u ||
        !vm_fdc_t242_command(session->board, machine, (const lib_u8[]){0x08u}, 1u) ||
        !vm_fdc_t242_read(machine, result, 2u) ||
        !test_board_fdc_interrupt_matches(session->board, LIB_FALSE) ||
        core_machine_bus_write(machine, 0x03f2u, 0x1cu) != LIB_STATUS_OK ||
        !vm_fdc_t242_write_dma2(machine) ||
        !vm_fdc_t242_command(session->board, machine, read_track, sizeof(read_track)))
        goto done;
    stage = '5';
    /* Bound phase-level progression for the entire 18-sector transfer,
     * not merely 1024 events. Reaching this bound is never success. */
    for (index = 0u; index < sizeof(expected) * 16u + 1024u &&
            !test_board_fdc_interrupt_matches(session->board, LIB_TRUE);
            ++index) {
        if (core_machine_advance_to_next_deadline(session->core_machine,
                &advanced) != LIB_STATUS_OK || !advanced) goto done;
    }
    if (!test_board_fdc_interrupt_matches(session->board, LIB_TRUE) ||
        core_machine_memory_read(session->core_machine, 0x0500u, actual,
            sizeof(actual)) != LIB_STATUS_OK) {
        goto done;
    }
    if (lib_memory_compare(expected, actual, sizeof(expected)) != 0) goto done;
    stage = '6';
    if (test_board_fdc_interrupt_matches(session->board, LIB_TRUE)) {
        if (!vm_fdc_t242_read(machine, result, sizeof(result)) ||
            result[0] != TEST_FDC_ST0_NORMAL || result[1] != 0u ||
            result[2] != 0u || result[3] != 0u || result[4] != 0u ||
            result[5] != 0x13u || result[6] != 0x02u ||
            !vm_fdc_t242_command(session->board, machine,
                (const lib_u8[]){0x08u}, 1u) ||
            !vm_fdc_t242_read(machine, result, 2u) ||
            !test_board_fdc_interrupt_matches(session->board, LIB_FALSE)) goto done;
    } else {
        if (core_machine_bus_read(machine, 0x03f4u, &value) != LIB_STATUS_OK ||
            value != TEST_FDC_MSR_RQM) goto done;
    }
    stage = '7';

    /* Non-MFM stays an owner-local no-data result, not a second command form. */
    if (!vm_fdc_t242_command(session->board, machine, (const lib_u8[]){
        0x02u, 0x00u, 0x00u, 0x00u, 0x01u, 0x02u, 0x12u, 0x1bu, 0xffu
    }, 9u) || !vm_fdc_t242_read(machine, result, sizeof(result)) ||
        result[0] != TEST_FDC_ST0_ABNORMAL || result[1] != 0x04u ||
        !vm_fdc_t242_command(session->board, machine, (const lib_u8[]){0x08u}, 1u) ||
        !vm_fdc_t242_read(machine, result, 2u) ||
        !test_board_fdc_interrupt_matches(session->board, LIB_FALSE)) goto done;

    stage = '8';
    failed = 0;
done:
    vm_machine_destroy(session);
    if (failed) {
        fprintf(stderr,
            "T242 read-track failed at %c, data=%02x/%02x result=%02x %02x %02x %02x %02x %02x %02x\n",
            stage, actual[512],
            expected[512], result[0], result[1], result[2], result[3], result[4],
            result[5], result[6]);
        return 1;
    }
    printf("M5:T268:S1:FDC-MOTOR:PORT:OK\n");
    printf("M5:T242:S2:FDC:READ-TRACK:OK\n");
    return 0;
}

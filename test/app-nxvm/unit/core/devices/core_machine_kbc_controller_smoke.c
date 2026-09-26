#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/machine.h"

#include "app-nxvm/devices/cpu_instructions.h"
#include "app-nxvm/devices/kbc.h"
#include "app-nxvm/devices/memory.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/port.h"
#include "support/core_machine_cpu_fixture.h"

static lib_u8 core_machine_kbc_read_byte(t_port *port, lib_u16 port_id)
{
    return (lib_u8)core_machine_port_read(port, port_id);
}

static void core_machine_kbc_initialize_pic(t_port *port)
{
    core_machine_port_write(port, 0x0020u, 0x11u);
    core_machine_port_write(port, 0x0021u, 0x08u);
    core_machine_port_write(port, 0x0021u, 0x04u);
    core_machine_port_write(port, 0x0021u, 0x01u);
    core_machine_port_write(port, 0x00a0u, 0x11u);
    core_machine_port_write(port, 0x00a1u, 0x70u);
    core_machine_port_write(port, 0x00a1u, 0x02u);
    core_machine_port_write(port, 0x00a1u, 0x01u);
}

static lib_i32 core_machine_kbc_mixed_fifo_lifecycle(void)
{
    t_kbc kbc;
    t_pic pic_master;
    t_pic pic_slave;
    t_ram memory = {0};
    core_machine_cpu_execution_context execution = {0};
    t_port port;
    lib_i32 failed = 0;

    core_machine_port_initialize(&port);
    core_machine_pic_initialize(&pic_master, &pic_slave, &port, CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_kbc_bind_core_services(&kbc, &pic_master, &pic_slave,
        &memory, &execution, LIB_TRUE);
    core_machine_kbc_initialize_pic(&port);

    core_machine_port_write(&port, 0x0064u, 0xd4u);
    core_machine_port_write(&port, 0x0060u, 0xf4u);
    core_machine_pic_refresh(&pic_master, &pic_slave);
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x74u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x00a0u, 0x20u);
    core_machine_port_write(&port, 0x0020u, 0x20u);

    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0x01u) != LIB_STATUS_OK;
    core_machine_port_write(&port, 0x0064u, 0x20u);
    core_machine_pic_refresh(&pic_master, &pic_slave);
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x03u;
    core_machine_port_write(&port, 0x0020u, 0x20u);
    core_machine_pic_refresh(&pic_master, &pic_slave);
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x74u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x09u;
    core_machine_port_write(&port, 0x00a0u, 0x20u);
    core_machine_port_write(&port, 0x0020u, 0x20u);
    failed |= !kbc.data.irq12_asserted ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x01u ||
        !kbc.data.irq12_asserted ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x01u ||
        kbc.data.irq1_asserted || kbc.data.irq12_asserted ||
        (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_AUX) != 0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x43u;

    core_machine_kbc_set_command_response_timing(&kbc, 2u);
    core_machine_port_write(&port, 0x0064u, 0x20u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    core_machine_kbc_advance(&kbc, 2u);
    core_machine_pic_refresh(&pic_master, &pic_slave);
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x03u;
    core_machine_port_write(&port, 0x0020u, 0x20u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x43u;
    core_machine_kbc_set_command_response_timing(&kbc, 0u);

    failed |= core_machine_kbc_submit_aux_report(&kbc, 2, 2, 0u) != LIB_STATUS_OK;
    core_machine_kbc_reset(&kbc);
    failed |= kbc.data.fifo_count != 0u || kbc.data.irq1_asserted ||
        kbc.data.irq12_asserted || !kbc.data.keyboard_enabled ||
        !kbc.data.aux_enabled || !kbc.data.scanning_enabled;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        !kbc.data.irq1_asserted;
    core_machine_kbc_finalize(&kbc);
    failed |= kbc.data.irq1_asserted || kbc.data.irq12_asserted;

    core_machine_pic_finalize(&pic_master, &pic_slave);
    core_machine_port_finalize(&port);
    return failed;
}

static lib_i32 core_machine_kbc_set2_translation(void)
{
    static const lib_u8 function_set2[] = { 0x05u, 0x06u, 0x04u,
        0x0cu, 0x03u, 0x0bu, 0x83u, 0x0au, 0x01u, 0x09u, 0x78u, 0x07u };
    static const lib_u8 function_set1[] = { 0x3bu, 0x3cu, 0x3du,
        0x3eu, 0x3fu, 0x40u, 0x41u, 0x42u, 0x43u, 0x44u, 0x57u, 0x58u };
    static const lib_u8 pause_set2[] = { 0xe1u, 0x14u, 0x77u,
        0xe1u, 0xf0u, 0x14u, 0xf0u, 0x77u };
    static const lib_u8 pause_set1[] = { 0xe1u, 0x1du, 0x45u,
        0xe1u, 0x9du, 0xc5u };
    t_kbc kbc;
    t_port port;
    lib_u8 index;
    lib_i32 failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    kbc.connect.aux_present = LIB_FALSE;
    core_machine_kbc_reset(&kbc);
    failed |= (kbc.data.command_byte & CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u ||
        core_machine_kbc_submit_native_byte(&kbc, 0x05u) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x3bu;
    for (index = 0u; index < sizeof(function_set2); ++index) {
        failed |= core_machine_kbc_submit_native_byte(&kbc, function_set2[index]) !=
                LIB_STATUS_OK ||
            core_machine_kbc_read_byte(&port, 0x0060u) != function_set1[index];
    }
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x01u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1cu ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xf0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1cu;
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x41u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x9eu;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0xe0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x75u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xe0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x75u) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xe0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x48u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xe0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xc8u;
    for (index = 0u; index < sizeof(pause_set2); ++index) {
        failed |= core_machine_kbc_submit_native_byte(&kbc, pause_set2[index]) !=
            LIB_STATUS_OK;
    }
    for (index = 0u; index < sizeof(pause_set1); ++index) {
        failed |= core_machine_kbc_read_byte(&port, 0x0060u) != pause_set1[index];
    }
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}


static lib_i32 core_machine_kbc_set2_break_cancels_typematic(void)
{
    static const lib_u8 make_b[] = { 0x32u };
    static const lib_u8 break_b[] = { 0xf0u, 0x32u };
    static const lib_u8 break_return[] = { 0xf0u, 0x5au };
    t_kbc kbc;
    t_port port;
    lib_i32 failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x41u);
    core_machine_kbc_set_typematic_timing(&kbc, 1u, 1u);
    core_machine_kbc_set_serial_delivery_timing(&kbc, 2u);
    failed |= core_machine_kbc_submit_native_bytes(&kbc, make_b,
        sizeof(make_b)) != LIB_STATUS_OK || !kbc.data.typematic_active;
    failed |= core_machine_kbc_submit_native_bytes(&kbc, break_b,
        sizeof(break_b)) != LIB_STATUS_OK || kbc.data.typematic_active;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x30u;
    core_machine_kbc_advance(&kbc, 2u);
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xb0u ||
        kbc.data.typematic_active;
    failed |= core_machine_kbc_submit_native_bytes(&kbc, break_return,
        sizeof(break_return)) != LIB_STATUS_OK || kbc.data.typematic_active;
    if (kbc.data.typematic_active)
        fprintf(stderr, "KBC unmatched Return break incorrectly started typematic\n");
    failed |= core_machine_kbc_submit_native_bytes(&kbc, make_b,
        sizeof(make_b)) != LIB_STATUS_OK || !kbc.data.typematic_active;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x5au) != LIB_STATUS_OK ||
        !kbc.data.typematic_active || kbc.data.typematic_scan_code != 0x32u;
    failed |= core_machine_kbc_submit_native_bytes(&kbc, break_b,
        sizeof(break_b)) != LIB_STATUS_OK || kbc.data.typematic_active;
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}

static lib_i32 core_machine_kbc_self_test_flushes_keyboard_output(void)
{
    t_kbc kbc;
    t_port port;
    lib_i32 failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        kbc.data.fifo_count != 1u;
    core_machine_port_write(&port, 0x0064u, 0xaau);
    failed |= kbc.data.fifo_count != 1u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x55u;
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}

static lib_i32 core_machine_kbc_bat_on_line_enable(void)
{
    t_kbc kbc;
    t_port port;
    lib_i32 failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    /* C0h bit 7 is the board keyboard-inhibit switch, not the 8042's
       serial data line.  The 5170's B0h board straps must therefore not
       suppress the command-byte's 45h -> 4Dh release edge. */
    core_machine_kbc_set_input_port(&kbc, 0xb0u);
    /* The 5170 can release only the serial-line override (45h -> 4Dh); its
       keyboard-disable bit need not change.  The clock/data enable edge, not
       a BIOS special case, releases BAT. */
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x45u);
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x4du);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) == 0u;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau;
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}

static lib_i32 core_machine_kbc_controller_enable_is_not_a_second_bat(void)
{
    t_kbc kbc;
    t_port port;
    lib_i32 failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_kbc_set_input_port(&kbc, 0u);
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x45u);
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x4du);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau;
    core_machine_port_write(&port, 0x0064u, 0xadu);
    core_machine_port_write(&port, 0x0064u, 0xaeu);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}

static lib_i32 core_machine_kbc_self_test_enable_releases_bat(void)
{
    t_kbc kbc;
    t_port port;
    lib_i32 failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_port_write(&port, 0x0064u, 0xaau);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x55u;
    core_machine_port_write(&port, 0x0064u, 0xaeu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau;
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}

/* A BIOS may reset the keyboard while its serial line is inhibited, consume
 * FFh's FAh/AAh pair, and only then issue AEh.  The latter is an interface
 * release, not a second device power-on. */
static lib_i32 core_machine_kbc_reset_then_enable_has_one_bat(void)
{
    t_kbc kbc;
    t_port port;
    lib_i32 failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x7du);
    core_machine_port_write(&port, 0x0060u, 0xffu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau;
    core_machine_port_write(&port, 0x0064u, 0xaeu);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}

/* IBM 5170 Rev-3 POST has one continuous 8042 contract: controller self-test
 * inhibits the keyboard; the later 5Dh -> 4Dh command-byte edge supplies the
 * power-on BAT; the fallback keyboard reset then has to leave ABh's interface
 * result unpolluted.  Keep the ROM-visible ordering in one test rather than
 * proving the pieces independently. */
static lib_i32 core_machine_kbc_ibm_5170_post_contract(void)
{
    t_kbc kbc;
    t_port port;
    lib_i32 failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    kbc.connect.aux_present = LIB_FALSE;
    core_machine_kbc_reset(&kbc);
    core_machine_kbc_set_input_port(&kbc, 0xb0u);
    core_machine_kbc_set_command_response_status_polls(&kbc, 1u);

    core_machine_port_write(&port, 0x0064u, 0xaau);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x55u;
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x5du);
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x4du);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau;

    core_machine_port_write(&port, 0x0064u, 0xadu);
    (void)core_machine_kbc_read_byte(&port, 0x0060u);
    core_machine_port_write(&port, 0x0064u, 0xe0u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0u;

    core_machine_port_write(&port, 0x0060u, 0xffu);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau;
    core_machine_port_write(&port, 0x0064u, 0xabu);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0u;

    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}

/* Automatic repeats must obey the same single-byte output and inhibit
 * boundary as externally supplied scan bytes. */
static lib_i32 core_machine_kbc_typematic_output_boundary(void)
{
    t_kbc kbc;
    t_port port;
    lib_i32 failed;
    lib_u8 lines, ack, interface_result;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_kbc_set_typematic_timing(&kbc, 1u, 1u);
    failed = core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != LIB_STATUS_OK;
    core_machine_kbc_advance(&kbc, 4u);
    failed |= kbc.data.fifo_count != 1u;
    if (kbc.data.fifo_count != 1u)
        fprintf(stderr, "KBC repeat observation: output-bytes=%u\n",
            (unsigned int)kbc.data.fifo_count);
    core_machine_port_write(&port, 0x0064u, 0xadu);
    while (kbc.data.fifo_count != 0u)
        (void)core_machine_kbc_read_byte(&port, 0x0060u);
    core_machine_kbc_advance(&kbc, 5u);
    failed |= kbc.data.fifo_count != 0u;
    if (kbc.data.fifo_count != 0u)
        fprintf(stderr, "KBC inhibited repeat observation: output-bytes=%u\n",
            (unsigned int)kbc.data.fifo_count);
    /* Diagnostic replay of the ROM's flush / E0h / FFh / ABh ordering.
     * This isolates controller state; it is not a full-ROM reproduction. */
    core_machine_kbc_set_command_response_status_polls(&kbc, 1u);
    (void)core_machine_kbc_read_byte(&port, 0x0060u);
    core_machine_port_write(&port, 0x0064u, 0xe0u);
    (void)core_machine_kbc_read_byte(&port, 0x0064u);
    lines = core_machine_kbc_read_byte(&port, 0x0060u);
    core_machine_port_write(&port, 0x0060u, 0xffu);
    if ((core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u)
        (void)core_machine_kbc_read_byte(&port, 0x0060u);
    (void)core_machine_kbc_read_byte(&port, 0x0064u);
    ack = core_machine_kbc_read_byte(&port, 0x0060u);
    if (ack == 0xfau)
        failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau;
    core_machine_port_write(&port, 0x0064u, 0xabu);
    (void)core_machine_kbc_read_byte(&port, 0x0064u);
    interface_result = core_machine_kbc_read_byte(&port, 0x0060u);
    failed |= lines != 0u || ack != 0xfau || interface_result != 0u;
    if (failed) fprintf(stderr, "KBC output/inhibit replay: E0=%02X FF=%02X AB=%02X\n",
        (unsigned int)lines, (unsigned int)ack, (unsigned int)interface_result);
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}

typedef struct core_machine_kbc_cpu_fixture {
    core_machine *machine;
} core_machine_kbc_cpu_fixture;

static void core_machine_kbc_cpu_reset(void *opaque)
{
    core_machine_kbc_cpu_fixture *fixture = opaque;

    if (fixture != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(fixture->machine);
}

static const core_machine_execution_provider core_machine_kbc_cpu_provider = {
    core_machine_kbc_cpu_reset, LIB_NULL
};

/* Keep the POST-relevant path owner-local: a real CPU issues FFh, reads its
 * synchronous FAh, then receives the queued AAh through IRQ1 after STI's
 * architectural interrupt shadow. */
static lib_i32 core_machine_kbc_cpu_reset_irq1(void)
{
    static const lib_u8 code[] = {
        0xb0u, 0xffu, 0xe6u, 0x60u, 0xe4u, 0x60u, 0xfbu, 0x90u
    };
    static const lib_u8 handler = 0xf4u;
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .shared_pit_personality = CORE_MACHINE_PIT_PERSONALITY_8253
    };
    core_machine_kbc_cpu_fixture fixture = {0};
    core_machine_run_result result;
    lib_u16 offset = 0x0100u;
    lib_u16 segment = 0u;
    lib_i32 failed = !test_core_machine_fixture_create_bind_freeze_reset(&config,
        &core_machine_kbc_cpu_provider, &fixture, &fixture.machine);

    if (!failed) {
        core_machine_kbc_initialize_pic(&fixture.machine->executor_port);
        core_machine_port_write(&fixture.machine->executor_port, 0x0021u, 0xfdu);
        failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                fixture.machine, 0u) ||
            core_machine_memory_write(fixture.machine, 0u, code, sizeof(code)) !=
                LIB_STATUS_OK ||
            core_machine_memory_write(fixture.machine, 0x0024u, &offset,
                sizeof(offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(fixture.machine, 0x0026u, &segment,
                sizeof(segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(fixture.machine, offset, &handler,
                sizeof(handler)) != LIB_STATUS_OK;
    }
    if (!failed) {
        failed |= core_machine_run(fixture.machine, (core_machine_run_budget){3u, 0u},
                &result) != LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            fixture.machine->executor_cpu.data.eip != 6u ||
            fixture.machine->shared_kbc.data.keyboard_bat_pending ||
            fixture.machine->shared_kbc.data.fifo_count != 1u ||
            fixture.machine->shared_kbc.data.fifo[
                fixture.machine->shared_kbc.data.fifo_head] != 0xaau ||
            !fixture.machine->shared_kbc.data.irq1_asserted;
    }
    if (!failed) {
        failed |= core_machine_run(fixture.machine, (core_machine_run_budget){2u, 0u},
                &result) != LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            fixture.machine->executor_cpu.data.eip != offset ||
            !CORE_MACHINE_BIT_IS_SET(fixture.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(1u));
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

lib_i32 main(void)
{
    t_kbc kbc;
    t_pic pic_master;
    t_pic pic_slave;
    t_ram memory = {0};
    core_machine_cpu_execution_context execution = {0};
    t_port port;
    lib_i32 failed = 0;
    lib_i32 mixed_failed;
    lib_i32 translation_failed;
    lib_i32 self_test_flush_failed;
    lib_i32 typematic_break_failed;
    lib_i32 line_bat_failed;
    lib_i32 controller_enable_bat_failed;
    lib_i32 self_test_enable_bat_failed;
    lib_i32 reset_enable_bat_failed;
    lib_i32 ibm_5170_post_contract_failed;
    lib_i32 cpu_reset_irq1_failed;
    lib_u8 index;

    core_machine_port_initialize(&port);
    core_machine_pic_initialize(&pic_master, &pic_slave, &port, CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_kbc_bind_core_services(&kbc, &pic_master, &pic_slave,
        &memory, &execution, LIB_TRUE);
    core_machine_kbc_initialize_pic(&port);

    mixed_failed = core_machine_kbc_mixed_fifo_lifecycle();
    translation_failed = core_machine_kbc_set2_translation();
    self_test_flush_failed = core_machine_kbc_self_test_flushes_keyboard_output();
    typematic_break_failed = core_machine_kbc_set2_break_cancels_typematic();
    line_bat_failed = core_machine_kbc_bat_on_line_enable();
    controller_enable_bat_failed = core_machine_kbc_controller_enable_is_not_a_second_bat();
    self_test_enable_bat_failed = core_machine_kbc_self_test_enable_releases_bat();
    reset_enable_bat_failed = core_machine_kbc_reset_then_enable_has_one_bat();
    ibm_5170_post_contract_failed = core_machine_kbc_ibm_5170_post_contract();
    cpu_reset_irq1_failed = core_machine_kbc_cpu_reset_irq1();
    failed |= mixed_failed;
    failed |= translation_failed;
    failed |= self_test_flush_failed;
    failed |= typematic_break_failed;
    failed |= line_bat_failed;
    failed |= controller_enable_bat_failed;
    failed |= self_test_enable_bat_failed;
    failed |= reset_enable_bat_failed;
    failed |= ibm_5170_post_contract_failed;
    failed |= cpu_reset_irq1_failed;
    failed |= core_machine_kbc_typematic_output_boundary();

    failed |= core_machine_kbc_read_byte(&port, 0x0064u) != 0x10u;
    core_machine_kbc_set_input_port(&kbc, 0x80u);
    core_machine_kbc_set_test_inputs(&kbc, 0x03u);
    core_machine_port_write(&port, 0x0064u, 0xc0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x80u;
    core_machine_port_write(&port, 0x0064u, 0xe0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x03u;
    core_machine_kbc_set_input_port(&kbc, 0u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_INHIBIT) != 0u ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_INVALID_STATE;
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x09u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_set_input_port(&kbc, 0x80u);
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x47u);
    core_machine_port_write(&port, 0x0064u, 0x20u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) == 0u;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x47u;
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x01u);
    core_machine_port_write(&port, 0x0064u, 0x20u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x01u;
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x01u);
    core_machine_port_write(&port, 0x0064u, 0x20u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x01u ||
        (kbc.data.command_byte & CORE_MACHINE_KBC_COMMAND_TRANSLATION) != 0u;

    core_machine_port_write(&port, 0x0064u, 0xadu);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) !=
        LIB_STATUS_INVALID_STATE;
    core_machine_port_write(&port, 0x0064u, 0xaeu);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= (pic_master.data.irr & VPIC_IRR_IRQ(1u)) == 0u;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u;
    core_machine_port_write(&port, 0x0020u, 0x20u);

    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x30u) != LIB_STATUS_OK;
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_port_write(&port, 0x0020u, 0x20u);
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x30u;
    core_machine_port_write(&port, 0x0020u, 0x20u);

    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x41u);
    core_machine_port_write(&port, 0x0060u, 0xf2u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xabu ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x41u;
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u;
    core_machine_port_write(&port, 0x0020u, 0x20u);
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x01u);

    core_machine_port_write(&port, 0x0060u, 0xf2u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xabu;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x83u;
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u;
    core_machine_port_write(&port, 0x0020u, 0x20u);
    core_machine_port_write(&port, 0x0064u, 0xaau);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_SYS) == 0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x55u ||
        kbc.data.keyboard_enabled ||
        (kbc.data.command_byte & CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD) == 0u ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) !=
            LIB_STATUS_INVALID_STATE ||
        (pic_master.data.irr & VPIC_IRR_IRQ(1u)) != 0u;
    core_machine_kbc_set_command_response_status_polls(&kbc, 1u);
    core_machine_port_write(&port, 0x0064u, 0xaau);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x55u;
    core_machine_kbc_set_command_response_status_polls(&kbc, 0u);
    core_machine_port_write(&port, 0x0064u, 0xaeu);
    failed |= !kbc.data.keyboard_enabled ||
        (kbc.data.command_byte & CORE_MACHINE_KBC_COMMAND_DISABLE_KEYBOARD) != 0u ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        (pic_master.data.irr & VPIC_IRR_IRQ(1u)) == 0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_set_command_response_status_polls(&kbc, 1u);
    core_machine_port_write(&port, 0x0060u, 0xffu);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau;
    core_machine_kbc_set_command_response_status_polls(&kbc, 0u);
    core_machine_port_write(&port, 0x0020u, 0x20u);
    core_machine_port_write(&port, 0x0064u, 0xabu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0u;

    core_machine_port_write(&port, 0x0060u, 0xf0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0x00u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) !=
        CORE_MACHINE_KEYBOARD_SCAN_SET_2;
    core_machine_port_write(&port, 0x0060u, 0xf0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0x01u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        kbc.data.scan_set != CORE_MACHINE_KEYBOARD_SCAN_SET_1;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_port_write(&port, 0x0060u, 0xf0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0x02u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        kbc.data.scan_set != CORE_MACHINE_KEYBOARD_SCAN_SET_2;

    core_machine_port_write(&port, 0x0060u, 0xedu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0x07u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        kbc.data.led_state != 0x07u;
    core_machine_kbc_set_typematic_timing(&kbc, 240u, 48u);
    core_machine_port_write(&port, 0x0060u, 0xf3u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0x1fu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        kbc.data.typematic != 0x1fu ||
        kbc.data.typematic_initial_ticks != 120u ||
        kbc.data.typematic_repeat_ticks != 240u;
    core_machine_port_write(&port, 0x0060u, 0xf3u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0x2cu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        kbc.data.typematic_initial_ticks != 240u ||
        kbc.data.typematic_repeat_ticks != 48u;

    core_machine_kbc_set_typematic_timing(&kbc, 0u, 0u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 1000000u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;

    core_machine_kbc_set_typematic_timing(&kbc, 3u, 2u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xf0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 8u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;

    core_machine_kbc_set_command_response_timing(&kbc, 2u);
    core_machine_port_write(&port, 0x0060u, 0xeeu);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xeeu;
    core_machine_kbc_set_command_response_timing(&kbc, 0u);

    core_machine_port_write(&port, 0x0060u, 0xf5u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        kbc.data.led_state != 0u || kbc.data.typematic != 0x2cu ||
        kbc.data.typematic_initial_ticks != 3u ||
        kbc.data.typematic_repeat_ticks != 2u ||
        kbc.data.scan_set != CORE_MACHINE_KEYBOARD_SCAN_SET_2 ||
        kbc.data.scanning_enabled || kbc.data.typematic_active ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) !=
            LIB_STATUS_INVALID_STATE;
    core_machine_port_write(&port, 0x0060u, 0xf4u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_port_write(&port, 0x0060u, 0xf6u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        kbc.data.led_state != 0u || kbc.data.typematic != 0x2cu ||
        kbc.data.typematic_initial_ticks != 3u ||
        kbc.data.typematic_repeat_ticks != 2u ||
        !kbc.data.scanning_enabled;
    core_machine_port_write(&port, 0x0060u, 0xfdu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        !kbc.data.scanning_enabled || kbc.data.led_state != 0u ||
        kbc.data.typematic != 0x2cu;
    core_machine_port_write(&port, 0x0060u, 0xfeu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x0060u, 0xffu);
    failed |= !kbc.data.keyboard_bat_pending || kbc.data.fifo_count != 1u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        kbc.data.keyboard_bat_pending || kbc.data.fifo_count != 1u ||
        kbc.data.fifo[kbc.data.fifo_head] != 0xaau || !kbc.data.irq1_asserted ||
        (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau ||
        kbc.data.scan_set != CORE_MACHINE_KEYBOARD_SCAN_SET_2;
    core_machine_kbc_set_command_response_timing(&kbc, 2u);
    core_machine_port_write(&port, 0x0060u, 0xffu);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau;
    core_machine_kbc_set_command_response_timing(&kbc, 0u);
    core_machine_port_write(&port, 0x0060u, 0x00u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfeu;
    core_machine_port_write(&port, 0x0060u, 0xfeu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_port_write(&port, 0x0064u, 0xd0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x01u;
    core_machine_port_write(&port, 0x0060u, 0xfeu);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;

    core_machine_port_write(&port, 0x0064u, 0xd1u);
    core_machine_port_write(&port, 0x0060u, 0x03u);
    failed |= !memory.data.flagA20;
    core_machine_port_write(&port, 0x0064u, 0xd0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x03u;
    core_machine_port_write(&port, 0x0064u, 0xffu);
    failed |= core_machine_cpu_execution_consume_reset_request(&execution) ||
        !memory.data.flagA20;
    core_machine_port_write(&port, 0x0064u, 0xfeu);
    failed |= !core_machine_cpu_execution_consume_reset_request(&execution) ||
        !memory.data.flagA20;
    core_machine_port_write(&port, 0x0064u, 0xd0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x03u;
    core_machine_port_write(&port, 0x0064u, 0xd1u);
    core_machine_port_write(&port, 0x0060u, 0x00u);
    failed |= memory.data.flagA20 ||
        !core_machine_cpu_execution_consume_reset_request(&execution);

    core_machine_kbc_reset(&kbc);
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x07u);
    {
        static const lib_u8 enter_break[] = { 0xf0u, 0x1eu };

        core_machine_kbc_set_typematic_timing(&kbc, 3u, 2u);
        failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
            core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu ||
            !kbc.data.typematic_active;
        for (index = 0u; index < 3u; ++index) {
            failed |= core_machine_kbc_submit_native_byte(&kbc, 0xe0u) != LIB_STATUS_OK;
        }
        /* A full physical OBF must not discard a complete Set-2 break;
         * accepting it cancels typematic before the bytes become CPU-visible. */
        failed |= core_machine_kbc_submit_native_bytes(&kbc, enter_break,
            sizeof(enter_break)) != LIB_STATUS_OK || kbc.data.keyboard_serial_count !=
            4u || kbc.data.typematic_active;
    }
    /* A command reply behind rapid typeahead remains KBC-owned until the
     * guest drains the one physical output buffer; it is never lost. */
    core_machine_port_write(&port, 0x0060u, 0xf2u);
    for (index = 0u; index < 3u; ++index) {
        failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xe0u;
        core_machine_kbc_advance(&kbc, 0u);
    }
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xf0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu ||
        kbc.data.keyboard_serial_count != 0u;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xabu;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x83u;

    core_machine_kbc_finalize(&kbc);
    core_machine_pic_finalize(&pic_master, &pic_slave);
    core_machine_port_finalize(&port);
    if (failed) {
        fprintf(stderr,
            "M5:T464:S2:KBC:FAIL:mixed=%d:translation=%d:self-flush=%d:typematic=%d:line-bat=%d:enable-bat=%d:self-enable-bat=%d:reset-enable-bat=%d:ibm-post=%d:cpu-irq1=%d\n",
            mixed_failed, translation_failed, self_test_flush_failed,
            typematic_break_failed, line_bat_failed, controller_enable_bat_failed,
            self_test_enable_bat_failed, reset_enable_bat_failed,
            ibm_5170_post_contract_failed, cpu_reset_irq1_failed);
        return 1;
    }
    printf("M5:T464:S2:KBC:OK\n");
    return 0;
}

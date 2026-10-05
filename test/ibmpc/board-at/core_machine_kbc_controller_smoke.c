#include "lib/types/file.h"
#include "kbc_fixture.h"
#include "controller_fixture.h"
#include "../board-common/pic_fixture.h"
#include "../board-common/kbc_irq_fixture.h"
lib_status test_kbc_initialize(t_kbc *kbc, core_machine *machine)
{
    return core_machine_kbc_initialize(kbc, machine);
}

void test_kbc_finalize(t_kbc *kbc)
{
    core_machine_kbc_finalize(kbc);
}

lib_bool test_kbc_unpublished(const t_kbc *kbc)
{
    return kbc->connect.aux_device == LIB_NULL &&
        kbc->connect.keyboard == LIB_NULL && kbc->chip == LIB_NULL;
}

lib_bool test_kbc_bat_ready(const t_kbc *kbc)
{
    return x86_keyboard_get_signals(kbc->connect.keyboard).bat_ready;
}

static lib_bool kbc_construction_rollback(void)
{
    t_kbc kbc;
    return test_kbc_core_construction(&kbc);
}

static lib_u8 core_machine_kbc_read_byte(core_machine *port, lib_u16 port_id)
{
    return (lib_u8)test_kbc_port_read(port, port_id);
}

static void count_reset_pulse(void *context)
{
    ++*(lib_u32 *)context;
}

static void test_signal_a20(void *context, lib_bool enabled)
{
    *(lib_bool *)context = enabled;
}

static lib_bool keyboard_has_repeat(const t_kbc *kbc)
{
    lib_u64 ticks;
    return x86_keyboard_ticks_until_repeat(kbc->connect.keyboard, &ticks) == LIB_STATUS_OK;
}

static void keyboard_repeat_byte(void *context, lib_u8 byte)
{
    *(lib_u8 *)context = byte;
}

static lib_bool keyboard_cadence_is(t_kbc *kbc, lib_u64 initial, lib_u64 interval)
{
    x86_keyboard *keyboard = kbc->connect.keyboard;
    lib_u64 ticks = 0u;
    lib_u8 byte = 0u;
    lib_bool failed = x86_keyboard_admit(keyboard, 0x1cu) != LIB_STATUS_OK;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != initial;
    failed |= !x86_keyboard_advance(keyboard, initial, keyboard_repeat_byte, &byte) || byte != 0x1cu;
    failed |= x86_keyboard_ticks_until_repeat(keyboard, &ticks) != LIB_STATUS_OK || ticks != interval;
    failed |= x86_keyboard_admit(keyboard, 0xf0u) != LIB_STATUS_OK;
    failed |= x86_keyboard_admit(keyboard, 0x1cu) != LIB_STATUS_OK;
    return !failed;
}

void test_kbc_initialize_pic(core_machine *port)
{
    test_kbc_port_write(port, 0x0020u, 0x11u);
    test_kbc_port_write(port, 0x0021u, 0x08u);
    test_kbc_port_write(port, 0x0021u, 0x04u);
    test_kbc_port_write(port, 0x0021u, 0x01u);
    test_kbc_port_write(port, 0x00a0u, 0x11u);
    test_kbc_port_write(port, 0x00a1u, 0x70u);
    test_kbc_port_write(port, 0x00a1u, 0x02u);
    test_kbc_port_write(port, 0x00a1u, 0x01u);
}

static lib_i32 core_machine_kbc_mixed_fifo_lifecycle(void)
{
    t_kbc kbc;
    test_kbc_irq_wiring irq = {0};
    core_machine_pic_bus *pic_master = LIB_NULL;
    core_machine_pic_bus *pic_slave = LIB_NULL;
    core_machine *machine = test_kbc_create_executor();
    core_machine *port = machine;
    lib_i32 failed = 0;

    core_machine_pic_initialize(&pic_master, &pic_slave, machine, CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_kbc_initialize(&kbc, machine);
    failed |= test_kbc_bind_irq(&irq, pic_master, pic_slave) != LIB_STATUS_OK;
    core_machine_kbc_bind_core_services(&kbc, test_kbc_irq_output, &irq,
        LIB_NULL, LIB_NULL, LIB_NULL, LIB_NULL, LIB_TRUE);
    test_kbc_ready(port);
    test_kbc_initialize_pic(port);

    test_kbc_port_write(port, 0x0064u, 0xd4u);
    test_kbc_port_write(port, 0x0060u, 0xf4u);
    core_machine_pic_refresh(pic_master, pic_slave);
    failed |= core_machine_pic_get_interrupt(pic_master, pic_slave) != 0x74u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xfau;
    test_kbc_port_write(port, 0x00a0u, 0x20u);
    test_kbc_port_write(port, 0x0020u, 0x20u);

    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0x01u) != LIB_STATUS_OK;
    test_kbc_port_write(port, 0x0064u, 0x20u);
    core_machine_pic_refresh(pic_master, pic_slave);
    failed |= core_machine_pic_get_interrupt(pic_master, pic_slave) != 0x09u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x03u;
    test_kbc_port_write(port, 0x0020u, 0x20u);
    core_machine_pic_refresh(pic_master, pic_slave);
    failed |= core_machine_pic_get_interrupt(pic_master, pic_slave) != 0x74u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x09u;
    test_kbc_port_write(port, 0x00a0u, 0x20u);
    test_kbc_port_write(port, 0x0020u, 0x20u);
    failed |= !core_machine_pic_irq_source_is_asserted(irq.auxiliary) ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x01u ||
        !core_machine_pic_irq_source_is_asserted(irq.auxiliary) ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x01u ||
        core_machine_pic_irq_source_is_asserted(irq.keyboard) || core_machine_pic_irq_source_is_asserted(irq.auxiliary) ||
        (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_AUX) != 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x43u;

    core_machine_kbc_set_command_response_timing(&kbc, 2u);
    test_kbc_port_write(port, 0x0064u, 0x20u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    core_machine_kbc_advance(&kbc, 2u);
    core_machine_pic_refresh(pic_master, pic_slave);
    failed |= core_machine_pic_get_interrupt(pic_master, pic_slave) != 0x09u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x03u;
    test_kbc_port_write(port, 0x0020u, 0x20u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x43u;
    core_machine_kbc_set_command_response_timing(&kbc, 0u);

    failed |= core_machine_kbc_submit_aux_report(&kbc, 2, 2, 0u) != LIB_STATUS_OK;
    core_machine_kbc_reset(&kbc);
    failed |= (core_machine_kbc_read_byte(port, 0x64u) & VKBC_STATUS_OBF) != 0u || core_machine_pic_irq_source_is_asserted(irq.keyboard) ||
        core_machine_pic_irq_source_is_asserted(irq.auxiliary) ||
        !x86_kbc8042_aux_enabled(kbc.chip) || !x86_keyboard_get_signals(kbc.connect.keyboard).scanning;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        !core_machine_pic_irq_source_is_asserted(irq.keyboard);
    core_machine_kbc_finalize(&kbc);
    failed |= core_machine_pic_irq_source_is_asserted(irq.keyboard) || core_machine_pic_irq_source_is_asserted(irq.auxiliary);

    core_machine_pic_finalize(pic_master, pic_slave);
    core_machine_destroy(port);
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
    core_machine *port;
    lib_u8 index;
    lib_i32 failed = 0;

    port = test_kbc_create(&kbc);
    x86_kbc8042_set_aux_present(kbc.chip, LIB_FALSE);
    core_machine_kbc_reset(&kbc);
    test_kbc_port_write(port, 0x64u, 0x20u);
    failed |= (core_machine_kbc_read_byte(port, 0x60u) & CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u ||
        core_machine_kbc_submit_native_byte(&kbc, 0x05u) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x3bu;
    for (index = 0u; index < sizeof(function_set2); ++index) {
        failed |= core_machine_kbc_submit_native_byte(&kbc, function_set2[index]) !=
                LIB_STATUS_OK ||
            core_machine_kbc_read_byte(port, 0x0060u) != function_set1[index];
    }
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x01u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x1cu ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xf0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x1cu;
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x41u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x9eu;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0xe0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x75u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xe0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x75u) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xe0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x48u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xe0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xc8u;
    for (index = 0u; index < sizeof(pause_set2); ++index) {
        failed |= core_machine_kbc_submit_native_byte(&kbc, pause_set2[index]) !=
            LIB_STATUS_OK;
    }
    for (index = 0u; index < sizeof(pause_set1); ++index) {
        failed |= core_machine_kbc_read_byte(port, 0x0060u) != pause_set1[index];
    }
    core_machine_kbc_finalize(&kbc);
    core_machine_destroy(port);
    return failed;
}


static lib_i32 core_machine_kbc_set2_break_cancels_typematic(void)
{
    static const lib_u8 make_b[] = { 0x32u };
    static const lib_u8 break_b[] = { 0xf0u, 0x32u };
    static const lib_u8 break_return[] = { 0xf0u, 0x5au };
    t_kbc kbc;
    core_machine *port;
    lib_i32 failed = 0;

    port = test_kbc_create(&kbc);
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x41u);
    core_machine_kbc_set_typematic_timing(&kbc, 1u, 1u);
    core_machine_kbc_set_serial_delivery_timing(&kbc, 2u);
    failed |= core_machine_kbc_submit_native_bytes(&kbc, make_b,
        sizeof(make_b)) != LIB_STATUS_OK || !keyboard_has_repeat(&kbc);
    failed |= core_machine_kbc_submit_native_bytes(&kbc, break_b,
        sizeof(break_b)) != LIB_STATUS_OK || keyboard_has_repeat(&kbc);
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x30u;
    core_machine_kbc_advance(&kbc, 2u);
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xb0u ||
        keyboard_has_repeat(&kbc);
    failed |= core_machine_kbc_submit_native_bytes(&kbc, break_return,
        sizeof(break_return)) != LIB_STATUS_OK || keyboard_has_repeat(&kbc);
    if (keyboard_has_repeat(&kbc))
        lib_c_fprintf(lib_c_stderr, "KBC unmatched Return break incorrectly started typematic\n");
    failed |= core_machine_kbc_submit_native_bytes(&kbc, make_b,
        sizeof(make_b)) != LIB_STATUS_OK || !keyboard_has_repeat(&kbc);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x5au) != LIB_STATUS_OK ||
        !keyboard_has_repeat(&kbc);
    {
        lib_u8 repeated = 0u;
        failed |= !x86_keyboard_advance(kbc.connect.keyboard, 1u,
            keyboard_repeat_byte, &repeated) || repeated != 0x32u;
    }
    failed |= core_machine_kbc_submit_native_bytes(&kbc, break_b,
        sizeof(break_b)) != LIB_STATUS_OK || keyboard_has_repeat(&kbc);
    core_machine_kbc_finalize(&kbc);
    core_machine_destroy(port);
    return failed;
}

static lib_i32 core_machine_kbc_self_test_flushes_keyboard_output(void)
{
    t_kbc kbc;
    core_machine *port;
    lib_i32 failed = 0;

    port = test_kbc_create(&kbc);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        (core_machine_kbc_read_byte(port, 0x64u) & VKBC_STATUS_OBF) == 0u;
    test_kbc_port_write(port, 0x0064u, 0xaau);
    failed |= (core_machine_kbc_read_byte(port, 0x64u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x55u;
    core_machine_kbc_finalize(&kbc);
    core_machine_destroy(port);
    return failed;
}

static lib_i32 core_machine_kbc_bat_on_line_enable(void)
{
    t_kbc kbc;
    core_machine *port;
    lib_i32 failed = 0;

    port = test_kbc_create(&kbc);
    /* C0h bit 7 is the board keyboard-inhibit switch, not the 8042's
       serial data line.  The 5170's B0h board straps must therefore not
       suppress the command-byte's 45h -> 4Dh release edge. */
    core_machine_kbc_set_input_port(&kbc, 0xb0u);
    /* The 5170 can release only the serial-line override (45h -> 4Dh); its
       keyboard-disable bit need not change.  The clock/data enable edge, not
       a BIOS special case, releases BAT. */
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x45u);
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x4du);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) == 0u;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xaau;
    core_machine_kbc_finalize(&kbc);
    core_machine_destroy(port);
    return failed;
}

static lib_i32 core_machine_kbc_controller_enable_is_not_a_second_bat(void)
{
    t_kbc kbc;
    core_machine *port;
    lib_i32 failed = 0;

    port = test_kbc_create(&kbc);
    core_machine_kbc_set_input_port(&kbc, 0u);
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x45u);
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x4du);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xaau;
    test_kbc_port_write(port, 0x0064u, 0xadu);
    test_kbc_port_write(port, 0x0064u, 0xaeu);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_finalize(&kbc);
    core_machine_destroy(port);
    return failed;
}

static lib_i32 core_machine_kbc_self_test_enable_releases_bat(void)
{
    t_kbc kbc;
    core_machine *port;
    lib_i32 failed = 0;

    port = test_kbc_create(&kbc);
    test_kbc_port_write(port, 0x0064u, 0xaau);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x55u;
    test_kbc_port_write(port, 0x0064u, 0xaeu);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xaau;
    core_machine_kbc_finalize(&kbc);
    core_machine_destroy(port);
    return failed;
}

/* A BIOS may reset the keyboard while its serial line is inhibited, consume
 * FFh's FAh/AAh pair, and only then issue AEh.  The latter is an interface
 * release, not a second device power-on. */
static lib_i32 core_machine_kbc_reset_then_enable_has_one_bat(void)
{
    t_kbc kbc;
    core_machine *port;
    lib_i32 failed = 0;

    port = test_kbc_create(&kbc);
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x7du);
    test_kbc_port_write(port, 0x0060u, 0xffu);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xaau;
    test_kbc_port_write(port, 0x0064u, 0xaeu);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_finalize(&kbc);
    core_machine_destroy(port);
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
    core_machine *port;
    lib_i32 failed = 0;

    port = test_kbc_create(&kbc);
    x86_kbc8042_set_aux_present(kbc.chip, LIB_FALSE);
    core_machine_kbc_reset(&kbc);
    core_machine_kbc_set_input_port(&kbc, 0xb0u);
    core_machine_kbc_set_command_response_status_polls(&kbc, 1u);

    test_kbc_port_write(port, 0x0064u, 0xaau);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x55u;
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x5du);
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x4du);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xaau;

    test_kbc_port_write(port, 0x0064u, 0xadu);
    (void)core_machine_kbc_read_byte(port, 0x0060u);
    test_kbc_port_write(port, 0x0064u, 0xe0u);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0u;

    test_kbc_port_write(port, 0x0060u, 0xffu);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xaau;
    test_kbc_port_write(port, 0x0064u, 0xabu);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0u;

    core_machine_kbc_finalize(&kbc);
    core_machine_destroy(port);
    return failed;
}

/* Automatic repeats must obey the same single-byte output and inhibit
 * boundary as externally supplied scan bytes. */
static lib_i32 core_machine_kbc_typematic_output_boundary(void)
{
    t_kbc kbc;
    core_machine *port;
    lib_i32 failed;
    lib_u8 lines, ack, interface_result;

    port = test_kbc_create(&kbc);
    core_machine_kbc_set_typematic_timing(&kbc, 1u, 1u);
    failed = core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != LIB_STATUS_OK;
    core_machine_kbc_advance(&kbc, 4u);
    failed |= (core_machine_kbc_read_byte(port, 0x64u) & VKBC_STATUS_OBF) == 0u;
    test_kbc_port_write(port, 0x0064u, 0xadu);
    failed |= core_machine_kbc_read_byte(port, 0x60u) != 0x1eu;
    failed |= (core_machine_kbc_read_byte(port, 0x64u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 5u);
    failed |= (core_machine_kbc_read_byte(port, 0x64u) & VKBC_STATUS_OBF) != 0u;
    /* Diagnostic replay of the ROM's flush / E0h / FFh / ABh ordering.
     * This isolates controller state; it is not a full-ROM reproduction. */
    core_machine_kbc_set_command_response_status_polls(&kbc, 1u);
    (void)core_machine_kbc_read_byte(port, 0x0060u);
    test_kbc_port_write(port, 0x0064u, 0xe0u);
    (void)core_machine_kbc_read_byte(port, 0x0064u);
    lines = core_machine_kbc_read_byte(port, 0x0060u);
    test_kbc_port_write(port, 0x0060u, 0xffu);
    if ((core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u)
        (void)core_machine_kbc_read_byte(port, 0x0060u);
    (void)core_machine_kbc_read_byte(port, 0x0064u);
    ack = core_machine_kbc_read_byte(port, 0x0060u);
    if (ack == 0xfau)
        failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xaau;
    test_kbc_port_write(port, 0x0064u, 0xabu);
    (void)core_machine_kbc_read_byte(port, 0x0064u);
    interface_result = core_machine_kbc_read_byte(port, 0x0060u);
    failed |= lines != 0u || ack != 0xfau || interface_result != 0u;
    if (failed) lib_c_fprintf(lib_c_stderr, "KBC output/inhibit replay: E0=%02X FF=%02X AB=%02X\n",
        (unsigned int)lines, (unsigned int)ack, (unsigned int)interface_result);
    core_machine_kbc_finalize(&kbc);
    core_machine_destroy(port);
    return failed;
}

lib_i32 main(void)
{
    t_kbc kbc;
    test_kbc_irq_wiring irq = {0};
    core_machine_pic_bus *pic_master = LIB_NULL;
    core_machine_pic_bus *pic_slave = LIB_NULL;
    lib_bool a20_enabled = LIB_FALSE;
    lib_u32 reset_pulses = 0u;
    core_machine *machine = test_kbc_create_executor();
    core_machine *port = machine;
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

    core_machine_pic_initialize(&pic_master, &pic_slave, machine, CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_kbc_initialize(&kbc, machine);
    failed |= test_kbc_bind_irq(&irq, pic_master, pic_slave) != LIB_STATUS_OK;
    core_machine_kbc_bind_core_services(&kbc, test_kbc_irq_output, &irq,
        test_signal_a20, &a20_enabled, count_reset_pulse, &reset_pulses, LIB_TRUE);
    test_kbc_ready(port);
    test_kbc_initialize_pic(port);

    mixed_failed = core_machine_kbc_mixed_fifo_lifecycle();
    failed |= kbc_construction_rollback();
    translation_failed = core_machine_kbc_set2_translation();
    self_test_flush_failed = core_machine_kbc_self_test_flushes_keyboard_output();
    typematic_break_failed = core_machine_kbc_set2_break_cancels_typematic();
    line_bat_failed = core_machine_kbc_bat_on_line_enable();
    controller_enable_bat_failed = core_machine_kbc_controller_enable_is_not_a_second_bat();
    self_test_enable_bat_failed = core_machine_kbc_self_test_enable_releases_bat();
    reset_enable_bat_failed = core_machine_kbc_reset_then_enable_has_one_bat();
    ibm_5170_post_contract_failed = core_machine_kbc_ibm_5170_post_contract();
    cpu_reset_irq1_failed = test_kbc_board_cpu_reset_irq1();
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

    failed |= core_machine_kbc_read_byte(port, 0x0064u) != 0x10u;
    core_machine_kbc_set_input_port(&kbc, 0x80u);
    core_machine_kbc_set_test_inputs(&kbc, 0x03u);
    test_kbc_port_write(port, 0x0064u, 0xc0u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x80u;
    test_kbc_port_write(port, 0x0064u, 0xe0u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x03u;
    core_machine_kbc_set_input_port(&kbc, 0u);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_INHIBIT) != 0u ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_INVALID_STATE;
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x09u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xaau ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    core_machine_kbc_set_input_port(&kbc, 0x80u);
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x47u);
    test_kbc_port_write(port, 0x0064u, 0x20u);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) == 0u;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x47u;
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x01u);
    test_kbc_port_write(port, 0x0064u, 0x20u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x01u;
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x01u);
    test_kbc_port_write(port, 0x0064u, 0x20u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x01u;

    test_kbc_port_write(port, 0x0064u, 0xadu);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) !=
        LIB_STATUS_INVALID_STATE;
    test_kbc_port_write(port, 0x0064u, 0xaeu);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= (test_pic_read(pic_master, 0x0au) & VPIC_IRR_IRQ(1u)) == 0u;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    failed |= core_machine_pic_get_interrupt(pic_master, pic_slave) != 0x09u;
    test_kbc_port_write(port, 0x0020u, 0x20u);

    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x30u) != LIB_STATUS_OK;
    failed |= core_machine_pic_get_interrupt(pic_master, pic_slave) != 0x09u;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    test_kbc_port_write(port, 0x0020u, 0x20u);
    failed |= core_machine_pic_get_interrupt(pic_master, pic_slave) != 0x09u;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x30u;
    test_kbc_port_write(port, 0x0020u, 0x20u);

    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x41u);
    test_kbc_port_write(port, 0x0060u, 0xf2u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xabu ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x41u;
    failed |= core_machine_pic_get_interrupt(pic_master, pic_slave) != 0x09u;
    test_kbc_port_write(port, 0x0020u, 0x20u);
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x01u);

    test_kbc_port_write(port, 0x0060u, 0xf2u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xabu;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x83u;
    failed |= core_machine_pic_get_interrupt(pic_master, pic_slave) != 0x09u;
    test_kbc_port_write(port, 0x0020u, 0x20u);
    test_kbc_port_write(port, 0x0064u, 0xaau);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_SYS) == 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x55u ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) !=
            LIB_STATUS_INVALID_STATE ||
        (test_pic_read(pic_master, 0x0au) & VPIC_IRR_IRQ(1u)) != 0u;
    core_machine_kbc_set_command_response_status_polls(&kbc, 1u);
    test_kbc_port_write(port, 0x0064u, 0xaau);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x55u;
    core_machine_kbc_set_command_response_status_polls(&kbc, 0u);
    test_kbc_port_write(port, 0x0064u, 0xaeu);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        (test_pic_read(pic_master, 0x0au) & VPIC_IRR_IRQ(1u)) == 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    core_machine_kbc_set_command_response_status_polls(&kbc, 1u);
    test_kbc_port_write(port, 0x0060u, 0xffu);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xaau;
    core_machine_kbc_set_command_response_status_polls(&kbc, 0u);
    test_kbc_port_write(port, 0x0020u, 0x20u);
    test_kbc_port_write(port, 0x0064u, 0xabu);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0u;

    test_kbc_port_write(port, 0x0060u, 0xf0u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau;
    test_kbc_port_write(port, 0x0060u, 0x00u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) !=
        2u;
    test_kbc_port_write(port, 0x0060u, 0xf0u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau;
    test_kbc_port_write(port, 0x0060u, 0x01u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        x86_keyboard_get_signals(kbc.connect.keyboard).scan_set != 1u;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    test_kbc_port_write(port, 0x0060u, 0xf0u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau;
    test_kbc_port_write(port, 0x0060u, 0x02u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        x86_keyboard_get_signals(kbc.connect.keyboard).scan_set != 2u;

    test_kbc_port_write(port, 0x0060u, 0xedu);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau;
    test_kbc_port_write(port, 0x0060u, 0x07u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        x86_keyboard_get_signals(kbc.connect.keyboard).leds != 0x07u;
    core_machine_kbc_set_typematic_timing(&kbc, 240u, 48u);
    test_kbc_port_write(port, 0x0060u, 0xf3u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau;
    test_kbc_port_write(port, 0x0060u, 0x1fu);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        !keyboard_cadence_is(&kbc, 120u, 240u);
    test_kbc_port_write(port, 0x0060u, 0xf3u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau;
    test_kbc_port_write(port, 0x0060u, 0x2cu);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        !keyboard_cadence_is(&kbc, 240u, 48u);

    core_machine_kbc_set_typematic_timing(&kbc, 0u, 0u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 1000000u);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u;

    core_machine_kbc_set_typematic_timing(&kbc, 3u, 2u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != LIB_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xf0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 8u);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u;

    core_machine_kbc_set_command_response_timing(&kbc, 2u);
    test_kbc_port_write(port, 0x0060u, 0xeeu);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xeeu;
    core_machine_kbc_set_command_response_timing(&kbc, 0u);

    test_kbc_port_write(port, 0x0060u, 0xf5u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        x86_keyboard_get_signals(kbc.connect.keyboard).leds != 0u ||
        x86_keyboard_get_signals(kbc.connect.keyboard).scan_set != 2u ||
        x86_keyboard_get_signals(kbc.connect.keyboard).scanning || keyboard_has_repeat(&kbc) ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) !=
            LIB_STATUS_INVALID_STATE;
    test_kbc_port_write(port, 0x0060u, 0xf4u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    test_kbc_port_write(port, 0x0060u, 0xf6u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        x86_keyboard_get_signals(kbc.connect.keyboard).leds != 0u ||
        !x86_keyboard_get_signals(kbc.connect.keyboard).scanning;
    test_kbc_port_write(port, 0x0060u, 0xfdu);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        !x86_keyboard_get_signals(kbc.connect.keyboard).scanning || x86_keyboard_get_signals(kbc.connect.keyboard).leds != 0u ||
        !keyboard_cadence_is(&kbc, 3u, 2u);
    test_kbc_port_write(port, 0x0060u, 0xfeu);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau;
    test_kbc_port_write(port, 0x0060u, 0xffu);
    failed |= !x86_keyboard_get_signals(kbc.connect.keyboard).bat_ready || (core_machine_kbc_read_byte(port, 0x64u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        x86_keyboard_get_signals(kbc.connect.keyboard).bat_ready || (core_machine_kbc_read_byte(port, 0x64u) & VKBC_STATUS_OBF) == 0u ||
        !core_machine_pic_irq_source_is_asserted(irq.keyboard) ||
        (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) == 0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xaau ||
        x86_keyboard_get_signals(kbc.connect.keyboard).scan_set != 2u;
    core_machine_kbc_set_command_response_timing(&kbc, 2u);
    test_kbc_port_write(port, 0x0060u, 0xffu);
    failed |= (core_machine_kbc_read_byte(port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0xaau;
    core_machine_kbc_set_command_response_timing(&kbc, 0u);
    test_kbc_port_write(port, 0x0060u, 0x00u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfeu;
    test_kbc_port_write(port, 0x0060u, 0xfeu);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xaau;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    test_kbc_port_write(port, 0x0064u, 0xd0u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x01u;
    test_kbc_port_write(port, 0x0060u, 0xfeu);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;

    test_kbc_port_write(port, 0x0064u, 0xd1u);
    test_kbc_port_write(port, 0x0060u, 0x03u);
    failed |= !a20_enabled;
    test_kbc_port_write(port, 0x0064u, 0xd0u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x03u;
    test_kbc_port_write(port, 0x0064u, 0xffu);
    failed |= reset_pulses != 0u ||
        !a20_enabled;
    test_kbc_port_write(port, 0x0064u, 0xfeu);
    failed |= reset_pulses != 1u ||
        !a20_enabled;
    test_kbc_port_write(port, 0x0064u, 0xd0u);
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x03u;
    test_kbc_port_write(port, 0x0064u, 0xd1u);
    test_kbc_port_write(port, 0x0060u, 0x00u);
    failed |= a20_enabled ||
        reset_pulses != 2u;

    core_machine_kbc_reset(&kbc);
    test_kbc_port_write(port, 0x0064u, 0x60u);
    test_kbc_port_write(port, 0x0060u, 0x07u);
    {
        static const lib_u8 enter_break[] = { 0xf0u, 0x1eu };

        core_machine_kbc_set_typematic_timing(&kbc, 3u, 2u);
        failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != LIB_STATUS_OK ||
            core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu ||
            !keyboard_has_repeat(&kbc);
        for (index = 0u; index < 3u; ++index) {
            failed |= core_machine_kbc_submit_native_byte(&kbc, 0xe0u) != LIB_STATUS_OK;
        }
        /* A full physical OBF must not discard a complete Set-2 break;
         * accepting it cancels typematic before the bytes become CPU-visible. */
        failed |= core_machine_kbc_submit_native_bytes(&kbc, enter_break,
            sizeof(enter_break)) != LIB_STATUS_OK || keyboard_has_repeat(&kbc);
    }
    /* A command reply behind rapid typeahead remains KBC-owned until the
     * guest drains the one physical output buffer; it is never lost. */
    test_kbc_port_write(port, 0x0060u, 0xf2u);
    for (index = 0u; index < 3u; ++index) {
        failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xe0u;
        core_machine_kbc_advance(&kbc, 0u);
    }
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xf0u ||
        core_machine_kbc_read_byte(port, 0x0060u) != 0x1eu;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xfau;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0xabu;
    failed |= core_machine_kbc_read_byte(port, 0x0060u) != 0x83u;
    failed |= (core_machine_kbc_read_byte(port, 0x64u) & VKBC_STATUS_OBF) != 0u;

    core_machine_kbc_finalize(&kbc);
    core_machine_pic_finalize(pic_master, pic_slave);
    core_machine_destroy(port);
    if (failed) {
        lib_c_fprintf(lib_c_stderr,
            "M5:T464:S2:KBC:FAIL:mixed=%d:translation=%d:self-flush=%d:typematic=%d:line-bat=%d:enable-bat=%d:self-enable-bat=%d:reset-enable-bat=%d:ibm-post=%d:cpu-irq1=%d\n",
            mixed_failed, translation_failed, self_test_flush_failed,
            typematic_break_failed, line_bat_failed, controller_enable_bat_failed,
            self_test_enable_bat_failed, reset_enable_bat_failed,
            ibm_5170_post_contract_failed, cpu_reset_irq1_failed);
        return 1;
    }
    lib_c_printf("M5:T464:S2:KBC:OK\n");
    return 0;
}

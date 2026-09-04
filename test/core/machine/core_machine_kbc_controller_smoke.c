#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"

#include "core/machine/cpu_instructions.h"
#include "core/machine/kbc.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"
#include "../support/core_machine_cpu_fixture.h"

static type_unsigned_8 core_machine_kbc_read_byte(t_port *port, type_unsigned_16 port_id)
{
    return (type_unsigned_8)core_machine_port_read(port, port_id);
}

static C_VOID core_machine_kbc_initialize_pic(t_port *port)
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

static C_INT core_machine_kbc_mixed_fifo_lifecycle(C_VOID)
{
    t_kbc kbc;
    t_pic pic_master;
    t_pic pic_slave;
    t_ram memory = {0};
    core_machine_cpu_execution_context execution = {0};
    t_port port;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_pic_initialize(&pic_master, &pic_slave, &port, CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_kbc_bind_core_services(&kbc, &pic_master, &pic_slave,
        &memory, &execution, TYPE_TRUE);
    core_machine_kbc_initialize_pic(&port);

    core_machine_port_write(&port, 0x0064u, 0xd4u);
    core_machine_port_write(&port, 0x0060u, 0xf4u);
    core_machine_pic_refresh(&pic_master, &pic_slave);
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x74u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau;
    core_machine_port_write(&port, 0x00a0u, 0x20u);
    core_machine_port_write(&port, 0x0020u, 0x20u);

    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_aux_report(&kbc, 1, 1, 0x01u) != TYPE_STATUS_OK;
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
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK;
    core_machine_kbc_advance(&kbc, 2u);
    core_machine_pic_refresh(&pic_master, &pic_slave);
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x03u;
    core_machine_port_write(&port, 0x0020u, 0x20u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x43u;
    core_machine_kbc_set_command_response_timing(&kbc, 0u);

    failed |= core_machine_kbc_submit_aux_report(&kbc, 2, 2, 0u) != TYPE_STATUS_OK;
    core_machine_kbc_reset(&kbc);
    failed |= kbc.data.fifo_count != 0u || kbc.data.irq1_asserted ||
        kbc.data.irq12_asserted || !kbc.data.keyboard_enabled ||
        !kbc.data.aux_enabled || !kbc.data.scanning_enabled;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK ||
        !kbc.data.irq1_asserted;
    core_machine_kbc_finalize(&kbc);
    failed |= kbc.data.irq1_asserted || kbc.data.irq12_asserted;

    core_machine_pic_finalize(&pic_master, &pic_slave);
    core_machine_port_finalize(&port);
    return failed;
}

static C_INT core_machine_kbc_set2_translation(C_VOID)
{
    static const type_unsigned_8 function_set2[] = { 0x05u, 0x06u, 0x04u,
        0x0cu, 0x03u, 0x0bu, 0x83u, 0x0au, 0x01u, 0x09u, 0x78u, 0x07u };
    static const type_unsigned_8 function_set1[] = { 0x3bu, 0x3cu, 0x3du,
        0x3eu, 0x3fu, 0x40u, 0x41u, 0x42u, 0x43u, 0x44u, 0x57u, 0x58u };
    static const type_unsigned_8 pause_set2[] = { 0xe1u, 0x14u, 0x77u,
        0xe1u, 0xf0u, 0x14u, 0xf0u, 0x77u };
    static const type_unsigned_8 pause_set1[] = { 0xe1u, 0x1du, 0x45u,
        0xe1u, 0x9du, 0xc5u };
    t_kbc kbc;
    t_port port;
    type_unsigned_8 index;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    kbc.connect.aux_present = TYPE_FALSE;
    core_machine_kbc_reset(&kbc);
    failed |= (kbc.data.command_byte & CORE_MACHINE_KBC_COMMAND_DISABLE_AUX) == 0u ||
        core_machine_kbc_submit_native_byte(&kbc, 0x05u) != TYPE_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x3bu;
    for (index = 0u; index < sizeof(function_set2); ++index) {
        failed |= core_machine_kbc_submit_native_byte(&kbc, function_set2[index]) !=
                TYPE_STATUS_OK ||
            core_machine_kbc_read_byte(&port, 0x0060u) != function_set1[index];
    }
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x01u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != TYPE_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1cu ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xf0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1cu;
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x41u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1cu) != TYPE_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x9eu;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0xe0u) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x75u) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xe0u) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x75u) != TYPE_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xe0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x48u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xe0u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xc8u;
    for (index = 0u; index < sizeof(pause_set2); ++index) {
        failed |= core_machine_kbc_submit_native_byte(&kbc, pause_set2[index]) !=
            TYPE_STATUS_OK;
    }
    for (index = 0u; index < sizeof(pause_set1); ++index) {
        failed |= core_machine_kbc_read_byte(&port, 0x0060u) != pause_set1[index];
    }
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}


static C_INT core_machine_kbc_set2_break_cancels_typematic(C_VOID)
{
    static const type_unsigned_8 make_b[] = { 0x32u };
    static const type_unsigned_8 break_b[] = { 0xf0u, 0x32u };
    t_kbc kbc;
    t_port port;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x41u);
    core_machine_kbc_set_typematic_timing(&kbc, 1u, 1u);
    core_machine_kbc_set_serial_delivery_timing(&kbc, 2u);
    failed |= core_machine_kbc_submit_native_bytes(&kbc, make_b,
        sizeof(make_b)) != TYPE_STATUS_OK || !kbc.data.typematic_active;
    failed |= core_machine_kbc_submit_native_bytes(&kbc, break_b,
        sizeof(break_b)) != TYPE_STATUS_OK || kbc.data.typematic_active;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x30u;
    core_machine_kbc_advance(&kbc, 2u);
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xb0u ||
        kbc.data.typematic_active;
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}

static C_INT core_machine_kbc_self_test_flushes_keyboard_output(C_VOID)
{
    t_kbc kbc;
    t_port port;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK ||
        kbc.data.fifo_count != 1u;
    core_machine_port_write(&port, 0x0064u, 0xaau);
    failed |= kbc.data.fifo_count != 1u ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0x55u;
    core_machine_kbc_finalize(&kbc);
    core_machine_port_finalize(&port);
    return failed;
}

static C_INT core_machine_kbc_bat_on_line_enable(C_VOID)
{
    t_kbc kbc;
    t_port port;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_kbc_set_input_port(&kbc, 0u);
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

static C_INT core_machine_kbc_controller_enable_is_not_a_second_bat(C_VOID)
{
    t_kbc kbc;
    t_port port;
    C_INT failed = 0;

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

static C_INT core_machine_kbc_self_test_enable_releases_bat(C_VOID)
{
    t_kbc kbc;
    t_port port;
    C_INT failed = 0;

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
static C_INT core_machine_kbc_reset_then_enable_has_one_bat(C_VOID)
{
    t_kbc kbc;
    t_port port;
    C_INT failed = 0;

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
static C_INT core_machine_kbc_ibm_5170_post_contract(C_VOID)
{
    t_kbc kbc;
    t_port port;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_kbc_initialize(&kbc, &port);
    kbc.connect.aux_present = TYPE_FALSE;
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
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK ||
        core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau;

    core_machine_port_write(&port, 0x0064u, 0xadu);
    (C_VOID)core_machine_kbc_read_byte(&port, 0x0060u);
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

typedef struct core_machine_kbc_cpu_fixture {
    core_machine *machine;
} core_machine_kbc_cpu_fixture;

static C_VOID core_machine_kbc_cpu_reset(C_VOID *opaque)
{
    core_machine_kbc_cpu_fixture *fixture = opaque;

    if (fixture != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(fixture->machine);
}

static const core_machine_execution_provider core_machine_kbc_cpu_provider = {
    core_machine_kbc_cpu_reset, STD_NULL
};

/* Keep the POST-relevant path owner-local: a real CPU issues FFh, reads its
 * synchronous FAh, then receives the queued AAh through IRQ1 after STI's
 * architectural interrupt shadow. */
static C_INT core_machine_kbc_cpu_reset_irq1(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0xb0u, 0xffu, 0xe6u, 0x60u, 0xe4u, 0x60u, 0xfbu, 0x90u
    };
    static const type_unsigned_8 handler = 0xf4u;
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .shared_pit_personality = CORE_MACHINE_PIT_PERSONALITY_8253
    };
    core_machine_kbc_cpu_fixture fixture = {0};
    core_machine_run_result result;
    type_unsigned_16 offset = 0x0100u;
    type_unsigned_16 segment = 0u;
    C_INT failed = !test_core_machine_fixture_create_bind_freeze_reset(&config,
        &core_machine_kbc_cpu_provider, &fixture, &fixture.machine);

    if (!failed) {
        core_machine_kbc_initialize_pic(&fixture.machine->executor_port);
        core_machine_port_write(&fixture.machine->executor_port, 0x0021u, 0xfdu);
        failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                fixture.machine, 0u) ||
            core_machine_memory_write(fixture.machine, 0u, code, sizeof(code)) !=
                TYPE_STATUS_OK ||
            core_machine_memory_write(fixture.machine, 0x0024u, &offset,
                sizeof(offset)) != TYPE_STATUS_OK ||
            core_machine_memory_write(fixture.machine, 0x0026u, &segment,
                sizeof(segment)) != TYPE_STATUS_OK ||
            core_machine_memory_write(fixture.machine, offset, &handler,
                sizeof(handler)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        failed |= core_machine_run(fixture.machine, (core_machine_run_budget){3u, 0u},
                &result) != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            fixture.machine->executor_cpu.data.eip != 6u ||
            fixture.machine->shared_kbc.data.keyboard_bat_pending ||
            fixture.machine->shared_kbc.data.fifo_count != 1u ||
            fixture.machine->shared_kbc.data.fifo[
                fixture.machine->shared_kbc.data.fifo_head] != 0xaau ||
            !fixture.machine->shared_kbc.data.irq1_asserted;
    }
    if (!failed) {
        failed |= core_machine_run(fixture.machine, (core_machine_run_budget){2u, 0u},
                &result) != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            fixture.machine->executor_cpu.data.eip != offset ||
            !TYPE_GET_BIT(fixture.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(1u));
    }
    core_machine_destroy(fixture.machine);
    return failed;
}

C_INT main(C_VOID)
{
    t_kbc kbc;
    t_pic pic_master;
    t_pic pic_slave;
    t_ram memory = {0};
    core_machine_cpu_execution_context execution = {0};
    t_port port;
    C_INT failed = 0;
    C_INT mixed_failed;
    C_INT translation_failed;
    C_INT self_test_flush_failed;
    C_INT typematic_break_failed;
    C_INT line_bat_failed;
    C_INT controller_enable_bat_failed;
    C_INT self_test_enable_bat_failed;
    C_INT reset_enable_bat_failed;
    C_INT ibm_5170_post_contract_failed;
    C_INT cpu_reset_irq1_failed;
    type_unsigned_8 index;

    core_machine_port_initialize(&port);
    core_machine_pic_initialize(&pic_master, &pic_slave, &port, CORE_MACHINE_PIC_TOPOLOGY_CASCADED);
    core_machine_kbc_initialize(&kbc, &port);
    core_machine_kbc_bind_core_services(&kbc, &pic_master, &pic_slave,
        &memory, &execution, TYPE_TRUE);
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

    failed |= core_machine_kbc_read_byte(&port, 0x0064u) != 0x10u;
    core_machine_kbc_set_input_port(&kbc, 0x80u);
    core_machine_kbc_set_test_inputs(&kbc, 0x03u);
    core_machine_port_write(&port, 0x0064u, 0xc0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x80u;
    core_machine_port_write(&port, 0x0064u, 0xe0u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x03u;
    core_machine_kbc_set_input_port(&kbc, 0u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_INHIBIT) != 0u ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_INVALID_STATE;
    core_machine_port_write(&port, 0x0064u, 0x60u);
    core_machine_port_write(&port, 0x0060u, 0x09u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xaau ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK;
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
        TYPE_STATUS_INVALID_STATE;
    core_machine_port_write(&port, 0x0064u, 0xaeu);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK;
    failed |= (pic_master.data.irr & VPIC_IRR_IRQ(1u)) == 0u;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    failed |= core_machine_pic_get_interrupt(&pic_master, &pic_slave) != 0x09u;
    core_machine_port_write(&port, 0x0020u, 0x20u);

    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x30u) != TYPE_STATUS_OK;
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
            TYPE_STATUS_INVALID_STATE ||
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
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK ||
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
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK ||
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
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 1000000u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;

    core_machine_kbc_set_typematic_timing(&kbc, 3u, 2u);
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK;
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= (core_machine_kbc_read_byte(&port, 0x0064u) & VKBC_STATUS_OBF) != 0u;
    core_machine_kbc_advance(&kbc, 1u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    core_machine_kbc_advance(&kbc, 2u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu;
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0xf0u) != TYPE_STATUS_OK ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK;
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
            TYPE_STATUS_INVALID_STATE;
    core_machine_port_write(&port, 0x0060u, 0xf4u);
    failed |= core_machine_kbc_read_byte(&port, 0x0060u) != 0xfau ||
        core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK ||
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
    failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK ||
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
        static const type_unsigned_8 enter_break[] = { 0xf0u, 0x1eu };

        core_machine_kbc_set_typematic_timing(&kbc, 3u, 2u);
        failed |= core_machine_kbc_submit_native_byte(&kbc, 0x1eu) != TYPE_STATUS_OK ||
            core_machine_kbc_read_byte(&port, 0x0060u) != 0x1eu ||
            !kbc.data.typematic_active;
        for (index = 0u; index < 3u; ++index) {
            failed |= core_machine_kbc_submit_native_byte(&kbc, 0xe0u) != TYPE_STATUS_OK;
        }
        /* A full physical OBF must not discard a complete Set-2 break;
         * accepting it cancels typematic before the bytes become CPU-visible. */
        failed |= core_machine_kbc_submit_native_bytes(&kbc, enter_break,
            sizeof(enter_break)) != TYPE_STATUS_OK || kbc.data.keyboard_serial_count !=
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
        STD_FPRINTF(STD_STDERR,
            "M5:T464:S2:KBC:FAIL:mixed=%d:translation=%d:self-flush=%d:typematic=%d:line-bat=%d:enable-bat=%d:self-enable-bat=%d:reset-enable-bat=%d:ibm-post=%d:cpu-irq1=%d\n",
            mixed_failed, translation_failed, self_test_flush_failed,
            typematic_break_failed, line_bat_failed, controller_enable_bat_failed,
            self_test_enable_bat_failed, reset_enable_bat_failed,
            ibm_5170_post_contract_failed, cpu_reset_irq1_failed);
        return 1;
    }
    STD_PRINTF("M5:T464:S2:KBC:OK\n");
    return 0;
}

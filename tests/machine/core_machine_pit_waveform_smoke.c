#include "type.h"

#include "core/machine/pic.h"
#include "core/machine/pit.h"
#include "core/machine/port.h"

typedef struct {
    type_bool level[16];
    type_unsigned_32 count;
    core_machine_pic_irq_source *irq0;
} core_machine_pit_waveform_probe;

static C_VOID core_machine_pit_waveform_output(C_VOID *owner,
    type_bool asserted)
{
    core_machine_pit_waveform_probe *probe =
        (core_machine_pit_waveform_probe *)owner;
    if (probe->count < 16u) probe->level[probe->count] = asserted;
    ++probe->count;
    if (probe->irq0 != STD_NULL) {
        core_machine_pic_timer_output(probe->irq0, asserted);
    }
}

static C_VOID core_machine_pit_waveform_write(t_port *port, type_unsigned_8 control,
    type_unsigned_16 count)
{
    core_machine_port_write(port, 0x0043u, control);
    core_machine_port_write(port, 0x0040u, count & 0xffu);
    core_machine_port_write(port, 0x0040u, count >> 8);
}

static C_INT core_machine_pit_waveform_expect(type_bool actual,
    type_bool expected)
{
    return actual == expected ? 0 : 1;
}

C_INT main(C_VOID)
{
    t_pit pit;
    t_port port;
    t_pic master;
    t_pic slave;
    core_machine_pic_irq_source irq0;
    core_machine_pit_waveform_probe probe;
    C_INT failed = 0;

    STD_MEMSET(&probe, 0, sizeof(probe));
    core_machine_port_initialize(&port);
    core_machine_pic_initialize(&master, &slave, &port);
    core_machine_pic_reset(&master, &slave);
    core_machine_pic_irq_source_bind(&irq0, &master, &slave, 0u);
    probe.irq0 = &irq0;
    core_machine_pit_initialize(&pit, &port);
    core_machine_pit_reset(&pit);
    core_machine_pit_set_output(&pit, 0u, core_machine_pit_waveform_output,
        &probe);

    /* Mode 0: a low GATE pauses the terminal-count transition. */
    core_machine_pit_waveform_write(&port, 0x30u, 3u);
    core_machine_pit_advance(&pit, 1u);
    core_machine_pit_set_gate(&pit, 0u, TYPE_FALSE);
    core_machine_pit_advance(&pit, 4u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_FALSE);
    core_machine_pit_set_gate(&pit, 0u, TYPE_TRUE);
    core_machine_pit_advance(&pit, 2u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);

    /* Mode 1 only starts on a rising GATE and supports retrigger. */
    core_machine_pit_set_gate(&pit, 0u, TYPE_FALSE);
    core_machine_pit_waveform_write(&port, 0x32u, 3u);
    core_machine_pit_advance(&pit, 3u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);
    core_machine_pit_set_gate(&pit, 0u, TYPE_TRUE);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_FALSE);
    core_machine_pit_advance(&pit, 1u);
    core_machine_pit_set_gate(&pit, 0u, TYPE_FALSE);
    core_machine_pit_set_gate(&pit, 0u, TYPE_TRUE);
    core_machine_pit_advance(&pit, 3u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);

    /* Mode 2 produces one low tick, then reloads high. */
    core_machine_pit_waveform_write(&port, 0x34u, 3u);
    core_machine_pit_advance(&pit, 3u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_FALSE);
    core_machine_pit_advance(&pit, 1u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);
    failed |= core_machine_pit_waveform_expect(irq0.asserted, TYPE_TRUE);
    core_machine_pit_advance(&pit, 2u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_FALSE);
    failed |= core_machine_pit_waveform_expect(irq0.asserted, TYPE_FALSE);

    /* Mode 3 has ceil(N/2) high ticks and floor(N/2) low ticks. */
    core_machine_pit_waveform_write(&port, 0x36u, 5u);
    core_machine_pit_advance(&pit, 2u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);
    core_machine_pit_advance(&pit, 1u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_FALSE);
    core_machine_pit_advance(&pit, 2u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);

    /* Modes 4 and 5 retain a single low strobe. */
    core_machine_pit_waveform_write(&port, 0x38u, 3u);
    core_machine_pit_advance(&pit, 1u);
    core_machine_pit_set_gate(&pit, 0u, TYPE_FALSE);
    core_machine_pit_advance(&pit, 3u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);
    core_machine_pit_set_gate(&pit, 0u, TYPE_TRUE);
    core_machine_pit_advance(&pit, 2u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_FALSE);
    core_machine_pit_advance(&pit, 1u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);

    /* The mode-3 count-one edge case stays high without counter underflow. */
    core_machine_pit_waveform_write(&port, 0x36u, 1u);
    core_machine_pit_advance(&pit, 3u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);
    core_machine_pit_set_gate(&pit, 0u, TYPE_FALSE);
    core_machine_pit_waveform_write(&port, 0x3au, 2u);
    core_machine_pit_set_gate(&pit, 0u, TYPE_TRUE);
    core_machine_pit_advance(&pit, 2u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_FALSE);
    core_machine_pit_advance(&pit, 1u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);

    /* Zero loads are 65536 binary ticks and 10000 packed-BCD ticks. */
    core_machine_pit_waveform_write(&port, 0x30u, 0u);
    core_machine_pit_advance(&pit, 65535u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_FALSE);
    core_machine_pit_advance(&pit, 1u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);
    core_machine_pit_waveform_write(&port, 0x31u, 0u);
    core_machine_pit_advance(&pit, 9999u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_FALSE);
    core_machine_pit_advance(&pit, 1u);
    failed |= core_machine_pit_waveform_expect(
        core_machine_pit_get_output(&pit, 0u), TYPE_TRUE);

    /* Mode 2 contributes an OUT low/high pair to the IRQ0 provider. */
    failed |= probe.count < 2u;
    core_machine_pit_finalize(&pit);
    core_machine_pic_finalize(&master, &slave);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T222:S1:PIT-WAVEFORM:OK\n");
    return 0;
}

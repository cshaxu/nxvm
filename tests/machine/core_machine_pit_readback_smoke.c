#include "type.h"

#include "core/machine/pit.h"
#include "core/machine/port.h"

static C_VOID core_machine_pit_write_counter(t_port *port, uint16_t control,
    uint16_t count)
{
    core_machine_port_write(port, 0x0043u, control);
    core_machine_port_write(port, 0x0040u + ((control >> 6) & 0x03u),
        count & 0xffu);
    core_machine_port_write(port, 0x0040u + ((control >> 6) & 0x03u),
        count >> 8);
}

static uint8_t core_machine_pit_read_byte(t_port *port, uint16_t port_id)
{
    return (uint8_t)core_machine_port_read(port, port_id);
}

C_INT main(C_VOID)
{
    t_pit pit;
    t_port port;
    uint8_t status_before;
    uint8_t status_after;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_pit_initialize(&pit, &port);
    core_machine_pit_reset(&pit);
    core_machine_pit_set_output(&pit, 0u, STD_NULL, STD_NULL);

    core_machine_port_write(&port, 0x0043u, 0x0036u);
    core_machine_port_write(&port, 0x0043u, 0x00ecu);
    status_before = core_machine_pit_read_byte(&port, 0x0040u);
    failed |= status_before != 0xf6u;
    core_machine_port_write(&port, 0x0040u, 0x0034u);
    core_machine_port_write(&port, 0x0040u, 0x0012u);
    core_machine_port_write(&port, 0x0043u, 0x00ecu);
    status_after = core_machine_pit_read_byte(&port, 0x0040u);
    failed |= status_after != 0xb6u;

    core_machine_pit_write_counter(&port, 0x0074u, 0x5678u);
    core_machine_port_write(&port, 0x0043u, 0x00d8u);
    failed |= core_machine_pit_read_byte(&port, 0x0040u) != 0x34u;
    failed |= core_machine_pit_read_byte(&port, 0x0040u) != 0x12u;
    failed |= core_machine_pit_read_byte(&port, 0x0041u) != 0x78u;
    failed |= core_machine_pit_read_byte(&port, 0x0041u) != 0x56u;

    core_machine_port_write(&port, 0x0043u, 0x00ccu);
    core_machine_pit_advance(&pit, 1u);
    failed |= core_machine_pit_read_byte(&port, 0x0040u) != 0xb6u;
    failed |= core_machine_pit_read_byte(&port, 0x0040u) != 0x34u;
    failed |= core_machine_pit_read_byte(&port, 0x0040u) != 0x12u;

    core_machine_port_write(&port, 0x0043u, 0x00dcu);
    core_machine_pit_advance(&pit, 1u);
    core_machine_port_write(&port, 0x0043u, 0x00dcu);
    failed |= core_machine_pit_read_byte(&port, 0x0040u) != 0x33u;
    failed |= core_machine_pit_read_byte(&port, 0x0040u) != 0x12u;
    failed |= core_machine_pit_read_byte(&port, 0x0040u) != 0x32u;

    core_machine_port_write(&port, 0x0043u, 0x0000u);
    core_machine_pit_advance(&pit, 1u);
    core_machine_port_write(&port, 0x0043u, 0x0000u);
    failed |= core_machine_pit_read_byte(&port, 0x0040u) != 0x32u;
    failed |= core_machine_pit_read_byte(&port, 0x0040u) != 0x12u;
    failed |= core_machine_pit_read_byte(&port, 0x0040u) != 0x31u;

    core_machine_pit_finalize(&pit);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T191:S2:PIT-READBACK:OK\n");
    return 0;
}

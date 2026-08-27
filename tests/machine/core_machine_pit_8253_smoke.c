#include "type.h"

#include "core/machine/pit.h"
#include "core/machine/port.h"

C_INT main(C_VOID)
{
    t_pit pit;
    t_port port;
    type_unsigned_8 control;
    type_unsigned_16 count;
    C_INT failed = 0;

    core_machine_port_initialize(&port);
    core_machine_pit_initialize_as(&pit, &port, CORE_MACHINE_PIT_PERSONALITY_8253);
    core_machine_pit_reset(&pit);
    core_machine_port_write(&port, 0x0043u, 0x0034u);
    core_machine_port_write(&port, 0x0040u, 3u);
    core_machine_port_write(&port, 0x0040u, 0u);
    core_machine_pit_advance(&pit, 1u);
    control = pit.data.cw[0u];
    count = pit.data.count[0u];
    core_machine_port_write(&port, 0x0043u, 0x00ecu);
    failed |= pit.personality != CORE_MACHINE_PIT_PERSONALITY_8253 ||
        pit.data.cw[0u] != control || pit.data.count[0u] != count ||
        pit.data.flagLatch[0u] || pit.data.flagStatusLatch[0u];
    core_machine_pit_finalize(&pit);
    core_machine_port_finalize(&port);
    if (failed) return 1;
    STD_PRINTF("M5:T490:S4:8253-PERSONALITY:OK\n");
    return 0;
}

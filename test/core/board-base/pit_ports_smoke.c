#include "core/x86/machine_interface.h"
#include "core/board-base/pit_bus_interface.h"

static lib_i32 check_ports(x86_pit_personality personality, lib_u16 base)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    core_machine *machine = LIB_NULL;
    x86_pit *pit = LIB_NULL;
    x86_pit *other = LIB_NULL;
    lib_i32 failed = 1;
    lib_u32 value;

    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_pit_create(personality, &pit) != LIB_STATUS_OK ||
        x86_pit_create(personality, &other) != LIB_STATUS_OK) goto done;
    if (core_machine_pit_install_ports(LIB_NULL, pit, base) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_pit_install_ports(machine, LIB_NULL, base) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_pit_install_ports(machine, pit, 0xfffdu) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_pit_install_ports(machine, pit, base) != LIB_STATUS_OK ||
        core_machine_pit_install_ports(machine, other, base) == LIB_STATUS_OK ||
        core_machine_remove_port_routes(machine, pit) != LIB_STATUS_OK ||
        core_machine_pit_install_ports(machine, other, base) != LIB_STATUS_OK ||
        core_machine_remove_port_routes(machine, other) != LIB_STATUS_OK ||
        core_machine_pit_install_ports(machine, pit, base) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK)
        goto done;
    for (lib_u8 counter = 0u; counter < 3u; ++counter) {
        const lib_u16 port = (lib_u16)(base + counter);
        const lib_u8 control = (lib_u8)((counter << 6u) | 0x30u);
        if (core_machine_bus_write(machine, (lib_u16)(base + 3u), control) !=
                LIB_STATUS_OK ||
            core_machine_bus_write(machine, port, 0x1234u + counter) !=
                LIB_STATUS_OK ||
            core_machine_bus_write(machine, port, 0x56u) != LIB_STATUS_OK)
            goto done;
        /* CR transfers to CE on the next chip input clock, not on a write. */
        x86_pit_advance(pit, 1u);
        if (
            core_machine_bus_write(machine, (lib_u16)(base + 3u),
                (lib_u32)(counter << 6u)) != LIB_STATUS_OK ||
            core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK ||
            value != (lib_u32)(0x34u + counter) ||
            core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK ||
            value != 0x56u) goto done;
    }
    /* Unbind does not reset/destroy the chip; the caller retains ownership. */
    if (core_machine_remove_port_routes(machine, pit) != LIB_STATUS_OK ||
        x86_pit_write_register(pit, 3u, 0x34u) != LIB_STATUS_OK) goto done;
    failed = 0;
done:
    /* Core discards its routes before standalone fixture chip storage. */
    core_machine_destroy(machine);
    x86_pit_destroy(other);
    x86_pit_destroy(pit);
    return failed;
}

lib_i32 main(void)
{
    const lib_u16 bases[] = {0x0040u, 0x0049u, 0xfffcu};
    lib_i32 failed = 0;
    for (lib_size index = 0u; index < sizeof(bases) / sizeof(bases[0]); ++index) {
        failed |= check_ports(X86_PIT_PERSONALITY_8253, bases[index]);
        failed |= check_ports(X86_PIT_PERSONALITY_8254, bases[index]);
    }
    return failed;
}

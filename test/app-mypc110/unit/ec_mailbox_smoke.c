#include "app-mypc110/pc110_ec_mailbox_interface.h"
#include "x86-core/machine/debug_interface.h"

static lib_bool advance_guest(x86_core_machine *machine)
{
    const lib_u8 loop[] = {0xebu, 0xfeu};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_CS] = 0u, [X86_CORE_CPU_DEBUG_EIP] = 0x1000u}
    };
    x86_core_machine_run_result result = {0};

    return x86_core_machine_memory_write(machine, 0x1000u, loop, sizeof(loop)) == LIB_STATUS_OK &&
        x86_core_machine_debug_patch_registers(machine, &entry) == LIB_STATUS_OK &&
        x86_core_machine_run(machine, (x86_core_machine_run_budget){100000u, 0u}, &result) ==
            LIB_STATUS_OK && result.executed == 100000u;
}

static lib_bool read_value(x86_core_machine *machine, lib_u16 port, lib_u8 expected)
{
    lib_u32 value = 0u;

    return x86_core_machine_bus_read(machine, port, &value) == LIB_STATUS_OK &&
        value == expected;
}

static lib_bool read_reply(x86_core_machine *machine, lib_u8 expected)
{
    return read_value(machine, 0x15ecu, 1u) &&
        read_value(machine, 0x15ecu, 1u) &&
        read_value(machine, 0x15e8u, expected) &&
        read_value(machine, 0x15ecu, 0u);
}

static lib_bool read_ram(x86_core_machine *machine, lib_u8 page,
    lib_u8 address, lib_u8 expected)
{
    return x86_core_machine_bus_write(machine, 0x15ecu, 0xd0u + page) == LIB_STATUS_OK &&
        x86_core_machine_bus_write(machine, 0x15e8u, address) == LIB_STATUS_OK &&
        read_reply(machine, expected);
}

int main(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_80486SX};
    x86_core_machine *machine = LIB_NULL;
    mypc110_ec_mailbox mailbox = {0};
    lib_bool failed = LIB_FALSE;

    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        mypc110_ec_mailbox_install_routes(machine, &mailbox) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        const lib_u8 query[][2] = {{0x05u, 0x21u}, {0x10u, 0xaau},
            {0x21u, 0xf0u}, {0xfcu, 0x30u}};
        const lib_u8 initial_ram[][2] = {
            {0x40u, 0x30u}, {0xd0u, 0u}, {0xd1u, 0x10u},
            {0xd2u, 0u}, {0xffu, 0xffu}};

        failed |= !read_value(machine, 0x15ecu, 0u);
        for (lib_size index = 0u; index < sizeof(query) / sizeof(query[0]); ++index) {
            failed |= x86_core_machine_bus_write(machine, 0x15ecu,
                query[index][0]) != LIB_STATUS_OK || !read_reply(machine, query[index][1]);
        }
        /* The controller remains powered across a CPU-only reset. Its 11h
         * no-op must not discard the 10h completion waiting in DBB1. */
        failed |= x86_core_machine_bus_write(machine, 0x15ecu, 0x10u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15ecu, 0x11u) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK || !read_reply(machine, 0xaau);
        for (lib_size index = 0u; index < sizeof(initial_ram) / sizeof(initial_ram[0]); ++index)
            failed |= !read_ram(machine, 0u, initial_ram[index][0], initial_ram[index][1]);
        failed |= !read_ram(machine, 1u, 0x12u, 0xffu) ||
            !read_ram(machine, 1u, 0x3fu, 0xeau) ||
            !read_ram(machine, 1u, 0x44u, 0x14u) ||
            !read_ram(machine, 2u, 0x3fu, 0u);

        /* Preserve write-only charging traffic; it does not manufacture a reply. */
        failed |= x86_core_machine_bus_write(machine, 0x15ecu, 0x31u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15e8u, 'Z') != LIB_STATUS_OK ||
            mailbox.command != 0x31u || mailbox.data != 'Z' ||
            !read_value(machine, 0x15ecu, 0u) ||
            x86_core_machine_bus_write(machine, 0x15ecu, 0x100u) !=
                LIB_STATUS_INVALID_ARGUMENT;

        failed |= x86_core_machine_bus_write(machine, 0x15ecu, 0xd4u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15e8u, 0x88u) != LIB_STATUS_OK ||
            !read_value(machine, 0x15ecu, 0u) ||
            x86_core_machine_bus_write(machine, 0x15e8u, 0xa5u) != LIB_STATUS_OK ||
            !read_ram(machine, 1u, 0x88u, 0xa5u) ||
            !read_ram(machine, 0u, 0x88u, 0u);
        /* A new command abandons an incomplete write, never its accepted RAM. */
        failed |= x86_core_machine_bus_write(machine, 0x15ecu, 0xd4u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15e8u, 0x88u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15ecu, 0x05u) != LIB_STATUS_OK ||
            !read_reply(machine, 0x21u) || !read_ram(machine, 1u, 0x88u, 0xa5u) ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            !read_ram(machine, 1u, 0x88u, 0xa5u);

        const lib_u8 settings[][3] = {
            {0x0au, 0x9bu, 0xffu}, {0x22u, 0xf0u, 0xd0u}, {0x23u, 0xffu, 0xd1u}};
        const lib_u8 expected[] = {0x9bu, 0x0fu, 0u};
        for (lib_size index = 0u; index < sizeof(settings) / sizeof(settings[0]); ++index) {
            failed |= x86_core_machine_bus_write(machine, 0x15ecu,
                    settings[index][0]) != LIB_STATUS_OK ||
                x86_core_machine_bus_write(machine, 0x15e8u,
                    settings[index][1]) != LIB_STATUS_OK ||
                !read_ram(machine, 0u, settings[index][2], expected[index]);
        }
        for (lib_u8 command = 0xd5u; command <= 0xd6u; ++command) {
            failed |= x86_core_machine_bus_write(machine, 0x15ecu, command) != LIB_STATUS_OK ||
                x86_core_machine_bus_write(machine, 0x15e8u, 0x3fu) != LIB_STATUS_OK ||
                x86_core_machine_bus_write(machine, 0x15e8u, command) != LIB_STATUS_OK ||
                !read_ram(machine, (lib_u8)(command - 0xd4u), 0x3fu, command);
        }
        /* A deferred control event uses the existing latch and survives a
         * CPU reset. An occupied latch must be delivered first. */
        failed |= x86_core_machine_bus_write(machine, 0x15ecu, 0x0au) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15e8u, 0x9bu) != LIB_STATUS_OK ||
            !read_value(machine, 0x15ecu, 0u) ||
            x86_core_machine_bus_write(machine, 0x15ecu, 0x10u) != LIB_STATUS_OK ||
            !advance_guest(machine) || !read_value(machine, 0x15ecu, 1u) ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            !read_value(machine, 0x15e8u, 0xaau) || !advance_guest(machine) ||
            !read_reply(machine, 0x9bu) ||
            !read_value(machine, 0x15ecu, 0u);
        /* A later control transaction cancels the pending notification. */
        failed |= x86_core_machine_bus_write(machine, 0x15ecu, 0x0au) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15e8u, 0x9bu) != LIB_STATUS_OK ||
            !read_value(machine, 0x15ecu, 0u) ||
            x86_core_machine_bus_write(machine, 0x15ecu, 0x0au) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15e8u, 0x40u) != LIB_STATUS_OK ||
            !advance_guest(machine) ||
            !read_value(machine, 0x15ecu, 0u);
        /* Replacing a transaction publishes only its latest control value. */
        failed |= x86_core_machine_bus_write(machine, 0x15ecu, 0x0au) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15e8u, 0x9bu) != LIB_STATUS_OK ||
            !read_value(machine, 0x15ecu, 0u) ||
            x86_core_machine_bus_write(machine, 0x15ecu, 0x0au) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15e8u, 0x1bu) != LIB_STATUS_OK ||
            !read_value(machine, 0x15ecu, 0u) || !advance_guest(machine) ||
            !read_reply(machine, 0x1bu) || !read_value(machine, 0x15ecu, 0u);
        /* Unknown commands and unimplemented SFR reads never claim OBF. */
        failed |= x86_core_machine_bus_write(machine, 0x15ecu, 0xfeu) != LIB_STATUS_OK ||
            !read_value(machine, 0x15ecu, 0u) ||
            x86_core_machine_bus_write(machine, 0x15ecu, 0xd0u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15e8u, 0x3fu) != LIB_STATUS_OK ||
            !read_value(machine, 0x15ecu, 0u) ||
            x86_core_machine_bus_write(machine, 0x15ecu, 0xd2u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x15e8u, 0x40u) != LIB_STATUS_OK ||
            !read_value(machine, 0x15ecu, 0u);
    }
    x86_core_machine_destroy(machine);
    return failed ? 1 : 0;
}

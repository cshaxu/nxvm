#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "../executor_fixture.h"

typedef struct port_fixture {
    lib_u16 last_port;
    lib_u32 value;
    x86_core_machine *machine;
    lib_status running_mask_status;
    lib_u32 running_clear_mask;
} port_fixture;

static lib_status port_read(
    void *owner,
    lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)tick;
    port_fixture *fixture = (port_fixture *)owner;

    fixture->last_port = port;
    *out_value = fixture->value;
    return LIB_STATUS_OK;
}

static lib_status port_write(
    void *owner,
    lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    port_fixture *fixture = (port_fixture *)owner;

    (void)access_bytes;
    fixture->last_port = port;
    fixture->value = value;
    if (fixture->machine != LIB_NULL) {
        x86_core_machine_lifecycle lifecycle;

        if (x86_core_machine_get_lifecycle(fixture->machine, &lifecycle) !=
                LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
        if (lifecycle == X86_CORE_MACHINE_RUNNING) {
            fixture->running_mask_status = x86_core_machine_set_address_clear_mask(
                fixture->machine, 0u);
            return x86_core_machine_observe_address_clear_mask(fixture->machine,
                &fixture->running_clear_mask);
        }
    }
    return LIB_STATUS_OK;
}

static lib_i32 expect_status(lib_status actual, lib_status expected)
{
    return actual == expected ? 0 : 1;
}

lib_i32 main(void)
{
    x86_core_machine *first = LIB_NULL;
    x86_core_machine *second = LIB_NULL;
    x86_core_cpu_state cpu;
    x86_core_machine_port_provider ops = { port_read, port_write };
    port_fixture first_port = {0};
    port_fixture second_port = {0};
    const lib_u8 program[] = {
        0xb0u, 0xd1u, 0xe6u, 0x64u, /* KBC output-port command */
        0xb0u, 0x03u, 0xe6u, 0x60u, /* Enable A20 through KBC */
        0xe4u, 0x92u, 0xa2u, 0x00u, 0x02u, /* Preserve port 92h */
        0xbau, 0x34u, 0x12u, 0xeeu, /* Observe while genuinely running */
        0xb0u, 0x00u, 0xe6u, 0x92u, 0xf4u /* Disable A20 and halt */
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_DS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x2000u}
    };
    x86_core_machine_run_result run;
    lib_u8 value;
    lib_u32 port_value;
    lib_u32 clear_mask;
    lib_i32 result = 0;

    result |= expect_status(test_board_create_executor(
                                (2u * 1024u * 1024u), &first),
                            LIB_STATUS_OK);
    result |= expect_status(test_board_create_executor(
                                (2u * 1024u * 1024u), &second),
                            LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_install_port_provider(
                                first, 0x1234u, 0x1234u, &ops, &first_port),
                            LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_install_port_provider(
                                second, 0x1234u, 0x1234u, &ops, &second_port),
                            LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_freeze_execution_providers(first),
                            LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_freeze_execution_providers(second),
                            LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_reset(first), LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_reset(second), LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_get_cpu_state(first, &cpu),
                            LIB_STATUS_OK);
    result |= cpu.cs != 0xf000u || cpu.cs_base != 0xffff0000u ||
              cpu.eip != 0x0000fff0u || cpu.eflags != 0x00000002u;

    value = 0x11u;
    result |= expect_status(x86_core_machine_memory_write(first, 0u, &value, 1u),
                            LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_set_address_clear_mask(first, 0u), LIB_STATUS_OK);
    value = 0x22u;
    result |= expect_status(x86_core_machine_memory_write(first, 0x100000u, &value, 1u),
                            LIB_STATUS_OK);
    value = 0u;
    result |= expect_status(x86_core_machine_memory_read(first, 0x100000u, &value, 1u),
                            LIB_STATUS_OK);
    result |= value != 0x22u;
    result |= expect_status(x86_core_machine_set_address_clear_mask(first, 0x00100000u), LIB_STATUS_OK);
    value = 0u;
    result |= expect_status(x86_core_machine_memory_read(first, 0x100000u, &value, 1u),
                            LIB_STATUS_OK);
    result |= value != 0x11u;
    result |= expect_status(x86_core_machine_bus_read(first, 0x0092u, &port_value),
                            LIB_STATUS_OK);
    result |= port_value != 0u;
    result |= expect_status(x86_core_machine_bus_write(first, 0x0092u, 0x02u),
                            LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_bus_read(first, 0x0092u, &port_value),
                            LIB_STATUS_OK);
    result |= port_value != 0x02u;
    value = 0u;
    result |= expect_status(x86_core_machine_memory_read(first, 0x100000u, &value, 1u),
                            LIB_STATUS_OK);
    result |= value != 0x22u;
    result |= expect_status(x86_core_machine_bus_read(second, 0x0092u, &port_value),
                            LIB_STATUS_OK);
    result |= port_value != 0u;
    result |= expect_status(x86_core_machine_bus_write(first, 0x0092u, 0u),
                            LIB_STATUS_OK);
    value = 0u;
    result |= expect_status(x86_core_machine_memory_read(first, 0x100000u, &value, 1u),
                            LIB_STATUS_OK);
    result |= value != 0x11u;

    first_port.machine = first;
    result |= x86_core_machine_memory_write(first, 0x2000u, program,
        sizeof(program)) != LIB_STATUS_OK ||
        x86_core_machine_debug_patch_registers(first, &entry) != LIB_STATUS_OK ||
        x86_core_machine_run(first, (x86_core_machine_run_budget){20u, 0u},
            &run) != LIB_STATUS_OK ||
        run.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        first_port.running_mask_status != LIB_STATUS_INVALID_STATE ||
        first_port.running_clear_mask != 0u ||
        x86_core_machine_memory_read(first, 0x200u, &value, 1u) != LIB_STATUS_OK ||
        value != 0x02u;
    result |= expect_status(x86_core_machine_observe_address_clear_mask(first, &clear_mask), LIB_STATUS_OK);
    result |= clear_mask != 0x00100000u;

    value = 0xffu;
    result |= expect_status(x86_core_machine_memory_read(second, 0u, &value, 1u),
                            LIB_STATUS_OK);
    result |= value != 0u;

    result |= expect_status(x86_core_machine_bus_write(first, 0x1234u, 0xa5a5u),
                            LIB_STATUS_OK);
    result |= expect_status(x86_core_machine_bus_read(first, 0x1234u, &port_value),
                            LIB_STATUS_OK);
    result |= port_value != 0xa5a5u || first_port.last_port != 0x1234u ||
              second_port.value != 0u;

    result |= expect_status(x86_core_machine_reset(first), LIB_STATUS_OK);
    value = 0xffu;
    result |= expect_status(x86_core_machine_memory_read(first, 0u, &value, 1u),
                            LIB_STATUS_OK);
    result |= value != 0u;

    x86_core_machine_destroy(second);
    x86_core_machine_destroy(first);
    if (result != 0) {
        return 1;
    }

    lib_c_printf("%s\n", "MACHINE-INSTANCE:OK");
    return 0;
}

#include "composition_fixture.h"
/* This fixture also belongs to root-registered Release tests. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "lib/types/test.h"

x86_core_machine *test_x86_nxvm_port_owner_create(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 65536u, .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    x86_core_machine *machine = LIB_NULL;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        return LIB_NULL;
    return machine;
}

void test_x86_nxvm_port_owner_destroy(x86_core_machine *machine)
{
    x86_core_machine_destroy(machine);
}

lib_bool test_x86_nxvm_port_accepts_read(x86_core_machine *machine, lib_u16 port)
{
    lib_u32 value = 0u;
    const lib_status status = x86_core_machine_bus_read(machine, port, &value);

    lib_test_assert(status == LIB_STATUS_OK || status == LIB_STATUS_UNSUPPORTED);
    return status == LIB_STATUS_OK;
}

lib_bool test_x86_nxvm_port_accepts_write(x86_core_machine *machine, lib_u16 port)
{
    const lib_status status = x86_core_machine_bus_write(machine, port, 0u);

    lib_test_assert(status == LIB_STATUS_OK || status == LIB_STATUS_UNSUPPORTED);
    return status == LIB_STATUS_OK;
}

/* Bounded run returns a paused boundary; keep the public bus admission. */
void test_x86_nxvm_write_port_after_run(x86_core_machine *machine, lib_u16 port, lib_u32 value)
{
    const lib_status status = x86_core_machine_bus_write(machine, port, value);
    lib_test_assert(status == LIB_STATUS_OK);
}

lib_bool test_x86_nxvm_a20_is_enabled(const x86_core_machine *machine)
{
    lib_u32 clear_mask;
    return x86_core_machine_observe_address_clear_mask(machine, &clear_mask) ==
        LIB_STATUS_OK && clear_mask == 0u;
}

lib_u64 test_x86_nxvm_elapsed_ticks(const x86_core_machine *machine)
{
    lib_u64 ticks = 0u;
    const lib_status status = x86_core_machine_get_elapsed_ticks(machine, &ticks);
    lib_test_assert(status == LIB_STATUS_OK);
    return ticks;
}

lib_bool test_x86_nxvm_cpu_is_halted(const x86_core_machine *machine)
{
    x86_core_cpu_state cpu;
    const lib_status status = x86_core_machine_get_cpu_state(machine, &cpu);
    lib_test_assert(status == LIB_STATUS_OK);
    return cpu.halted;
}

lib_u32 test_x86_nxvm_read_port_after_run(x86_core_machine *machine, lib_u16 port)
{
    lib_u32 value = 0u;
    const lib_status status = x86_core_machine_bus_read(machine, port, &value);
    lib_test_assert(status == LIB_STATUS_OK);
    return value;
}

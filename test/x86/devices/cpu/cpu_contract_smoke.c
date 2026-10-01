#include "lib/types/types_interface.h"
#include "x86/devices/cpu/cpu_interface.h"

typedef struct cpu_contract_bus {
    lib_u8 memory[32];
} cpu_contract_bus;

static lib_status cpu_contract_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    cpu_contract_bus *bus = (cpu_contract_bus *)opaque;

    (void)provenance;
    (void)observe_only;
    (void)reset_fetch;
    if (address + bytes > sizeof(bus->memory)) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(destination, &bus->memory[address], bytes);
    return LIB_STATUS_OK;
}

static lib_status cpu_contract_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance)
{
    cpu_contract_bus *bus = (cpu_contract_bus *)opaque;

    (void)provenance;
    if (address + bytes > sizeof(bus->memory)) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(&bus->memory[address], source, bytes);
    return LIB_STATUS_OK;
}

static const core_machine_cpu_bus_provider cpu_contract_bus_provider = {
    .read_memory = cpu_contract_read,
    .write_memory = cpu_contract_write
};

lib_i32 main(void)
{
    cpu_contract_bus bus = {0};
    core_machine_cpu_execution_context *cpu = LIB_NULL;
    core_machine_cpu_state state = {0};
    const core_machine_instruction_timing timing = { .base_ticks = 1u };

    if (core_machine_cpu_create(&cpu_contract_bus_provider, &bus,
            &cpu) != LIB_STATUS_OK) return 1;
    core_machine_cpu_execution_context_bind_profiles(cpu,
        CORE_MACHINE_CPU_PROFILE_8086, X86_FPU_PROFILE_NONE, LIB_FALSE,
        &timing);
    core_machine_cpu_state_initialize(cpu);
    core_machine_cpu_state_reset(cpu);
    core_machine_cpu_capture_state(cpu, &state);
    if (state.cs != 0xf000u || state.eip != 0xfff0u) {
        core_machine_cpu_destroy(cpu);
        return 1;
    }
    core_machine_cpu_destroy(cpu);
    return 0;
}

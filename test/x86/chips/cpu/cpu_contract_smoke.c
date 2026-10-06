#include "lib/types/types_interface.h"
#include "x86/chips/cpu/cpu_interface.h"

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

    const struct { core_machine_cpu_profile profile; const char *name; lib_bool early; } profiles[] = {
        {CORE_MACHINE_CPU_PROFILE_8086, "8086", LIB_TRUE},
        {CORE_MACHINE_CPU_PROFILE_8088, "8088", LIB_TRUE},
        {CORE_MACHINE_CPU_PROFILE_80186, "80186", LIB_FALSE},
        {CORE_MACHINE_CPU_PROFILE_80286, "80286", LIB_FALSE},
        {CORE_MACHINE_CPU_PROFILE_80386, "80386", LIB_FALSE}
    };
    for (lib_size i = 0u; i < sizeof(profiles) / sizeof(profiles[0]); ++i)
        if (lib_text_compare(core_machine_cpu_profile_name(profiles[i].profile), profiles[i].name) != 0 ||
            core_machine_cpu_profile_has_8086_semantics(profiles[i].profile) != profiles[i].early) return 1;
    const core_machine_instruction_timing bounded = {.base_ticks = 2u, .prefix_surcharge = 3u,
        .taken_branch_surcharge = 4u, .data_memory_surcharge = 5u,
        .io_surcharge = 6u, .rep_iteration_surcharge = 7u};
    if (core_machine_cpu_timing_maximum_ticks(CORE_MACHINE_CPU_PROFILE_8088, &bounded) != 69u ||
        core_machine_cpu_timing_maximum_ticks(CORE_MACHINE_CPU_PROFILE_8088, LIB_NULL) != 0u) return 1;

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
    if (core_machine_cpu_execution_consume_shutdown_request(cpu)) {
        core_machine_cpu_destroy(cpu);
        return 1;
    }
    core_machine_cpu_execution_request_shutdown(cpu);
    core_machine_cpu_execution_request_shutdown(cpu);
    if (!core_machine_cpu_execution_consume_shutdown_request(cpu) ||
        core_machine_cpu_execution_consume_shutdown_request(cpu)) {
        core_machine_cpu_destroy(cpu);
        return 1;
    }
    core_machine_cpu_destroy(cpu);
    return 0;
}

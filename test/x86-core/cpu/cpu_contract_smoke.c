#include "lib/types/types_interface.h"
#include "x86-core/cpu/cpu_interface.h"

typedef struct cpu_contract_bus {
    lib_u8 memory[32];
} cpu_contract_bus;

static lib_status cpu_contract_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    (void)cache_control;
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
    x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    (void)cache_control;
    cpu_contract_bus *bus = (cpu_contract_bus *)opaque;

    (void)provenance;
    if (address + bytes > sizeof(bus->memory)) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(&bus->memory[address], source, bytes);
    return LIB_STATUS_OK;
}

static const x86_core_cpu_bus_provider cpu_contract_bus_provider = {
    .read_memory = cpu_contract_read,
    .write_memory = cpu_contract_write
};

lib_i32 main(void)
{
    cpu_contract_bus bus = {0};
    x86_core_cpu_execution_context *cpu = LIB_NULL;
    x86_core_cpu_state state = {0};
    const x86_core_cpu_instruction_timing timing = { .base_ticks = 1u };

    const struct { x86_core_cpu_profile profile; const char *name; lib_bool early; } profiles[] = {
        {X86_CORE_CPU_PROFILE_8086, "8086", LIB_TRUE},
        {X86_CORE_CPU_PROFILE_8088, "8088", LIB_TRUE},
        {X86_CORE_CPU_PROFILE_80186, "80186", LIB_FALSE},
        {X86_CORE_CPU_PROFILE_80286, "80286", LIB_FALSE},
        {X86_CORE_CPU_PROFILE_80386, "80386", LIB_FALSE},
        {X86_CORE_CPU_PROFILE_80486SX, "80486SX", LIB_FALSE}
    };
    for (lib_size i = 0u; i < sizeof(profiles) / sizeof(profiles[0]); ++i)
        if (lib_text_compare(x86_core_cpu_profile_name(profiles[i].profile), profiles[i].name) != 0 ||
            x86_core_cpu_profile_has_8086_semantics(profiles[i].profile) != profiles[i].early) return 1;
    const x86_core_cpu_instruction_timing bounded = {.base_ticks = 2u, .prefix_surcharge = 3u,
        .taken_branch_surcharge = 4u, .data_memory_surcharge = 5u,
        .io_surcharge = 6u, .rep_iteration_surcharge = 7u};
    if (x86_core_cpu_timing_maximum_ticks(X86_CORE_CPU_PROFILE_8088, &bounded) != 69u ||
        x86_core_cpu_timing_maximum_ticks(X86_CORE_CPU_PROFILE_8088, LIB_NULL) != 0u) return 1;

    if (x86_core_cpu_create(&cpu_contract_bus_provider, &bus,
            &cpu) != LIB_STATUS_OK) return 1;
    x86_core_cpu_execution_context_bind_profiles(cpu,
        X86_CORE_CPU_PROFILE_8086, X86_FPU_PROFILE_NONE, LIB_FALSE,
        X86_CORE_CPU_PROTECTED_DATA_RELOAD_STRICT, &timing);
    x86_core_cpu_state_initialize(cpu);
    x86_core_cpu_state_reset(cpu);
    x86_core_cpu_capture_state(cpu, &state);
    if (state.cs != 0xffffu || state.eip != 0u) {
        x86_core_cpu_destroy(cpu);
        return 1;
    }
    if (x86_core_cpu_execution_consume_shutdown_request(cpu)) {
        x86_core_cpu_destroy(cpu);
        return 1;
    }
    x86_core_cpu_execution_request_shutdown(cpu);
    x86_core_cpu_execution_request_shutdown(cpu);
    if (!x86_core_cpu_execution_consume_shutdown_request(cpu) ||
        x86_core_cpu_execution_consume_shutdown_request(cpu)) {
        x86_core_cpu_destroy(cpu);
        return 1;
    }
    x86_core_cpu_destroy(cpu);
    return 0;
}

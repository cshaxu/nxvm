#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/debug_interface.h"

typedef struct neutral_probe {
    lib_u32 port_value;
    lib_u32 trace_events;
    lib_u64 read_tick;
} neutral_probe;

static lib_status neutral_port_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    neutral_probe *probe = owner;
    if (port != 0x1234u) return LIB_STATUS_INVALID_ARGUMENT;
    probe->read_tick = tick;
    *out_value = probe->port_value;
    return LIB_STATUS_OK;
}

static lib_status neutral_port_write(void *owner, lib_u16 port, lib_u32 value)
{
    neutral_probe *probe = owner;
    if (port != 0x1234u) return LIB_STATUS_INVALID_ARGUMENT;
    probe->port_value = value;
    return LIB_STATUS_OK;
}

static void neutral_trace(void *owner, const core_machine_trace_event *event)
{
    neutral_probe *probe = owner;
    if (event != LIB_NULL) ++probe->trace_events;
}

static lib_i32 neutral_memory_aliases(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    const core_machine_memory_alias_config aliases[2] = {
        {0x20000u, 0x100u, 16u}, {0x30000u, 0x100u, 0u}
    };
    core_machine *machine = LIB_NULL;
    const lib_u8 written = 0x5au;
    lib_u8 read = 0u;
    lib_i32 failed = 1;

    if (core_machine_neutral_create(&config, &machine) !=
            LIB_STATUS_OK) goto done;
    if (core_machine_install_memory_aliases(machine, LIB_NULL, 1u, LIB_TRUE) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_install_memory_aliases(machine, aliases, 1u, 2u) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_install_memory_aliases(machine, aliases, 2u, LIB_TRUE) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        machine->executor_memory.connect.mapping_count != 0u ||
        core_machine_install_memory_aliases(machine, aliases, 1u, LIB_TRUE) !=
            LIB_STATUS_OK ||
        core_machine_install_memory_aliases(machine, aliases, 2u, LIB_FALSE) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        machine->executor_memory.connect.mapping_count != 1u ||
        core_machine_install_memory_aliases(machine, LIB_NULL, 0u, LIB_FALSE) !=
            LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_install_memory_aliases(machine, aliases, 1u, LIB_TRUE) !=
            LIB_STATUS_INVALID_STATE ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0x20000u, &written, 1u) != LIB_STATUS_OK ||
        core_machine_memory_read(machine, 0x100u, &read, 1u) != LIB_STATUS_OK ||
        read != written ||
        core_machine_memory_read(machine, 0x20000u, &read, 1u) != LIB_STATUS_OK ||
        read != written) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 neutral_ready_levels(void)
{
    core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    const lib_i32 levels[] = {0, 1, -3};
    core_machine *machine = LIB_NULL;
    lib_i32 failed = 1;

    if (core_machine_set_cpu_bus_ready(LIB_NULL, 1) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_set_dma_bus_ready(LIB_NULL, 1) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_neutral_create(&config, &machine) !=
            LIB_STATUS_OK) goto done;
    if (core_machine_set_cpu_bus_ready(machine, 0) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_set_dma_bus_ready(machine, 0) != LIB_STATUS_INVALID_ARGUMENT ||
        !machine->cpu_cycle_bus_ready || !machine->dma_cycle_bus_ready) goto done;
    core_machine_destroy(machine);
    machine = LIB_NULL;
    config.transaction_contract.cpu_cycle_bus_ready_gate_enabled = LIB_TRUE;
    config.transaction_contract.dma_cycle_bus_ready_gate_enabled = LIB_TRUE;
    if (core_machine_neutral_create(&config, &machine) !=
            LIB_STATUS_OK) goto done;
    for (lib_size index = 0u; index < sizeof(levels) / sizeof(levels[0]); ++index) {
        const lib_bool expected = levels[index] != 0 ? LIB_TRUE : LIB_FALSE;
        if (core_machine_set_cpu_bus_ready(machine, levels[index]) != LIB_STATUS_OK ||
            core_machine_set_dma_bus_ready(machine, levels[index]) != LIB_STATUS_OK ||
            machine->cpu_cycle_bus_ready != expected ||
            machine->dma_cycle_bus_ready != expected) goto done;
    }
    machine->firmware_operation_active = LIB_TRUE;
    if (core_machine_set_cpu_bus_ready(machine, 0) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_set_dma_bus_ready(machine, 0) != LIB_STATUS_INVALID_ARGUMENT ||
        !machine->cpu_cycle_bus_ready || !machine->dma_cycle_bus_ready) goto done;
    machine->firmware_operation_active = LIB_FALSE;
    if (core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_set_cpu_bus_ready(machine, 0) != LIB_STATUS_OK ||
        core_machine_set_dma_bus_ready(machine, 0) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        !machine->cpu_cycle_bus_ready || !machine->dma_cycle_bus_ready) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 neutral_timing_declarations(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    core_machine_timing_declaration declarations[CORE_MACHINE_TIMING_CAPABILITY_COUNT];
    core_machine_timing_declaration observed;
    const core_machine_timing_declaration invalid_entries[] = {
        {CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG, CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK, CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM},
        {(core_machine_timing_capability)-1, CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK, CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM},
        {(core_machine_timing_capability)CORE_MACHINE_TIMING_CAPABILITY_COUNT, CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK, CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM},
        {CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC, (core_machine_timing_disposition)-1, CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM},
        {CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC, (core_machine_timing_disposition)(CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED + 1), CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM},
        {CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC, CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK, (core_machine_timing_seam)-1},
        {CORE_MACHINE_TIMING_CAPABILITY_CPU_EXEC, CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK, (core_machine_timing_seam)(CORE_MACHINE_TIMING_SEAM_OBSERVATION + 1)}
    };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = 1;
    lib_size index;

    /* Neutral representation is tested independently of a board's source
     * policy. Reverse order proves publication indexes capability, not input. */
    for (index = 0u; index < CORE_MACHINE_TIMING_CAPABILITY_COUNT; ++index)
        declarations[index] = (core_machine_timing_declaration){
            (core_machine_timing_capability)(CORE_MACHINE_TIMING_CAPABILITY_COUNT - 1u - index),
            CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK, CORE_MACHINE_TIMING_SEAM_CPU_PROGRAM};
    if (core_machine_validate_timing_declarations(LIB_NULL,
            CORE_MACHINE_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_validate_timing_declarations(declarations, 0u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_validate_timing_declarations(declarations,
            CORE_MACHINE_TIMING_CAPABILITY_COUNT - 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_validate_timing_declarations(declarations,
            CORE_MACHINE_TIMING_CAPABILITY_COUNT + 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_install_timing_declarations(LIB_NULL, declarations,
            CORE_MACHINE_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE ||
        core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        goto done;
    if (core_machine_install_timing_declarations(machine, LIB_NULL,
            CORE_MACHINE_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_ARGUMENT) goto done;
    for (index = 0u; index < sizeof(invalid_entries) / sizeof(invalid_entries[0]); ++index) {
        core_machine_timing_declaration saved = declarations[CORE_MACHINE_TIMING_CAPABILITY_COUNT - 1u];
        core_machine_timing_declaration *invalid = &declarations[CORE_MACHINE_TIMING_CAPABILITY_COUNT - 1u];

        *invalid = invalid_entries[index];
        if (core_machine_install_timing_declarations(machine, declarations,
                CORE_MACHINE_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_ARGUMENT ||
            machine->timing_declarations_copied) goto done;
        for (lib_size entry = 0u; entry < CORE_MACHINE_TIMING_CAPABILITY_COUNT; ++entry)
            if (machine->timing_declarations[entry].capability != 0) goto done;
        *invalid = saved;
    }
    machine->firmware_operation_active = LIB_TRUE;
    if (core_machine_install_timing_declarations(machine, declarations,
            CORE_MACHINE_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE) goto done;
    machine->firmware_operation_active = LIB_FALSE;
    if (core_machine_install_timing_declarations(machine, declarations,
            CORE_MACHINE_TIMING_CAPABILITY_COUNT) != LIB_STATUS_OK ||
        core_machine_install_timing_declarations(machine, declarations,
            CORE_MACHINE_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE) goto done;
    declarations[0].disposition = CORE_MACHINE_TIMING_DISPOSITION_L3_REQUIRED;
    if (core_machine_get_timing_declaration(machine,
            CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG, &observed) != LIB_STATUS_OK ||
        observed.disposition != CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_get_timing_declaration(machine,
            CORE_MACHINE_TIMING_CAPABILITY_PRODUCT_DEBUG, &observed) != LIB_STATUS_OK ||
        observed.disposition != CORE_MACHINE_TIMING_DISPOSITION_L2_FALLBACK ||
        core_machine_install_timing_declarations(machine, declarations,
            CORE_MACHINE_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE) goto done;
    core_machine_destroy(machine);
    machine = LIB_NULL;
    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_install_timing_declarations(machine, declarations,
            CORE_MACHINE_TIMING_CAPABILITY_COUNT) != LIB_STATUS_INVALID_STATE ||
        machine->timing_declarations_copied) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

static lib_status neutral_firmware_configure(void *owner,
    core_machine_firmware_context *firmware)
{
    static const lib_u8 code[16] = {0xb8u, 0x34u, 0x12u, 0x90u, 0xf4u};
    (void)owner;
    return core_machine_firmware_register_immutable_rom(firmware,
        0xffff0u, code, sizeof(code));
}

static lib_status neutral_firmware_reset(void *owner,
    core_machine_firmware_context *firmware)
{
    (void)owner;
    (void)firmware;
    return LIB_STATUS_OK;
}

static lib_i32 neutral_rom_windows(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    const lib_u8 image[4] = {0x11u, 0x22u, 0x33u, 0x44u};
    core_machine *machine = LIB_NULL;
    lib_u8 observed[2] = {0};
    lib_i32 failed = 1;

    if (core_machine_neutral_create(&config, &machine) !=
            LIB_STATUS_OK) goto done;
    if (core_machine_register_immutable_rom_mapping(machine, 0x100u,
            image, sizeof(image)) != LIB_STATUS_OK ||
        core_machine_register_immutable_rom_mapping(machine, 0x108u,
            image, sizeof(image)) != LIB_STATUS_OK ||
        core_machine_immutable_rom_mapping_contains(LIB_NULL, 0x100u, 1u) ||
        core_machine_immutable_rom_mapping_contains(machine, 0x100u, 0u) ||
        core_machine_immutable_rom_mapping_contains(machine, LIB_UINT32_MAX, 2u) ||
        core_machine_register_immutable_rom_mapping_reset_window(machine,
            0x102u, LIB_UINT32_MAX, 8u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_register_immutable_rom_mapping_reset_window(machine,
            0x800u, 0x20000u, 8u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_register_immutable_rom_mapping_reset_window(machine,
            0x102u, 0x20000u, 8u) != LIB_STATUS_OK ||
        !core_machine_immutable_rom_mapping_contains(machine, 0x20000u, 2u) ||
        core_machine_immutable_rom_mapping_contains(machine, 0x20002u, 1u) ||
        !core_machine_immutable_rom_mapping_contains(machine, 0x20006u, 2u) ||
        core_machine_immutable_rom_mapping_contains(machine, 0x20008u, 1u) ||
        core_machine_memory_read_reset_physical(&machine->executor_memory,
            0x20000u, (lib_uptr)observed, 2u) != LIB_STATUS_OK ||
        observed[0] != 0x33u || observed[1] != 0x44u) goto done;
    while (machine->immutable_rom_mapping_count <
            CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY - 1u) {
        if (core_machine_register_immutable_rom_mapping(machine,
                0x1000u + (lib_u32)machine->immutable_rom_mapping_count,
                image, 1u) != LIB_STATUS_OK) goto done;
    }
    const lib_size routes = machine->executor_memory.connect.device_provider_count;
    if (core_machine_register_immutable_rom_mapping_reset_window(machine,
            0x102u, 0x30000u, 8u) != LIB_STATUS_NO_MEMORY ||
        machine->immutable_rom_mapping_count !=
            CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY - 1u ||
        machine->executor_memory.connect.device_provider_count != routes ||
        core_machine_immutable_rom_mapping_contains(machine, 0x30000u, 1u) ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_register_immutable_rom_mapping_reset_window(machine,
            0x102u, 0x40000u, 8u) != LIB_STATUS_INVALID_STATE) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 neutral_construction_rejects_invalid_input(void)
{
    core_machine sentinel = {0};
    core_machine *machine = &sentinel;
    const core_machine_executor_config invalid[] = {
        {.cpu_profile = (core_machine_cpu_profile)-1},
        {.provider_clock = {1u, 0u}},
        {.time_axis = {CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 0u}}
    };

    if (core_machine_neutral_create(LIB_NULL, LIB_NULL) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_neutral_create(LIB_NULL, &machine) !=
            LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL) return 1;
    for (lib_size index = 0u; index < sizeof(invalid) / sizeof(invalid[0]); ++index) {
        machine = &sentinel;
        if (core_machine_neutral_config_is_valid(&invalid[index]) ||
            core_machine_neutral_create(&invalid[index], &machine) !=
                LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .ticks_per_instruction = 1u
    };
    const lib_u8 code[16] = {0xb8u, 0x34u, 0x12u, 0x90u, 0xf4u};
    const core_machine_port_provider port = {neutral_port_read, neutral_port_write};
    neutral_probe probe = {0};
    const core_machine_trace_provider trace = {neutral_trace, &probe};
    const core_machine_run_budget budget = {1u, 0u};
    const core_machine_firmware_provider firmware = {
        neutral_firmware_configure, neutral_firmware_reset, LIB_NULL
    };
    core_machine *machine = LIB_NULL;
    core_machine_run_result result;
    core_machine_time_observation time;
    core_machine_observation observation;
    lib_u32 value = 0u;
    lib_u8 byte = 0u;
    lib_i32 failed = 1;

    if (neutral_construction_rejects_invalid_input() || neutral_memory_aliases() || neutral_rom_windows() || neutral_ready_levels() ||
        neutral_timing_declarations()) goto done;
    if (core_machine_neutral_create(&config, &machine) !=
            LIB_STATUS_OK) goto done;
    if (machine->attachment.context != LIB_NULL ||
        core_machine_install_port_provider(machine, 0x1234u, 0x1234u,
            &port, &probe) != LIB_STATUS_OK ||
        core_machine_bind_firmware_provider(machine, &firmware, LIB_NULL) !=
            LIB_STATUS_OK ||
        machine->immutable_rom_mapping_count != 1u ||
        core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    if (core_machine_debug_write_port(machine, 0x1234u, 0x5au) != LIB_STATUS_OK ||
        core_machine_debug_read_port(machine, 0x1234u, &value) != LIB_STATUS_OK ||
        value != 0x5au ||
        core_machine_memory_write(machine, 0x100u, code, 1u) != LIB_STATUS_OK ||
        core_machine_debug_read_memory(machine, 0x100u, &byte, 1u) != LIB_STATUS_OK ||
        byte != code[0]) goto done;
    byte = 0xccu;
    if (core_machine_memory_write(machine, 0xffff0u, &byte, 1u) != LIB_STATUS_OK ||
        core_machine_memory_read(machine, 0xffff0u, &byte, 1u) != LIB_STATUS_OK ||
        byte != code[0]) goto done;
    if (core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        result.executed != 1u ||
        core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_EAX,
            &value) != LIB_STATUS_OK || value != 0x1234u ||
        core_machine_debug_write_register(machine, CORE_MACHINE_DEBUG_EAX,
            0x4321u) != LIB_STATUS_OK ||
        core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_EAX,
            &value) != LIB_STATUS_OK || value != 0x4321u ||
        core_machine_debug_continue(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
        time.elapsed_ticks == 0u ||
        core_machine_capture_observation(machine, &observation) != LIB_STATUS_OK ||
        observation.elapsed_ticks != time.elapsed_ticks ||
        (probe.trace_events != 0u) != (CORE_MACHINE_RUNTIME_TRACE_ENABLED != 0)) goto done;
    core_machine_signal_processor_reset(LIB_NULL);
    if (core_machine_signal_nmi(LIB_NULL) ||
        core_machine_set_nmi_mask(machine, LIB_TRUE) != LIB_STATUS_OK ||
        core_machine_signal_nmi(machine) ||
        core_machine_set_nmi_mask(machine, LIB_FALSE) != LIB_STATUS_OK ||
        !core_machine_signal_nmi(machine)) goto done;
    core_machine_signal_processor_reset(machine);
    if (core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_RESET_REQUESTED ||
        result.executed != 0u || result.linear_pc != 0xffff0u ||
        core_machine_capture_time_observation(machine, &time) != LIB_STATUS_OK ||
        time.elapsed_ticks != observation.elapsed_ticks ||
        core_machine_debug_read_memory(machine, 0x100u, &byte, 1u) != LIB_STATUS_OK ||
        byte != code[0] ||
        core_machine_debug_read_port(machine, 0x1234u, &value) != LIB_STATUS_OK ||
        value != 0x5au || probe.read_tick != time.elapsed_ticks ||
        core_machine_bus_read(machine, 0x1234u, &value) != LIB_STATUS_OK ||
        value != 0x5au || probe.read_tick != time.elapsed_ticks ||
        core_machine_request_stop(machine) != LIB_STATUS_OK ||
        core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_REQUESTED) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    if (failed == 0) lib_c_printf("M5:T540:S69:NEUTRAL-LINK:OK\n");
    return failed;
}

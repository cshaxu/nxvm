#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86/core/machine.h"
#include "x86/core/debug_interface.h"

typedef struct neutral_probe {
    lib_u32 port_value;
    lib_u32 trace_events;
    lib_u64 read_tick;
    lib_status port_status;
    lib_u32 reads, writes;
    lib_bool firmware_alias;
    lib_bool firmware_fail;
} neutral_probe;

static lib_status neutral_port_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value)
{
    neutral_probe *probe = owner;
    if (port != 0x1234u) return LIB_STATUS_INVALID_ARGUMENT;
    if (probe->port_status != LIB_STATUS_OK) return probe->port_status;
    probe->read_tick = tick;
    ++probe->reads;
    *out_value = probe->port_value;
    return LIB_STATUS_OK;
}

static lib_status neutral_port_write(void *owner, lib_u16 port, lib_u32 value)
{
    neutral_probe *probe = owner;
    if (port != 0x1234u) return LIB_STATUS_INVALID_ARGUMENT;
    if (probe->port_status != LIB_STATUS_OK) return probe->port_status;
    ++probe->writes;
    probe->port_value = value;
    return LIB_STATUS_OK;
}

static void neutral_trace(void *owner, const core_machine_trace_event *event)
{
    neutral_probe *probe = owner;
    if (event != LIB_NULL) ++probe->trace_events;
}

static void neutral_time_reset(void *owner) { *(lib_u64 *)owner = 0u; }
static void neutral_time_advance(void *owner, lib_u64 ticks) { *(lib_u64 *)owner = ticks; }
static lib_i32 neutral_time_publication(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    const core_machine_execution_provider provider = {neutral_time_reset, neutral_time_advance};
    core_machine *machine = LIB_NULL;
    core_machine_time_observation observed;
    core_machine_timeline_observation timeline;
    lib_u64 delivered = 99u, ticks = 99u;
    lib_u64 event_tick = 99u;
    lib_u8 advanced = 0xa5u;
    core_machine_timeline_token token;
    lib_i32 failed = 1;
    if (core_machine_advance_time(LIB_NULL, 1u) != LIB_STATUS_INVALID_STATE ||
        core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_bind_execution_provider(machine, &provider, &delivered) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK || delivered != 0u) goto done;
    if (core_machine_advance_time(machine, 0u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_advance_time(machine, 37u) != LIB_STATUS_OK || delivered != 37u ||
        core_machine_capture_time_observation(machine, &observed) != LIB_STATUS_OK ||
        observed.elapsed_ticks != 37u ||
        core_machine_get_timeline_observation(machine, &timeline) != LIB_STATUS_OK || timeline.now != 37u ||
        core_machine_advance_time(machine, LIB_UINT64_MAX) != LIB_STATUS_INVALID_ARGUMENT || delivered != 37u ||
        core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 37u ||
        core_machine_advance_time(machine, 1u) != LIB_STATUS_OK || delivered != 1u ||
        core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 38u ||
        core_machine_reset(machine) != LIB_STATUS_OK || delivered != 0u ||
        core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 0u) goto done;
    if (core_machine_advance_to_next_deadline(LIB_NULL, &advanced) != LIB_STATUS_INVALID_STATE || advanced != 0xa5u ||
        core_machine_advance_to_next_deadline(machine, LIB_NULL) != LIB_STATUS_INVALID_STATE ||
        core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK || advanced ||
        core_machine_timeline_schedule(&machine->timeline, 5u, neutral_time_advance,
            &event_tick, &token) != LIB_STATUS_OK ||
        core_machine_capture_time_observation(machine, &observed) != LIB_STATUS_OK ||
        !observed.next_deadline_valid || observed.next_deadline_tick != 5u ||
        core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK || !advanced || event_tick != 5u ||
        core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 5u ||
        core_machine_advance_l1_compatibility(machine, &advanced) != LIB_STATUS_OK || advanced ||
        core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 5u ||
        core_machine_timeline_schedule(&machine->timeline, 5u, neutral_time_advance,
            &event_tick, &token) != LIB_STATUS_OK ||
        core_machine_capture_time_observation(machine, &observed) != LIB_STATUS_OK ||
        observed.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IMMEDIATE || observed.next_deadline_valid ||
        core_machine_advance_to_next_deadline(machine, &advanced) != LIB_STATUS_OK || advanced ||
        core_machine_get_elapsed_ticks(machine, &ticks) != LIB_STATUS_OK || ticks != 5u) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 neutral_port_routes(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    neutral_probe first = {.port_value = 0x0fu}, second = {.port_value = 0xf0u};
    const core_machine_port_route initial = {
        .address = 0x1234u, .read = neutral_port_read, .write = neutral_port_write, .owner = &first
    };
    const core_machine_port_route rejected[] = {
        {.address = 0x1235u, .read = neutral_port_read, .owner = &second},
        {.address = 0x1234u, .read = neutral_port_read, .owner = &second}
    };
    const core_machine_port_route joined = {
        .address = 0x1234u, .read = neutral_port_read, .owner = &second, .wired_or_read = LIB_TRUE
    };
    core_machine *machine = LIB_NULL;
    lib_u32 value = 0u;
    lib_i32 failed = 1;
    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK) goto done;
    if (core_machine_install_port_routes(machine, LIB_NULL, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_install_port_routes(machine, &initial, 0u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_install_port_routes(machine, &initial, 1u) != LIB_STATUS_OK ||
        core_machine_install_port_routes(machine, rejected, 2u) != LIB_STATUS_INVALID_STATE ||
        core_machine_port_has_read(&machine->executor_port, 0x1235u) ||
        core_machine_install_port_routes(machine, &joined, 1u) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    if (core_machine_bus_read(machine, 0x1234u, &value) != LIB_STATUS_OK ||
        value != 0xffu || first.reads != 1u || second.reads != 1u ||
        first.read_tick != second.read_tick ||
        core_machine_bus_write(machine, 0x1234u, 0x5au) != LIB_STATUS_OK ||
        first.port_value != 0x5au || first.writes != 1u || second.writes != 0u) goto done;
    second.port_status = LIB_STATUS_IO_ERROR;
    value = 0x12345678u;
    const lib_u32 prior_latch = machine->executor_port.data.ioDWord;
    if (core_machine_bus_read(machine, 0x1234u, &value) != LIB_STATUS_IO_ERROR ||
        value != 0x12345678u || machine->executor_port.data.ioDWord != prior_latch ||
        core_machine_remove_port_routes(machine, &second) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x1234u, &value) != LIB_STATUS_OK || value != 0x5au) goto done;
    first.port_status = LIB_STATUS_IO_ERROR;
    if (core_machine_bus_write(machine, 0x1234u, 0xffu) != LIB_STATUS_IO_ERROR ||
        first.port_value != 0x5au || machine->executor_port.data.ioDWord != 0x5au ||
        core_machine_install_port_routes(machine, &initial, 1u) != LIB_STATUS_INVALID_STATE ||
        core_machine_remove_port_routes(machine, &first) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x1234u, &value) != LIB_STATUS_UNSUPPORTED ||
        core_machine_bus_write(machine, 0x1234u, 1u) != LIB_STATUS_UNSUPPORTED) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

typedef struct bank_probe {
    lib_u8 values[2];
    lib_u32 reads, writes;
    lib_bool fail_second;
} bank_probe;
static lib_status bank_read(void *owner, lib_u16 port, lib_u64 tick, lib_u32 *out)
{
    bank_probe *probe = owner;
    if (port < 0x2000u || port > 0x2001u || tick != 7u) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == 0x2001u && probe->fail_second) return LIB_STATUS_IO_ERROR;
    ++probe->reads;
    *out = probe->values[port - 0x2000u];
    return LIB_STATUS_OK;
}
static lib_status bank_write(void *owner, lib_u16 port, lib_u32 value)
{
    bank_probe *probe = owner;
    if (port < 0x2000u || port > 0x2001u) return LIB_STATUS_INVALID_ARGUMENT;
    ++probe->writes;
    probe->values[port - 0x2000u] = (lib_u8)value;
    return LIB_STATUS_OK;
}
static lib_i32 neutral_port_bank(void)
{
    const core_machine_executor_config config = {.memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086};
    bank_probe probe = {.values = {0x12u, 0x34u}};
    const core_machine_port_route route = {.address = 0x2000u, .read = bank_read,
        .write = bank_write, .owner = &probe, .byte_lane_end = 0x2002u};
    core_machine *machine = LIB_NULL;
    lib_i32 failed = 1;
    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_install_port_routes(machine, &route, 1u) != LIB_STATUS_OK ||
        core_machine_port_execute_read_width(&machine->executor_port, 0x2000u, 2u, 7u) != LIB_STATUS_OK ||
        machine->executor_port.data.ioDWord != 0x3412u || probe.reads != 2u) goto done;
    probe.fail_second = LIB_TRUE;
    machine->executor_port.data.ioDWord = 0xdeadbeefu;
    if (core_machine_port_execute_read_width(&machine->executor_port, 0x2000u, 2u, 7u) != LIB_STATUS_IO_ERROR ||
        machine->executor_port.data.ioDWord != 0xdeadbeefu || probe.reads != 3u) goto done;
    probe.fail_second = LIB_FALSE;
    machine->executor_port.data.ioDWord = 0x7856u;
    if (core_machine_port_execute_write_width(&machine->executor_port, 0x2000u, 2u) != LIB_STATUS_OK ||
        probe.writes != 2u || probe.values[0] != 0x56u || probe.values[1] != 0x78u ||
        core_machine_port_execute_read_width(&machine->executor_port, 0x2000u, 3u, 7u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_port_execute_read_width(&machine->executor_port, 0xffffu, 2u, 7u) != LIB_STATUS_INVALID_ARGUMENT) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
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
    neutral_probe *probe = owner;
    lib_u8 unchanged = 0xa5u;
    if (core_machine_firmware_memory_read(firmware, 0u, &unchanged, 1u) !=
            LIB_STATUS_INVALID_STATE || unchanged != 0xa5u) return LIB_STATUS_INTERNAL_ERROR;
    lib_status status = core_machine_firmware_register_immutable_rom(firmware,
        0xffff0u, code, sizeof(code));
    if (status == LIB_STATUS_OK && probe != LIB_NULL && probe->firmware_alias)
        status = core_machine_firmware_register_immutable_rom_alias(firmware,
            0xffff0u, 0x1000u, sizeof(code));
    if (status == LIB_STATUS_OK && probe != LIB_NULL && probe->firmware_fail)
        return LIB_STATUS_IO_ERROR;
    return status;
}

static lib_status neutral_firmware_reset(void *owner,
    core_machine_firmware_context *firmware)
{
    (void)owner;
    const lib_u8 written = 0x6bu, clear = 0u;
    lib_u8 read = 0u;
    lib_u32 port = 0u;
    if (core_machine_firmware_memory_write(firmware, 0x120u, &written, 1u) != LIB_STATUS_OK ||
        core_machine_firmware_memory_read(firmware, 0x120u, &read, 1u) != LIB_STATUS_OK || read != written ||
        core_machine_firmware_memory_write(firmware, 0x120u, &clear, 1u) != LIB_STATUS_OK ||
        core_machine_firmware_port_write(firmware, 0x1234u, 0xa5u) != LIB_STATUS_OK ||
        core_machine_firmware_port_read(firmware, 0x1234u, &port) != LIB_STATUS_OK || port != 0xa5u ||
        core_machine_firmware_port_write(firmware, 0x1234u, 0u) != LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
    return LIB_STATUS_OK;
}

static lib_i32 neutral_firmware_alias_contract(void)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086
    };
    const core_machine_firmware_provider firmware = {neutral_firmware_configure, neutral_firmware_reset, LIB_NULL};
    const core_machine_port_provider ports = {neutral_port_read, neutral_port_write};
    neutral_probe probe = {.firmware_alias = LIB_TRUE, .firmware_fail = LIB_TRUE};
    core_machine *machine = LIB_NULL;
    lib_u8 byte = 0xa5u;
    lib_u32 port = 0xfeedu;
    core_machine_memory_route route;
    lib_i32 failed = 1;
    if (core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x1234u, 0x1234u, &ports, &probe) != LIB_STATUS_OK ||
        core_machine_bind_firmware_provider(machine, &firmware, &probe) != LIB_STATUS_IO_ERROR ||
        machine->immutable_rom_mapping_count != 0u || machine->firmware_provider != LIB_NULL ||
        core_machine_memory_query_physical(&machine->executor_memory, 0x1000u, 1u,
            CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
        route != CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM) goto done;
    probe.firmware_fail = LIB_FALSE;
    if (core_machine_bind_firmware_provider(machine, &firmware, &probe) != LIB_STATUS_OK ||
        machine->immutable_rom_mapping_count != 2u ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_read(machine, 0x1000u, &byte, 1u) != LIB_STATUS_OK || byte != 0xb8u ||
        core_machine_firmware_memory_read(&machine->firmware_context, 0x1000u, &byte, 1u) != LIB_STATUS_INVALID_STATE ||
        byte != 0xb8u || core_machine_firmware_port_read(&machine->firmware_context, 0x1234u, &port) != LIB_STATUS_INVALID_STATE ||
        port != 0xfeedu) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
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

    if (neutral_construction_rejects_invalid_input() || neutral_firmware_alias_contract() || neutral_time_publication() || neutral_port_bank() || neutral_port_routes() || neutral_memory_aliases() || neutral_rom_windows() || neutral_ready_levels() ||
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
    if (failed == 0) lib_c_printf("NEUTRAL-LINK:OK\n");
    return failed;
}

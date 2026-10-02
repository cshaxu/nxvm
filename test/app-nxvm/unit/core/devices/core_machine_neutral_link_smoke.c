#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/debug_interface.h"

typedef struct neutral_probe {
    lib_u32 port_value;
    lib_u32 trace_events;
} neutral_probe;

static lib_status neutral_port_read(void *owner, lib_u16 port, lib_u32 *out_value)
{
    neutral_probe *probe = owner;
    if (port != 0x1234u) return LIB_STATUS_INVALID_ARGUMENT;
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
    core_machine *machine = LIB_NULL;
    core_machine_run_result result;
    core_machine_time_observation time;
    core_machine_observation observation;
    lib_u32 value = 0u;
    lib_u8 byte = 0u;
    lib_i32 failed = 1;

    if (core_machine_neutral_create(&config, LIB_NULL, LIB_NULL, &machine) !=
            LIB_STATUS_OK) goto done;
    if (machine->board != LIB_NULL ||
        core_machine_install_port_provider(machine, 0x1234u, 0x1234u,
            &port, &probe) != LIB_STATUS_OK ||
        core_machine_register_immutable_rom_mapping(machine, 0xffff0u,
            code, sizeof(code)) != LIB_STATUS_OK ||
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
        value != 0x5au || core_machine_request_stop(machine) != LIB_STATUS_OK ||
        core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_REQUESTED) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    if (failed == 0) lib_c_printf("M5:T540:S69:NEUTRAL-LINK:OK\n");
    return failed;
}

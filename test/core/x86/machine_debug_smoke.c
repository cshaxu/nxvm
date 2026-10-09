#include "lib/types/types_interface.h"
#include "lib/types/file.h"



#include "core/x86/debug_interface.h"
#include "executor_fixture.h"

typedef struct debug_port_probe { lib_status status; lib_u32 value; } debug_port_probe;
static lib_status debug_port_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out)
{ (void)tick; debug_port_probe *probe = owner; (void)port; if (probe->status != LIB_STATUS_OK) return probe->status; *out = probe->value; return LIB_STATUS_OK; }
static lib_status debug_port_write(void *owner, lib_u16 port, lib_u32 value)
{ debug_port_probe *probe = owner; (void)port; if (probe->status != LIB_STATUS_OK) return probe->status; probe->value = value; return LIB_STATUS_OK; }

static lib_i32 debug_observation_contract(core_machine *machine)
{
    const lib_u8 bytes[2] = {0x12u, 0x34u};
    lib_u8 actual[2] = {0};
    lib_u32 base = 0u, address = 0u;
    lib_i32 size = -1;
    lib_u8 enabled = LIB_FALSE;
    if (core_machine_debug_get_code_base(machine, &base) != LIB_STATUS_OK || base != 0xffff0000u ||
        core_machine_debug_get_code_default_size(machine, &size) != LIB_STATUS_OK || size != 0 ||
        core_machine_debug_get_code_base(machine, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_debug_get_code_default_size(machine, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_debug_write_linear(machine, 0x100u, bytes, 1u) != LIB_STATUS_OK ||
        core_machine_debug_read_real(machine, 0x10u, 0u, actual, 1u) != LIB_STATUS_OK || actual[0] != bytes[0] ||
        core_machine_debug_write_real(machine, 0x10u, 1u, bytes + 1u, 1u) != LIB_STATUS_OK ||
        core_machine_debug_read_linear(machine, 0x100u, actual, 2u) != LIB_STATUS_OK ||
        lib_memory_compare(actual, bytes, sizeof(bytes)) != 0) return 1;
    for (lib_u32 kind = CORE_MACHINE_DEBUG_WATCH_READ; kind <= CORE_MACHINE_DEBUG_WATCH_EXECUTE; ++kind) {
        const core_machine_debug_watch_kind watch = (core_machine_debug_watch_kind)kind;
        if (core_machine_debug_set_watchpoint(machine, watch, 0x12340000u + kind) != LIB_STATUS_OK ||
            core_machine_debug_get_watchpoint(machine, watch, &enabled, &address) != LIB_STATUS_OK ||
            !enabled || address != 0x12340000u + kind ||
            core_machine_debug_clear_watchpoint(machine, watch) != LIB_STATUS_OK ||
            core_machine_debug_get_watchpoint(machine, watch, &enabled, &address) != LIB_STATUS_OK ||
            enabled || address != 0x12340000u + kind) return 1;
    }
    address = 0xfeedu;
    enabled = LIB_TRUE;
    if (core_machine_debug_get_watchpoint(machine, (core_machine_debug_watch_kind)3,
            &enabled, &address) != LIB_STATUS_INVALID_ARGUMENT || !enabled || address != 0xfeedu ||
        core_machine_debug_get_watchpoint(machine, CORE_MACHINE_DEBUG_WATCH_READ,
            LIB_NULL, &address) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_debug_get_watchpoint(machine, CORE_MACHINE_DEBUG_WATCH_READ,
            &enabled, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT) return 1;
    return 0;
}

lib_i32 main(void)
{
    core_machine *machine = LIB_NULL;
    core_machine_cpu_state cpu;
    core_machine_debug_instruction_observation observation;
    core_machine_debug_register_patch patch = {0};
    core_machine_run_result result;
    core_machine_run_budget budget = { 2u, 0u };
    lib_u32 value;
    lib_u8 byte = 0x5au;
    lib_u8 nop = 0x90u;
    lib_u8 read_moffs[] = {0xa0u, 0x00u, 0x00u};
    lib_u8 write_moffs[] = {0xa2u, 0x00u, 0x00u};
    debug_port_probe port_probe = {LIB_STATUS_OK, 0x11u};
    core_machine_port_provider port_provider = {debug_port_read, debug_port_write};

    if (test_core_machine_create_executor(0u, &machine) != LIB_STATUS_OK ||
        core_machine_install_port_provider(machine, 0x00e0u, 0x00e0u,
            &port_provider, &port_probe) != LIB_STATUS_OK ||
        core_machine_debug_read_cpu(machine, &cpu) != LIB_STATUS_INVALID_STATE ||
        core_machine_debug_capture_instruction_observation(machine,
            &observation) != LIB_STATUS_INVALID_STATE ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, &byte, 1u) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0xffff0u, &nop, 1u) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0xffff1u, &nop, 1u) != LIB_STATUS_OK ||
        core_machine_debug_read_cpu(machine, &cpu) != LIB_STATUS_OK ||
        cpu.cs != 0xf000u || cpu.eip != 0xfff0u ||
        core_machine_debug_read_memory(machine, 0u, &byte, 1u) != LIB_STATUS_OK ||
        byte != 0x5au ||
        core_machine_debug_capture_instruction_observation(machine,
            &observation) != LIB_STATUS_OK || observation.cs != 0xf000u ||
        observation.cs_base != 0xffff0000u || observation.eip != 0xfff0u ||
        observation.instruction_byte_count >
            CORE_MACHINE_DEBUG_INSTRUCTION_BYTES ||
        observation.memory_access_count >
            CORE_MACHINE_DEBUG_MEMORY_ACCESS_CAPACITY ||
        core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_debug_continue(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET) {
        core_machine_destroy(machine);
        return 1;
    }

    if (debug_observation_contract(machine)) {
        core_machine_destroy(machine);
        return 1;
    }
    port_probe.status = LIB_STATUS_INTERNAL_ERROR;
    value = 0xdeadbeefu;
    if (core_machine_debug_read_port(machine, 0x00e0u, &value) !=
            LIB_STATUS_INTERNAL_ERROR || value != 0xdeadbeefu ||
        core_machine_debug_write_port(machine, 0x00e0u, 0x55u) !=
            LIB_STATUS_INTERNAL_ERROR) {
        core_machine_destroy(machine);
        return 1;
    }
    port_probe.status = LIB_STATUS_OK;
    if (core_machine_debug_read_port(machine, 0x00e0u, &value) !=
            LIB_STATUS_OK || value != 0x11u ||
        core_machine_debug_write_port(machine, 0x00e0u, 0x55u) !=
            LIB_STATUS_OK || port_probe.value != 0x55u) {
        core_machine_destroy(machine);
        return 1;
    }

    patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
        CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX);
    patch.values[CORE_MACHINE_DEBUG_EAX] = 0x12345678u;
    patch.values[CORE_MACHINE_DEBUG_EBX] = 0x87654321u;
    if (core_machine_debug_patch_registers(machine, &patch) != LIB_STATUS_OK ||
        core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_EAX,
            &value) != LIB_STATUS_OK || value != 0x12345678u ||
        core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_EBX,
            &value) != LIB_STATUS_OK || value != 0x87654321u) {
        core_machine_destroy(machine);
        return 1;
    }
    patch.mask |= 0x80000000u;
    patch.values[CORE_MACHINE_DEBUG_EAX] = 0u;
    if (core_machine_debug_patch_registers(machine, &patch) !=
            LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_debug_read_register(machine, CORE_MACHINE_DEBUG_EAX,
            &value) != LIB_STATUS_OK || value != 0x12345678u) {
        core_machine_destroy(machine);
        return 1;
    }

    if (core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0xffff0u, &nop, 1u) != LIB_STATUS_OK ||
        core_machine_debug_set_watchpoint(machine, CORE_MACHINE_DEBUG_WATCH_EXECUTE,
            0xfffffff0u) != LIB_STATUS_OK ||
        core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_PAUSED ||
        core_machine_debug_capture_instruction_observation(machine,
        &observation) != LIB_STATUS_OK || !observation.watch_hit ||
        observation.watch_kind != CORE_MACHINE_DEBUG_WATCH_EXECUTE ||
        observation.watch_address != 0xfffffff0u ||
        core_machine_debug_clear_watchpoint(machine,
            CORE_MACHINE_DEBUG_WATCH_EXECUTE) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    if (core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0u, &byte, 1u) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0xffff0u, read_moffs,
            sizeof(read_moffs)) != LIB_STATUS_OK ||
        core_machine_debug_set_watchpoint(machine, CORE_MACHINE_DEBUG_WATCH_READ,
            0u) != LIB_STATUS_OK ||
        core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_PAUSED ||
        core_machine_debug_capture_instruction_observation(machine,
            &observation) != LIB_STATUS_OK || !observation.watch_hit ||
        observation.watch_kind != CORE_MACHINE_DEBUG_WATCH_READ ||
        observation.watch_address != 0u ||
        core_machine_debug_clear_watchpoint(machine,
            CORE_MACHINE_DEBUG_WATCH_READ) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    if (core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0xffff0u, write_moffs,
            sizeof(write_moffs)) != LIB_STATUS_OK ||
        core_machine_debug_set_watchpoint(machine, CORE_MACHINE_DEBUG_WATCH_WRITE,
            0u) != LIB_STATUS_OK ||
        core_machine_debug_step(machine, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_PAUSED ||
        core_machine_debug_capture_instruction_observation(machine,
            &observation) != LIB_STATUS_OK || !observation.watch_hit ||
        observation.watch_kind != CORE_MACHINE_DEBUG_WATCH_WRITE ||
        observation.watch_address != 0u ||
        core_machine_debug_clear_watchpoint(machine,
            CORE_MACHINE_DEBUG_WATCH_WRITE) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }

    core_machine_destroy(machine);
    lib_c_printf("DEBUG:OK\n");
    return 0;
}

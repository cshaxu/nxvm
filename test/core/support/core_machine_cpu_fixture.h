#ifndef TEST_CORE_MACHINE_CPU_FIXTURE_H
#define TEST_CORE_MACHINE_CPU_FIXTURE_H

/* Private prepared-state operations for CPU execution corpus fixtures. */
#include "core/machine/machine.h"

static inline C_INT test_core_machine_fixture_reset_real_mode(core_machine *machine)
{
    t_cpu *cpu;
    core_machine_cpu_execution_context *execution;

    if (machine == STD_NULL) return 0;
    cpu = &machine->executor_cpu;
    execution = &machine->executor_cpu_execution;
    return core_machine_cpu_execution_load_segment(execution, &cpu->data.cs, 0u) == 0 &&
        core_machine_cpu_execution_load_segment(execution, &cpu->data.ds, 0u) == 0 &&
        core_machine_cpu_execution_load_segment(execution, &cpu->data.es, 0u) == 0 &&
        core_machine_cpu_execution_load_segment(execution, &cpu->data.ss, 0u) == 0 &&
        ((cpu->data.eip = 0u), 1);
}

static inline C_INT test_core_machine_fixture_set_control_zero(
    core_machine *machine, type_unsigned_32 value)
{
    if (machine == STD_NULL) return 0;
    machine->executor_cpu.data.cr0 = value;
    return 1;
}

static inline t_cpu test_core_machine_fixture_capture_cpu_after_run(
    core_machine *machine)
{
    t_cpu observation = {0};

    if (machine != STD_NULL) observation = machine->executor_cpu;
    return observation;
}

/* This is the lifecycle tail after owner-local construction/setup. */
static inline C_INT test_core_machine_fixture_bind_freeze_reset(
    core_machine *machine, const core_machine_execution_provider *provider,
    C_VOID *provider_owner)
{
    return core_machine_bind_execution_provider(machine, provider,
        provider_owner) == TYPE_STATUS_OK &&
        core_machine_freeze_execution_providers(machine) == TYPE_STATUS_OK &&
        core_machine_reset(machine) == TYPE_STATUS_OK;
}

/*
 * This preserves the corpus' established short-circuit lifecycle: it does
 * not add cleanup or validation policy.  Owner smokes retain all device and
 * instruction-specific setup before or after this fixed sequence.
 */
static inline C_INT test_core_machine_fixture_create_bind_freeze_reset(
    const core_machine_config *config,
    const core_machine_execution_provider *provider, C_VOID *provider_owner,
    core_machine **out_machine)
{
    return core_machine_create(config, out_machine) == TYPE_STATUS_OK &&
        test_core_machine_fixture_bind_freeze_reset(*out_machine, provider,
            provider_owner);
}

static inline type_status test_core_machine_fixture_register_reset_mapping(
    core_machine *machine, type_unsigned_32 linear, type_unsigned_32 physical,
    STD_SIZE_T bytes)
{
    type_status status;
    STD_SIZE_T mapped_bytes;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    /* Instruction refresh can prefetch up to 15 bytes.  A reset fixture which
     * supplies a shorter program must still map that complete window; otherwise
     * the trailing fetch can escape a high-ROM alias before the first opcode. */
    mapped_bytes = bytes < 15u ? 15u : bytes;
    status = core_machine_memory_register_mapping(&machine->executor_memory, linear,
        physical, mapped_bytes, TYPE_FALSE);
    /* The corpus names every reset fixture through the 80386 alias.  Each
     * earlier CPU fetches the same bytes through its narrower physical bus. */
    if (status == TYPE_STATUS_OK &&
        machine->cpu_profile <= CORE_MACHINE_CPU_PROFILE_80286 &&
        linear == 0xfffffff0u) {
        status = core_machine_memory_register_mapping(&machine->executor_memory,
            machine->cpu_profile <= CORE_MACHINE_CPU_PROFILE_80186 ?
                0x000ffff0u : 0x00fffff0u, physical, mapped_bytes, TYPE_FALSE);
    }
    return status;
}

static inline type_status test_core_machine_fixture_register_memory_device_provider(
    core_machine *machine, type_unsigned_32 physical, STD_SIZE_T bytes,
    core_machine_memory_device_read read,
    core_machine_memory_device_write write,
    core_machine_memory_device_query query, C_VOID *owner)
{
    return machine == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        core_machine_memory_register_device_provider(&machine->executor_memory,
            physical, bytes, read, write, query, owner);
}

static inline C_VOID test_core_machine_fixture_program_pit_divider(
    core_machine *machine, type_unsigned_8 control, type_unsigned_16 divisor,
    core_machine_pit_output_provider output, C_VOID *owner)
{
    if (machine == STD_NULL) return;
    core_machine_pit_set_output(&machine->shared_pit, 0u, output, owner);
    core_machine_port_write(&machine->executor_port, 0x0043u, control);
    core_machine_port_write(&machine->executor_port, 0x0040u, divisor & 0xffu);
    core_machine_port_write(&machine->executor_port, 0x0040u, divisor >> 8u);
}

static inline type_unsigned_8 test_core_machine_fixture_read_port(
    const core_machine *machine, type_unsigned_16 address)
{
    return machine == STD_NULL ? 0u : core_machine_port_read(
        (t_port *)&machine->executor_port, address);
}

static inline C_INT test_core_machine_fixture_capture_instruction_exception(
    const core_machine *machine, type_unsigned_32 *out_mask, type_unsigned_32 *out_code)
{
    if (machine == STD_NULL || out_mask == STD_NULL || out_code == STD_NULL) return 0;
    *out_mask = machine->executor_cpu_instructions.data.except;
    *out_code = machine->executor_cpu_instructions.data.excode;
    return 1;
}

static inline C_INT test_core_machine_fixture_prepare_real_mode_execution(
    core_machine *machine, type_unsigned_32 eip)
{
    if (!test_core_machine_fixture_reset_real_mode(machine)) return 0;
    machine->executor_cpu.data.eip = eip;
    machine->executor_cpu.data.flagHalt = TYPE_FALSE;
    return 1;
}

/*
 * A negative real-mode #UD test that proves only producer rollback must make
 * vector 6 unavailable explicitly. Real hardware otherwise consumes the IVT
 * entry and publishes an interrupt frame, even when its contents are zero.
 * Call this immediately before the negative run; owners that prove delivery
 * instead install and validate a vector-6 handler themselves.
 */
static inline C_INT test_core_machine_fixture_preflight_real_ud_terminal(
    core_machine *machine)
{
    if (machine == STD_NULL) return 0;
    if (machine->executor_cpu.data.idtr.limit >= 0x18u) {
        machine->executor_cpu.data.idtr.limit = 0x17u;
    }
    return 1;
}

static inline C_VOID test_core_machine_fixture_resume_after_halt_at(
    core_machine *machine, type_unsigned_32 eip)
{
    if (machine == STD_NULL) return;
    machine->executor_cpu.data.flagHalt = TYPE_FALSE;
    machine->executor_cpu.data.eip = eip;
}

/* Legacy corpus targets which assert handler-visible state can opt into this
 * adapter while retaining the production two-round fault boundary.  It makes
 * the delivery round observable as zero retirement before invoking the
 * handler's distinct execution round.  Focused S3 tests call core_machine_run
 * directly and assert that first result themselves. */
static inline type_status test_core_machine_fixture_run_after_delivery(
    core_machine *machine, core_machine_run_budget budget,
    core_machine_run_result *out_result)
{
    type_status status = core_machine_run(machine, budget, out_result);
    core_machine_cpu_diagnostic diagnostic;
    type_bool delivered = TYPE_FALSE;

    if (status == TYPE_STATUS_OK && out_result != STD_NULL &&
        out_result->reason == CORE_MACHINE_STOP_BUDGET &&
        core_machine_get_cpu_diagnostic(machine, &diagnostic) == TYPE_STATUS_OK) {
        delivered = diagnostic.last_delivered_exception.valid;
    }
    if (delivered) {
        status = core_machine_run(machine, budget, out_result);
    }
    return status;
}

#ifdef CORE_MACHINE_TEST_CONTINUE_DELIVERED_FAULT
#define core_machine_run test_core_machine_fixture_run_after_delivery
#endif

static inline C_INT test_core_machine_fixture_read_linear(
    core_machine *machine, type_unsigned_32 address, type_virtual_address destination,
    STD_SIZE_T bytes)
{
    return machine != STD_NULL && core_machine_cpu_execution_read_linear(
        &machine->executor_cpu_execution, address, destination, bytes) == 0;
}

static inline type_status test_core_machine_fixture_query_configuration_memory_route(
    const core_machine *machine, type_unsigned_32 physical, STD_SIZE_T bytes,
    core_machine_memory_access access, core_machine_memory_route *out_route)
{
    return machine == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        core_machine_memory_query_physical(&machine->executor_memory, physical,
            bytes, access, out_route);
}

static inline C_VOID test_core_machine_fixture_initialize_rtc_with_shared_pic(
    core_machine *machine, core_machine_rtc *rtc,
    const core_machine_rtc_config *config)
{
    if (machine != STD_NULL && rtc != STD_NULL && config != STD_NULL) {
        core_machine_rtc_initialize(rtc, &machine->shared_pic_master,
            &machine->shared_pic_slave, config);
    }
}

static inline C_INT test_core_machine_fixture_executor_storage_is_coherent(
    const core_machine *machine)
{
    return machine != STD_NULL && machine->executor_cpu_execution.cpu ==
        &machine->executor_cpu && machine->executor_cpu_execution.instructions ==
        &machine->executor_cpu_instructions;
}

static inline C_INT test_core_machine_fixture_sessions_are_isolated(
    core_machine *first, core_machine *second)
{
    type_unsigned_8 first_value = 0x11u;
    type_unsigned_8 second_value = 0x22u;
    type_unsigned_8 observed = 0u;

    if (first == STD_NULL || second == STD_NULL || first == second ||
        &first->executor_cpu == &second->executor_cpu ||
        &first->executor_memory == &second->executor_memory ||
        &first->executor_port == &second->executor_port ||
        &first->shared_rtc == &second->shared_rtc || &first->fdc == &second->fdc ||
        &first->hdc == &second->hdc ||
        !test_core_machine_fixture_executor_storage_is_coherent(first) ||
        !test_core_machine_fixture_executor_storage_is_coherent(second)) return 0;
    core_machine_memory_write_physical(&first->executor_memory, 0u,
        (type_virtual_address)&first_value, 1u);
    core_machine_memory_write_physical(&second->executor_memory, 0u,
        (type_virtual_address)&second_value, 1u);
    core_machine_memory_read_physical(&first->executor_memory, 0u,
        (type_virtual_address)&observed, 1u);
    if (observed != first_value) return 0;
    core_machine_memory_read_physical(&second->executor_memory, 0u,
        (type_virtual_address)&observed, 1u);
    if (observed != second_value) return 0;
    first->executor_cpu.data.eax = 0x11111111u;
    second->executor_cpu.data.eax = 0x22222222u;
    first->executor_cpu_instructions.data.flagWR = TYPE_TRUE;
    return second->executor_cpu.data.eax == 0x22222222u &&
        second->executor_cpu_instructions.data.flagWR == TYPE_FALSE;
}

#endif

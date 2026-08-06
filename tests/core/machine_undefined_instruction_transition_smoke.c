#include "type.h"

#include "core/machine/machine_interface.h"

#define RESET_LINEAR 0xfffffff0u
#define RESET_PHYSICAL 0x000ffff0u

typedef struct transition_probe {
    C_UINT calls;
    core_machine_undefined_instruction_outcome outcome;
} transition_probe;

static C_VOID transition_consumer(C_VOID *owner,
    const core_machine_undefined_instruction_input *input,
    core_machine_undefined_instruction_response *out_response)
{
    transition_probe *probe = (transition_probe *)owner;

    if (probe == STD_NULL || input == STD_NULL || out_response == STD_NULL) return;
    ++probe->calls;
    out_response->outcome = probe->outcome;
    out_response->patch.eax = 0x12345678u;
    out_response->patch.ebx = 0x87654321u;
    out_response->patch.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
    out_response->fault_detail = 0x244u;
}

static C_INT create_machine(core_machine **out_machine, transition_probe *probe,
    C_INT register_transition)
{
    static const uint8_t pattern[] = { 0xd6u };
    static const uint8_t prefix[] = { 0xd6u, 0x90u };
    static const uint8_t reset_jump[] = { 0xeau, 0x00u, 0x00u, 0x00u, 0x00u };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || core_machine_create(&config, &machine) !=
            TYPE_STATUS_OK || core_machine_memory_register_mapping(
            core_machine_configuration_memory_borrow(machine), RESET_LINEAR,
            RESET_PHYSICAL, 16u) != TYPE_STATUS_OK) goto fail;
    if (register_transition) {
        if (core_machine_register_undefined_instruction_transition(machine, pattern,
                sizeof(pattern), transition_consumer, probe) != TYPE_STATUS_OK ||
            core_machine_register_undefined_instruction_transition(machine, pattern,
                sizeof(pattern), transition_consumer, probe) != TYPE_STATUS_INVALID_ARGUMENT ||
            core_machine_register_undefined_instruction_transition(machine, prefix,
                sizeof(prefix), transition_consumer, probe) != TYPE_STATUS_INVALID_ARGUMENT) {
            goto fail;
        }
    }
    if (core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        (register_transition && core_machine_register_undefined_instruction_transition(
            machine, pattern, sizeof(pattern), transition_consumer, probe) !=
            TYPE_STATUS_INVALID_STATE) || core_machine_reset(machine) !=
            TYPE_STATUS_OK || core_machine_memory_write(machine, RESET_LINEAR,
            reset_jump, sizeof(reset_jump)) != TYPE_STATUS_OK) goto fail;
    *out_machine = machine;
    return 0;
fail:
    core_machine_destroy(machine);
    return 1;
}

static C_INT test_unhandled(C_VOID)
{
    core_machine *machine = STD_NULL;
    const core_machine_run_budget budget = { 16u, 0u };
    core_machine_run_result result;
    static const uint8_t program[] = { 0xd6u };
    C_INT failed = create_machine(&machine, STD_NULL, 0) ||
        core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
            TYPE_STATUS_OK || core_machine_run(machine, budget, &result) !=
            TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT;

    core_machine_destroy(machine);
    return failed;
}

static C_INT test_handled_resume(C_VOID)
{
    core_machine *machine = STD_NULL;
    transition_probe probe = { 0u, CORE_MACHINE_UNDEFINED_INSTRUCTION_HANDLED_RESUME };
    const core_machine_run_budget budget = { 16u, 0u };
    core_machine_run_result result;
    core_machine_cpu_state state;
    static const uint8_t program[] = { 0xd6u, 0xf4u };
    C_INT failed = create_machine(&machine, &probe, 1) ||
        core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
            TYPE_STATUS_OK || core_machine_run(machine, budget, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        core_machine_get_cpu_state(machine, &state) != TYPE_STATUS_OK ||
        probe.calls != 1u || state.eip != 2u || state.eflags & VCPU_EFLAGS_IF ||
        !(state.eflags & VCPU_EFLAGS_CF);

    core_machine_destroy(machine);
    return failed;
}

static C_INT test_fault(C_VOID)
{
    core_machine *machine = STD_NULL;
    transition_probe probe = { 0u, CORE_MACHINE_UNDEFINED_INSTRUCTION_FAULT };
    const core_machine_run_budget budget = { 16u, 0u };
    core_machine_run_result result;
    static const uint8_t program[] = { 0xd6u };
    C_INT failed = create_machine(&machine, &probe, 1) ||
        core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
            TYPE_STATUS_OK || core_machine_run(machine, budget, &result) !=
            TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
        result.detail != 0x244u || probe.calls != 1u;

    core_machine_destroy(machine);
    return failed;
}

static C_INT test_stop(C_VOID)
{
    core_machine *machine = STD_NULL;
    transition_probe probe = { 0u, CORE_MACHINE_UNDEFINED_INSTRUCTION_STOP };
    const core_machine_run_budget budget = { 16u, 0u };
    core_machine_run_result result;
    static const uint8_t program[] = { 0xd6u };
    C_INT failed = create_machine(&machine, &probe, 1) ||
        core_machine_memory_write(machine, 0u, program, sizeof(program)) !=
            TYPE_STATUS_OK || core_machine_run(machine, budget, &result) !=
            TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_REQUESTED ||
        probe.calls != 1u;

    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    if (test_unhandled() || test_handled_resume() || test_fault() || test_stop()) return 1;
    puts("M5:T244:S2:UNDEFINED-INSTRUCTION-TRANSITION:OK");
    return 0;
}

#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory_interface.h"
#include "core/machine/port_interface.h"
#include "../support/core_machine_cpu_fixture.h"

/* T337_REAL_UD_VECTOR6_DELIVERY: the corpus installs and observes vector 6. */

#define CORPUS_RESET_LINEAR 0xfffffff0u
#define CORPUS_RESET_PHYSICAL 0x000ffff0u
#define CORPUS_RESET_WINDOW 16u

typedef struct corpus_port_event {
    C_INT write;
    type_unsigned_16 port;
    type_unsigned_32 value;
} corpus_port_event;

typedef struct corpus_port_state {
    corpus_port_event events[4];
    STD_SIZE_T event_count;
} corpus_port_state;

static C_INT corpus_prepare_machine(core_machine **out_machine,
    const core_machine_port_provider *port_provider, C_VOID *port_owner)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || core_machine_create(&config, &machine) !=
            TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(machine,
            CORPUS_RESET_LINEAR, CORPUS_RESET_PHYSICAL,
            CORPUS_RESET_WINDOW) != TYPE_STATUS_OK ||
        (port_provider != STD_NULL && core_machine_install_port_provider(
            machine, 0x00e0u, 0x00e1u, port_provider, port_owner) !=
            TYPE_STATUS_OK) ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    *out_machine = machine;
    return 0;
}

static C_INT corpus_run_to_ud(core_machine *machine, const type_unsigned_8 *program,
    STD_SIZE_T program_bytes, core_machine_cpu_fault_snapshot *out_fault)
{
    static const type_unsigned_8 reset_jump[] = { 0xeau, 0x00u, 0x00u, 0x00u, 0x00u };
    static const type_unsigned_8 ud_vector[] = { 0x00u, 0x03u, 0x00u, 0x00u };
    static const type_unsigned_8 ud_handler[] = { 0xf4u };
    const core_machine_run_budget budget = { 128u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_status status;
    C_INT failed = 0;

    if (machine == STD_NULL || program == STD_NULL || out_fault == STD_NULL)
        return 1;
    failed |= core_machine_memory_write(machine, CORPUS_RESET_LINEAR, reset_jump,
        sizeof(reset_jump)) != TYPE_STATUS_OK || core_machine_memory_write(
        machine, 0u, program, program_bytes) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0x0018u, ud_vector,
        sizeof(ud_vector)) != TYPE_STATUS_OK || core_machine_memory_write(
        machine, 0x0300u, ud_handler, sizeof(ud_handler)) != TYPE_STATUS_OK;
    status = core_machine_run(machine, budget, &result);
    failed |= status != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET;
    failed |= core_machine_get_cpu_diagnostic(machine, &diagnostic) != TYPE_STATUS_OK ||
        diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
        !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
            VCPUINS_EXCEPT_UD) ||
        diagnostic.last_delivered_exception.point.bytes[0] != 0x66u;
    status = core_machine_run(machine, (core_machine_run_budget){ 1u, 0u },
        &result);
    failed |= status != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    *out_fault = diagnostic.last_delivered_exception;
    return failed;
}

static C_INT corpus_run_to_halt(core_machine *machine, const type_unsigned_8 *program,
    STD_SIZE_T program_bytes)
{
    static const type_unsigned_8 reset_jump[] = { 0xeau, 0x00u, 0x00u, 0x00u, 0x00u };
    const core_machine_run_budget budget = { 128u, 0u };
    core_machine_run_result result;

    return machine == STD_NULL || program == STD_NULL ||
        core_machine_memory_write(machine, CORPUS_RESET_LINEAR, reset_jump,
            sizeof(reset_jump)) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0u, program, program_bytes) !=
            TYPE_STATUS_OK ||
        core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT corpus_test_segment_override(C_VOID)
{
    static const type_unsigned_8 program[] = { 0x2eu, 0xa0u, 0x00u, 0x01u, 0x66u };
    static const type_unsigned_8 source = 0xa5u;
    core_machine_cpu_fault_snapshot fault;
    core_machine *machine = STD_NULL;
    C_INT failed = corpus_prepare_machine(&machine, STD_NULL, STD_NULL);

    if (!failed) {
        failed |= core_machine_memory_write(machine, 0x0100u, &source,
            sizeof(source)) != TYPE_STATUS_OK;
        failed |= corpus_run_to_ud(machine, program, sizeof(program), &fault);
        failed |= (fault.eax & 0xffu) != source;
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT corpus_test_rep_direction(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0xb8u, 0x00u, 0x00u,
        0x8eu, 0xd8u,
        0x8eu, 0xc0u,
        0xbeu, 0x00u, 0x01u,
        0xbfu, 0x00u, 0x02u,
        0xb9u, 0x03u, 0x00u,
        0xfcu,
        0xf3u, 0xa4u,
        0xfdu,
        0xb0u, 0x7eu,
        0xaau,
        0xb0u, 0x7du,
        0xaau,
        0xf4u
    };
    static const type_unsigned_8 source[] = { 0x11u, 0x22u, 0x33u };
    type_unsigned_8 source_before[3] = { 0u, 0u, 0u };
    type_unsigned_8 source_after[3] = { 0u, 0u, 0u };
    type_unsigned_8 copied[3] = { 0u, 0u, 0u };
    type_unsigned_8 stored = 0u;
    core_machine *machine = STD_NULL;
    C_INT failed = corpus_prepare_machine(&machine, STD_NULL, STD_NULL);

    if (!failed) {
        failed |= core_machine_memory_write(machine, 0x0100u, source,
            sizeof(source)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_read(machine, 0x0100u, source_before,
            sizeof(source_before)) != TYPE_STATUS_OK;
        failed |= source_before[0] != source[0] || source_before[1] != source[1] ||
            source_before[2] != source[2];
        failed |= corpus_run_to_halt(machine, program, sizeof(program));
        failed |= core_machine_memory_read(machine, 0x0200u, copied,
            sizeof(copied)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_read(machine, 0x0100u, source_after,
            sizeof(source_after)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_read(machine, 0x0203u, &stored,
            sizeof(stored)) != TYPE_STATUS_OK;
        failed |= copied[0] != source[0] || copied[1] != source[1] ||
            copied[2] != 0x7du || stored != 0x7eu ||
            source_after[0] != source[0] || source_after[1] != source[1] ||
            source_after[2] != source[2];
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "M5:T240:S2:8086-CORPUS:REP copied=%02x%02x%02x stored=%02x\n",
                copied[0], copied[1], copied[2], stored);
            STD_FPRINTF(STD_STDERR,
                "M5:T240:S2:8086-CORPUS:REP source=%02x%02x%02x\n",
                source_after[0], source_after[1], source_after[2]);
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT corpus_test_int_iret(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0xbcu, 0x00u, 0x10u,
        0xf9u,
        0xcdu, 0x60u,
        0x66u
    };
    static const type_unsigned_8 ivt_entry[] = { 0x00u, 0x02u, 0x00u, 0x00u };
    static const type_unsigned_8 handler[] = { 0xb8u, 0x34u, 0x12u, 0xcfu };
    core_machine_cpu_fault_snapshot fault;
    core_machine *machine = STD_NULL;
    C_INT failed = corpus_prepare_machine(&machine, STD_NULL, STD_NULL);

    if (!failed) {
        failed |= core_machine_memory_write(machine, 0x0180u, ivt_entry,
            sizeof(ivt_entry)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(machine, 0x0200u, handler,
            sizeof(handler)) != TYPE_STATUS_OK;
        failed |= corpus_run_to_ud(machine, program, sizeof(program), &fault);
        failed |= (fault.eax & 0xffffu) != 0x1234u ||
            (fault.esp & 0xffffu) != 0x1000u ||
            !TYPE_GET_BIT(fault.eflags, VCPU_EFLAGS_CF);
    }
    core_machine_destroy(machine);
    return failed;
}

static type_status corpus_port_read(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    corpus_port_state *state = (corpus_port_state *)owner;

    if (state == STD_NULL || out_value == STD_NULL || state->event_count >=
            sizeof(state->events) / sizeof(state->events[0])) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    state->events[state->event_count].write = 0;
    state->events[state->event_count].port = port;
    state->events[state->event_count].value = port == 0x00e0u ? 0x11u : 0x22u;
    *out_value = state->events[state->event_count].value;
    ++state->event_count;
    return TYPE_STATUS_OK;
}

static type_status corpus_port_write(C_VOID *owner, type_unsigned_16 port,
    type_unsigned_32 value)
{
    corpus_port_state *state = (corpus_port_state *)owner;

    if (state == STD_NULL || state->event_count >=
            sizeof(state->events) / sizeof(state->events[0])) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    state->events[state->event_count].write = 1;
    state->events[state->event_count].port = port;
    state->events[state->event_count].value = value;
    ++state->event_count;
    return TYPE_STATUS_OK;
}

static C_INT corpus_test_port_transactions(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0xb0u, 0x5au,
        0xe6u, 0xe0u,
        0xbau, 0xe1u, 0x00u,
        0xeeu,
        0xe4u, 0xe0u,
        0xecu,
        0x66u
    };
    static const core_machine_port_provider provider = {
        corpus_port_read,
        corpus_port_write
    };
    corpus_port_state port_state = { { { 0, 0u, 0u } }, 0u };
    core_machine_cpu_fault_snapshot fault;
    core_machine *machine = STD_NULL;
    C_INT failed = corpus_prepare_machine(&machine, &provider, &port_state);

    if (!failed) {
        failed |= corpus_run_to_ud(machine, program, sizeof(program), &fault);
        failed |= port_state.event_count !=
            sizeof(port_state.events) / sizeof(port_state.events[0]);
        failed |= !port_state.events[0].write || port_state.events[0].port != 0x00e0u ||
            port_state.events[0].value != 0x5au;
        failed |= !port_state.events[1].write || port_state.events[1].port != 0x00e1u ||
            port_state.events[1].value != 0x5au;
        failed |= port_state.events[2].write || port_state.events[2].port != 0x00e0u ||
            port_state.events[2].value != 0x11u;
        failed |= port_state.events[3].write || port_state.events[3].port != 0x00e1u ||
            port_state.events[3].value != 0x22u;
        failed |= (fault.eax & 0xffu) != 0x22u;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    C_INT failed = 0;

    if (corpus_test_segment_override()) {
        STD_FPRINTF(STD_STDERR, "M5:T240:S2:8086-CORPUS:FAIL case=segment-override\n");
        failed = 1;
    }
    if (corpus_test_rep_direction()) {
        STD_FPRINTF(STD_STDERR, "M5:T240:S2:8086-CORPUS:FAIL case=rep-direction\n");
        failed = 1;
    }
    if (corpus_test_int_iret()) {
        STD_FPRINTF(STD_STDERR, "M5:T240:S2:8086-CORPUS:FAIL case=int-iret\n");
        failed = 1;
    }
    if (corpus_test_port_transactions()) {
        STD_FPRINTF(STD_STDERR, "M5:T240:S2:8086-CORPUS:FAIL case=port-transactions\n");
        failed = 1;
    }
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T240:S2:8086-CORPUS:FAIL\n");
        return 1;
    }
    STD_PRINTF("M5:T240:S2:8086-CORPUS:OK\n");
    return 0;
}

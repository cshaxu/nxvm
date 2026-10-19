#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/machine.h"

typedef struct retirement_probe {
    x86_core_machine *machine;
    x86_core_machine_retirement_observation records[3];
    lib_u32 count;
    lib_status set_while_running;
    lib_status snapshot_while_running[2];
} retirement_probe;

static void retirement_capture(void *opaque,
    const x86_core_machine_retirement_observation *observation)
{
    retirement_probe *probe = (retirement_probe *)opaque;
    x86_core_cpu_debug_cpu_snapshot snapshot;

    if (probe == LIB_NULL || observation == LIB_NULL) return;
    if (probe->count < 3u) probe->records[probe->count] = *observation;
    ++probe->count;
    probe->set_while_running = x86_core_machine_set_retirement_observation_provider(
        probe->machine, LIB_NULL);
    probe->snapshot_while_running[0] = x86_core_machine_debug_capture_cpu_snapshot(
        probe->machine, X86_CORE_CPU_SNAPSHOT_CURRENT, &snapshot);
    probe->snapshot_while_running[1] = x86_core_machine_debug_capture_cpu_snapshot(
        probe->machine, X86_CORE_CPU_SNAPSHOT_INSTRUCTION_ENTRY, &snapshot);
}

static lib_i32 retirement_prepare(x86_core_machine **out_machine,
    const x86_core_machine_executor_config *config, const lib_u8 *code,
    lib_size bytes)
{
    x86_core_cpu_profile profile;
    x86_core_machine_memory_alias_config aliases[] = {
        {0xfffffff0u, 0x000ffff0u, 16u},
        {0x000ffff0u, 0x000ffff0u, 16u}
    };

    if (out_machine == LIB_NULL ||
        x86_core_machine_neutral_create(config, out_machine) != LIB_STATUS_OK ||
        x86_core_machine_get_cpu_profile(*out_machine, &profile) != LIB_STATUS_OK) return 0;
    aliases[1].physical_start = profile <= X86_CORE_CPU_PROFILE_80186 ?
        0x000ffff0u : 0x00fffff0u;
    return x86_core_machine_install_memory_aliases(*out_machine, aliases, 1u,
            LIB_FALSE) == LIB_STATUS_OK &&
        (profile > X86_CORE_CPU_PROFILE_80286 ||
            x86_core_machine_install_memory_aliases(*out_machine, &aliases[1], 1u,
                LIB_FALSE) == LIB_STATUS_OK) &&
        x86_core_machine_freeze_execution_providers(*out_machine) == LIB_STATUS_OK &&
        x86_core_machine_reset(*out_machine) == LIB_STATUS_OK &&
        x86_core_machine_set_address_clear_mask(*out_machine, 0u) == LIB_STATUS_OK &&
        x86_core_machine_memory_write(*out_machine, 0xfffffff0u, code, bytes) ==
            LIB_STATUS_OK;
}

static lib_i32 retirement_async_context_case(lib_bool branch)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u, .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x100u, [X86_CORE_CPU_DEBUG_ESP] = 0x8000u}
    };
    const lib_u8 code[] = {branch ? 0xebu : 0x90u, 0x7eu};
    const lib_u8 target[] = {0x81u, 0xc0u, 1u, 0u};
    const lib_u8 handler = 0x90u;
    const lib_u16 vector[] = {0x300u,0u};
    x86_core_machine *machine = LIB_NULL;
    retirement_probe probe = {0};
    const x86_core_machine_retirement_observation_provider provider = {
        .context = &probe, .callback = retirement_capture
    };
    x86_core_machine_run_result result;
    lib_i32 failed = !retirement_prepare(&machine, &config, code, sizeof(code));

    probe.machine = machine;
    if (!failed)
        failed = x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x100u, code, sizeof(code)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x180u, target, sizeof(target)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x300u, &handler, 1u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 8u, (const lib_u8 *)vector, sizeof(vector)) != LIB_STATUS_OK ||
            x86_core_machine_set_retirement_observation_provider(machine, &provider) != LIB_STATUS_OK ||
            !x86_core_cpu_request_nmi(machine->executor_cpu_execution) ||
            x86_core_machine_run(machine, (x86_core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_OK ||
            result.executed != 1u || result.linear_pc != 0x300u || probe.count != 1u ||
            probe.records[0].point.eip != 0x100u ||
            probe.records[0].eligibility_key.opcode != code[0] ||
            probe.records[0].next_lexeme_components != (branch ? 3u :
                X86_CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE) ||
            probe.records[0].instruction_entry_cpu.eip != 0x100u ||
            probe.records[0].current_cpu.eip != 0x300u;
    if (failed && probe.count != 0u)
        lib_c_printf("Async retirement identity: point=%x entry=%x current=%x opcode=%x next=%u\n",
            probe.records[0].point.eip, probe.records[0].instruction_entry_cpu.eip,
            probe.records[0].current_cpu.eip, probe.records[0].eligibility_key.opcode,
            (lib_u32)probe.records[0].next_lexeme_components);
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 retirement_control_context_cases(void)
{
    static const struct {
        lib_u8 code[9], bytes;
        lib_u32 count;
        x86_core_machine_retirement_control_outcome outcome;
        lib_u8 components;
    } cases[] = {
        {{0x75u,1u,0x90u,0x90u},4u,0u,X86_CORE_MACHINE_RETIREMENT_CONTROL_TAKEN,1u},
        {{0x74u,1u,0x90u,0x90u},4u,0u,X86_CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,0xffu},
        {{0x75u,0u,0x90u},3u,0u,X86_CORE_MACHINE_RETIREMENT_CONTROL_TAKEN,1u},
        {{0x74u,0u,0x90u},3u,0u,X86_CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,0xffu},
        {{0x0fu,0x85u,0u,0u,0x90u},5u,0u,X86_CORE_MACHINE_RETIREMENT_CONTROL_TAKEN,1u},
        {{0x0fu,0x84u,0u,0u,0x90u},5u,0u,X86_CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,0xffu},
        {{0x66u,0x0fu,0x85u,0u,0u,0u,0u,0x90u},8u,0u,X86_CORE_MACHINE_RETIREMENT_CONTROL_TAKEN,1u},
        {{0xffu,0xc0u,0x90u},3u,0u,X86_CORE_MACHINE_RETIREMENT_CONTROL_NONE,0xffu},
        {{0xffu,0xc8u,0x90u},3u,0u,X86_CORE_MACHINE_RETIREMENT_CONTROL_NONE,0xffu},
        {{0x67u,0xe2u,0u,0x90u},4u,0x10001u,X86_CORE_MACHINE_RETIREMENT_CONTROL_TAKEN,1u},
        {{0x67u,0xe3u,0u,0x90u},4u,0x10000u,X86_CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,0xffu},
        {{0xe2u,0u,0x90u},3u,1u,X86_CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,0xffu},
        {{0x67u,0xe3u,0u,0x90u},4u,0u,X86_CORE_MACHINE_RETIREMENT_CONTROL_TAKEN,1u}
    };
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const x86_core_machine_run_budget budget = { 1u, 0u };
    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        x86_core_machine_retirement_observation_provider provider;
        x86_core_machine_run_result result;
        retirement_probe probe = { LIB_NULL, { { 0 } }, 0u, LIB_STATUS_OK,
            { LIB_STATUS_OK, LIB_STATUS_OK } };
        x86_core_machine *machine = LIB_NULL;
        lib_i32 failed = !retirement_prepare(&machine, &config, cases[index].code, cases[index].bytes);

        provider.callback = retirement_capture;
        provider.context = &probe;
        probe.machine = machine;
        if (!failed) {
            failed |= x86_core_machine_debug_write_register(machine, X86_CORE_CPU_DEBUG_ECX,
                    cases[index].count) != LIB_STATUS_OK ||
                x86_core_machine_set_retirement_observation_provider(machine,
                    &provider) != LIB_STATUS_OK ||
                x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
                result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
                probe.count != 1u ||
                probe.records[0].control_outcome != cases[index].outcome ||
                probe.records[0].next_lexeme_components != cases[index].components;
        }
        x86_core_machine_destroy(machine);
        if (failed) {
            lib_c_printf("Retirement branch context: case=%zu outcome=%u components=%u\n",
                index, (lib_u32)probe.records[0].control_outcome,
                (lib_u32)probe.records[0].next_lexeme_components);
            return failed;
        }
    }
    return 0;
}
static lib_i32 retirement_pre_mode_snapshot_case(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const x86_core_machine_run_budget budget = { 1u, 0u };
    const lib_u8 program[] = { 0x0fu, 0x01u, 0xf0u };
    x86_core_machine_retirement_observation_provider provider;
    x86_core_machine_run_result result;
    x86_core_cpu_debug_cpu_snapshot current_cpu;
    x86_core_cpu_debug_cpu_snapshot entry_cpu;
    retirement_probe probe = { LIB_NULL, { { 0 } }, 0u, LIB_STATUS_OK,
        { LIB_STATUS_OK, LIB_STATUS_OK } };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = !retirement_prepare(&machine, &config, program, sizeof(program));

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        failed |= x86_core_machine_debug_write_register(machine,
            X86_CORE_CPU_DEBUG_EAX, 1u) != LIB_STATUS_OK ||
            x86_core_machine_set_retirement_observation_provider(machine,
            &provider) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            probe.count != 1u || probe.records[0].protected_mode ||
            probe.records[0].eligibility_key.protected_mode ||
            (probe.records[0].instruction_entry_cpu.cr0 & VCPU_CR0_PE) != 0u ||
            (probe.records[0].current_cpu.cr0 & VCPU_CR0_PE) == 0u ||
            probe.records[0].instruction_entry_cpu.eax != 1u ||
            probe.records[0].current_cpu.eax != 1u ||
            probe.records[0].instruction_entry_cpu.eip != 0xfff0u ||
            probe.records[0].current_cpu.eip != 0xfff3u ||
            probe.snapshot_while_running[0] != LIB_STATUS_INVALID_STATE ||
            probe.snapshot_while_running[1] != LIB_STATUS_INVALID_STATE ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &current_cpu) != LIB_STATUS_OK ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_INSTRUCTION_ENTRY, &entry_cpu) != LIB_STATUS_OK ||
            current_cpu.cr0 != probe.records[0].current_cpu.cr0 ||
            current_cpu.eip != probe.records[0].current_cpu.eip ||
            entry_cpu.cr0 != probe.records[0].instruction_entry_cpu.cr0 ||
            entry_cpu.eip != probe.records[0].instruction_entry_cpu.eip ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                (x86_core_cpu_snapshot_point)2, &current_cpu) !=
                LIB_STATUS_INVALID_ARGUMENT ||
            current_cpu.eip != 0xfff3u;
        /* Published copies survive later debug writes and CPU reset. Reset
         * must not publish an old pending instruction as a new retirement. */
        failed |= x86_core_machine_debug_write_register(machine,
            X86_CORE_CPU_DEBUG_EAX, 0x12345678u) != LIB_STATUS_OK ||
            x86_core_machine_reset(machine) != LIB_STATUS_OK || probe.count != 1u ||
            probe.records[0].instruction_entry_cpu.eax != 1u ||
            probe.records[0].current_cpu.eax != 1u ||
            probe.records[0].instruction_entry_cpu.eip != 0xfff0u ||
            probe.records[0].current_cpu.eip != 0xfff3u ||
            (probe.records[0].instruction_entry_cpu.cr0 & VCPU_CR0_PE) != 0u ||
            (probe.records[0].current_cpu.cr0 & VCPU_CR0_PE) == 0u ||
            x86_core_machine_debug_capture_cpu_snapshot(machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &current_cpu) != LIB_STATUS_OK ||
            current_cpu.eip != 0xfff0u || (current_cpu.cr0 & VCPU_CR0_PE) != 0u;
    }
    x86_core_machine_destroy(machine);
    return failed;
}
static lib_i32 retirement_unallocated_profile_case(x86_core_cpu_profile profile,
    x86_core_cpu_retirement_timing_origin expected_origin,
    const lib_u8 *program, lib_size bytes, lib_i32 expected_repeat)
{
    const x86_core_machine_executor_config physical = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = profile,
        .time_axis = { X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u },
        .retirement_time_contract = X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL
    };
    const x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_retirement_observation_provider provider;
    x86_core_machine_run_result result;
    x86_core_machine_timeline_observation timeline;
    retirement_probe probe = { LIB_NULL, { { 0 } }, 0u, LIB_STATUS_OK,
        { LIB_STATUS_OK, LIB_STATUS_OK } };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = !retirement_prepare(&machine, &physical, program, bytes);

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        failed |= x86_core_machine_set_retirement_observation_provider(machine,
            &provider) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != X86_CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
            probe.count != 1u || probe.set_while_running != LIB_STATUS_INVALID_STATE ||
            probe.records[0].timing_disposition !=
                X86_CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ||
            probe.records[0].source_timing_form_id !=
                X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
            probe.records[0].timing_key_id !=
                X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
            ((expected_repeat != 0) != ((probe.records[0].formula_inputs &
                X86_CORE_CPU_TIMING_INPUT_REPEAT) != 0u)) ||
            probe.records[0].timing_origin != expected_origin ||
            probe.records[0].elapsed_ticks != 0u || probe.records[0].timeline_ticks != 0u ||
            x86_core_machine_get_timeline_observation(machine, &timeline) != LIB_STATUS_OK ||
            timeline.now != 0u;
    }
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 retirement_8086_context_formula_case(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_8086,
        .fpu_profile = X86_FPU_PROFILE_8087
    };
    const x86_core_machine_run_budget budget = { 1u, 0u };
    const lib_u8 segment_movsb[] = { 0x26u, 0xa4u };
    const lib_u8 lock_add[] = { 0xf0u, 0x01u, 0x06u, 0x00u, 0x10u };
    const lib_u8 wait[] = { 0x9bu };
    const lib_u16 value = 0u;
    x86_core_machine_retirement_observation_provider provider;
    x86_core_machine_run_result result;
    retirement_probe probe = { LIB_NULL, { { 0 } }, 0u, LIB_STATUS_OK,
        { LIB_STATUS_OK, LIB_STATUS_OK } };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = !retirement_prepare(&machine, &config, segment_movsb,
        sizeof(segment_movsb));

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        failed |= x86_core_machine_set_retirement_observation_provider(machine,
            &provider) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.executed != 1u || probe.count != 1u ||
            probe.records[0].source_ticks != 20u ||
            probe.records[0].timing_disposition !=
                X86_CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[0].timing_origin !=
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO ||
            (probe.records[0].formula_inputs &
                X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE) == 0u;
    }
    if (!failed) {
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_set_address_clear_mask(machine, 0u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0xfffffff0u, lock_add,
                sizeof(lock_add)) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x1000u, &value,
                sizeof(value)) != LIB_STATUS_OK ||
            x86_core_machine_debug_write_register(machine,
                X86_CORE_CPU_DEBUG_EAX, 1u) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.executed != 1u || probe.count != 2u ||
            probe.records[1].source_ticks != 24u ||
            probe.records[1].timing_disposition !=
                X86_CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[1].timing_origin !=
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY ||
            probe.records[1].modrm_form != X86_CORE_MACHINE_RETIREMENT_MODRM_MEMORY ||
            probe.records[1].modrm_extension != 0u ||
            (probe.records[1].formula_inputs &
                (X86_CORE_CPU_TIMING_INPUT_MODRM |
                X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
                X86_CORE_CPU_TIMING_INPUT_LOCK)) !=
                (X86_CORE_CPU_TIMING_INPUT_MODRM |
                X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS |
                X86_CORE_CPU_TIMING_INPUT_LOCK);
    }
    if (!failed) {
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK ||
            x86_core_machine_set_address_clear_mask(machine, 0u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0xfffffff0u, wait,
                sizeof(wait)) != LIB_STATUS_OK ||
            (x86_fpu_begin_command(machine->fpu, 0xdbu, 0xe3u), 0) ||
            (x86_fpu_advance(machine->fpu, 2u), 0) ||
            x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.executed != 1u || probe.count != 3u ||
            probe.records[2].source_ticks != 3u ||
            probe.records[2].timing_disposition !=
                X86_CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[2].timing_origin !=
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY;
    }
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 retirement_8088_primary_case(const lib_u8 *program,
    lib_size bytes, lib_u64 expected_ticks,
    x86_core_cpu_retirement_timing_origin expected_origin)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_8088
    };
    const x86_core_machine_run_budget budget = { 1u, 0u };
    const lib_u16 value = 1u;
    x86_core_machine_retirement_observation_provider provider;
    x86_core_machine_run_result result;
    retirement_probe probe = { LIB_NULL, { { 0 } }, 0u, LIB_STATUS_OK,
        { LIB_STATUS_OK, LIB_STATUS_OK } };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = !retirement_prepare(&machine, &config, program, bytes);

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        failed |= x86_core_machine_memory_write(machine, 0x1000u, &value,
                sizeof(value)) != LIB_STATUS_OK ||
            x86_core_machine_set_retirement_observation_provider(machine,
                &provider) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.executed != 1u || probe.count != 1u ||
            probe.records[0].source_ticks != expected_ticks ||
            probe.records[0].timing_disposition !=
                X86_CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[0].timing_origin !=
                expected_origin ||
            probe.records[0].source_timing_form_id ==
                X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    }
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 retirement_8088_branch_case(const lib_u8 *program,
    lib_size bytes, lib_u16 count, lib_u32 flags,
    lib_u64 expected_ticks)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_8088
    };
    const x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_retirement_observation_provider provider;
    x86_core_machine_run_result result;
    retirement_probe probe = { LIB_NULL, { { 0 } }, 0u, LIB_STATUS_OK,
        { LIB_STATUS_OK, LIB_STATUS_OK } };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = !retirement_prepare(&machine, &config, program, bytes);

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        failed |= x86_core_machine_debug_write_register(machine,
                X86_CORE_CPU_DEBUG_ECX, count) != LIB_STATUS_OK ||
            x86_core_machine_debug_write_register(machine,
                X86_CORE_CPU_DEBUG_EFLAGS, flags) != LIB_STATUS_OK ||
            x86_core_machine_set_retirement_observation_provider(machine,
                &provider) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.executed != 1u || probe.count != 1u ||
            probe.records[0].source_ticks != expected_ticks ||
            probe.records[0].timing_disposition !=
                X86_CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[0].timing_origin !=
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            probe.records[0].source_timing_form_id ==
                X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    }
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 retirement_8088_iret_case(const lib_u8 *program,
    lib_size bytes)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_8088
    };
    const x86_core_machine_run_budget budget = { 1u, 0u };
    const lib_u16 frame[] = { 0u, 0u, 2u };
    x86_core_machine_retirement_observation_provider provider;
    x86_core_machine_run_result result;
    retirement_probe probe = { LIB_NULL, { { 0 } }, 0u, LIB_STATUS_OK,
        { LIB_STATUS_OK, LIB_STATUS_OK } };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = !retirement_prepare(&machine, &config, program, bytes);

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        failed |= x86_core_machine_debug_write_register(machine,
                X86_CORE_CPU_DEBUG_ESP, 0x1000u) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x1000u, frame,
                sizeof(frame)) != LIB_STATUS_OK ||
            x86_core_machine_set_retirement_observation_provider(machine,
                &provider) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.executed != 1u || probe.count != 1u ||
            probe.records[0].source_ticks != 36u ||
            probe.records[0].timing_disposition !=
                X86_CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[0].timing_origin !=
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK ||
            probe.records[0].source_timing_form_id ==
                X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    }
    x86_core_machine_destroy(machine);
    return failed;
}

static lib_i32 retirement_8088_jcc_forms_case(void)
{
    static const lib_u32 flags[16][2] = {
        { X86_CORE_CPU_DEBUG_EFLAGS_OF, 0u }, { 0u, X86_CORE_CPU_DEBUG_EFLAGS_OF },
        { X86_CORE_CPU_DEBUG_EFLAGS_CF, 0u }, { 0u, X86_CORE_CPU_DEBUG_EFLAGS_CF },
        { X86_CORE_CPU_DEBUG_EFLAGS_ZF, 0u }, { 0u, X86_CORE_CPU_DEBUG_EFLAGS_ZF },
        { X86_CORE_CPU_DEBUG_EFLAGS_CF, 0u }, { 0u, X86_CORE_CPU_DEBUG_EFLAGS_CF },
        { X86_CORE_CPU_DEBUG_EFLAGS_SF, 0u }, { 0u, X86_CORE_CPU_DEBUG_EFLAGS_SF },
        { X86_CORE_CPU_DEBUG_EFLAGS_PF, 0u }, { 0u, X86_CORE_CPU_DEBUG_EFLAGS_PF },
        { X86_CORE_CPU_DEBUG_EFLAGS_SF, 0u }, { 0u, X86_CORE_CPU_DEBUG_EFLAGS_SF },
        { X86_CORE_CPU_DEBUG_EFLAGS_ZF, 0u }, { 0u, X86_CORE_CPU_DEBUG_EFLAGS_ZF }
    };
    lib_u8 opcode;
    lib_i32 failed = 0;

    for (opcode = 0x70u; opcode <= 0x7fu; ++opcode) {
        const lib_u8 program[] = { opcode, 0x01u };
        lib_u8 index = opcode - 0x70u;

        failed |= retirement_8088_branch_case(program, sizeof(program), 0u,
            flags[index][0], 16u) || retirement_8088_branch_case(program,
            sizeof(program), 0u, flags[index][1], 4u);
    }
    return failed;
}

static lib_i32 retirement_8088_string_case(const lib_u8 *program,
    lib_size bytes, lib_u16 count, lib_u32 executions,
    lib_u64 first_ticks, lib_u64 next_ticks)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_8088
    };
    const x86_core_machine_run_budget budget = { executions, 0u };
    x86_core_machine_retirement_observation_provider provider;
    x86_core_machine_run_result result;
    retirement_probe probe = { LIB_NULL, { { 0 } }, 0u, LIB_STATUS_OK,
        { LIB_STATUS_OK, LIB_STATUS_OK } };
    x86_core_machine *machine = LIB_NULL;
    lib_i32 failed = !retirement_prepare(&machine, &config, program, bytes);

    provider.callback = retirement_capture;
    provider.context = &probe;
    probe.machine = machine;
    if (!failed) {
        failed |= x86_core_machine_debug_write_register(machine,
                X86_CORE_CPU_DEBUG_ECX, count) != LIB_STATUS_OK ||
            x86_core_machine_set_retirement_observation_provider(machine,
                &provider) != LIB_STATUS_OK ||
            x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.executed != executions || probe.count != executions ||
            probe.records[0].source_ticks != first_ticks ||
            probe.records[0].timing_disposition !=
                X86_CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
            probe.records[0].timing_origin !=
                X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO;
        if (executions > 1u) {
            failed |= probe.records[1].source_ticks != next_ticks ||
                probe.records[0].repeat_phase !=
                    X86_CORE_CPU_RETIREMENT_REPEAT_FIRST ||
                probe.records[1].repeat_phase !=
                    X86_CORE_CPU_RETIREMENT_REPEAT_CONTINUATION;
        }
    }
    x86_core_machine_destroy(machine);
    return failed;
}

/* Receiver-format proof only: V30 runtime remains gated by its unfinished
 * time contract. Borrow an Intel executor for the existing timing accessor;
 * never execute it as V30 or modify its configured profile. */
static lib_i32 retirement_nec_format_case(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    static const struct {
        lib_u8 bytes[4];
        lib_u8 modrm;
        lib_u8 emulation_io;
    } cases[] = {
        {{0x66u,0x06u,0u,0u}, X86_CORE_MACHINE_RETIREMENT_MODRM_MEMORY, 0u},
        {{0x67u,0xc0u,0u,0u}, X86_CORE_MACHINE_RETIREMENT_MODRM_REGISTER, 0u},
        {{0x26u,0x66u,0xc0u,0u}, X86_CORE_MACHINE_RETIREMENT_MODRM_REGISTER, 0u},
        {{0x64u,0xd8u,0xc0u,0u}, X86_CORE_MACHINE_RETIREMENT_MODRM_REGISTER, 0u},
        {{0x0fu,0x10u,0xc0u,0u}, X86_CORE_MACHINE_RETIREMENT_MODRM_REGISTER, 0u},
        {{0x0fu,0x20u,0xc0u,0u}, X86_CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE, 0u},
        {{0xd3u,0x27u,0u,0u}, X86_CORE_MACHINE_RETIREMENT_MODRM_MEMORY,
            X86_CORE_MACHINE_RETIREMENT_IO_WRITE},
        {{0xdbu,0x27u,0u,0u}, X86_CORE_MACHINE_RETIREMENT_MODRM_MEMORY,
            X86_CORE_MACHINE_RETIREMENT_IO_READ},
        {{0xecu,0u,0u,0u}, X86_CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE, 0u}
    };
    x86_core_machine *owner = LIB_NULL;
    static x86_core_machine receiver;
    lib_i32 failed = 0;
    if (x86_core_machine_neutral_create(&config, &owner) != LIB_STATUS_OK) return 1;
    lib_memory_set(&receiver, 0, sizeof(receiver));
    receiver.cpu_profile = X86_CORE_CPU_PROFILE_V30;
    receiver.executor_cpu_execution = owner->executor_cpu_execution;
    x86_core_cpu_instruction_observation intel = {0};
    x86_core_cpu_execution_copy_observation(owner->executor_cpu_execution, &intel);
    failed |= intel.nec_emulation_mode;
    for (lib_size item = 0u; item < sizeof(cases)/sizeof(cases[0]); ++item)
    for (lib_u8 emulation = 0u; emulation < 2u; ++emulation) {
        x86_core_cpu_instruction_observation data = {0};
        data.nec_emulation_mode = emulation != 0u;
        data.point.byte_count = sizeof(cases[item].bytes);
        lib_memory_copy(data.point.bytes, cases[item].bytes, sizeof(cases[item].bytes));
        data.eax = 0x1234u;
        data.dx = 0x40u;
        x86_core_machine_retirement_observation_capture_instruction(&receiver, &data);
        const x86_core_machine_retirement_observation *result =
            &receiver.retirement_observation.pending_observation;
        failed |= result->modrm_form != (emulation ?
            X86_CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE : cases[item].modrm);
        if (emulation) {
            failed |= result->io_direction != cases[item].emulation_io;
            if (cases[item].emulation_io != 0u) {
                failed |= result->io_port != 0x27u || result->io_bytes != 1u;
                if (result->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_WRITE)
                    failed |= result->io_value != 0x34u;
            }
        } else {
            failed |= result->io_direction != (cases[item].bytes[0] == 0xecu ?
                X86_CORE_MACHINE_RETIREMENT_IO_READ : X86_CORE_MACHINE_RETIREMENT_IO_NONE);
        }
    }
    x86_core_machine_destroy(owner);
    return failed;
}

lib_i32 main(void)
{
    if (retirement_nec_format_case()) return 1;
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_executor_config deterministic = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    x86_core_machine_executor_config physical = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .time_axis = { X86_CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u },
        .retirement_time_contract = X86_CORE_MACHINE_RETIREMENT_TIME_PHYSICAL
    };
    x86_core_machine_retirement_observation_provider provider;
    x86_core_machine_run_budget budget = { 1u, 0u };
    x86_core_machine_run_result result;
    x86_core_machine_timeline_observation timeline;
    retirement_probe probe = { LIB_NULL, { { 0 } }, 0u, LIB_STATUS_OK,
        { LIB_STATUS_OK, LIB_STATUS_OK } };
    lib_size index;
    lib_u8 nop = 0x90u;
    static const lib_u8 scalar_opcodes[] = {
        0x90u, 0xf8u, 0xfcu, 0xfau, 0xf5u,
        0xf9u, 0xfdu, 0xfbu, 0x9eu, 0x9fu
    };
    static const lib_u64 scalar_ticks[] = {
        3u, 2u, 2u, 2u, 2u, 2u, 2u, 2u, 4u, 4u
    };
    lib_u8 rep_nop[] = { 0xf3u, 0x90u };
    const lib_u8 add_register[] = { 0x01u, 0xc8u };
    const lib_u8 add_register_memory[] = { 0x03u, 0x06u, 0x00u, 0x10u };
    const lib_u8 add_memory_register[] = { 0x01u, 0x06u, 0x00u, 0x10u };
    const lib_u8 mov_sreg_memory[] = { 0x8cu, 0x06u, 0x00u, 0x10u };
    const lib_u8 mov_sreg_register[] = { 0x8cu, 0xd8u };
    const lib_u8 mov_ds_memory[] = { 0x8eu, 0x1eu, 0x00u, 0x10u };
    const lib_u8 lds_memory[] = { 0xc5u, 0x06u, 0x00u, 0x10u };
    const lib_u8 xlat[] = { 0xd7u };
    const lib_u8 esc_register[] = { 0xd8u, 0xc0u };
    const lib_u8 shift_memory[] = { 0xd1u, 0x26u, 0x00u, 0x10u };
    const lib_u8 push_ds[] = { 0x1eu };
    const lib_u8 pop_ds[] = { 0x1fu };
    const lib_u8 pop_cs[] = { 0x0fu };
    const lib_u8 lock_add_memory[] = { 0xf0u, 0x01u, 0x06u, 0x00u, 0x10u };
    const lib_u8 mul_al[] = { 0xf6u, 0xe0u };
    const lib_u8 wait[] = { 0x9bu };
    const lib_u8 push_register[] = { 0x50u };
    const lib_u8 pop_register[] = { 0x58u };
    const lib_u8 call_near[] = { 0xe8u, 0x00u, 0x00u };
    const lib_u8 ret_near[] = { 0xc3u };
    const lib_u8 ret_far[] = { 0xcbu };
    const lib_u8 call_memory[] = { 0xffu, 0x16u, 0x00u, 0x10u };
    const lib_u8 push_memory[] = { 0xffu, 0x36u, 0x00u, 0x10u };
    const lib_u8 pop_memory[] = { 0x8fu, 0x06u, 0x00u, 0x10u };
    const lib_u8 in_immediate_byte[] = { 0xe4u, 0x00u };
    const lib_u8 in_immediate_word[] = { 0xe5u, 0x00u };
    const lib_u8 in_dx_byte[] = { 0xecu };
    const lib_u8 in_dx_word[] = { 0xedu };
    const lib_u8 out_immediate_byte[] = { 0xe6u, 0x00u };
    const lib_u8 out_immediate_word[] = { 0xe7u, 0x00u };
    const lib_u8 out_dx_byte[] = { 0xeeu };
    const lib_u8 out_dx_word[] = { 0xefu };
    const lib_u8 jcxz[] = { 0xe3u, 0x01u };
    const lib_u8 loop[] = { 0xe2u, 0x01u };
    const lib_u8 loope[] = { 0xe1u, 0x01u };
    const lib_u8 loopne[] = { 0xe0u, 0x01u };
    const lib_u8 int3[] = { 0xccu };
    const lib_u8 int_type3[] = { 0xcdu, 0x03u };
    const lib_u8 int_other[] = { 0xcdu, 0x67u };
    const lib_u8 into[] = { 0xceu };
    const lib_u8 iret[] = { 0xcfu };
    const lib_u8 hlt[] = { 0xf4u };
    const lib_u8 movsb[] = { 0xa4u };
    const lib_u8 movsw[] = { 0xa5u };
    const lib_u8 cmpsb[] = { 0xa6u };
    const lib_u8 cmpsw[] = { 0xa7u };
    const lib_u8 stosb[] = { 0xaau };
    const lib_u8 stosw[] = { 0xabu };
    const lib_u8 lodsb[] = { 0xacu };
    const lib_u8 lodsw[] = { 0xadu };
    const lib_u8 scasb[] = { 0xaeu };
    const lib_u8 scasw[] = { 0xafu };
    const lib_u8 segment_movsw[] = { 0x26u, 0xa5u };
    const lib_u8 rep_movsw[] = { 0xf3u, 0xa5u };
    lib_i32 failed = retirement_async_context_case(LIB_FALSE) |
        retirement_async_context_case(LIB_TRUE);

    provider.callback = retirement_capture;
    provider.context = &probe;
    failed |= !retirement_prepare(&machine, &deterministic, &nop, 1u);
    probe.machine = machine;
    failed |= x86_core_machine_set_retirement_observation_provider(machine,
        &provider) != LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_set_address_clear_mask(machine, 0u) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    if (!failed) failed |= result.reason != X86_CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        probe.count != 1u || probe.set_while_running != LIB_STATUS_INVALID_STATE ||
        probe.records[0].sequence != 0u ||
        probe.records[0].point.linear_pc != 0xfffffff0u ||
        probe.records[0].point.byte_count != X86_CORE_CPU_DIAGNOSTIC_BYTES ||
        probe.records[0].point.bytes[0] != nop ||
        probe.records[0].cpu_profile != X86_CORE_CPU_PROFILE_80386 ||
        probe.records[0].timing_disposition !=
            X86_CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED ||
        probe.records[0].source_timing_form_id != 0u ||
        probe.records[0].timing_key_id != 0u ||
        probe.records[0].formula_inputs != 0u ||
        probe.records[0].timing_origin !=
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK ||
        probe.records[0].modrm_form != X86_CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE ||
        probe.records[0].modrm_extension !=
            X86_CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE ||
        probe.records[0].control_outcome != X86_CORE_MACHINE_RETIREMENT_CONTROL_NONE ||
        probe.records[0].next_lexeme_components !=
            X86_CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE ||
        probe.records[0].repeat_phase != X86_CORE_CPU_RETIREMENT_REPEAT_NONE ||
        probe.records[0].elapsed_ticks != 0u || probe.records[0].timeline_ticks != 0u ||
        probe.records[0].source_ticks == 0u || probe.records[0].protected_mode ||
        probe.records[0].virtual_8086_mode || probe.records[0].operand_size_32 ||
        probe.records[0].address_size_32 || probe.records[0].lock_prefix ||
        probe.records[0].repeat_prefix != 0u;
    failed |= retirement_unallocated_profile_case(X86_CORE_CPU_PROFILE_8086,
        X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY, rep_nop,
        sizeof(rep_nop), 1) ||
        retirement_unallocated_profile_case(X86_CORE_CPU_PROFILE_8088,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY, rep_nop,
            sizeof(rep_nop), 1) ||
        retirement_unallocated_profile_case(X86_CORE_CPU_PROFILE_80186,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK, rep_nop,
            sizeof(rep_nop), 1) ||
        retirement_unallocated_profile_case(X86_CORE_CPU_PROFILE_80286,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80286_FALLBACK, rep_nop,
            sizeof(rep_nop), 1) ||
        retirement_unallocated_profile_case(X86_CORE_CPU_PROFILE_80386,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK, rep_nop,
            sizeof(rep_nop), 1);
    for (index = 0u; index < sizeof(scalar_opcodes) / sizeof(scalar_opcodes[0]);
        ++index) {
        failed |= retirement_8088_primary_case(&scalar_opcodes[index], 1u,
            scalar_ticks[index], X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY);
    }
    failed |= retirement_control_context_cases() ||
        retirement_pre_mode_snapshot_case() || retirement_8086_context_formula_case() ||
        retirement_8088_primary_case(add_register, sizeof(add_register), 3u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(add_register_memory,
            sizeof(add_register_memory), 19u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(add_memory_register,
            sizeof(add_memory_register), 30u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(mov_sreg_memory, sizeof(mov_sreg_memory), 19u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(mov_sreg_register, sizeof(mov_sreg_register),
            2u, X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(mov_ds_memory, sizeof(mov_ds_memory), 18u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(lds_memory, sizeof(lds_memory), 30u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(xlat, sizeof(xlat), 15u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(esc_register, sizeof(esc_register), 2u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(shift_memory, sizeof(shift_memory), 29u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(push_ds, sizeof(push_ds), 14u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(pop_ds, sizeof(pop_ds), 12u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(pop_cs, sizeof(pop_cs), 12u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(lock_add_memory, sizeof(lock_add_memory), 32u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(mul_al, sizeof(mul_al), 70u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC) ||
        retirement_8088_primary_case(wait, sizeof(wait), 3u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY) ||
        retirement_8088_primary_case(push_register, sizeof(push_register), 15u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(pop_register, sizeof(pop_register), 12u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(call_near, sizeof(call_near), 23u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(ret_near, sizeof(ret_near), 12u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(ret_far, sizeof(ret_far), 26u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(call_memory, sizeof(call_memory), 35u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(push_memory, sizeof(push_memory), 30u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(pop_memory, sizeof(pop_memory), 31u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK) ||
        retirement_8088_primary_case(in_immediate_byte,
            sizeof(in_immediate_byte), 10u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(in_immediate_word,
            sizeof(in_immediate_word), 14u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(in_dx_byte, sizeof(in_dx_byte), 8u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(in_dx_word, sizeof(in_dx_word), 12u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(out_immediate_byte,
            sizeof(out_immediate_byte), 10u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(out_immediate_word,
            sizeof(out_immediate_word), 14u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(out_dx_byte, sizeof(out_dx_byte), 8u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_primary_case(out_dx_word, sizeof(out_dx_word), 12u,
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO) ||
        retirement_8088_jcc_forms_case() ||
        retirement_8088_branch_case(jcxz, sizeof(jcxz), 0u, 0u, 18u) ||
        retirement_8088_branch_case(jcxz, sizeof(jcxz), 1u, 0u, 6u) ||
        retirement_8088_branch_case(loop, sizeof(loop), 2u, 0u, 17u) ||
        retirement_8088_branch_case(loop, sizeof(loop), 1u, 0u, 5u) ||
        retirement_8088_branch_case(loope, sizeof(loope), 2u,
            X86_CORE_CPU_DEBUG_EFLAGS_ZF, 18u) ||
        retirement_8088_branch_case(loope, sizeof(loope), 1u,
            X86_CORE_CPU_DEBUG_EFLAGS_ZF, 6u) ||
        retirement_8088_branch_case(loopne, sizeof(loopne), 2u, 0u, 19u) ||
        retirement_8088_branch_case(loopne, sizeof(loopne), 1u, 0u, 5u) ||
        retirement_8088_branch_case(hlt, sizeof(hlt), 0u, 0u, 2u) ||
        retirement_8088_branch_case(int3, sizeof(int3), 0u, 0u, 72u) ||
        retirement_8088_branch_case(int_type3, sizeof(int_type3), 0u, 0u,
            72u) ||
        retirement_8088_branch_case(int_other, sizeof(int_other), 0u, 0u,
            71u) ||
        retirement_8088_branch_case(into, sizeof(into), 0u, X86_CORE_CPU_DEBUG_EFLAGS_OF,
            73u) ||
        retirement_8088_branch_case(into, sizeof(into), 0u, 0u, 4u) ||
        retirement_8088_iret_case(iret, sizeof(iret)) ||
        retirement_8088_string_case(movsb, sizeof(movsb), 0u, 1u, 18u, 0u) ||
        retirement_8088_string_case(movsw, sizeof(movsw), 0u, 1u, 26u, 0u) ||
        retirement_8088_string_case(cmpsb, sizeof(cmpsb), 0u, 1u, 22u, 0u) ||
        retirement_8088_string_case(cmpsw, sizeof(cmpsw), 0u, 1u, 30u, 0u) ||
        retirement_8088_string_case(stosb, sizeof(stosb), 0u, 1u, 11u, 0u) ||
        retirement_8088_string_case(stosw, sizeof(stosw), 0u, 1u, 15u, 0u) ||
        retirement_8088_string_case(lodsb, sizeof(lodsb), 0u, 1u, 12u, 0u) ||
        retirement_8088_string_case(lodsw, sizeof(lodsw), 0u, 1u, 16u, 0u) ||
        retirement_8088_string_case(scasb, sizeof(scasb), 0u, 1u, 15u, 0u) ||
        retirement_8088_string_case(scasw, sizeof(scasw), 0u, 1u, 19u, 0u) ||
        retirement_8088_string_case(segment_movsw, sizeof(segment_movsw), 0u,
            1u, 28u, 0u) ||
        retirement_8088_string_case(rep_movsw, sizeof(rep_movsw), 2u, 2u,
            34u, 25u) ||
        retirement_8088_string_case(rep_movsw, sizeof(rep_movsw), 0u, 1u,
            9u, 0u);
    failed |= x86_core_machine_set_retirement_observation_provider(machine, LIB_NULL) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= x86_core_machine_set_address_clear_mask(machine, 0u) != LIB_STATUS_OK;
    failed |= x86_core_machine_memory_write(machine, 0xfffffff0u, &nop, 1u) !=
        LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != X86_CORE_MACHINE_STOP_BUDGET || probe.count != 1u;
    x86_core_machine_destroy(machine);
    machine = LIB_NULL;

    probe.machine = LIB_NULL;
    probe.count = 0u;
    probe.set_while_running = LIB_STATUS_OK;
    failed |= !retirement_prepare(&machine, &physical, rep_nop, sizeof(rep_nop));
    probe.machine = machine;
    failed |= x86_core_machine_set_retirement_observation_provider(machine,
        &provider) != LIB_STATUS_OK;
    failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR;
    if (!failed) failed |= result.reason != X86_CORE_MACHINE_STOP_FAULT || result.executed != 0u ||
        probe.count != 1u || probe.set_while_running != LIB_STATUS_INVALID_STATE ||
        probe.records[0].point.byte_count != X86_CORE_CPU_DIAGNOSTIC_BYTES ||
        probe.records[0].point.bytes[0] != rep_nop[0] ||
        probe.records[0].point.bytes[1] != rep_nop[1] ||
        probe.records[0].timing_disposition !=
            X86_CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED ||
        probe.records[0].source_timing_form_id !=
            X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
        probe.records[0].timing_key_id !=
            X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ||
        (probe.records[0].formula_inputs &
            X86_CORE_CPU_TIMING_INPUT_REPEAT) == 0u ||
        probe.records[0].timing_origin !=
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK ||
        probe.records[0].modrm_form != X86_CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE ||
        probe.records[0].modrm_extension !=
            X86_CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE ||
        probe.records[0].control_outcome != X86_CORE_MACHINE_RETIREMENT_CONTROL_NONE ||
        probe.records[0].next_lexeme_components !=
            X86_CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE ||
        probe.records[0].repeat_phase != X86_CORE_CPU_RETIREMENT_REPEAT_NONE ||
        probe.records[0].elapsed_ticks != 0u || probe.records[0].timeline_ticks != 0u;
    failed |= x86_core_machine_get_timeline_observation(machine, &timeline) != LIB_STATUS_OK ||
        timeline.now != 0u;
    x86_core_machine_destroy(machine);

    if (failed) return 1;
    lib_c_printf("RETIREMENT-OBSERVATION:OK\n");
    return 0;
}

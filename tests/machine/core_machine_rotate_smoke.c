#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct rotate_machine { core_machine *machine; } rotate_machine;

static C_VOID rotate_reset(C_VOID *opaque)
{
    rotate_machine *state = (rotate_machine *)opaque;
    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider rotate_provider = {
    rotate_reset, STD_NULL
};

static C_INT rotate_prepare(core_machine_cpu_profile profile, rotate_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    if (state == STD_NULL)
        return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &rotate_provider, state, &state->machine)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT rotate_run_real(rotate_machine *state, const type_unsigned_8 *code, STD_SIZE_T bytes,
    C_INT fault, t_cpu *after, core_machine_cpu_diagnostic *diagnostic)
{
    core_machine_run_result result;
    type_status status;
    if (state == STD_NULL || state->machine == STD_NULL ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK)
        return 0;
    if (fault && !test_core_machine_fixture_preflight_real_ud_terminal(
            state->machine)) return 0;
    status = core_machine_run(state->machine, (core_machine_run_budget){ 1u, 0u }, &result);
    if (status != (fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) ||
        result.reason != (fault ? CORE_MACHINE_STOP_FAULT : CORE_MACHINE_STOP_BUDGET) ||
        core_machine_get_cpu_diagnostic(state->machine, diagnostic) != TYPE_STATUS_OK)
        return 0;
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static type_unsigned_32 rotate_mask(type_unsigned_8 width)
{
    return width == 8u ? 0xffu : width == 16u ? 0xffffu : 0xffffffffu;
}

static type_unsigned_32 rotate_result(type_unsigned_8 operation, type_unsigned_8 width, type_unsigned_32 value,
    type_unsigned_8 count, type_unsigned_32 *carry, type_unsigned_8 *effective)
{
    type_unsigned_32 mask = rotate_mask(width);
    type_unsigned_8 index;
    value &= mask;
    count &= 0x1fu;
    if (operation < 2u)
        count %= width;
    else if (width != 32u)
        count %= (type_unsigned_8)(width + 1u);
    *effective = count;
    for (index = 0u; index < count; ++index) {
        type_unsigned_32 next;
        if (operation == 0u) {
            *carry = (value >> (width - 1u)) & 1u;
            value = ((value << 1u) | *carry) & mask;
        } else if (operation == 1u) {
            *carry = value & 1u;
            value = (value >> 1u) | (*carry << (width - 1u));
        } else if (operation == 2u) {
            next = (value >> (width - 1u)) & 1u;
            value = ((value << 1u) | *carry) & mask;
            *carry = next;
        } else {
            next = value & 1u;
            value = (value >> 1u) | (*carry << (width - 1u));
            *carry = next;
        }
    }
    return value;
}

static type_unsigned_32 rotate_overflow(type_unsigned_8 operation, type_unsigned_8 width, type_unsigned_32 result,
    type_unsigned_32 carry)
{
    type_unsigned_32 msb = (result >> (width - 1u)) & 1u;
    if (operation == 1u || operation == 3u)
        return msb ^ ((result >> (width - 2u)) & 1u);
    return msb ^ carry;
}

static C_INT rotate_test_forms(C_VOID)
{
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    type_unsigned_8 operation;
    type_unsigned_8 width_index;
    type_unsigned_8 mode;
    type_unsigned_8 memory;

    for (operation = 0u; operation != 4u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index)
    for (mode = 0u; mode != 3u; ++mode)
    for (memory = 0u; memory != 2u; ++memory) {
        const type_unsigned_8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        const type_unsigned_8 count = mode == 1u ? 1u : 0x21u;
        const type_unsigned_32 initial = 0x11223381u;
        const type_unsigned_32 source = 0x55667721u;
        type_unsigned_32 carry = 1u;
        type_unsigned_8 effective;
        type_unsigned_32 expected = rotate_result(operation, width, initial, count, &carry, &effective);
        type_unsigned_32 expected_eax = width == 8u ? (initial & 0xffffff00u) | expected :
            width == 16u ? (initial & 0xffff0000u) | expected : expected;
        type_unsigned_32 flag_mask = VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF |
            VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
        type_unsigned_8 code[10] = { 0 };
        STD_SIZE_T bytes = 0u;
        type_unsigned_32 observed = 0u;
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (memory && width == 32u)
            code[bytes++] = 0x67u;
        if (width == 32u)
            code[bytes++] = 0x66u;
        if (mode == 0u)
            code[bytes++] = width == 8u ? 0xc0u : 0xc1u;
        else if (mode == 1u)
            code[bytes++] = width == 8u ? 0xd0u : 0xd1u;
        else
            code[bytes++] = width == 8u ? 0xd2u : 0xd3u;
        code[bytes++] = (type_unsigned_8)(operation << 3u) |
            (memory ? (width == 32u ? 0x86u : 0x06u) : 0xc0u);
        if (memory) {
            if (width == 32u) {
                code[bytes++] = 0u;
                code[bytes++] = 0u;
                code[bytes++] = 0u;
                code[bytes++] = 0u;
            } else {
                code[bytes++] = 0u;
                code[bytes++] = 0x40u;
            }
        }
        if (mode == 0u)
            code[bytes++] = count;
        if (effective == 1u)
            flag_mask |= VCPU_EFLAGS_OF;
        if (!failed) {
            state.machine->executor_cpu.data.eax = initial;
            state.machine->executor_cpu.data.ecx = source;
            state.machine->executor_cpu.data.esi = 0x4000u;
            state.machine->executor_cpu.data.eflags = flags;
            if (memory)
                failed |= core_machine_memory_write(state.machine, 0x4000u, &initial,
                    width == 8u ? 1u : width == 16u ? 2u : 4u) != TYPE_STATUS_OK;
            failed |= !rotate_run_real(&state, code, bytes, 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid;
            if (memory) {
                failed |= core_machine_memory_read(state.machine, 0x4000u, &observed,
                    width == 8u ? 1u : width == 16u ? 2u : 4u) != TYPE_STATUS_OK;
            } else
                observed = after.data.eax;
            failed |= (width == 8u ? (observed & 0xffu) : width == 16u ?
                (observed & 0xffffu) : observed) != expected ||
                (after.data.eflags & VCPU_EFLAGS_CF) != (carry ? VCPU_EFLAGS_CF : 0u) ||
                (effective == 1u && (after.data.eflags & VCPU_EFLAGS_OF) !=
                    (rotate_overflow(operation, width, expected, carry) ? VCPU_EFLAGS_OF : 0u)) ||
                (after.data.eflags & (flag_mask & ~VCPU_EFLAGS_CF & ~VCPU_EFLAGS_OF)) !=
                    (flags & (flag_mask & ~VCPU_EFLAGS_CF & ~VCPU_EFLAGS_OF)) ||
                (memory ? after.data.eax != initial : after.data.eax != expected_eax) ||
                after.data.ecx != source || after.data.eip != bytes;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT rotate_test_count_zero(C_VOID)
{
    const type_unsigned_32 flags = VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    type_unsigned_8 operation;
    type_unsigned_8 width_index;
    type_unsigned_8 cl;
    for (operation = 0u; operation != 4u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index)
    for (cl = 0u; cl != 2u; ++cl) {
        const type_unsigned_8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        const type_unsigned_32 initial = 0x11223381u;
        type_unsigned_8 code[5] = { 0 };
        STD_SIZE_T bytes = 0u;
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (width == 32u)
            code[bytes++] = 0x66u;
        code[bytes++] = cl ? (width == 8u ? 0xd2u : 0xd3u) : (width == 8u ? 0xc0u : 0xc1u);
        code[bytes++] = (type_unsigned_8)(operation << 3u) | 0xc0u;
        if (!cl)
            code[bytes++] = 0u;
        if (!failed) {
            state.machine->executor_cpu.data.eax = initial;
            state.machine->executor_cpu.data.ecx = 0x55667700u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !rotate_run_real(&state, code, bytes, 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.eax != initial ||
                after.data.ecx != 0x55667700u || after.data.eflags != flags ||
                after.data.eip != bytes;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT rotate_test_non_one(C_VOID)
{
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    type_unsigned_8 operation;
    type_unsigned_8 width_index;
    for (operation = 0u; operation != 4u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index) {
        const type_unsigned_8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        const type_unsigned_32 initial = 0x11223381u;
        type_unsigned_32 carry = 1u;
        type_unsigned_8 effective;
        type_unsigned_32 expected = rotate_result(operation, width, initial, 2u, &carry, &effective);
        type_unsigned_32 expected_eax = width == 8u ? (initial & 0xffffff00u) | expected :
            width == 16u ? (initial & 0xffff0000u) | expected : expected;
        type_unsigned_8 code[] = {
            width == 32u ? 0x66u : 0u,
            width == 8u ? 0xc0u : 0xc1u,
            (type_unsigned_8)(operation << 3u) | 0xc0u,
            2u
        };
        const type_unsigned_8 offset = width == 32u ? 0u : 1u;
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = initial;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !rotate_run_real(&state, code + offset, sizeof(code) - offset, 0,
                &after, &diagnostic) || diagnostic.first_fault.valid || effective != 2u ||
                after.data.eax != expected_eax ||
                (after.data.eflags & VCPU_EFLAGS_CF) != (carry ? VCPU_EFLAGS_CF : 0u) ||
                (after.data.eflags & (VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF |
                    VCPU_EFLAGS_SF)) != (flags & (VCPU_EFLAGS_AF | VCPU_EFLAGS_PF |
                    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF)) ||
                after.data.eip != sizeof(code) - offset;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT rotate_test_profile(C_VOID)
{
    static const type_unsigned_8 legacy[] = { 0xc0u, 0xc0u, 1u };
    static const type_unsigned_8 rejected[] = { 0x66u, 0xd1u, 0xc0u };
    rotate_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x11223381u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= !rotate_run_real(&state, legacy, sizeof(legacy), 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || (after.data.eax & 0xffu) != 3u;
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;
    failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x11223381u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        failed |= !rotate_run_real(&state, rejected, sizeof(rejected), 1, &after, &diagnostic) ||
            !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x11223381u ||
            after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF) || after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT rotate_prepare_protected(C_INT writable, C_INT out_of_limit, rotate_machine *state)
{
    static const type_unsigned_8 pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,0x8eu,
        0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    core_machine_run_result result;
    gdt[16u] = out_of_limit ? 0x0fu : 0xffu;
    gdt[17u] = out_of_limit ? 0u : 0xffu;
    gdt[21u] = writable ? 0x92u : 0x90u;
    return rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, pointer, sizeof(pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap, sizeof(bootstrap)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, halt, sizeof(halt)) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 96u, 0u }, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT rotate_test_access_failure(C_VOID)
{
    static const type_unsigned_8 code[] = { 0xc1u,0x06u,0x10u,0u,1u };
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    type_unsigned_8 pass;
    for (pass = 0u; pass != 2u; ++pass) {
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before = 0x8123u;
        type_unsigned_16 observed = 0u;
        C_INT failed = !rotate_prepare_protected(pass == 0u, pass == 0u, &state);
        if (!failed) {
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before, sizeof(before)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, code, sizeof(code)) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_FAULT ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid ||
                core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                    TYPE_REFERENCE_OF(observed), sizeof(observed)) != TYPE_STATUS_OK || observed != before ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static type_unsigned_32 shift_parity(type_unsigned_32 value)
{
    type_unsigned_8 bits = 0u;
    value &= 0xffu;
    while (value) {
        bits ^= (type_unsigned_8)(value & 1u);
        value >>= 1u;
    }
    return bits ? 0u : VCPU_EFLAGS_PF;
}

static type_unsigned_32 shift_result(type_unsigned_8 operation, type_unsigned_8 width, type_unsigned_32 value,
    type_unsigned_8 count, type_unsigned_32 *carry)
{
    const type_unsigned_32 mask = rotate_mask(width);
    type_unsigned_8 index;
    value &= mask;
    count &= 0x1fu;
    for (index = 0u; index != count; ++index) {
        *carry = operation == 0u ? ((value >> (width - 1u)) & 1u) : (value & 1u);
        if (operation == 0u)
            value = (value << 1u) & mask;
        else if (operation == 1u)
            value >>= 1u;
        else
            value = (value >> 1u) | (value & (1u << (width - 1u)));
    }
    return value;
}

static C_INT rotate_test_cl_count_profile_matrix(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 extension;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (extension = 0u; extension != 8u; ++extension) {
        const type_unsigned_8 code[] = { 0xd2u,
            (type_unsigned_8)((extension << 3u) | 0xc0u) };
        const type_unsigned_8 count = profiles[profile_index] ==
            CORE_MACHINE_CPU_PROFILE_8086 ? 0x21u : 1u;
        type_unsigned_8 index;
        type_unsigned_32 value = 0x81u;
        type_unsigned_32 carry = 1u;
        rotate_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !rotate_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223381u;
            state.machine->executor_cpu.data.ecx = 0x55667721u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            for (index = 0u; index != count; ++index) {
                type_unsigned_32 next;
                switch (extension) {
                case 0u: carry = (value >> 7u) & 1u; value = ((value << 1u) | carry) & 0xffu; break;
                case 1u: carry = value & 1u; value = (value >> 1u) | (carry << 7u); break;
                case 2u: next = (value >> 7u) & 1u; value = ((value << 1u) | carry) & 0xffu; carry = next; break;
                case 3u: next = value & 1u; value = (value >> 1u) | (carry << 7u); carry = next; break;
                case 4u: carry = (value >> 7u) & 1u; value = (value << 1u) & 0xffu; break;
                case 5u: carry = value & 1u; value >>= 1u; break;
                case 7u: carry = value & 1u; value = (value >> 1u) | (value & 0x80u); break;
                default: break;
                }
            }
            if (extension == 6u) {
                failed |= !rotate_run_real(&state, code, sizeof(code), 1, &after, &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) || after.data.eip != 0u || after.data.eax != before.data.eax || after.data.ecx != before.data.ecx || after.data.eflags != before.data.eflags;
            } else {
                failed |= !rotate_run_real(&state, code, sizeof(code), 0, &after, &diagnostic) || diagnostic.first_fault.valid || after.data.eip != sizeof(code) || (after.data.eax & 0xffu) != value || after.data.eax != ((before.data.eax & 0xffffff00u) | value) || after.data.ecx != before.data.ecx;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}
static C_INT rotate_test_shift_forms(C_VOID)
{
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    type_unsigned_8 operation;
    type_unsigned_8 width_index;
    type_unsigned_8 mode;
    type_unsigned_8 memory;
    for (operation = 0u; operation != 3u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index)
    for (mode = 0u; mode != 3u; ++mode)
    for (memory = 0u; memory != 2u; ++memory) {
        const type_unsigned_8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        const type_unsigned_8 count = mode == 1u ? 1u : 0x21u;
        const type_unsigned_32 initial = 0x11223381u;
        const type_unsigned_32 source = 0x55667721u;
        type_unsigned_32 carry = 1u;
        type_unsigned_32 expected = shift_result(operation, width, initial, count, &carry);
        type_unsigned_32 expected_eax = width == 8u ? (initial & 0xffffff00u) | expected :
            width == 16u ? (initial & 0xffff0000u) | expected : expected;
        type_unsigned_32 flag_mask = VCPU_EFLAGS_CF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
        type_unsigned_32 expected_flags = carry ? VCPU_EFLAGS_CF : 0u;
        type_unsigned_8 code[10] = { 0 };
        STD_SIZE_T bytes = 0u;
        type_unsigned_32 observed = 0u;
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (expected & (1u << (width - 1u))) expected_flags |= VCPU_EFLAGS_SF;
        if ((expected & rotate_mask(width)) == 0u) expected_flags |= VCPU_EFLAGS_ZF;
        expected_flags |= shift_parity(expected);
        if (mode == 1u) {
            if (operation == 0u)
                expected_flags |= ((expected >> (width - 1u)) & 1u) ^ carry ? VCPU_EFLAGS_OF : 0u;
            else if (operation == 1u)
                expected_flags |= (initial >> (width - 1u)) & 1u ? VCPU_EFLAGS_OF : 0u;
            flag_mask |= VCPU_EFLAGS_OF;
        }
        if (memory && width == 32u) code[bytes++] = 0x67u;
        if (width == 32u) code[bytes++] = 0x66u;
        code[bytes++] = mode == 0u ? (width == 8u ? 0xc0u : 0xc1u) :
            mode == 1u ? (width == 8u ? 0xd0u : 0xd1u) : (width == 8u ? 0xd2u : 0xd3u);
        code[bytes++] = (type_unsigned_8)((operation == 2u ? 7u : operation + 4u) << 3u) |
            (memory ? (width == 32u ? 0x86u : 0x06u) : 0xc0u);
        if (memory) {
            if (width == 32u) { code[bytes++] = 0u; code[bytes++] = 0u; code[bytes++] = 0u; code[bytes++] = 0u; }
            else { code[bytes++] = 0u; code[bytes++] = 0x40u; }
        }
        if (mode == 0u) code[bytes++] = count;
        if (!failed) {
            state.machine->executor_cpu.data.eax = initial;
            state.machine->executor_cpu.data.ecx = source;
            state.machine->executor_cpu.data.esi = 0x4000u;
            state.machine->executor_cpu.data.eflags = flags;
            if (memory) failed |= core_machine_memory_write(state.machine, 0x4000u, &initial,
                width == 8u ? 1u : width == 16u ? 2u : 4u) != TYPE_STATUS_OK;
            failed |= !rotate_run_real(&state, code, bytes, 0, &after, &diagnostic) || diagnostic.first_fault.valid;
            if (memory) failed |= core_machine_memory_read(state.machine, 0x4000u, &observed,
                width == 8u ? 1u : width == 16u ? 2u : 4u) != TYPE_STATUS_OK;
            else observed = after.data.eax;
            failed |= (width == 8u ? observed & 0xffu : width == 16u ? observed & 0xffffu : observed) != expected ||
                (after.data.eflags & flag_mask) != (expected_flags & flag_mask) ||
                (memory ? after.data.eax != initial : after.data.eax != expected_eax) ||
                after.data.ecx != source || after.data.eip != bytes;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT rotate_test_shift_boundaries(C_VOID)
{
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    static const type_unsigned_8 undefined[] = { 0xc0u, 0xf0u, 1u };
    type_unsigned_8 operation;
    type_unsigned_8 width_index;
    for (operation = 0u; operation != 3u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index) {
        const type_unsigned_8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        type_unsigned_8 code[] = { width == 32u ? 0x66u : 0u, width == 8u ? 0xc0u : 0xc1u,
            (type_unsigned_8)((operation == 2u ? 7u : operation + 4u) << 3u) | 0xc0u, 0u };
        type_unsigned_8 offset = width == 32u ? 0u : 1u;
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223381u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !rotate_run_real(&state, code + offset, sizeof(code) - offset, 0,
                &after, &diagnostic) || after.data.eax != 0x11223381u ||
                after.data.eflags != flags || after.data.eip != sizeof(code) - offset;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
        {
            type_unsigned_32 carry = 1u;
            type_unsigned_32 expected = shift_result(operation, width, 0x11223381u, 2u, &carry);
            type_unsigned_32 expected_eax = width == 8u ? 0x11223300u | expected :
                width == 16u ? 0x11220000u | expected : expected;
            code[sizeof(code) - 1u] = 2u;
            failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
            if (!failed) {
                state.machine->executor_cpu.data.eax = 0x11223381u;
                state.machine->executor_cpu.data.eflags = flags;
                failed |= !rotate_run_real(&state, code + offset, sizeof(code) - offset, 0,
                    &after, &diagnostic) || after.data.eax != expected_eax ||
                    (after.data.eflags & VCPU_EFLAGS_CF) != (carry ? VCPU_EFLAGS_CF : 0u) ||
                    (after.data.eflags & (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)) !=
                    ((expected & (1u << (width - 1u)) ? VCPU_EFLAGS_SF : 0u) |
                    ((expected & rotate_mask(width)) == 0u ? VCPU_EFLAGS_ZF : 0u) |
                    shift_parity(expected));
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    {
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223381u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !rotate_run_real(&state, undefined, sizeof(undefined), 1, &after, &diagnostic) ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eax != 0x11223381u || after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT rotate_test_shift_profile_and_fault(C_VOID)
{
    static const type_unsigned_8 legacy[] = { 0xc0u, 0xe0u, 1u };
    static const type_unsigned_8 rejected[] = { 0x66u, 0xd1u, 0xe0u };
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
    type_unsigned_8 group;
    rotate_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = 0;
    for (group = 0u; group != 3u; ++group) {
        type_unsigned_8 legacy_code[] = {
            legacy[0], (type_unsigned_8)((group == 2u ? 7u : group + 4u) << 3u) | 0xc0u, legacy[2]
        };
        type_unsigned_8 rejected_code[] = {
            rejected[0], rejected[1], (type_unsigned_8)((group == 2u ? 7u : group + 4u) << 3u) | 0xc0u
        };
        type_unsigned_32 legacy_carry = 1u;
        type_unsigned_32 legacy_expected = shift_result(group, 8u, 0x81u, 1u, &legacy_carry);
        failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223381u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !rotate_run_real(&state, legacy_code, sizeof(legacy_code), 0,
                &after, &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eax & 0xffu) != legacy_expected;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
        failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223381u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !rotate_run_real(&state, rejected_code, sizeof(rejected_code), 1,
                &after, &diagnostic) ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eax != 0x11223381u || after.data.eflags != flags ||
                after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        type_unsigned_8 pass;
        for (group = 0u; group != 3u; ++group)
        for (pass = 0u; pass != 2u; ++pass) {
            type_unsigned_8 code[] = { 0xc1u, (type_unsigned_8)((group == 2u ? 7u : group + 4u) << 3u) | 0x06u,
                0x10u, 0u, 1u };
            type_unsigned_16 before = 0x8123u;
            type_unsigned_16 observed = 0u;
            core_machine_run_result result;
            failed = !rotate_prepare_protected(pass == 0u, pass == 0u, &state);
            if (!failed) {
                state.machine->executor_cpu.data.eflags = flags;
                failed |= core_machine_memory_write(state.machine, 0x3010u, &before, sizeof(before)) != TYPE_STATUS_OK ||
                    core_machine_memory_write(state.machine, 0x2000u, code, sizeof(code)) != TYPE_STATUS_OK;
                test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
                failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_FAULT ||
                    result.reason != CORE_MACHINE_STOP_FAULT ||
                    core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
                after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= !diagnostic.first_fault.valid ||
                    core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                        TYPE_REFERENCE_OF(observed), sizeof(observed)) != TYPE_STATUS_OK || observed != before ||
                    after.data.eflags != flags || after.data.eip != 0u;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static C_INT rotate_test_8086_immediate_rejection(C_VOID)
{
    type_unsigned_8 width;
    type_unsigned_8 extension;

    for (width = 0u; width != 2u; ++width)
    for (extension = 0u; extension != 8u; ++extension) {
        const type_unsigned_8 code[] = {width == 0u ? 0xc0u : 0xc1u,
            (type_unsigned_8)((extension << 3u) | 0xc0u), 1u};
        rotate_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_8086, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223381u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !rotate_run_real(&state, code, sizeof(code), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != 0u || after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx || after.data.eflags !=
                before.data.eflags;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}
static C_INT rotate_test_80186_immediate_extensions(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 profile;
    type_unsigned_8 width_index;
    type_unsigned_8 extension;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (width_index = 0u; width_index != 2u; ++width_index)
    for (extension = 0u; extension != 8u; ++extension) {        const type_unsigned_8 width = width_index == 0u ? 8u : 16u;
        const type_unsigned_8 opcode = width == 8u ? 0xc0u : 0xc1u;
        const type_unsigned_8 code[] = {
            opcode, (type_unsigned_8)((extension << 3u) | 0xc0u), 1u
        };
        const type_unsigned_32 initial = width == 8u ? 0x11223381u :
            0x11228181u;
        type_unsigned_32 expected = initial;
        type_unsigned_32 carry = 1u;
        type_unsigned_8 effective = 0u;
        rotate_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !rotate_prepare(profiles[profile], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = initial;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF;
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            if (extension < 4u)
                expected = rotate_result(extension, width, initial, 1u,
                    &carry, &effective);
            else if (extension == 4u)
                expected = shift_result(0u, width, initial, 1u, &carry);
            else if (extension == 5u)
                expected = shift_result(1u, width, initial, 1u, &carry);
            else if (extension == 7u)
                expected = shift_result(2u, width, initial, 1u, &carry);
            if (extension == 6u) {
                failed |= !rotate_run_real(&state, code, sizeof(code), 1,
                    &after, &diagnostic) || !diagnostic.first_fault.valid ||
                    !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                        VCPUINS_EXCEPT_UD) || after.data.eip != 0u ||
                    after.data.eax != before.data.eax || after.data.ecx !=
                        before.data.ecx || after.data.edx != before.data.edx ||
                    after.data.ebx != before.data.ebx || after.data.esp !=
                        before.data.esp || after.data.ebp != before.data.ebp ||
                    after.data.esi != before.data.esi || after.data.edi !=
                        before.data.edi || after.data.eflags !=
                        before.data.eflags;
            } else {
                failed |= !rotate_run_real(&state, code, sizeof(code), 0,
                    &after, &diagnostic) || diagnostic.first_fault.valid ||
                    after.data.eip != sizeof(code) ||
                    (width == 8u ? after.data.al : after.data.ax) !=
                        (width == 8u ? (type_unsigned_8)expected :
                            (type_unsigned_16)expected) ||
                    (width == 8u && after.data.eax !=
                        ((initial & 0xffffff00u) | (expected & 0xffu))) ||
                    (width == 16u && after.data.eax !=
                        ((initial & 0xffff0000u) | (expected & 0xffffu)));
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!rotate_test_forms() || !rotate_test_count_zero() || !rotate_test_non_one() || !rotate_test_cl_count_profile_matrix() ||
        !rotate_test_shift_forms() ||
        !rotate_test_shift_boundaries() ||
        !rotate_test_shift_profile_and_fault() ||
        !rotate_test_8086_immediate_rejection() ||
        !rotate_test_80186_immediate_extensions() ||
        !rotate_test_profile() || !rotate_test_access_failure())
        return 1;
    STD_PRINTF("M5:T316:S18:ROTATE:OK\n");
    STD_PRINTF("M5:T316:S19:SHIFT:OK\n");
    STD_PRINTF("M5:T401:S8:GROUP2-CL-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S21:GROUP2-IMMEDIATE-PROFILES:OK\n");
    return 0;
}

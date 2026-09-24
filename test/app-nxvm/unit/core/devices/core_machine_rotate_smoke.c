#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct rotate_machine { core_machine *machine; } rotate_machine;

static void rotate_reset(void *opaque)
{
    rotate_machine *state = (rotate_machine *)opaque;
    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider rotate_provider = {
    rotate_reset, LIB_NULL
};

static lib_i32 rotate_prepare(core_machine_cpu_profile profile, rotate_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    if (state == LIB_NULL)
        return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &rotate_provider, state, &state->machine)) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 rotate_run_real(rotate_machine *state, const lib_u8 *code, lib_size bytes,
    lib_i32 fault, t_cpu *after, core_machine_cpu_diagnostic *diagnostic)
{
    core_machine_run_result result;
    lib_status status;
    if (state == LIB_NULL || state->machine == LIB_NULL ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, 0u, code, bytes) != LIB_STATUS_OK)
        return 0;
    if (fault && !test_core_machine_fixture_preflight_real_ud_terminal(
            state->machine)) return 0;
    status = core_machine_run(state->machine, (core_machine_run_budget){ 1u, 0u }, &result);
    if (status != (fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
        result.reason != (fault ? CORE_MACHINE_STOP_FAULT : CORE_MACHINE_STOP_BUDGET) ||
        core_machine_get_cpu_diagnostic(state->machine, diagnostic) != LIB_STATUS_OK)
        return 0;
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static lib_u32 rotate_mask(lib_u8 width)
{
    return width == 8u ? 0xffu : width == 16u ? 0xffffu : 0xffffffffu;
}

static lib_u32 rotate_result(lib_u8 operation, lib_u8 width, lib_u32 value,
    lib_u8 count, lib_u32 *carry, lib_u8 *effective)
{
    lib_u32 mask = rotate_mask(width);
    lib_u8 index;
    value &= mask;
    count &= 0x1fu;
    if (operation < 2u)
        count %= width;
    else if (width != 32u)
        count %= (lib_u8)(width + 1u);
    *effective = count;
    for (index = 0u; index < count; ++index) {
        lib_u32 next;
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

static lib_u32 rotate_overflow(lib_u8 operation, lib_u8 width, lib_u32 result,
    lib_u32 carry)
{
    lib_u32 msb = (result >> (width - 1u)) & 1u;
    if (operation == 1u || operation == 3u)
        return msb ^ ((result >> (width - 2u)) & 1u);
    return msb ^ carry;
}

static lib_i32 rotate_test_forms(void)
{
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    lib_u8 operation;
    lib_u8 width_index;
    lib_u8 mode;
    lib_u8 memory;

    for (operation = 0u; operation != 4u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index)
    for (mode = 0u; mode != 3u; ++mode)
    for (memory = 0u; memory != 2u; ++memory) {
        const lib_u8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        const lib_u8 count = mode == 1u ? 1u : 0x21u;
        const lib_u32 initial = 0x11223381u;
        const lib_u32 source = 0x55667721u;
        lib_u32 carry = 1u;
        lib_u8 effective;
        lib_u32 expected = rotate_result(operation, width, initial, count, &carry, &effective);
        lib_u32 expected_eax = width == 8u ? (initial & 0xffffff00u) | expected :
            width == 16u ? (initial & 0xffff0000u) | expected : expected;
        lib_u32 flag_mask = VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF |
            VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
        lib_u8 code[10] = { 0 };
        lib_size bytes = 0u;
        lib_u32 observed = 0u;
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

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
        code[bytes++] = (lib_u8)(operation << 3u) |
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
                    width == 8u ? 1u : width == 16u ? 2u : 4u) != LIB_STATUS_OK;
            failed |= !rotate_run_real(&state, code, bytes, 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid;
            if (memory) {
                failed |= core_machine_memory_read(state.machine, 0x4000u, &observed,
                    width == 8u ? 1u : width == 16u ? 2u : 4u) != LIB_STATUS_OK;
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

static lib_i32 rotate_test_count_zero(void)
{
    const lib_u32 flags = VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    lib_u8 operation;
    lib_u8 width_index;
    lib_u8 cl;
    for (operation = 0u; operation != 4u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index)
    for (cl = 0u; cl != 2u; ++cl) {
        const lib_u8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        const lib_u32 initial = 0x11223381u;
        lib_u8 code[5] = { 0 };
        lib_size bytes = 0u;
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (width == 32u)
            code[bytes++] = 0x66u;
        code[bytes++] = cl ? (width == 8u ? 0xd2u : 0xd3u) : (width == 8u ? 0xc0u : 0xc1u);
        code[bytes++] = (lib_u8)(operation << 3u) | 0xc0u;
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

static lib_i32 rotate_test_non_one(void)
{
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    lib_u8 operation;
    lib_u8 width_index;
    for (operation = 0u; operation != 4u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index) {
        const lib_u8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        const lib_u32 initial = 0x11223381u;
        lib_u32 carry = 1u;
        lib_u8 effective;
        lib_u32 expected = rotate_result(operation, width, initial, 2u, &carry, &effective);
        lib_u32 expected_eax = width == 8u ? (initial & 0xffffff00u) | expected :
            width == 16u ? (initial & 0xffff0000u) | expected : expected;
        lib_u8 code[] = {
            width == 32u ? 0x66u : 0u,
            width == 8u ? 0xc0u : 0xc1u,
            (lib_u8)(operation << 3u) | 0xc0u,
            2u
        };
        const lib_u8 offset = width == 32u ? 0u : 1u;
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
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

static lib_i32 rotate_test_profile(void)
{
    static const lib_u8 legacy[] = { 0xc0u, 0xc0u, 1u };
    static const lib_u8 rejected[] = { 0x66u, 0xd1u, 0xc0u };
    rotate_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_i32 failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
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
            !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x11223381u ||
            after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF) || after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 rotate_prepare_protected(lib_i32 writable, lib_i32 out_of_limit, rotate_machine *state)
{
    static const lib_u8 pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0, 0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0, 0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,0x8eu,
        0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const lib_u8 halt[] = { 0xf4u };
    core_machine_run_result result;
    gdt[16u] = out_of_limit ? 0x0fu : 0xffu;
    gdt[17u] = out_of_limit ? 0u : 0xffu;
    gdt[21u] = writable ? 0x92u : 0x90u;
    return rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, pointer, sizeof(pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap, sizeof(bootstrap)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, halt, sizeof(halt)) == LIB_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 96u, 0u }, &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static lib_i32 rotate_test_access_failure(void)
{
    static const lib_u8 code[] = { 0xc1u,0x06u,0x10u,0u,1u };
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    lib_u8 pass;
    for (pass = 0u; pass != 2u; ++pass) {
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        lib_u16 before = 0x8123u;
        lib_u16 observed = 0u;
        lib_i32 failed = !rotate_prepare_protected(pass == 0u, pass == 0u, &state);
        if (!failed) {
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before, sizeof(before)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, code, sizeof(code)) != LIB_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid ||
                core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                    CORE_MACHINE_REFERENCE_OF(observed), sizeof(observed)) != LIB_STATUS_OK || observed != before ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_u32 shift_parity(lib_u32 value)
{
    lib_u8 bits = 0u;
    value &= 0xffu;
    while (value) {
        bits ^= (lib_u8)(value & 1u);
        value >>= 1u;
    }
    return bits ? 0u : VCPU_EFLAGS_PF;
}

static lib_u32 shift_result(lib_u8 operation, lib_u8 width, lib_u32 value,
    lib_u8 count, lib_u32 *carry)
{
    const lib_u32 mask = rotate_mask(width);
    lib_u8 index;
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

static lib_i32 rotate_test_cl_count_profile_matrix(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile_index;
    lib_u8 extension;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (extension = 0u; extension != 8u; ++extension) {
        const lib_u8 code[] = { 0xd2u,
            (lib_u8)((extension << 3u) | 0xc0u) };
        const lib_u8 count = profiles[profile_index] ==
            CORE_MACHINE_CPU_PROFILE_8086 ? 0x21u : 1u;
        lib_u8 index;
        lib_u32 value = 0x81u;
        lib_u32 carry = 1u;
        rotate_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !rotate_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223381u;
            state.machine->executor_cpu.data.ecx = 0x55667721u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            for (index = 0u; index != count; ++index) {
                lib_u32 next;
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
                failed |= !rotate_run_real(&state, code, sizeof(code), 1, &after, &diagnostic) || !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) || after.data.eip != 0u || after.data.eax != before.data.eax || after.data.ecx != before.data.ecx || after.data.eflags != before.data.eflags;
            } else {
                failed |= !rotate_run_real(&state, code, sizeof(code), 0, &after, &diagnostic) || diagnostic.first_fault.valid || after.data.eip != sizeof(code) || (after.data.eax & 0xffu) != value || after.data.eax != ((before.data.eax & 0xffffff00u) | value) || after.data.ecx != before.data.ecx;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}
static lib_i32 rotate_test_shift_forms(void)
{
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    lib_u8 operation;
    lib_u8 width_index;
    lib_u8 mode;
    lib_u8 memory;
    for (operation = 0u; operation != 3u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index)
    for (mode = 0u; mode != 3u; ++mode)
    for (memory = 0u; memory != 2u; ++memory) {
        const lib_u8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        const lib_u8 count = mode == 1u ? 1u : 0x21u;
        const lib_u32 initial = 0x11223381u;
        const lib_u32 source = 0x55667721u;
        lib_u32 carry = 1u;
        lib_u32 expected = shift_result(operation, width, initial, count, &carry);
        lib_u32 expected_eax = width == 8u ? (initial & 0xffffff00u) | expected :
            width == 16u ? (initial & 0xffff0000u) | expected : expected;
        lib_u32 flag_mask = VCPU_EFLAGS_CF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
        lib_u32 expected_flags = carry ? VCPU_EFLAGS_CF : 0u;
        lib_u8 code[10] = { 0 };
        lib_size bytes = 0u;
        lib_u32 observed = 0u;
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
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
        code[bytes++] = (lib_u8)((operation == 2u ? 7u : operation + 4u) << 3u) |
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
                width == 8u ? 1u : width == 16u ? 2u : 4u) != LIB_STATUS_OK;
            failed |= !rotate_run_real(&state, code, bytes, 0, &after, &diagnostic) || diagnostic.first_fault.valid;
            if (memory) failed |= core_machine_memory_read(state.machine, 0x4000u, &observed,
                width == 8u ? 1u : width == 16u ? 2u : 4u) != LIB_STATUS_OK;
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

static lib_i32 rotate_test_shift_boundaries(void)
{
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    static const lib_u8 undefined[] = { 0xc0u, 0xf0u, 1u };
    lib_u8 operation;
    lib_u8 width_index;
    for (operation = 0u; operation != 3u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index) {
        const lib_u8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        lib_u8 code[] = { width == 32u ? 0x66u : 0u, width == 8u ? 0xc0u : 0xc1u,
            (lib_u8)((operation == 2u ? 7u : operation + 4u) << 3u) | 0xc0u, 0u };
        lib_u8 offset = width == 32u ? 0u : 1u;
        rotate_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
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
            lib_u32 carry = 1u;
            lib_u32 expected = shift_result(operation, width, 0x11223381u, 2u, &carry);
            lib_u32 expected_eax = width == 8u ? 0x11223300u | expected :
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
        lib_i32 failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223381u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !rotate_run_real(&state, undefined, sizeof(undefined), 1, &after, &diagnostic) ||
                !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eax != 0x11223381u || after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 rotate_test_shift_profile_and_fault(void)
{
    static const lib_u8 legacy[] = { 0xc0u, 0xe0u, 1u };
    static const lib_u8 rejected[] = { 0x66u, 0xd1u, 0xe0u };
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
    lib_u8 group;
    rotate_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_i32 failed = 0;
    for (group = 0u; group != 3u; ++group) {
        lib_u8 legacy_code[] = {
            legacy[0], (lib_u8)((group == 2u ? 7u : group + 4u) << 3u) | 0xc0u, legacy[2]
        };
        lib_u8 rejected_code[] = {
            rejected[0], rejected[1], (lib_u8)((group == 2u ? 7u : group + 4u) << 3u) | 0xc0u
        };
        lib_u32 legacy_carry = 1u;
        lib_u32 legacy_expected = shift_result(group, 8u, 0x81u, 1u, &legacy_carry);
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
                !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eax != 0x11223381u || after.data.eflags != flags ||
                after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        lib_u8 pass;
        for (group = 0u; group != 3u; ++group)
        for (pass = 0u; pass != 2u; ++pass) {
            lib_u8 code[] = { 0xc1u, (lib_u8)((group == 2u ? 7u : group + 4u) << 3u) | 0x06u,
                0x10u, 0u, 1u };
            lib_u16 before = 0x8123u;
            lib_u16 observed = 0u;
            core_machine_run_result result;
            failed = !rotate_prepare_protected(pass == 0u, pass == 0u, &state);
            if (!failed) {
                state.machine->executor_cpu.data.eflags = flags;
                failed |= core_machine_memory_write(state.machine, 0x3010u, &before, sizeof(before)) != LIB_STATUS_OK ||
                    core_machine_memory_write(state.machine, 0x2000u, code, sizeof(code)) != LIB_STATUS_OK;
                test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
                failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_INTERNAL_ERROR ||
                    result.reason != CORE_MACHINE_STOP_FAULT ||
                    core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK;
                after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= !diagnostic.first_fault.valid ||
                    core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                        CORE_MACHINE_REFERENCE_OF(observed), sizeof(observed)) != LIB_STATUS_OK || observed != before ||
                    after.data.eflags != flags || after.data.eip != 0u;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static lib_i32 rotate_test_8086_immediate_rejection(void)
{
    lib_u8 width;
    lib_u8 extension;

    for (width = 0u; width != 2u; ++width)
    for (extension = 0u; extension != 8u; ++extension) {
        const lib_u8 code[] = {width == 0u ? 0xc0u : 0xc1u,
            (lib_u8)((extension << 3u) | 0xc0u), 1u};
        rotate_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !rotate_prepare(CORE_MACHINE_CPU_PROFILE_8086, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223381u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !rotate_run_real(&state, code, sizeof(code), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
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
static lib_i32 rotate_test_80186_immediate_extensions(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 profile;
    lib_u8 width_index;
    lib_u8 extension;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (width_index = 0u; width_index != 2u; ++width_index)
    for (extension = 0u; extension != 8u; ++extension) {        const lib_u8 width = width_index == 0u ? 8u : 16u;
        const lib_u8 opcode = width == 8u ? 0xc0u : 0xc1u;
        const lib_u8 code[] = {
            opcode, (lib_u8)((extension << 3u) | 0xc0u), 1u
        };
        const lib_u32 initial = width == 8u ? 0x11223381u :
            0x11228181u;
        lib_u32 expected = initial;
        lib_u32 carry = 1u;
        lib_u8 effective = 0u;
        rotate_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !rotate_prepare(profiles[profile], &state);

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
                    !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
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
                        (width == 8u ? (lib_u8)expected :
                            (lib_u16)expected) ||
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

lib_i32 main(void)
{
    if (!rotate_test_forms() || !rotate_test_count_zero() || !rotate_test_non_one() || !rotate_test_cl_count_profile_matrix() ||
        !rotate_test_shift_forms() ||
        !rotate_test_shift_boundaries() ||
        !rotate_test_shift_profile_and_fault() ||
        !rotate_test_8086_immediate_rejection() ||
        !rotate_test_80186_immediate_extensions() ||
        !rotate_test_profile() || !rotate_test_access_failure())
        return 1;
    printf("M5:T316:S18:ROTATE:OK\n");
    printf("M5:T316:S19:SHIFT:OK\n");
    printf("M5:T401:S8:GROUP2-CL-PROFILES:OK\n");
    printf("M5:T401:S21:GROUP2-IMMEDIATE-PROFILES:OK\n");
    return 0;
}

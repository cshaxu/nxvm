#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"
#include "core/chips/cpu/cpu_timing.h"
/* REAL_UD_TERMINAL_CPU_OWNER: unsupported Group-2 forms are CPU-owned. */

typedef cpu_instruction_fixture rotate_fixture;

static lib_i32 rotate_run_cpu(rotate_fixture *state, const lib_u8 *code,
    lib_size bytes, lib_i32 fault, t_cpu *after)
{
    lib_status status;

    if (fault) state->cpu.data.idtr.limit = 0x17u;
    status = cpu_instruction_run(state, code, (lib_u8)bytes, after);

    return status == (fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) &&
        state->fault.valid == (fault ? LIB_TRUE : LIB_FALSE);
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
        rotate_fixture state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

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
        if ((count & 0x1fu) == 1u)
            flag_mask |= VCPU_EFLAGS_OF;
        state.cpu.data.eax = initial;
        state.cpu.data.ecx = source;
        state.cpu.data.esi = 0x4000u;
        state.cpu.data.eflags = flags;
        if (memory)
            failed |= cpu_instruction_write(&state, 0x4000u, &initial, width == 8u ? 1u : width == 16u ? 2u : 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        failed |= !rotate_run_cpu(&state, code, bytes, 0, &after) ||
            state.fault.valid;
        if (memory) {
            failed |= cpu_instruction_read(&state, 0x4000u, &observed, width == 8u ? 1u : width == 16u ? 2u : 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK;
        } else
            observed = after.data.eax;
        failed |= (width == 8u ? (observed & 0xffu) : width == 16u ?
            (observed & 0xffffu) : observed) != expected ||
            (after.data.eflags & VCPU_EFLAGS_CF) != (carry ? VCPU_EFLAGS_CF : 0u) ||
            ((count & 0x1fu) == 1u && (after.data.eflags & VCPU_EFLAGS_OF) !=
                (rotate_overflow(operation, width, expected, carry) ? VCPU_EFLAGS_OF : 0u)) ||
            (after.data.eflags & (flag_mask & ~VCPU_EFLAGS_CF & ~VCPU_EFLAGS_OF)) !=
                (flags & (flag_mask & ~VCPU_EFLAGS_CF & ~VCPU_EFLAGS_OF)) ||
            (memory ? after.data.eax != initial : after.data.eax != expected_eax) ||
            after.data.ecx != source || after.data.eip != bytes;

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
        rotate_fixture state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (width == 32u)
            code[bytes++] = 0x66u;
        code[bytes++] = cl ? (width == 8u ? 0xd2u : 0xd3u) : (width == 8u ? 0xc0u : 0xc1u);
        code[bytes++] = (lib_u8)(operation << 3u) | 0xc0u;
        if (!cl)
            code[bytes++] = 0u;
        state.cpu.data.eax = initial;
        state.cpu.data.ecx = 0x55667700u;
        state.cpu.data.eflags = flags;
        failed |= !rotate_run_cpu(&state, code, bytes, 0, &after) ||
            state.fault.valid || after.data.eax != initial ||
            after.data.ecx != 0x55667700u || after.data.eflags != flags ||
            after.data.eip != bytes;

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
        rotate_fixture state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = initial;
        state.cpu.data.eflags = flags;
        failed |= !rotate_run_cpu(&state, code + offset, sizeof(code) - offset, 0,
            &after) || state.fault.valid || effective != 2u ||
            after.data.eax != expected_eax ||
            (after.data.eflags & VCPU_EFLAGS_CF) != (carry ? VCPU_EFLAGS_CF : 0u) ||
            (after.data.eflags & (VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF |
                VCPU_EFLAGS_SF)) != (flags & (VCPU_EFLAGS_AF | VCPU_EFLAGS_PF |
                VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF)) ||
            after.data.eip != sizeof(code) - offset;

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 rotate_test_profile(void)
{
    static const lib_u8 legacy[] = { 0xc0u, 0xc0u, 1u };
    static const lib_u8 rejected[] = { 0x66u, 0xd1u, 0xc0u };
    rotate_fixture state;
    t_cpu after;

    lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80186);
    state.cpu.data.eax = 0x11223381u;
    state.cpu.data.eflags = VCPU_EFLAGS_CF;
    failed |= !rotate_run_cpu(&state, legacy, sizeof(legacy), 0, &after) ||
        state.fault.valid || (after.data.eax & 0xffu) != 3u;

    if (failed)
        return 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
        failed = 0;
    state.cpu.data.eax = 0x11223381u;
    state.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
    failed |= !cpu_instruction_expect_real_fault(&state, rejected,
        sizeof(rejected), 6u);

    return !failed;
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
        rotate_fixture state;
        t_cpu before;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eax = 0x11223381u;
        state.cpu.data.ecx = 0x55667721u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
        before = state.cpu;
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
            failed |= !cpu_instruction_expect_real_fault(&state, code,
                sizeof(code), 6u);
            after = state.cpu;
        } else {
            failed |= !rotate_run_cpu(&state, code, sizeof(code), 0, &after) || state.fault.valid || after.data.eip != sizeof(code) || (after.data.eax & 0xffu) != value || after.data.eax != ((before.data.eax & 0xffffff00u) | value) || after.data.ecx != before.data.ecx;
        }

        if (failed) {
            lib_c_fprintf(lib_c_stderr, "cl profile=%u extension=%u before=%x/%x after=%x/%x value=%x fault=%u/%x stop=%u\n",
                (unsigned)profile_index, (unsigned)extension,
                (unsigned)before.data.eax, (unsigned)before.data.eflags,
                (unsigned)after.data.eax, (unsigned)after.data.eflags,
                (unsigned)value, (unsigned)state.fault.valid,
                (unsigned)state.fault.exception_mask,
                (unsigned)state.execution.stop_requested);
            return 0;
        }
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
        rotate_fixture state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
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
        state.cpu.data.eax = initial;
        state.cpu.data.ecx = source;
        state.cpu.data.esi = 0x4000u;
        state.cpu.data.eflags = flags;
        if (memory) failed |= cpu_instruction_write(&state, 0x4000u, &initial, width == 8u ? 1u : width == 16u ? 2u : 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        failed |= !rotate_run_cpu(&state, code, bytes, 0, &after) || state.fault.valid;
        if (memory) failed |= cpu_instruction_read(&state, 0x4000u, &observed, width == 8u ? 1u : width == 16u ? 2u : 4u,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK;
        else observed = after.data.eax;
        failed |= (width == 8u ? observed & 0xffu : width == 16u ? observed & 0xffffu : observed) != expected ||
            (after.data.eflags & flag_mask) != (expected_flags & flag_mask) ||
            (memory ? after.data.eax != initial : after.data.eax != expected_eax) ||
            after.data.ecx != source || after.data.eip != bytes;

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
        rotate_fixture state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = 0x11223381u;
        state.cpu.data.eflags = flags;
        failed |= !rotate_run_cpu(&state, code + offset, sizeof(code) - offset, 0,
            &after) || after.data.eax != 0x11223381u ||
            after.data.eflags != flags || after.data.eip != sizeof(code) - offset;

        if (failed) return 0;
        {
            lib_u32 carry = 1u;
            lib_u32 expected = shift_result(operation, width, 0x11223381u, 2u, &carry);
            lib_u32 expected_eax = width == 8u ? 0x11223300u | expected :
                width == 16u ? 0x11220000u | expected : expected;
            code[sizeof(code) - 1u] = 2u;
            cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        failed = 0;
            state.cpu.data.eax = 0x11223381u;
            state.cpu.data.eflags = flags;
            failed |= !rotate_run_cpu(&state, code + offset, sizeof(code) - offset, 0,
                &after) || after.data.eax != expected_eax ||
                (after.data.eflags & VCPU_EFLAGS_CF) != (carry ? VCPU_EFLAGS_CF : 0u) ||
                (after.data.eflags & (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)) !=
                ((expected & (1u << (width - 1u)) ? VCPU_EFLAGS_SF : 0u) |
                ((expected & rotate_mask(width)) == 0u ? VCPU_EFLAGS_ZF : 0u) |
                shift_parity(expected));

            if (failed) return 0;
        }
    }
    {
        rotate_fixture state;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = 0x11223381u;
        state.cpu.data.eflags = flags;
        failed |= !cpu_instruction_expect_real_fault(&state, undefined,
            sizeof(undefined), 6u);

        if (failed) return 0;
    }
    return 1;
}

static lib_i32 rotate_test_shift_profile(void)
{
    static const lib_u8 legacy[] = { 0xc0u, 0xe0u, 1u };
    static const lib_u8 rejected[] = { 0x66u, 0xd1u, 0xe0u };
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
    lib_u8 group;
    rotate_fixture state;
    t_cpu after;

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
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80186);
        failed = 0;
        state.cpu.data.eax = 0x11223381u;
        state.cpu.data.eflags = flags;
        failed |= !rotate_run_cpu(&state, legacy_code, sizeof(legacy_code), 0,
            &after) || state.fault.valid ||
            (after.data.eax & 0xffu) != legacy_expected;

        if (failed) return 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
        failed = 0;
        state.cpu.data.eax = 0x11223381u;
        state.cpu.data.eflags = flags;
        failed |= !cpu_instruction_expect_real_fault(&state, rejected_code,
            sizeof(rejected_code), 6u);

        if (failed) return 0;
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
        rotate_fixture state;
        t_cpu before;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_8086);

        state.cpu.data.eax = 0x11223381u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
        before = state.cpu;
        failed |= !rotate_run_cpu(&state, code, sizeof(code), 1, &after) || !state.fault.valid || !X86_CPU_BIT_IS_SET(
            state.fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.eflags !=
            before.data.eflags;

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
        rotate_fixture state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile]);

        state.cpu.data.eax = initial;
        state.cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF;
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
            failed |= !cpu_instruction_expect_real_fault(&state, code,
                sizeof(code), 6u);
        } else {
            failed |= !rotate_run_cpu(&state, code, sizeof(code), 0,
                &after) || state.fault.valid ||
                after.data.eip != sizeof(code) ||
                (width == 8u ? after.data.al : after.data.ax) !=
                    (width == 8u ? (lib_u8)expected :
                        (lib_u16)expected) ||
                (width == 8u && after.data.eax !=
                    ((initial & 0xffffff00u) | (expected & 0xffu))) ||
                (width == 16u && after.data.eax !=
                    ((initial & 0xffff0000u) | (expected & 0xffffu)));
        }

        if (failed) return 0;
    }
    return 1;
}

/* Intel count rules apply before reduction of the carry ring's loop steps.
 * The closed-form oracle rotates a width+1-bit value, not the CPU's loop. */
static lib_bool rotate_test_carry_ring_boundaries(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 counts[] = {0u, 1u, 8u, 9u, 10u, 16u,
        17u, 18u, 31u, 32u, 33u, 255u};
    const lib_u32 preserved = VCPU_EFLAGS_AF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF;
    lib_u8 profile, width_index, direction, count_index, carry, memory;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (width_index = 0u; width_index < (profile == 4u ? 3u : 2u); ++width_index)
    for (direction = 0u; direction < 2u; ++direction)
    for (count_index = 0u; count_index < sizeof(counts); ++count_index)
    for (carry = 0u; carry < 2u; ++carry)
    for (memory = 0u; memory < 2u; ++memory) {
        const lib_u8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        const lib_u8 count = profile < 2u ? counts[count_index] :
            (lib_u8)(counts[count_index] & 0x1fu);
        const lib_u8 steps = (lib_u8)(count % (width + 1u));
        const lib_u32 mask = rotate_mask(width);
        const lib_u32 initial = 0x8123a581u;
        const lib_u32 flags = preserved | VCPU_EFLAGS_OF |
            (carry ? VCPU_EFLAGS_CF : 0u);
        const lib_u64 ring_mask = (UINT64_C(1) << (width + 1u)) - 1u;
        lib_u64 ring = ((lib_u64)carry << width) | (initial & mask);
        lib_u8 code[7] = {0};
        lib_u8 bytes = 0u;
        lib_u32 observed = 0u, expected, expected_cf;
        rotate_fixture state;
        t_cpu after;

        if (steps != 0u)
            ring = direction ? ((ring >> steps) | (ring << (width + 1u - steps))) :
                ((ring << steps) | (ring >> (width + 1u - steps)));
        ring &= ring_mask;
        expected = (lib_u32)ring & mask;
        expected_cf = (lib_u32)(ring >> width);
        if (width == 32u) code[bytes++] = 0x66u;
        code[bytes++] = width == 8u ? 0xd2u : 0xd3u;
        code[bytes++] = (lib_u8)((direction ? 3u : 2u) << 3u) |
            (memory ? 0x06u : 0xc0u);
        if (memory) { code[bytes++] = 0u; code[bytes++] = 0x40u; }
        cpu_instruction_prepare(&state, profiles[profile]);
        state.cpu.data.eax = initial;
        state.cpu.data.ecx = 0x55667700u | counts[count_index];
        state.cpu.data.eflags = flags;
        if (memory && cpu_instruction_write(&state, 0x4000u, &initial,
                width / 8u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK)
            return LIB_FALSE;
        if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
                state.fault.valid) return LIB_FALSE;
        if (memory) {
            if (cpu_instruction_read(&state, 0x4000u, &observed, width / 8u,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                    LIB_STATUS_OK) return LIB_FALSE;
        } else observed = after.data.eax;
        if ((observed & mask) != expected ||
                (after.data.eflags & VCPU_EFLAGS_CF) !=
                    (expected_cf ? VCPU_EFLAGS_CF : 0u) ||
                (after.data.eflags & preserved) != preserved ||
                state.instructions.data.opr2 != count ||
                (count == 0u && after.data.eflags != flags) ||
                (count == 1u && (after.data.eflags & VCPU_EFLAGS_OF) !=
                    (rotate_overflow(direction ? 3u : 2u, width, expected,
                        expected_cf) ? VCPU_EFLAGS_OF : 0u)) ||
                ((state.instructions.data.udf & VCPU_EFLAGS_OF) != 0u) !=
                    (count > 1u) ||
                after.data.ecx != (0x55667700u | counts[count_index]) ||
                after.data.eip != bytes ||
                (memory ? after.data.eax != initial :
                    after.data.eax != ((initial & ~mask) | expected))) {
            lib_c_fprintf(lib_c_stderr,
                "carry ring profile=%u width=%u direction=%u count=%u cf=%u memory=%u\n",
                (unsigned)profile, (unsigned)width, (unsigned)direction,
                (unsigned)counts[count_index], (unsigned)carry, (unsigned)memory);
            return LIB_FALSE;
        }
        if (!memory && profile != 1u && profile != 4u) {
            core_machine_cpu_timing_result timing;
            const lib_u64 expected_ticks = profile == 0u ?
                8u + 4u * count : 5u + count;
            if (!core_machine_cpu_timing_select(&state.execution, &timing) ||
                    timing.source_timing_unallocated || timing.ticks != expected_ticks)
                return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool rotate_test_sar_extremes(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 counts[] = {0u, 1u, 7u, 8u, 15u, 16u, 31u, 32u, 33u, 255u};
    lib_u8 profile, width_index, count_index, negative, memory;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (width_index = 0u; width_index < (profile == 4u ? 3u : 2u); ++width_index)
    for (count_index = 0u; count_index < sizeof(counts); ++count_index)
    for (negative = 0u; negative < 2u; ++negative)
    for (memory = 0u; memory < 2u; ++memory) {
        const lib_u8 width = width_index == 0u ? 8u : width_index == 1u ? 16u : 32u;
        const lib_u8 count = profile < 2u ? counts[count_index] :
            (lib_u8)(counts[count_index] & 0x1fu);
        const lib_u32 mask = rotate_mask(width);
        const lib_u32 sign = UINT32_C(1) << (width - 1u);
        const lib_u32 initial = negative ? mask - 2u : sign - 3u;
        /* Division with explicit floor is independent of the shift loop. */
        const lib_i64 value = negative ? (lib_i64)initial - (INT64_C(1) << width) : initial;
        const lib_i64 divisor = INT64_C(1) << (count < width ? count : width);
        const lib_i64 quotient = value / divisor - (value < 0 && value % divisor != 0);
        const lib_u32 expected = (lib_u32)quotient & mask;
        const lib_u32 result_flags = shift_parity(expected) |
            (expected & sign ? VCPU_EFLAGS_SF : 0u) |
            (expected == 0u ? VCPU_EFLAGS_ZF : 0u);
        const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_IF;
        lib_u8 code[6] = {0};
        lib_u8 bytes = 0u;
        lib_u32 observed = 0u;
        cpu_instruction_fixture state;
        t_cpu after;

        if (width == 32u) code[bytes++] = 0x66u;
        code[bytes++] = width == 8u ? 0xd2u : 0xd3u;
        code[bytes++] = memory ? 0x3eu : 0xf8u;
        if (memory) { code[bytes++] = 0u; code[bytes++] = 0x40u; }
        cpu_instruction_prepare(&state, profiles[profile]);
        state.cpu.data.eax = initial;
        state.cpu.data.ecx = counts[count_index];
        state.cpu.data.eflags = flags;
        if (memory && cpu_instruction_write(&state, 0x4000u, &initial,
                width / 8u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK)
            return LIB_FALSE;
        if (cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
                state.fault.valid) return LIB_FALSE;
        if (memory) {
            if (cpu_instruction_read(&state, 0x4000u, &observed, width / 8u,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                    LIB_STATUS_OK) return LIB_FALSE;
        } else observed = after.data.eax;
        if ((observed & mask) != expected || state.instructions.data.opr2 != count ||
                after.data.eip != bytes || after.data.ecx != counts[count_index] ||
                !(after.data.eflags & VCPU_EFLAGS_IF) ||
                (count == 0u && (after.data.eflags != flags ||
                    (state.instructions.data.udf & VCPU_EFLAGS_OF))) ||
                (count == 1u && (after.data.eflags & VCPU_EFLAGS_OF)) ||
                (count != 0u && (after.data.eflags &
                    (VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)) != result_flags) ||
                (count != 0u && (after.data.eflags & VCPU_EFLAGS_CF) !=
                    (((count > width ? negative :
                        (initial >> (count - 1u)) & 1u)) ? VCPU_EFLAGS_CF : 0u)) ||
                (memory && after.data.eax != initial)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 main(void)
{
    if (!rotate_test_carry_ring_boundaries()) return 1;
    if (!rotate_test_sar_extremes()) return 1;
    if (!rotate_test_forms()) { lib_c_fprintf(lib_c_stderr, "forms\n"); return 1; }
    if (!rotate_test_count_zero()) { lib_c_fprintf(lib_c_stderr, "zero\n"); return 1; }
    if (!rotate_test_non_one()) { lib_c_fprintf(lib_c_stderr, "non-one\n"); return 1; }
    if (!rotate_test_cl_count_profile_matrix()) { lib_c_fprintf(lib_c_stderr, "cl matrix\n"); return 1; }
    if (!rotate_test_shift_forms()) { lib_c_fprintf(lib_c_stderr, "shift forms\n"); return 1; }
    if (!rotate_test_shift_boundaries()) { lib_c_fprintf(lib_c_stderr, "shift boundaries\n"); return 1; }
    if (!rotate_test_shift_profile()) { lib_c_fprintf(lib_c_stderr, "shift profile\n"); return 1; }
    if (!rotate_test_8086_immediate_rejection()) { lib_c_fprintf(lib_c_stderr, "8086\n"); return 1; }
    if (!rotate_test_80186_immediate_extensions()) { lib_c_fprintf(lib_c_stderr, "80186\n"); return 1; }
    if (!rotate_test_profile()) { lib_c_fprintf(lib_c_stderr, "profile\n"); return 1; }
    lib_c_printf("ROTATE:OK\n");
    lib_c_printf("SHIFT:OK\n");
    lib_c_printf("GROUP2-CL-PROFILES:OK\n");
    lib_c_printf("GROUP2-IMMEDIATE-PROFILES:OK\n");
    return 0;
}

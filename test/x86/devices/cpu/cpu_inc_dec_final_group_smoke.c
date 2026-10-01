#include "lib/types/types_interface.h"
#include <stdio.h>
#include "support/cpu_instruction_fixture.h"
/* T337_REAL_UD_TERMINAL_CPU_OWNER: original invalid forms stay CPU-owned. */

#define INC_DEC_MEMORY 0x5000u
#define OR_DEFINED_FLAGS (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF)
#define AND_DEFINED_FLAGS (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF)
#define ADD_DEFINED_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)

typedef cpu_instruction_fixture inc_dec_machine;

static lib_i32 inc_dec_prepare(core_machine_cpu_profile profile,
    inc_dec_machine *state)
{
    cpu_instruction_prepare(state, profile);
    return 1;
}

static lib_i32 inc_dec_run(inc_dec_machine *state, const lib_u8 *code,
    lib_size bytes, lib_i32 fault, t_cpu *out,
    core_machine_cpu_diagnostic *diagnostic)
{
    lib_status status;

    if (fault) state->cpu.data.idtr.limit = 0x17u;
    status = cpu_instruction_run(state, code, (lib_u8)bytes, out);
    *diagnostic = (core_machine_cpu_diagnostic){0};
    diagnostic->first_fault = state->fault;
    return status == (fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
}

static lib_i32 inc_dec_run_xlat_es(inc_dec_machine *state, const lib_u8 *code,
    t_cpu *out, core_machine_cpu_diagnostic *diagnostic)
{
    if (core_machine_cpu_execution_load_segment(&state->execution,
        &state->cpu.data.es, 0x10u) != LIB_STATUS_OK) return 0;
    return inc_dec_run(state, code, 2u, 0, out, diagnostic);
}

static lib_i32 inc_dec_test_or_rm_reg_forms(void)
{
    static const lib_u8 forms[][6] = {
        { 0x08u, 0xd1u },
        { 0x09u, 0xd1u },
        { 0x66u, 0x09u, 0xd1u },
        { 0x0au, 0xd1u },
        { 0x0bu, 0xd1u },
        { 0x66u, 0x0bu, 0xd1u },
        { 0x08u, 0x16u, 0u, 0x50u },
        { 0x09u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x09u, 0x16u, 0u, 0x50u },
        { 0x0au, 0x16u, 0u, 0x50u },
        { 0x0bu, 0x16u, 0u, 0x50u },
        { 0x66u, 0x0bu, 0x16u, 0u, 0x50u }
    };
    static const lib_u8 lengths[] = {
        2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u, 5u, 4u, 4u, 5u
    };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u || form == 3u || form == 6u ||
            form == 9u ? 1u : (form == 1u || form == 4u || form == 7u ||
                form == 10u ? 2u : 4u);
        const lib_i32 memory = form >= 6u;
        const lib_i32 register_destination = !memory && form >= 3u;
        const lib_i32 memory_source = memory && form >= 9u;
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 destination = bytes == 1u ? 0x11223380u :
            (bytes == 2u ? 0x11228000u : 0x80000000u);
        const lib_u32 expected = destination | 1u;
        const lib_u32 flags = VCPU_EFLAGS_SF |
            (bytes == 1u ? VCPU_EFLAGS_PF : 0u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.ecx = register_destination || memory_source ?
                1u : destination;
            state.cpu.data.edx = register_destination || memory_source ?
                destination : 1u;
            state.cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            if (memory) {
                const lib_u32 value = memory_source ? 1u : destination;
                failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
                    &value, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    flags;
            if (memory) {
                failed |= cpu_instruction_read(&state, INC_DEC_MEMORY,
                    &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                    observed != (memory_source ? 1u : (expected & mask));
                failed |= after.data.edx != (memory_source ? expected : 1u);
            } else if (register_destination) {
                failed |= after.data.edx != expected || after.data.ecx != 1u;
            } else {
                failed |= after.data.ecx != expected || after.data.edx != 1u;
            }
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_or_immediate_forms(void)
{
    static const lib_u8 forms[][9] = {
        { 0x0cu, 1u },
        { 0x0du, 1u, 0x80u },
        { 0x66u, 0x0du, 1u, 0u, 0u, 0x80u },
        { 0x80u, 0xc9u, 1u },
        { 0x81u, 0xc9u, 1u, 0x80u },
        { 0x66u, 0x81u, 0xc9u, 1u, 0u, 0u, 0x80u },
        { 0x83u, 0xc9u, 0xffu },
        { 0x66u, 0x83u, 0xc9u, 0xffu },
        { 0x80u, 0x0eu, 0u, 0x50u, 1u },
        { 0x81u, 0x0eu, 0u, 0x50u, 1u, 0x80u },
        { 0x66u, 0x81u, 0x0eu, 0u, 0x50u, 1u, 0u, 0u, 0x80u },
        { 0x83u, 0x0eu, 0u, 0x50u, 0xffu },
        { 0x66u, 0x83u, 0x0eu, 0u, 0x50u, 0xffu }
    };
    static const lib_u8 lengths[] = {
        2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u, 5u, 6u, 9u, 5u, 6u
    };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u || form == 3u || form == 8u ? 1u :
            (form == 1u || form == 4u || form == 6u || form == 9u ||
                form == 11u ? 2u : 4u);
        const lib_i32 memory = form >= 8u;
        const lib_i32 accumulator = form < 3u;
        const lib_i32 signed_immediate = form == 6u || form == 7u ||
            form == 11u || form == 12u;
        const lib_u32 destination = signed_immediate ? 0u :
            (bytes == 1u ? 0x11223380u : (bytes == 2u ? 0x11228000u :
                0x80000000u));
        const lib_u32 expected = signed_immediate ?
            (bytes == 2u ? 0xffffu : 0xffffffffu) : destination | 1u;
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 flags = VCPU_EFLAGS_SF |
            (signed_immediate || bytes == 1u ? VCPU_EFLAGS_PF : 0u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = destination;
            state.cpu.data.ecx = destination;
            state.cpu.data.edx = 0x55667788u;
            state.cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            if (memory) {
                failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
                    &destination, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.edx != 0x55667788u ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    flags;
            if (memory) {
                failed |= cpu_instruction_read(&state, INC_DEC_MEMORY,
                    &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || observed != (expected & mask);
            } else if (accumulator) {
                failed |= after.data.eax != expected;
            } else {
                failed |= after.data.ecx != expected;
            }
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_or_attribute_profile_fault(void)
{
    static const lib_u8 address_code[] = { 0x67u, 0x66u, 0x09u, 0x16u };
    static const lib_u8 rejected[] = { 0x66u, 0x09u, 0xd1u };
    static const lib_u8 legacy[] = { 0x09u, 0xd1u };

    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 value = 0x80000000u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.esi = INC_DEC_MEMORY;
            state.cpu.data.edx = 1u;
            state.cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            failed |= cpu_instruction_write(&state, INC_DEC_MEMORY, &value,
                sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !inc_dec_run(&state, address_code,
                    sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.edx != 1u ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    VCPU_EFLAGS_SF ||
                cpu_instruction_read(&state, INC_DEC_MEMORY, &value,
                    sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || value != 0x80000001u;
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

        if (!failed) {
            state.cpu.data.ecx = 0x11228000u;
            state.cpu.data.edx = 1u;
            state.cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !X86_CPU_BIT_IS_SET(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.ecx != 0x11228000u || after.data.edx != 1u ||
                after.data.eflags != (VCPU_EFLAGS_AF | VCPU_EFLAGS_CF |
                    VCPU_EFLAGS_OF) || after.data.eip != 0u;
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);

        if (!failed) {
            state.cpu.data.ecx = 0xaabb8000u;
            state.cpu.data.edx = 1u;
            state.cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != 0xaabb8001u || after.data.edx != 1u ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    VCPU_EFLAGS_SF;
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_and_forms(void)
{
    static const lib_u8 forms[][6] = {
        { 0x20u, 0xd1u }, { 0x21u, 0xd1u }, { 0x66u, 0x21u, 0xd1u },
        { 0x22u, 0xd1u }, { 0x23u, 0xd1u }, { 0x66u, 0x23u, 0xd1u },
        { 0x20u, 0x16u, 0u, 0x50u }, { 0x21u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x21u, 0x16u, 0u, 0x50u }, { 0x22u, 0x16u, 0u, 0x50u },
        { 0x23u, 0x16u, 0u, 0x50u }, { 0x66u, 0x23u, 0x16u, 0u, 0x50u }
    };
    static const lib_u8 lengths[] = { 2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u,
        5u, 4u, 4u, 5u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u || form == 3u || form == 6u ||
            form == 9u ? 1u : (form == 1u || form == 4u || form == 7u ||
                form == 10u ? 2u : 4u);
        const lib_i32 memory = form >= 6u;
        const lib_i32 register_destination = !memory && form >= 3u;
        const lib_i32 memory_source = memory && form >= 9u;
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 destination = bytes == 1u ? 0x11223381u :
            (bytes == 2u ? 0x11228081u : 0x80000081u);
        const lib_u32 result = destination & mask;
        const lib_u32 expected = (destination & ~mask) | result;
        inc_dec_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.ecx = register_destination || memory_source ?
                mask : destination;
            state.cpu.data.edx = register_destination || memory_source ?
                destination : mask;
            state.cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            if (memory) {
                const lib_u32 value = memory_source ? mask : destination;
                failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
                    &value, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & AND_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= cpu_instruction_read(&state, INC_DEC_MEMORY,
                    &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                    observed != (memory_source ? mask : result);
                failed |= after.data.edx != (memory_source ? expected : mask);
            } else if (register_destination) {
                failed |= after.data.edx != expected || after.data.ecx != mask;
            } else {
                failed |= after.data.ecx != expected || after.data.edx != mask;
            }
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_and_immediate(void)
{
    static const lib_u8 forms[][9] = {
        { 0x24u, 0xffu }, { 0x25u, 0xffu, 0xffu },
        { 0x66u, 0x25u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x80u, 0xe1u, 0xffu }, { 0x81u, 0xe1u, 0xffu, 0xffu },
        { 0x66u, 0x81u, 0xe1u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x83u, 0xe1u, 0xffu }, { 0x66u, 0x83u, 0xe1u, 0xffu },
        { 0x80u, 0x26u, 0u, 0x50u, 0xffu },
        { 0x81u, 0x26u, 0u, 0x50u, 0xffu, 0xffu },
        { 0x66u, 0x81u, 0x26u, 0u, 0x50u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x83u, 0x26u, 0u, 0x50u, 0xffu },
        { 0x66u, 0x83u, 0x26u, 0u, 0x50u, 0xffu }
    };
    static const lib_u8 lengths[] = { 2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u,
        5u, 6u, 9u, 5u, 6u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u || form == 3u || form == 8u ? 1u :
            (form == 1u || form == 4u || form == 6u || form == 9u ||
                form == 11u ? 2u : 4u);
        const lib_i32 memory = form >= 8u;
        const lib_i32 accumulator = form < 3u;
        const lib_u32 destination = bytes == 1u ? 0x11223381u :
            (bytes == 2u ? 0x11228081u : 0x80000081u);
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 result = destination & mask;
        const lib_u32 expected = (destination & ~mask) | result;
        inc_dec_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = destination;
            state.cpu.data.ecx = destination;
            state.cpu.data.edx = 0x55667788u;
            state.cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            if (memory) failed |= cpu_instruction_write(&state,
                INC_DEC_MEMORY, &destination, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.edx != 0x55667788u ||
                (after.data.eflags & AND_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
            if (memory) failed |= cpu_instruction_read(&state,
                INC_DEC_MEMORY, &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                observed != result;
            else if (accumulator) failed |= after.data.eax != expected;
            else failed |= after.data.ecx != expected;
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_and_attribute_profile_fault(void)
{
    static const lib_u8 address_code[] = { 0x67u, 0x66u, 0x21u, 0x16u };
    static const lib_u8 rejected[] = { 0x66u, 0x21u, 0xd1u };
    static const lib_u8 legacy[] = { 0x21u, 0xd1u };

    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 value = 0x80000081u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            state.cpu.data.esi = INC_DEC_MEMORY;
            state.cpu.data.edx = 0xffffffffu;
            state.cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            failed |= cpu_instruction_write(&state, INC_DEC_MEMORY, &value,
                sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !inc_dec_run(&state, address_code,
                    sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.edx != 0xffffffffu ||
                (after.data.eflags & AND_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF) ||
                cpu_instruction_read(&state, INC_DEC_MEMORY, &value,
                    sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || value != 0x80000081u;
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        const lib_u32 flags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
        if (!failed) {
            state.cpu.data.ecx = 0x11228081u;
            state.cpu.data.edx = 0xffffu;
            state.cpu.data.eflags = flags;
            failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !X86_CPU_BIT_IS_SET(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.ecx != 0x11228081u || after.data.edx != 0xffffu ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
        if (!failed) {
            state.cpu.data.ecx = 0xaabb8081u;
            state.cpu.data.edx = 0xffffu;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != 0xaabb8081u || after.data.edx != 0xffffu ||
                (after.data.eflags & AND_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_and_zero_flags(void)
{
    static const lib_u8 forms[][6] = {
        { 0x24u, 0u },
        { 0x25u, 0u, 0u },
        { 0x66u, 0x25u, 0u, 0u, 0u, 0u }
    };
    static const lib_u8 lengths[] = { 2u, 3u, 6u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = 0xffffffffu;
            state.cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & AND_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF) ||
                (form == 0u ? after.data.eax != 0xffffff00u :
                    (form == 1u ? after.data.eax != 0xffff0000u :
                        after.data.eax != 0u));
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_sub_forms(void)
{
    static const lib_u8 forms[][9] = {
        { 0x28u, 0xd1u }, { 0x29u, 0xd1u }, { 0x66u, 0x29u, 0xd1u },
        { 0x2au, 0xd1u }, { 0x2bu, 0xd1u }, { 0x66u, 0x2bu, 0xd1u },
        { 0x28u, 0x16u, 0u, 0x50u }, { 0x29u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x29u, 0x16u, 0u, 0x50u }, { 0x2au, 0x16u, 0u, 0x50u },
        { 0x2bu, 0x16u, 0u, 0x50u }, { 0x66u, 0x2bu, 0x16u, 0u, 0x50u },
        { 0x2cu, 1u }, { 0x2du, 1u, 0u }, { 0x66u, 0x2du, 1u, 0u, 0u, 0u },
        { 0x80u, 0xe9u, 1u }, { 0x81u, 0xe9u, 1u, 0u },
        { 0x66u, 0x81u, 0xe9u, 1u, 0u, 0u, 0u }, { 0x83u, 0xe9u, 1u },
        { 0x66u, 0x83u, 0xe9u, 1u }, { 0x80u, 0x2eu, 0u, 0x50u, 1u },
        { 0x81u, 0x2eu, 0u, 0x50u, 1u, 0u },
        { 0x66u, 0x81u, 0x2eu, 0u, 0x50u, 1u, 0u, 0u, 0u },
        { 0x83u, 0x2eu, 0u, 0x50u, 1u }, { 0x66u, 0x83u, 0x2eu, 0u, 0x50u, 1u }
    };
    static const lib_u8 lengths[] = { 2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u, 5u, 4u,
        4u, 5u, 2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u, 5u, 6u, 9u, 5u, 6u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = (form == 0u || form == 3u || form == 6u ||
            form == 9u || form == 12u || form == 15u || form == 20u) ? 1u :
            ((form == 1u || form == 4u || form == 7u || form == 10u ||
              form == 13u || form == 16u || form == 18u || form == 21u ||
              form == 23u) ? 2u : 4u);
        const lib_i32 memory = (form >= 6u && form < 12u) || form >= 20u;
        const lib_i32 register_destination = (form >= 3u && form < 6u) ||
            (form >= 9u && form < 12u);
        const lib_i32 memory_source = form >= 9u && form < 12u;
        const lib_i32 accumulator = form >= 12u && form < 15u;
        const lib_i32 immediate = form >= 12u;
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = 0u;
            state.cpu.data.ecx = register_destination || memory_source ?
                1u : 0u;
            state.cpu.data.edx = register_destination || memory_source ?
                0u : 1u;
            if (memory) {
                const lib_u32 value = memory_source ? 1u : 0u;
                failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
                    &value, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= cpu_instruction_read(&state, INC_DEC_MEMORY,
                    &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                    observed != (memory_source ? 1u : mask) ||
                    after.data.edx != (memory_source ? mask : 1u);
            } else if (accumulator) {
                failed |= (after.data.eax & mask) != mask;
            } else if (immediate || !register_destination) {
                failed |= (after.data.ecx & mask) != mask ||
                    (!immediate && after.data.edx != 1u);
            } else {
                failed |= (after.data.edx & mask) != mask || after.data.ecx != 1u;
            }
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_sub_boundaries(void)
{
    static const lib_u8 overflow[][6] = {
        { 0x2cu, 1u }, { 0x2du, 1u, 0u }, { 0x66u, 0x2du, 1u, 0u, 0u, 0u }
    };
    static const lib_u8 lengths[] = { 2u, 3u, 6u };
    static const lib_u8 signext[][4] = {
        { 0x80u, 0xe9u, 0xffu }, { 0x83u, 0xe9u, 0xffu },
        { 0x66u, 0x83u, 0xe9u, 0xffu }
    };
    static const lib_u8 signlengths[] = { 3u, 3u, 4u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u ? 1u : (form == 1u ? 2u : 4u);
        const lib_u32 initial = bytes == 1u ? 0x11223380u :
            (bytes == 2u ? 0x11228000u : 0x80000000u);
        const lib_u32 expected = bytes == 1u ? 0x1122337fu :
            (bytes == 2u ? 0x11227fffu : 0x7fffffffu);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = initial;
            failed |= !inc_dec_run(&state, overflow[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != expected ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
                        (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        }
                if (failed) return 0;
    }
    for (form = 0u; form != sizeof(signlengths); ++form) {
        const lib_u32 expected = form == 2u ? 1u : 0xaabb0001u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.ecx = form == 2u ? 0u : 0xaabb0000u;
            state.cpu.data.edx = 0x55667788u;
            failed |= !inc_dec_run(&state, signext[form], signlengths[form], 0,
                &after, &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != expected || after.data.edx != 0x55667788u ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_AF);
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_sub_attribute_profile_fault(void)
{
    static const lib_u8 address_code[] = { 0x67u, 0x66u, 0x29u, 0x16u };
    static const lib_u8 rejected[] = { 0x66u, 0x29u, 0xd1u };
    static const lib_u8 legacy[] = { 0x29u, 0xd1u };
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 value = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            state.cpu.data.esi = INC_DEC_MEMORY;
            state.cpu.data.edx = 1u;
            failed |= cpu_instruction_write(&state, INC_DEC_MEMORY, &value,
                sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !inc_dec_run(&state, address_code,
                    sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.edx != 1u ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF) ||
                cpu_instruction_read(&state, INC_DEC_MEMORY, &value,
                    sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || value != 0xffffffffu;
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        const lib_u32 flags = VCPU_EFLAGS_CF;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
        if (!failed) {
            state.cpu.data.ecx = 0x11220000u;
            state.cpu.data.edx = 1u;
            state.cpu.data.eflags = flags;
            failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !X86_CPU_BIT_IS_SET(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.ecx != 0x11220000u || after.data.edx != 1u ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
        if (!failed) {
            state.cpu.data.ecx = 0xaabb0000u;
            state.cpu.data.edx = 1u;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != 0xaabbffffu || after.data.edx != 1u;
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_xor_forms(void)
{
    static const lib_u8 forms[][9] = {
        { 0x30u, 0xd1u }, { 0x31u, 0xd1u }, { 0x66u, 0x31u, 0xd1u },
        { 0x32u, 0xd1u }, { 0x33u, 0xd1u }, { 0x66u, 0x33u, 0xd1u },
        { 0x30u, 0x16u, 0u, 0x50u }, { 0x31u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x31u, 0x16u, 0u, 0x50u }, { 0x32u, 0x16u, 0u, 0x50u },
        { 0x33u, 0x16u, 0u, 0x50u }, { 0x66u, 0x33u, 0x16u, 0u, 0x50u }
    };
    static const lib_u8 lengths[] = { 2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u, 5u,
        4u, 4u, 5u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u || form == 3u || form == 6u || form == 9u ?
            1u : (form == 1u || form == 4u || form == 7u || form == 10u ? 2u : 4u);
        const lib_i32 memory = form >= 6u;
        const lib_i32 register_destination = !memory && form >= 3u;
        const lib_i32 memory_source = memory && form >= 9u;
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 value = bytes == 1u ? 0x11223381u :
            (bytes == 2u ? 0x11228081u : 0x80000081u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.ecx = value;
            state.cpu.data.edx = value;
            state.cpu.data.eflags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF |
                VCPU_EFLAGS_OF;
            if (memory) {
                failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
                    &value, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= cpu_instruction_read(&state, INC_DEC_MEMORY, &observed,
                    bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || observed != (memory_source ? (value & mask) : 0u) ||
                    (memory_source ? (after.data.edx & mask) : after.data.edx) !=
                        (memory_source ? 0u : value);
            } else if (register_destination) {
                failed |= (after.data.edx & mask) != 0u || after.data.ecx != value;
            } else {
                failed |= (after.data.ecx & mask) != 0u || after.data.edx != value;
            }
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_xor_immediates(void)
{
    static const lib_u8 forms[][9] = {
        { 0x34u, 0xffu }, { 0x35u, 0xffu, 0xffu },
        { 0x66u, 0x35u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x80u, 0xf1u, 0xffu }, { 0x81u, 0xf1u, 0xffu, 0xffu },
        { 0x66u, 0x81u, 0xf1u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x83u, 0xf1u, 0xffu }, { 0x66u, 0x83u, 0xf1u, 0xffu },
        { 0x80u, 0x36u, 0u, 0x50u, 0xffu },
        { 0x81u, 0x36u, 0u, 0x50u, 0xffu, 0xffu },
        { 0x66u, 0x81u, 0x36u, 0u, 0x50u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x83u, 0x36u, 0u, 0x50u, 0xffu },
        { 0x66u, 0x83u, 0x36u, 0u, 0x50u, 0xffu }
    };
    static const lib_u8 lengths[] = { 2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u,
        5u, 6u, 9u, 5u, 6u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u || form == 3u || form == 8u ? 1u :
            (form == 1u || form == 4u || form == 6u || form == 9u ||
                form == 11u ? 2u : 4u);
        const lib_i32 memory = form >= 8u;
        const lib_i32 accumulator = form < 3u;
        const lib_u32 value = bytes == 1u ? 0x112233ffu :
            (bytes == 2u ? 0x1122ffffu : 0xffffffffu);
        inc_dec_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            state.cpu.data.eax = value;
            state.cpu.data.ecx = value;
            state.cpu.data.edx = 0x55667788u;
            state.cpu.data.eflags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF |
                VCPU_EFLAGS_OF;
            if (memory) failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
                &value, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.edx != 0x55667788u ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= cpu_instruction_read(&state, INC_DEC_MEMORY,
                    &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || observed != 0u;
            } else if (accumulator) {
                failed |= (after.data.eax & (bytes == 1u ? 0xffu :
                    (bytes == 2u ? 0xffffu : 0xffffffffu))) != 0u;
            } else {
                failed |= (after.data.ecx & (bytes == 1u ? 0xffu :
                    (bytes == 2u ? 0xffffu : 0xffffffffu))) != 0u;
            }
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_xor_attribute_profile_fault(void)
{
    static const lib_u8 address_code[] = { 0x67u, 0x66u, 0x31u, 0x16u };
    static const lib_u8 rejected[] = { 0x66u, 0x31u, 0xd1u };
    static const lib_u8 legacy[] = { 0x31u, 0xd1u };

    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 value = 0x80000081u;
        const lib_u32 flags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.esi = INC_DEC_MEMORY;
            state.cpu.data.edx = 0xffffffffu;
            state.cpu.data.eflags = flags;
            failed |= cpu_instruction_write(&state, INC_DEC_MEMORY, &value,
                sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !inc_dec_run(&state, address_code,
                    sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.edx != 0xffffffffu ||
                (after.data.eflags & OR_DEFINED_FLAGS) != VCPU_EFLAGS_PF ||
                cpu_instruction_read(&state, INC_DEC_MEMORY, &value,
                    sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || value != 0x7fffff7eu;
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        const lib_u32 flags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

        if (!failed) {
            state.cpu.data.ecx = 0x11228081u;
            state.cpu.data.edx = 0xffffu;
            state.cpu.data.eflags = flags;
            failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !X86_CPU_BIT_IS_SET(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.ecx != 0x11228081u || after.data.edx != 0xffffu ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);

        if (!failed) {
            state.cpu.data.ecx = 0xaabb8081u;
            state.cpu.data.edx = 0xffffu;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != 0xaabb7f7eu || after.data.edx != 0xffffu;
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_cmp_forms(void)
{
    static const lib_u8 forms[][9] = {
        { 0x38u, 0xd1u }, { 0x39u, 0xd1u }, { 0x66u, 0x39u, 0xd1u },
        { 0x3au, 0xd1u }, { 0x3bu, 0xd1u }, { 0x66u, 0x3bu, 0xd1u },
        { 0x38u, 0x16u, 0u, 0x50u }, { 0x39u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x39u, 0x16u, 0u, 0x50u }, { 0x3au, 0x16u, 0u, 0x50u },
        { 0x3bu, 0x16u, 0u, 0x50u }, { 0x66u, 0x3bu, 0x16u, 0u, 0x50u },
        { 0x3cu, 1u }, { 0x3du, 1u, 0u }, { 0x66u, 0x3du, 1u, 0u, 0u, 0u },
        { 0x80u, 0xf9u, 1u }, { 0x81u, 0xf9u, 1u, 0u },
        { 0x66u, 0x81u, 0xf9u, 1u, 0u, 0u, 0u }, { 0x83u, 0xf9u, 1u },
        { 0x66u, 0x83u, 0xf9u, 1u }, { 0x80u, 0x3eu, 0u, 0x50u, 1u },
        { 0x81u, 0x3eu, 0u, 0x50u, 1u, 0u },
        { 0x66u, 0x81u, 0x3eu, 0u, 0x50u, 1u, 0u, 0u, 0u },
        { 0x83u, 0x3eu, 0u, 0x50u, 1u }, { 0x66u, 0x83u, 0x3eu, 0u, 0x50u, 1u }
    };
    static const lib_u8 lengths[] = { 2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u, 5u, 4u,
        4u, 5u, 2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u, 5u, 6u, 9u, 5u, 6u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = (form == 0u || form == 3u || form == 6u ||
            form == 9u || form == 12u || form == 15u || form == 20u) ? 1u :
            ((form == 1u || form == 4u || form == 7u || form == 10u ||
              form == 13u || form == 16u || form == 18u || form == 21u ||
              form == 23u) ? 2u : 4u);
        const lib_i32 memory = (form >= 6u && form < 12u) || form >= 20u;
        const lib_i32 register_destination = (form >= 3u && form < 6u) ||
            (form >= 9u && form < 12u);
        const lib_i32 memory_source = form >= 9u && form < 12u;
        const lib_i32 accumulator = form >= 12u && form < 15u;
        const lib_i32 immediate = form >= 12u;
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 destination = 0u;
        const lib_u32 source = 1u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = destination;
            state.cpu.data.ecx = register_destination || memory_source ?
                source : destination;
            state.cpu.data.edx = register_destination || memory_source ?
                destination : source;
            if (memory) {
                const lib_u32 value = memory_source ? source : destination;
                failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
                    &value, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= cpu_instruction_read(&state, INC_DEC_MEMORY,
                    &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                    observed != (memory_source ? source : destination) ||
                    after.data.edx != (memory_source ? destination : source);
            } else if (accumulator) {
                failed |= (after.data.eax & mask) != destination;
            } else if (immediate || !register_destination) {
                failed |= (after.data.ecx & mask) != destination ||
                    (!immediate && after.data.edx != source);
            } else {
                failed |= (after.data.edx & mask) != destination || after.data.ecx != source;
            }
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_cmp_boundaries(void)
{
    static const lib_u8 overflow[][6] = {
        { 0x3cu, 1u }, { 0x3du, 1u, 0u }, { 0x66u, 0x3du, 1u, 0u, 0u, 0u }
    };
    static const lib_u8 lengths[] = { 2u, 3u, 6u };
    static const lib_u8 signext[][4] = {
        { 0x80u, 0xf9u, 0xffu }, { 0x83u, 0xf9u, 0xffu },
        { 0x66u, 0x83u, 0xf9u, 0xffu }
    };
    static const lib_u8 signlengths[] = { 3u, 3u, 4u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u ? 1u : (form == 1u ? 2u : 4u);
        const lib_u32 initial = bytes == 1u ? 0x11223380u :
            (bytes == 2u ? 0x11228000u : 0x80000000u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = initial;
            failed |= !inc_dec_run(&state, overflow[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != initial ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
                        (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        }
                if (failed) return 0;
    }
    for (form = 0u; form != sizeof(signlengths); ++form) {
        const lib_u32 initial = form == 2u ? 0u : 0xaabb0000u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.ecx = initial;
            state.cpu.data.edx = 0x55667788u;
            failed |= !inc_dec_run(&state, signext[form], signlengths[form], 0,
                &after, &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != initial || after.data.edx != 0x55667788u ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_AF);
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_cmp_attribute_profile_fault(void)
{
    static const lib_u8 address_code[] = { 0x67u, 0x66u, 0x39u, 0x16u };
    static const lib_u8 rejected[] = { 0x66u, 0x39u, 0xd1u };
    static const lib_u8 legacy[] = { 0x39u, 0xd1u };

    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 value = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.esi = INC_DEC_MEMORY;
            state.cpu.data.edx = 1u;
            failed |= cpu_instruction_write(&state, INC_DEC_MEMORY, &value,
                sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !inc_dec_run(&state, address_code,
                sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.edx != 1u ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF) ||
                cpu_instruction_read(&state, INC_DEC_MEMORY, &value,
                    sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || value != 0u;
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        const lib_u32 flags = VCPU_EFLAGS_CF;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

        if (!failed) {
            state.cpu.data.ecx = 0x11220000u;
            state.cpu.data.edx = 1u;
            state.cpu.data.eflags = flags;
            failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !X86_CPU_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.ecx != 0x11220000u || after.data.edx != 1u ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);

        if (!failed) {
            state.cpu.data.ecx = 0u;
            state.cpu.data.edx = 1u;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.ecx != 0u ||
                after.data.edx != 1u;
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_decimal_adjust(void)
{
    static const lib_u8 code[][2] = {
        { 0x27u, 0u }, { 0x27u, 0u }, { 0x2fu, 0u },
        { 0x37u, 0u }, { 0x3fu, 0u }, { 0xd4u, 0x10u }, { 0xd5u, 0x10u }
    };
    static const lib_u8 lengths[] = { 1u, 1u, 1u, 1u, 1u, 2u, 2u };
    static const lib_u32 eax[] = {
        0x1122330au, 0x1122339au, 0x11223300u, 0x11220a0au,
        0x11220a0au, 0x1122002fu, 0x1122020fu
    };
    static const lib_u32 input_flags[] = { 0u, 0u, VCPU_EFLAGS_AF, 0u, 0u, 0u, 0u };
    static const lib_u32 result_eax[] = {
        0x11223310u, 0x11223300u, 0x1122339au, 0x11220b00u,
        0x11220904u, 0x1122020fu, 0x1122002fu
    };
    static const lib_u32 flag_masks[] = {
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF, VCPU_EFLAGS_CF | VCPU_EFLAGS_AF,
        VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF
    };
    static const lib_u32 expected_flags[] = {
        VCPU_EFLAGS_AF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF, VCPU_EFLAGS_CF | VCPU_EFLAGS_AF,
        VCPU_EFLAGS_PF, 0u
    };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(form == 6u ? CORE_MACHINE_CPU_PROFILE_80186 :
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = eax[form];
            state.cpu.data.eflags = input_flags[form];
            failed |= !inc_dec_run(&state, code[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != result_eax[form] ||
                (after.data.eflags & flag_masks[form]) != expected_flags[form];
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_xlat(void)
{
    static const lib_u8 code[][2] = { { 0xd7u, 0u }, { 0x67u, 0xd7u }, { 0x26u, 0xd7u } };
    lib_u8 form;
    for (form = 0u; form != 3u; ++form) {
        const lib_u32 base = form == 1u ? 0x00010000u : 0xaabb0010u;
        const lib_u8 value = form == 1u ? 0x5au : (form == 2u ? 0x3cu : 0xa5u);
        inc_dec_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(form == 0u ? CORE_MACHINE_CPU_PROFILE_80186 : CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.ebx = base;
            state.cpu.data.eax = 0x11223304u;
            state.cpu.data.ecx = 0x55667788u;
            state.cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= cpu_instruction_write(&state, form == 1u ? 0x10004u :
                (form == 2u ? 0x114u : 0x14u), &value, 1, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            if (form == 2u)
                failed |= !inc_dec_run_xlat_es(&state, code[form], &after, &diagnostic);
            else
                failed |= !inc_dec_run(&state, code[form], form == 0u ? 1u : 2u,
                    0, &after, &diagnostic);
            failed |= diagnostic.first_fault.valid ||
                after.data.eax != (0x11223300u | value) || after.data.ebx != base ||
                after.data.ecx != 0x55667788u || after.data.eflags != VCPU_EFLAGS_CF;
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_group1_profile_matrix(void)
{
    static const lib_u8 opcodes[] = { 0x80u, 0x81u, 0x82u, 0x83u };
    core_machine_cpu_profile profile;
    lib_u8 opcode_index;
    lib_u8 selector;

    for (profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        for (opcode_index = 0u; opcode_index != sizeof(opcodes); ++opcode_index) {
            const lib_u8 opcode = opcodes[opcode_index];
            const lib_u8 operand_bytes = (opcode == 0x80u || opcode == 0x82u) ? 1u : 2u;
            const lib_u8 immediate_bytes = opcode == 0x81u ? 2u : 1u;
            const lib_u32 mask = operand_bytes == 1u ? 0xffu : 0xffffu;

            for (selector = 0u; selector != 8u; ++selector) {
                lib_u8 code[] = { opcode,
                    (lib_u8)(0xc0u | (selector << 3u)), 0xffu, 0xffu };
                const lib_u32 expected = selector == 3u || selector == 5u ?
                    1u : (selector == 4u || selector == 7u ? 0u : mask);
                inc_dec_machine state;
                t_cpu after;
                core_machine_cpu_diagnostic diagnostic;
                lib_i32 failed = !inc_dec_prepare(profile, &state);

                if (!failed) {
                    state.cpu.data.eax = 0u;
                    state.cpu.data.eflags = 0u;
                    failed |= !inc_dec_run(&state, code, (lib_size)(2u + immediate_bytes), 0,
                        &after, &diagnostic) || diagnostic.first_fault.valid ||
                        (after.data.eax & mask) != expected ||
                        after.data.eip != 2u + immediate_bytes;
                }
                                if (failed) return 0;
            }
        }
    }
    return 1;
}
int main(void)
{
    if (!inc_dec_test_or_rm_reg_forms() ||
        !inc_dec_test_or_immediate_forms() ||
        !inc_dec_test_or_attribute_profile_fault() ||
        !inc_dec_test_and_forms() ||
        !inc_dec_test_and_immediate() ||
        !inc_dec_test_and_attribute_profile_fault() ||
        !inc_dec_test_and_zero_flags() ||
        !inc_dec_test_sub_forms() ||
        !inc_dec_test_sub_boundaries() ||
        !inc_dec_test_sub_attribute_profile_fault() ||
        !inc_dec_test_xor_forms() ||
        !inc_dec_test_xor_immediates() ||
        !inc_dec_test_xor_attribute_profile_fault() ||
        !inc_dec_test_cmp_forms() ||
        !inc_dec_test_cmp_boundaries() ||
        !inc_dec_test_cmp_attribute_profile_fault() ||
        !inc_dec_test_decimal_adjust() ||
        !inc_dec_test_xlat() ||
        !inc_dec_test_group1_profile_matrix()) {
        fputs("M5:T539:S35:CPU-LOGICAL-DECIMAL-XLAT:FAIL\n", stderr);
        return 1;
    }
    puts("M5:T316:S11:OR:OK");
    puts("M5:T316:S12:AND:OK");
    puts("M5:T316:S13:SUB:OK");
    puts("M5:T316:S14:XOR:OK");
    puts("M5:T316:S15:CMP:OK");
    puts("M5:T401:S7:GROUP1-PROFILE-MATRIX:OK");
    puts("M5:T316:S16:DECIMAL-ADJUST:OK");
    puts("M5:T316:S17:XLAT:OK");
    puts("M5:T401:S35:XLAT-PROFILES:OK");
    puts("M5:T539:S35:CPU-LOGICAL-DECIMAL-XLAT:OK");
    return 0;
}

#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"
/* REAL_UD_TERMINAL_CPU_OWNER: original invalid forms stay CPU-owned. */

#define INC_DEC_MEMORY 0x5000u
#define TEST_DEFINED_FLAGS (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
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

static lib_i32 inc_dec_test_test_rm_reg_forms(void)
{
    static const lib_u8 forms[][5] = {
        { 0x84u,0xd1u }, { 0x85u,0xd1u }, { 0x66u,0x85u,0xd1u },
        { 0x84u,0x16u,0u,0x50u }, { 0x85u,0x16u,0u,0x50u },
        { 0x66u,0x85u,0x16u,0u,0x50u }
    };
    static const lib_u8 lengths[] = { 2u,2u,3u,4u,4u,5u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u || form == 3u ? 1u :
            (form == 1u || form == 4u ? 2u : 4u);
        const lib_i32 memory = form >= 3u;
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        lib_u8 pass;

        for (pass = 0u; pass != 2u; ++pass) {
            const lib_u32 destination = mask;
            const lib_u32 source = pass ? 0u : (bytes == 1u ? 0x80u :
                (bytes == 2u ? 0x8000u : 0x80000000u));
            const lib_u32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
                VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
            const lib_u32 expected_flags = pass ?
                (VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF) :
                (VCPU_EFLAGS_SF | (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
            inc_dec_machine state;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            lib_u32 observed = 0u;
            lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

            if (!failed) {
                state.cpu.data.ecx = destination;
                state.cpu.data.edx = source;
                state.cpu.data.eflags = initial_flags;
                if (memory) failed |= cpu_instruction_write(&state,
                    INC_DEC_MEMORY, &destination, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
                failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                    &diagnostic) || diagnostic.first_fault.valid ||
                    after.data.ecx != destination || after.data.edx != source ||
                    (after.data.eflags & TEST_DEFINED_FLAGS) != expected_flags;
                if (memory) failed |= cpu_instruction_read(&state,
                    INC_DEC_MEMORY, &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                    observed != (destination & mask);
            }
                        if (failed) return 0;
        }
    }
    return 1;
}

static lib_i32 inc_dec_test_test_rm_reg_attribute_and_profile(void)
{
    static const lib_u8 address_code[] = { 0x67u,0x66u,0x85u,0x16u };
    static const lib_u8 rejected_prefix[] = { 0x66u,0x85u,0xd1u };
    static const lib_u8 accepted_legacy[] = { 0x85u,0xd1u };
    const lib_u32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_u32 destination = 0xffffffffu;
    lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.cpu.data.esi = INC_DEC_MEMORY;
        state.cpu.data.edx = 0x80000000u;
        state.cpu.data.eflags = initial_flags;
        failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
            &destination, sizeof(destination), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            !inc_dec_run(&state, address_code, sizeof(address_code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
            after.data.edx != 0x80000000u ||
            (after.data.eflags & TEST_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF) ||
            cpu_instruction_read(&state, INC_DEC_MEMORY, &destination,
                sizeof(destination), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || destination != 0xffffffffu;
    }
        if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.cpu.data.ecx = 0x1122ffffu;
        state.cpu.data.edx = 0xaabb8000u;
        state.cpu.data.eflags = initial_flags;
        failed |= !cpu_instruction_expect_real_fault(&state, rejected_prefix,
            sizeof(rejected_prefix), 6u);
    }
        if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.cpu.data.ecx = 0xaabbffffu;
        state.cpu.data.edx = 0x11228000u;
        state.cpu.data.eflags = initial_flags;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.ecx != 0xaabbffffu || after.data.edx != 0x11228000u ||
            (after.data.eflags & TEST_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
    }
        return !failed;
}

static lib_i32 inc_dec_test_add_rm_reg_forms(void)
{
    static const lib_u8 forms[][5] = {
        { 0x00u,0xd1u }, { 0x01u,0xd1u }, { 0x66u,0x01u,0xd1u },
        { 0x02u,0xd1u }, { 0x03u,0xd1u }, { 0x66u,0x03u,0xd1u },
        { 0x00u,0x16u,0u,0x50u }, { 0x01u,0x16u,0u,0x50u },
        { 0x66u,0x01u,0x16u,0u,0x50u }, { 0x02u,0x16u,0u,0x50u },
        { 0x03u,0x16u,0u,0x50u }, { 0x66u,0x03u,0x16u,0u,0x50u }
    };
    static const lib_u8 lengths[] = { 2u,2u,3u,2u,2u,3u,4u,4u,5u,4u,4u,5u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u || form == 3u || form == 6u || form == 9u ?
            1u : (form == 1u || form == 4u || form == 7u || form == 10u ? 2u : 4u);
        const lib_i32 memory = form >= 6u;
        const lib_i32 reg_destination = !memory && form >= 3u;
        const lib_i32 memory_source = memory && form >= 9u;
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 destination = bytes == 1u ? 0x112233ffu :
            (bytes == 2u ? 0x1122ffffu : mask);
        const lib_u32 expected = destination & ~mask;
        const lib_u32 source = 1u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.ecx = reg_destination ? source : destination;
            state.cpu.data.edx = (reg_destination || memory_source) ?
                destination : source;
            state.cpu.data.eflags = 0u;
            if (memory) failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
                &(lib_u32){ memory_source ? source : destination }, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= cpu_instruction_read(&state, INC_DEC_MEMORY, &observed,
                    bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || observed != (memory_source ? source : 0u);
                if (memory_source) failed |= (after.data.edx & mask) != 0u;
            } else if (reg_destination) {
                failed |= after.data.edx != expected || after.data.ecx != source;
            } else {
                failed |= after.data.ecx != expected || after.data.edx != source;
            }
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_add_immediate_forms(void)
{
    static const lib_u8 forms[][9] = {
        { 0x04u,1u }, { 0x05u,1u,0u }, { 0x66u,0x05u,1u,0u,0u,0u },
        { 0x80u,0xc1u,1u }, { 0x81u,0xc1u,1u,0u },
        { 0x66u,0x81u,0xc1u,1u,0u,0u,0u }, { 0x83u,0xc1u,0xffu },
        { 0x66u,0x83u,0xc1u,0xffu }, { 0x80u,0x06u,0u,0x50u,1u },
        { 0x81u,0x06u,0u,0x50u,1u,0u },
        { 0x66u,0x81u,0x06u,0u,0x50u,1u,0u,0u,0u },
        { 0x83u,0x06u,0u,0x50u,0xffu },
        { 0x66u,0x83u,0x06u,0u,0x50u,0xffu }
    };
    static const lib_u8 lengths[] = { 2u,3u,6u,3u,4u,7u,3u,4u,5u,6u,9u,5u,6u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u || form == 3u || form == 8u ? 1u :
            (form == 1u || form == 4u || form == 6u || form == 9u || form == 11u ? 2u : 4u);
        const lib_i32 memory = form >= 8u;
        const lib_i32 signed_immediate = form == 6u || form == 7u || form == 11u || form == 12u;
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 destination = signed_immediate ? 0u : mask;
        const lib_u32 expected = signed_immediate ? mask : 0u;
        const lib_u32 flags = signed_immediate ?
            (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF) :
            (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = destination;
            state.cpu.data.ecx = destination;
            if (memory) failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
                &destination, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & ADD_DEFINED_FLAGS) != flags;
            if (memory) failed |= cpu_instruction_read(&state, INC_DEC_MEMORY,
                &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || observed != expected;
            else if (form < 3u) failed |= (after.data.eax & mask) != expected;
            else failed |= (after.data.ecx & mask) != expected;
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_add_signed_overflow(void)
{
    static const lib_u8 forms[][6] = {
        { 0x04u,1u }, { 0x05u,1u,0u }, { 0x66u,0x05u,1u,0u,0u,0u }
    };
    static const lib_u8 lengths[] = { 2u,3u,6u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u ? 1u : (form == 1u ? 2u : 4u);
        const lib_u32 initial = bytes == 1u ? 0x1122337fu :
            (bytes == 2u ? 0x11227fffu : 0x7fffffffu);
        const lib_u32 expected = bytes == 1u ? 0x11223380u :
            (bytes == 2u ? 0x11228000u : 0x80000000u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = initial;
            state.cpu.data.eflags = 0u;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != expected ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                        (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_add_attribute_and_profile(void)
{
    static const lib_u8 address_code[] = { 0x67u,0x66u,0x01u,0x16u };
    static const lib_u8 rejected_prefix[] = { 0x66u,0x01u,0xd1u };
    static const lib_u8 accepted_legacy[] = { 0x01u,0xd1u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_u32 destination = 0xffffffffu;
    lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.cpu.data.esi = INC_DEC_MEMORY;
        state.cpu.data.edx = 1u;
        failed |= cpu_instruction_write(&state, INC_DEC_MEMORY, &destination,
            sizeof(destination), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !inc_dec_run(&state, address_code,
                sizeof(address_code), 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.edx != 1u ||
            (after.data.eflags & ADD_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF) ||
            cpu_instruction_read(&state, INC_DEC_MEMORY, &destination,
                sizeof(destination), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || destination != 0u;
    }
        if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.cpu.data.ecx = 0x1122ffffu;
        state.cpu.data.edx = 1u;
        state.cpu.data.eflags = VCPU_EFLAGS_OF;
        failed |= !cpu_instruction_expect_real_fault(&state, rejected_prefix,
            sizeof(rejected_prefix), 6u);
    }
        if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.cpu.data.ecx = 0xaabbffffu;
        state.cpu.data.edx = 1u;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.ecx != 0xaabb0000u || after.data.edx != 1u ||
            (after.data.eflags & ADD_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    }
        return !failed;
}

static lib_i32 inc_dec_test_adc_forms(void)
{
    static const lib_u8 forms[][9] = {
        { 0x10u,0xd1u }, { 0x11u,0xd1u }, { 0x66u,0x11u,0xd1u },
        { 0x12u,0xd1u }, { 0x13u,0xd1u }, { 0x66u,0x13u,0xd1u },
        { 0x10u,0x16u,0u,0x50u }, { 0x11u,0x16u,0u,0x50u },
        { 0x66u,0x11u,0x16u,0u,0x50u }, { 0x12u,0x16u,0u,0x50u },
        { 0x13u,0x16u,0u,0x50u }, { 0x66u,0x13u,0x16u,0u,0x50u },
        { 0x14u,0u }, { 0x15u,0u,0u }, { 0x66u,0x15u,0u,0u,0u,0u },
        { 0x80u,0xd1u,0u }, { 0x81u,0xd1u,0u,0u },
        { 0x66u,0x81u,0xd1u,0u,0u,0u,0u }, { 0x83u,0xd1u,0u },
        { 0x66u,0x83u,0xd1u,0u }, { 0x80u,0x16u,0u,0x50u,0u },
        { 0x81u,0x16u,0u,0x50u,0u,0u },
        { 0x66u,0x81u,0x16u,0u,0x50u,0u,0u },
        { 0x83u,0x16u,0u,0x50u,0u }, { 0x66u,0x83u,0x16u,0u,0x50u,0u }
    };
    static const lib_u8 lengths[] = { 2u,2u,3u,2u,2u,3u,4u,4u,5u,4u,4u,5u,
        2u,3u,6u,3u,4u,7u,3u,4u,5u,6u,9u,5u,6u };
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
        const lib_u32 destination = bytes == 1u ? 0x112233ffu :
            (bytes == 2u ? 0x1122ffffu : mask);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = destination;
            state.cpu.data.ecx = register_destination ? 0u : destination;
            state.cpu.data.edx = (register_destination || memory_source) ?
                destination : 0u;
            state.cpu.data.eflags = VCPU_EFLAGS_CF;
            if (memory) {
                const lib_u32 memory_value = memory_source ? 0u : destination;
                failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
                    &memory_value, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
            if (memory) failed |= cpu_instruction_read(&state, INC_DEC_MEMORY,
                &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || observed != (memory_source ? 0u : 0u);
            else if (accumulator) failed |= (after.data.eax & mask) != 0u;
            else if (immediate || !register_destination) failed |= (after.data.ecx & mask) != 0u;
            else failed |= (after.data.edx & mask) != 0u;
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_adc_signed_overflow(void)
{
    static const lib_u8 forms[][6] = {
        { 0x14u,0u }, { 0x15u,0u,0u }, { 0x66u,0x15u,0u,0u,0u,0u }
    };
    static const lib_u8 lengths[] = { 2u,3u,6u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u ? 1u : (form == 1u ? 2u : 4u);
        const lib_u32 initial = bytes == 1u ? 0x1122337fu :
            (bytes == 2u ? 0x11227fffu : 0x7fffffffu);
        const lib_u32 expected = bytes == 1u ? 0x11223380u :
            (bytes == 2u ? 0x11228000u : 0x80000000u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = initial;
            state.cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != expected ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                        (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_adc_attribute_profile_fault(void)
{
    static const lib_u8 address_code[] = { 0x67u,0x66u,0x11u,0x16u };
    static const lib_u8 rejected[] = { 0x66u,0x11u,0xd1u };
    static const lib_u8 legacy[] = { 0x11u,0xd1u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_u32 value = 0xffffffffu;
    lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.cpu.data.esi = INC_DEC_MEMORY;
        state.cpu.data.edx = 0u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= cpu_instruction_write(&state, INC_DEC_MEMORY, &value,
            sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !inc_dec_run(&state, address_code,
                sizeof(address_code), 0, &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.edx != 0u || (after.data.eflags & ADD_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF) ||
            cpu_instruction_read(&state, INC_DEC_MEMORY, &value,
                sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || value != 0u;
    }
        if (failed) return 0;
    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.cpu.data.ecx = 0x1122ffffu;
        state.cpu.data.edx = 0u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= !cpu_instruction_expect_real_fault(&state, rejected,
            sizeof(rejected), 6u);
    }
        if (failed) return 0;
    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.cpu.data.ecx = 0xaabbffffu;
        state.cpu.data.edx = 0u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.ecx != 0xaabb0000u;
    }
        if (failed) return 0;
    return 1;
}

static lib_i32 inc_dec_test_sbb_forms(void)
{
    static const lib_u8 forms[][9] = {
        { 0x18u, 0xd1u },
        { 0x19u, 0xd1u },
        { 0x66u, 0x19u, 0xd1u },
        { 0x1au, 0xd1u },
        { 0x1bu, 0xd1u },
        { 0x66u, 0x1bu, 0xd1u },
        { 0x18u, 0x16u, 0u, 0x50u },
        { 0x19u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x19u, 0x16u, 0u, 0x50u },
        { 0x1au, 0x16u, 0u, 0x50u },
        { 0x1bu, 0x16u, 0u, 0x50u },
        { 0x66u, 0x1bu, 0x16u, 0u, 0x50u },
        { 0x1cu, 0u },
        { 0x1du, 0u, 0u },
        { 0x66u, 0x1du, 0u, 0u, 0u, 0u },
        { 0x80u, 0xd9u, 0u },
        { 0x81u, 0xd9u, 0u, 0u },
        { 0x66u, 0x81u, 0xd9u, 0u, 0u, 0u, 0u },
        { 0x83u, 0xd9u, 0u },
        { 0x66u, 0x83u, 0xd9u, 0u },
        { 0x80u, 0x1eu, 0u, 0x50u, 0u },
        { 0x81u, 0x1eu, 0u, 0x50u, 0u, 0u },
        { 0x66u, 0x81u, 0x1eu, 0u, 0x50u, 0u, 0u },
        { 0x83u, 0x1eu, 0u, 0x50u, 0u },
        { 0x66u, 0x83u, 0x1eu, 0u, 0x50u, 0u }
    };
    static const lib_u8 lengths[] = {
        2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u, 5u, 4u, 4u, 5u,
        2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u, 5u, 6u, 9u, 5u, 6u
    };
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
            state.cpu.data.ecx = 0u;
            state.cpu.data.edx = 0u;
            state.cpu.data.eflags = VCPU_EFLAGS_CF;
            if (memory) {
                const lib_u32 value = 0u;
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
                    observed != (memory_source ? 0u : mask);
            } else if (accumulator) {
                failed |= (after.data.eax & mask) != mask;
            } else if (immediate || !register_destination) {
                failed |= (after.data.ecx & mask) != mask;
            } else {
                failed |= (after.data.edx & mask) != mask;
            }
            if (!memory && !accumulator && !immediate && register_destination) {
                failed |= after.data.ecx != 0u;
            }
            if (!memory && !accumulator && !immediate && !register_destination) {
                failed |= after.data.edx != 0u;
            }
            if (memory && !memory_source) {
                failed |= after.data.edx != 0u;
            }
        }
                if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_sbb_boundaries(void)
{
    static const lib_u8 overflow[][6] = {
        { 0x1cu, 0u },
        { 0x1du, 0u, 0u },
        { 0x66u, 0x1du, 0u, 0u, 0u, 0u }
    };
    static const lib_u8 overflow_lengths[] = { 2u, 3u, 6u };
    static const lib_u8 sign_extended_immediate[][4] = {
        { 0x80u, 0xd9u, 0xffu },
        { 0x83u, 0xd9u, 0xffu },
        { 0x66u, 0x83u, 0xd9u, 0xffu }
    };
    static const lib_u8 sign_extended_immediate_lengths[] = { 3u, 3u, 4u };
    static const lib_u8 address_code[] = { 0x67u, 0x66u, 0x19u, 0x16u };
    static const lib_u8 rejected[] = { 0x66u, 0x19u, 0xd1u };
    static const lib_u8 legacy[] = { 0x19u, 0xd1u };
    lib_u8 form;

    for (form = 0u; form != sizeof(overflow_lengths); ++form) {
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
            state.cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= !inc_dec_run(&state, overflow[form], overflow_lengths[form], 0,
                &after, &diagnostic) || diagnostic.first_fault.valid ||
                after.data.eax != expected || (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
                        (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        }
                if (failed) return 0;
    }
    for (form = 0u; form != sizeof(sign_extended_immediate_lengths); ++form) {
        const lib_u32 initial = form == 2u ? 0u : 0xaabb0000u;
        const lib_u32 expected = form == 2u ? 1u : 0xaabb0001u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.ecx = initial;
            state.cpu.data.edx = 0x55667788u;
            state.cpu.data.eflags = 0u;
            failed |= !inc_dec_run(&state, sign_extended_immediate[form],
                sign_extended_immediate_lengths[form], 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.ecx != expected ||
                after.data.edx != 0x55667788u ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_AF);
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 value = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.esi = INC_DEC_MEMORY;
            state.cpu.data.edx = 0u;
            state.cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= cpu_instruction_write(&state, INC_DEC_MEMORY, &value,
                sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !inc_dec_run(&state, address_code,
                    sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || cpu_instruction_read(&state,
                    INC_DEC_MEMORY, &value, sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                value != 0xffffffffu;
        }
                if (failed) return 0;
    }
    {
        inc_dec_machine state;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

        if (!failed) {
            state.cpu.data.ecx = 0x11220000u;
            state.cpu.data.edx = 0u;
            state.cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= !cpu_instruction_expect_real_fault(&state, rejected,
                sizeof(rejected), 6u);
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
            state.cpu.data.edx = 0u;
            state.cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != 0xaabbffffu || after.data.edx != 0u;
        }
                if (failed) return 0;
    }
    return 1;
}
int main(void)
{
    if (!inc_dec_test_test_rm_reg_forms() ||
        !inc_dec_test_test_rm_reg_attribute_and_profile() ||
        !inc_dec_test_add_rm_reg_forms() ||
        !inc_dec_test_add_immediate_forms() ||
        !inc_dec_test_add_signed_overflow() ||
        !inc_dec_test_add_attribute_and_profile() ||
        !inc_dec_test_adc_forms() ||
        !inc_dec_test_adc_signed_overflow() ||
        !inc_dec_test_adc_attribute_profile_fault() ||
        !inc_dec_test_sbb_forms() ||
        !inc_dec_test_sbb_boundaries()) {
        lib_c_fprintf(lib_c_stderr, "%s", "CPU-TEST-ADD-ADC-SBB:FAIL\n");
        return 1;
    }
    lib_c_printf("%s\n", "TEST-RM-REG:OK");
    lib_c_printf("%s\n", "ADD:OK");
    lib_c_printf("%s\n", "ADC:OK");
    lib_c_printf("%s\n", "SBB:OK");
    lib_c_printf("%s\n", "CPU-TEST-ADD-ADC-SBB:OK");
    return 0;
}

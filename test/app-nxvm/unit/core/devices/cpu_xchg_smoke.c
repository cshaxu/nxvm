#include "support/cpu_instruction_fixture.h"
#include <stdio.h>

/* T337_REAL_UD_TERMINAL_CPU_OWNER: terminal-UD assertions stay CPU-owned. */
typedef struct xchg_vector {
    const lib_u8 *code;
    lib_u8 bytes;
    lib_u8 memory_width;
    lib_u32 memory_address;
    lib_u32 memory_before;
    lib_u32 memory_after;
    lib_u32 eax_after;
    lib_u32 ecx_after;
} xchg_vector;

static lib_i32 xchg_test_real(void)
{
    static const lib_u8 r8[] = { 0x86u, 0xc1u };
    static const lib_u8 m8[] = { 0x86u, 0x06u, 0x00u, 0x10u };
    static const lib_u8 r16[] = { 0x87u, 0xc1u };
    static const lib_u8 m16[] = { 0x87u, 0x06u, 0x00u, 0x10u };
    static const lib_u8 r32[] = { 0x66u, 0x87u, 0xc1u };
    static const lib_u8 m32[] = { 0x67u, 0x66u, 0x87u, 0x46u, 0x10u };
    static const xchg_vector vectors[] = {
        { r8, sizeof(r8), 0u, 0u, 0u, 0u, 0xaabb3388u, 0x55667744u },
        { m8, sizeof(m8), 1u, 0x1000u, 0x22u, 0x44u, 0xaabb3322u, 0x55667788u },
        { r16, sizeof(r16), 0u, 0u, 0u, 0u, 0xaabb7788u, 0x55663344u },
        { m16, sizeof(m16), 2u, 0x1000u, 0x7788u, 0x3344u, 0xaabb7788u, 0x55667788u },
        { r32, sizeof(r32), 0u, 0u, 0u, 0u, 0x55667788u, 0xaabb3344u },
        { m32, sizeof(m32), 4u, 0x1010u, 0x11223344u, 0xaabb3344u, 0x11223344u, 0x55667788u }
    };
    lib_u8 form;

    for (form = 0u; form != sizeof(vectors) / sizeof(vectors[0]); ++form)
    {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u32 memory_after = 0u;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        state.cpu.data.eax = 0xaabb3344u;
        state.cpu.data.ecx = 0x55667788u;
        state.cpu.data.esi = 0x00001000u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
        if (vectors[form].memory_width != 0u)
            failed |= cpu_instruction_write(&state, vectors[form].memory_address, &vectors[form].memory_before, vectors[form].memory_width, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        failed |= cpu_instruction_run(&state, vectors[form].code, vectors[form].bytes, &after) != LIB_STATUS_OK || state.fault.valid ||
            after.data.eip != vectors[form].bytes ||
            after.data.eax != vectors[form].eax_after ||
            after.data.ecx != vectors[form].ecx_after ||
            after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF);
        if (vectors[form].memory_width != 0u)
            failed |= cpu_instruction_read(&state, vectors[form].memory_address, &memory_after, vectors[form].memory_width, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                memory_after != vectors[form].memory_after;
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 xchg_test_profiles_and_lock(void)
{
    static const core_machine_cpu_profile profiles[] = { CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286 };
    static const lib_u8 prefixes[][4] = { {0x66u,0x87u,0xc1u,0u},
        {0x67u,0x87u,0xc1u,0u}, {0x66u,0x67u,0x87u,0xc1u} };
    lib_u8 profile, form;
    for (profile = 0u; profile != 3u; ++profile)
    {
        for (form = 0u; form != 3u; ++form)
        {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile]);
        state.cpu.data.eax=0x11223344u;
        state.cpu.data.eflags=VCPU_EFLAGS_CF;
        state.cpu.data.idtr.limit = 0x17u;
        before=state.cpu;
        failed |= cpu_instruction_run(&state, prefixes[form], form==2u?4u:3u, &after) != LIB_STATUS_INTERNAL_ERROR || !state.fault.valid || !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
            after.data.eip!=before.data.eip || after.data.eax!=before.data.eax || after.data.eflags!=before.data.eflags;
        if (failed)
            return 0;
        }
    }
    return 1;
}

static lib_i32 xchg_test_legacy_default16(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 register_code[] = { 0x87u, 0xc1u };
    static const lib_u8 memory_code[] = { 0x87u, 0x06u, 0x00u, 0x10u };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u16 memory = 0x7788u;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile]);

        state.cpu.data.eax = 0xaabb3344u;
        state.cpu.data.ecx = 0x55667788u;
        state.cpu.data.edx = 0x12345678u;
        state.cpu.data.eflags =
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
        failed |= cpu_instruction_run(&state, register_code, sizeof(register_code), &after) != LIB_STATUS_OK ||
            state.fault.valid || after.data.eip != 2u ||
            after.data.eax != 0xaabb7788u ||
            after.data.ecx != 0x55663344u ||
            after.data.edx != 0x12345678u ||
            after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF);
        state.cpu.data.eip = 0u;
        state.cpu.data.eax = 0xaabb3344u;
        state.cpu.data.ecx = 0x55667788u;
        failed |= cpu_instruction_write(&state, 0x1000u, &memory, sizeof(memory), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, memory_code, sizeof(memory_code), &after) != LIB_STATUS_OK ||
            state.fault.valid || after.data.eip != 4u ||
            after.data.eax != 0xaabb7788u ||
            after.data.ecx != 0x55667788u ||
            after.data.edx != 0x12345678u ||
            after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 xchg_test_lock(void)
{
    static const lib_u8 plain_code[] = {0x87u,0x06u,0x00u,0x10u};
    static const lib_u8 memory_code[] = {0xf0u,0x87u,0x06u,0x00u,0x10u};
    static const lib_u8 register_code[] = {0xf0u,0x87u,0xc1u};
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u16 memory = 0x7788u;
    lib_i32 failed = 0;

    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    state.cpu.data.eax = 0xaabb3344u;
    failed |= cpu_instruction_write(&state, 0x1000u, &memory, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA)!=LIB_STATUS_OK ||
        cpu_instruction_run(&state, plain_code, sizeof(plain_code), &after) != LIB_STATUS_OK;
    state.cpu.data.eip = 0u;
    state.cpu.data.eax = 0xaabb3344u;
    memory = 0x7788u;
    failed |= cpu_instruction_write(&state, 0x1000u, &memory, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA)!=LIB_STATUS_OK ||
        cpu_instruction_run(&state, memory_code, sizeof(memory_code), &after) != LIB_STATUS_OK ||
        after.data.eax!=0xaabb7788u;
    state.cpu.data.eip = 0u;
    state.cpu.data.idtr.limit = 0x17u;
    before=state.cpu;
    failed |= cpu_instruction_run(&state, register_code, sizeof(register_code), &after) != LIB_STATUS_INTERNAL_ERROR || !state.fault.valid || !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
        after.data.eip!=before.data.eip || after.data.eax!=before.data.eax || after.data.eflags!=before.data.eflags;
    return !failed;
}

static lib_u32 *xchg_acc_target(t_cpu *cpu, lib_u8 opcode)
{
    switch (opcode)
    {
    case 0x91u: return &cpu->data.ecx;
    case 0x92u: return &cpu->data.edx;
    case 0x93u: return &cpu->data.ebx;
    case 0x94u: return &cpu->data.esp;
    case 0x95u: return &cpu->data.ebp;
    case 0x96u: return &cpu->data.esi;
    case 0x97u: return &cpu->data.edi;
    default: return LIB_NULL;
    }
}

static lib_i32 xchg_acc_state_equal(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eflags == after->data.eflags &&
        before->data.eip == after->data.eip;
}

static lib_i32 xchg_acc_gpr_flags_equal(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eflags == after->data.eflags;
}

static lib_i32 xchg_acc_nonparticipants_equal(const t_cpu *before,
    const t_cpu *after, lib_u8 opcode)
{
    if (opcode == 0x90u)
        return xchg_acc_gpr_flags_equal(before, after);

    return before->data.eflags == after->data.eflags &&
        (opcode == 0x91u || before->data.ecx == after->data.ecx) &&
        (opcode == 0x92u || before->data.edx == after->data.edx) &&
        (opcode == 0x93u || before->data.ebx == after->data.ebx) &&
        (opcode == 0x94u || before->data.esp == after->data.esp) &&
        (opcode == 0x95u || before->data.ebp == after->data.ebp) &&
        (opcode == 0x96u || before->data.esi == after->data.esi) &&
        (opcode == 0x97u || before->data.edi == after->data.edi);
}

static lib_i32 xchg_test_accumulator(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;
    lib_u8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile)
    {
        for (opcode = 0x90u; opcode <= 0x97u; ++opcode)
        {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_status status;
            lib_u8 code[] = { opcode };
            lib_u32 *target;
            lib_i32 failed;

            lib_memory_set(&before, 0, sizeof(before));
            lib_memory_set(&after, 0, sizeof(after));
            status = LIB_STATUS_INVALID_ARGUMENT;
            failed = 0;
            cpu_instruction_prepare(&state, profiles[profile]);

            state.cpu.data.eax = 0xaabb3344u;
            state.cpu.data.ecx = 0x11112222u;
            state.cpu.data.edx = 0x33334444u;
            state.cpu.data.ebx = 0x55556666u;
            state.cpu.data.esp = 0x77778888u;
            state.cpu.data.ebp = 0x9999aaaau;
            state.cpu.data.esi = 0xbbbbccccu;
            state.cpu.data.edi = 0xddddeeeeu;
            state.cpu.data.eflags =
                VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
            target = xchg_acc_target(&state.cpu, opcode);
            if (target != LIB_NULL)
                *target = 0x55667788u;
            before = state.cpu;
            failed |= (status = cpu_instruction_run(&state, code, sizeof(code), &after)) != LIB_STATUS_OK ||
                state.fault.valid ||
                after.data.eip != 1u ||
                after.data.eflags != before.data.eflags;
            failed |= !xchg_acc_nonparticipants_equal(&before, &after,
                opcode);
            if (opcode != 0x90u)
            {
                target = xchg_acc_target(&after, opcode);
                failed |= after.data.eax != 0xaabb7788u ||
                    target == LIB_NULL || *target != 0x55663344u;
            }
            if (failed)
            {
                printf(
                    "XCHG acc default profile=%u opcode=%02x status=%d "
                    "fault=%08x before=%08x/%08x/%08x/%08x "
                    "after=%08x/%08x/%08x/%08x\n",
                    profile,
                    opcode,
                    status,
                    state.fault.exception_mask,
                    before.data.eip,
                    before.data.eax,
                    before.data.ecx,
                    before.data.eflags,
                    after.data.eip,
                    after.data.eax,
                    after.data.ecx,
                    after.data.eflags);
            }
            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 xchg_test_accumulator_reject(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 profile;
    lib_u8 opcode;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile)
    {
        for (opcode = 0x90u; opcode <= 0x97u; ++opcode)
        {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_status status;
            lib_u8 code[] = { 0x66u, opcode };
            lib_i32 failed;

            lib_memory_set(&before, 0, sizeof(before));
            lib_memory_set(&after, 0, sizeof(after));
            status = LIB_STATUS_INVALID_ARGUMENT;
            failed = 0;
            cpu_instruction_prepare(&state, profiles[profile]);
            state.cpu.data.eax = 0xaabb3344u;
            state.cpu.data.ecx = 0x55667788u;
            state.cpu.data.eflags = VCPU_EFLAGS_CF;
            state.cpu.data.idtr.limit = 0x17u;
            before = state.cpu;
            failed |= (status = cpu_instruction_run(&state, code, sizeof(code), &after)) != LIB_STATUS_INTERNAL_ERROR ||
                !state.fault.valid ||
                !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
                !xchg_acc_state_equal(&before, &after);
            if (failed)
            {
                printf(
                    "XCHG acc 66 profile=%u opcode=%02x status=%d "
                    "fault=%08x before=%08x/%08x/%08x after=%08x/%08x/%08x\n",
                    profile,
                    opcode,
                    status,
                    state.fault.exception_mask,
                    before.data.eip,
                    before.data.eax,
                    before.data.eflags,
                    after.data.eip,
                    after.data.eax,
                    after.data.eflags);
                return 0;
            }
        }
    }
    return 1;
}

static lib_i32 xchg_test_accumulator_lock(void)
{
    lib_u8 opcode;

    for (opcode = 0x90u; opcode <= 0x97u; ++opcode)
    {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_u8 code[] = { 0xf0u, opcode };
        lib_i32 failed;

        lib_memory_set(&before, 0, sizeof(before));
        lib_memory_set(&after, 0, sizeof(after));
        status = LIB_STATUS_INVALID_ARGUMENT;
        failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eax = 0xaabb3344u;
        state.cpu.data.ecx = 0x55667788u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF;
        state.cpu.data.idtr.limit = 0x17u;
        before = state.cpu;
        failed |= (status = cpu_instruction_run(&state, code, sizeof(code), &after)) != LIB_STATUS_INTERNAL_ERROR ||
            !state.fault.valid ||
            !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
            !xchg_acc_state_equal(&before, &after);
        if (failed)
        {
            printf(
                "XCHG acc lock opcode=%02x status=%d fault=%08x "
                "before=%08x/%08x/%08x after=%08x/%08x/%08x\n",
                opcode,
                status,
                state.fault.exception_mask,
                before.data.eip,
                before.data.eax,
                before.data.eflags,
                after.data.eip,
                after.data.eax,
                after.data.eflags);
            return 0;
        }
    }
    return 1;
}

static lib_i32 xchg_test_accumulator_386_boundaries(void)
{
    lib_u8 opcode;

    for (opcode = 0x90u; opcode <= 0x97u; ++opcode)
    {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_u8 code[] = { 0x66u, opcode };
        lib_u32 *target;
        lib_i32 failed;

        lib_memory_set(&before, 0, sizeof(before));
        lib_memory_set(&after, 0, sizeof(after));
        status = LIB_STATUS_INVALID_ARGUMENT;
        failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        state.cpu.data.eax = 0xaabb3344u;
        state.cpu.data.ecx = 0x11112222u;
        state.cpu.data.edx = 0x33334444u;
        state.cpu.data.ebx = 0x55556666u;
        state.cpu.data.esp = 0x77778888u;
        state.cpu.data.ebp = 0x9999aaaau;
        state.cpu.data.esi = 0xbbbbccccu;
        state.cpu.data.edi = 0xddddeeeeu;
        state.cpu.data.eflags = VCPU_EFLAGS_CF;
        target = xchg_acc_target(&state.cpu, opcode);
        if (target != LIB_NULL)
            *target = 0x55667788u;
        before = state.cpu;
        failed |= (status = cpu_instruction_run(&state, code, sizeof(code), &after)) != LIB_STATUS_OK ||
            state.fault.valid || after.data.eip != 2u ||
            after.data.eflags != before.data.eflags;
        failed |= !xchg_acc_nonparticipants_equal(&before, &after,
            opcode);
        if (opcode != 0x90u)
        {
            target = xchg_acc_target(&after, opcode);
            failed |= after.data.eax != 0x55667788u || target == LIB_NULL ||
                *target != 0xaabb3344u;
        }
        if (failed)
        {
            printf(
                "XCHG acc 386 opcode=%02x status=%d fault=%08x "
                "before=%08x/%08x/%08x after=%08x/%08x/%08x\n",
                opcode,
                status,
                state.fault.exception_mask,
                before.data.eip,
                before.data.eax,
                before.data.eflags,
                after.data.eip,
                after.data.eax,
                after.data.eflags);
        }
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!xchg_test_real())
    {
        printf("XCHG stage=real\n");
        return 1;
    }
    if (!xchg_test_profiles_and_lock())
    {
        printf("XCHG stage=profile\n");
        return 1;
    }
    if (!xchg_test_legacy_default16())
    {
        printf("XCHG stage=legacy\n");
        return 1;
    }
    if (!xchg_test_lock())
    {
        printf("XCHG stage=lock\n");
        return 1;
    }
    if (!xchg_test_accumulator())
    {
        printf("XCHG acc stage=default\n");
        return 1;
    }
    if (!xchg_test_accumulator_386_boundaries())
    {
        printf("XCHG acc stage=386\n");
        return 1;
    }
    if (!xchg_test_accumulator_reject())
    {
        printf("XCHG acc stage=reject\n");
        return 1;
    }
    if (!xchg_test_accumulator_lock())
    {
        printf("XCHG acc stage=lock\n");
        return 1;
    }
    printf("M5:T539:S23:XCHG:CPU:OK\n");
    return 0;
}

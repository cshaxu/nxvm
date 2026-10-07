#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static void debug_state_enter_protected(cpu_instruction_fixture *fixture,
    lib_u8 cpl, lib_bool vm86)
{
    t_cpu *cpu = &fixture->cpu;

    cpu->data.cr0 |= VCPU_CR0_PE;
    cpu->data.cs.selector = (lib_u16)(0x0008u | cpl);
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    if (vm86) {
        cpu->data.eflags |= VCPU_EFLAGS_VM;
        cpu->data.cs.dpl = 3u;
    }
}

static lib_i32 debug_state_same(const t_cpu *before, const t_cpu *after)
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
        before->data.dr0 == after->data.dr0 &&
        before->data.dr1 == after->data.dr1 &&
        before->data.dr2 == after->data.dr2 &&
        before->data.dr3 == after->data.dr3 &&
        before->data.dr6 == after->data.dr6 &&
        before->data.dr7 == after->data.dr7;
}

static lib_u32 *debug_state_dr(t_cpu *cpu, lib_u8 index)
{
    switch (index) {
    case 0: return &cpu->data.dr0;
    case 1: return &cpu->data.dr1;
    case 2: return &cpu->data.dr2;
    case 3: return &cpu->data.dr3;
    case 6: return &cpu->data.dr6;
    case 7: return &cpu->data.dr7;
    default: return LIB_NULL;
    }
}

static lib_i32 debug_state_run(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    lib_u8 steps;

    lib_memory_copy(fixture->memory, code, bytes);
    for (steps = 0u; steps != 16u && fixture->cpu.data.eip < bytes &&
        !fixture->execution.stop_requested; ++steps)
        core_machine_cpu_execution_refresh(&fixture->execution);
    *after = fixture->cpu;
    return fixture->cpu.data.eip >= bytes && !fixture->execution.stop_requested &&
        !fixture->fault.valid;
}

static lib_i32 debug_state_fault(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, lib_u32 exception, t_cpu *after)
{
    (void)cpu_instruction_run(fixture, code, bytes, after);
    /* This protected fixture has no handlers: UD/GP entry causes GP,
     * another failed GP entry requires DF, whose entry is absent as well. */
    if (fixture->execution.cpu_profile >= CORE_MACHINE_CPU_PROFILE_80286 &&
        (fixture->cpu.data.cr0 & VCPU_CR0_PE) != 0u &&
        (exception == VCPUINS_EXCEPT_GP || exception == VCPUINS_EXCEPT_UD))
        return core_machine_cpu_is_shutdown(&fixture->execution) &&
            fixture->delivered_exception.valid &&
            fixture->delivered_exception.exception_mask == VCPUINS_EXCEPT_SHUTDOWN &&
            !fixture->execution.stop_requested && !fixture->fault.valid;
    return (fixture->execution.stop_requested && fixture->fault.valid &&
        (fixture->fault.exception_mask & exception) != 0u) ||
        (fixture->instructions.data.except & exception) != 0u;
}

static lib_i32 debug_state_test_mov_dr(void)
{
    static const lib_u8 indices[] = {0u,1u,2u,3u,6u,7u};
    static const lib_u8 rejected[][3] = {
        {0x0fu,0x21u,0xe0u}, {0x0fu,0x23u,0xe8u},
        {0x0fu,0x21u,0x00u}, {0x0fu,0x23u,0x00u}
    };
    static const lib_u8 prefixes[][2] = {
        {0x66u,0u}, {0x67u,0u}, {0x66u,0x67u}
    };
    static const lib_u8 lock[][4] = {
        {0xf0u,0x0fu,0x21u,0xc0u}, {0xf0u,0x0fu,0x23u,0xc1u}
    };
    lib_size index;

    for (index = 0u; index != 6u; ++index) {
        cpu_instruction_fixture fixture;
        lib_u8 code[] = {0x0fu,0x23u,(lib_u8)(0xc1u | (indices[index] << 3u)),
            0x0fu,0x21u,(lib_u8)(0xc0u | (indices[index] << 3u))};
        t_cpu before, after;
        const lib_u32 value = 0x10203040u + (lib_u32)indices[index];

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.ecx = value;
        fixture.cpu.data.eax = 0xaabbccddu;
        before = fixture.cpu;
        if (!debug_state_run(&fixture, code, sizeof(code), &after) ||
            after.data.eip != sizeof(code) || after.data.eax != value ||
            *debug_state_dr(&after, indices[index]) != value ||
            after.data.ecx != before.data.ecx || after.data.eflags !=
            before.data.eflags) return 0;
    }
    for (index = 0u; index != 4u; ++index) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        debug_state_enter_protected(&fixture, 0u, LIB_FALSE);
        fixture.cpu.data.eax = 0xaabbccddu;
        fixture.cpu.data.ecx = 0x11223344u;
        fixture.cpu.data.dr0 = 0x55667788u;
        fixture.cpu.data.dr7 = 0x99aabbccu;
        before = fixture.cpu;
        if (!debug_state_fault(&fixture, rejected[index], sizeof(rejected[index]),
                VCPUINS_EXCEPT_UD, &after) || !debug_state_same(&before,
                &after)) return 0;
    }
    for (index = 0u; index != 3u; ++index) {
        cpu_instruction_fixture fixture;
        lib_u8 code[8u] = {0u};
        const lib_u8 prefix_bytes = prefixes[index][1] == 0u ? 1u : 2u;
        t_cpu before, after;

        code[0] = prefixes[index][0];
        if (prefix_bytes == 2u) code[1] = prefixes[index][1];
        code[prefix_bytes] = 0x0fu;
        code[prefix_bytes + 1u] = 0x23u;
        code[prefix_bytes + 2u] = 0xc1u;
        code[prefix_bytes + 3u] = 0x0fu;
        code[prefix_bytes + 4u] = 0x21u;
        code[prefix_bytes + 5u] = 0xc0u;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.ecx = 0x5a5aa5a5u;
        before = fixture.cpu;
        if (!debug_state_run(&fixture, code, prefix_bytes + 6u, &after) ||
            after.data.eax != before.data.ecx || after.data.dr0 !=
            before.data.ecx || after.data.eflags != before.data.eflags) return 0;
    }
    for (index = 0u; index != 2u; ++index) {
        cpu_instruction_fixture fixture;
        static const lib_u8 mov_dr[] = {0x0fu,0x21u,0xc0u};
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        debug_state_enter_protected(&fixture, 3u, index != 0u);
        fixture.cpu.data.eax = 0xaabbccddu;
        fixture.cpu.data.dr0 = 0x55667788u;
        before = fixture.cpu;
        if (!debug_state_fault(&fixture, mov_dr, sizeof(mov_dr),
                VCPUINS_EXCEPT_GP, &after) || !debug_state_same(&before,
                &after)) return 0;
    }
    for (index = 0u; index != 2u; ++index) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        debug_state_enter_protected(&fixture, 0u, LIB_FALSE);
        fixture.cpu.data.eax = 0xaabbccddu;
        fixture.cpu.data.ecx = 0x11223344u;
        fixture.cpu.data.dr0 = 0x55667788u;
        before = fixture.cpu;
        if (!debug_state_fault(&fixture, lock[index], sizeof(lock[index]),
                VCPUINS_EXCEPT_UD, &after) || !debug_state_same(&before,
                &after)) return 0;
    }
    return 1;
}

static lib_i32 debug_state_test_debug_exceptions(void)
{
    static const lib_u8 nop[] = {0x90u};
    {
        cpu_instruction_fixture fixture;
        t_cpu after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.dr0 = 0u;
        fixture.cpu.data.dr7 = 0x00000001u;
        if (cpu_instruction_run(&fixture, nop, sizeof(nop), &after) !=
                LIB_STATUS_OK || fixture.fault.valid ||
            (after.data.dr6 & 1u) == 0u)
            return 0;
    }
    {
        cpu_instruction_fixture fixture;
        t_cpu after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.eflags = VCPU_EFLAGS_RF | VCPU_EFLAGS_IF;
        fixture.cpu.data.dr0 = 0u;
        fixture.cpu.data.dr7 = 0x00000001u;
        if (cpu_instruction_run(&fixture, nop, sizeof(nop), &after) !=
                LIB_STATUS_OK || fixture.fault.valid ||
            (after.data.eflags & VCPU_EFLAGS_RF) != 0u || after.data.dr6 != 0u)
            return 0;
    }
    return 1;
}

static lib_i32 debug_state_test_data_breakpoints(void)
{
    static const lib_u8 read[] = {0xa1u,0x00u,0x10u};
    static const lib_u8 write[] = {0xc6u,0x06u,0x00u,0x10u,0x5au};
    static const lib_u8 source[] = {0x5au,0x34u,0x56u,0x78u};
    cpu_instruction_fixture fixture;
    t_cpu after;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    lib_memory_copy(fixture.memory + 0x1000u, source, sizeof(source));
    fixture.cpu.data.dr0 = 0x1001u;
    fixture.cpu.data.dr1 = 0x1001u;
    fixture.cpu.data.dr7 = 0x00ff0008u;
    if (cpu_instruction_run(&fixture, read, sizeof(read), &after) !=
            LIB_STATUS_OK || fixture.fault.valid ||
        (after.data.dr6 & 3u) != 3u || (after.data.eax & 0xffffu) != 0x345au)
        return 0;

    cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
    fixture.cpu.data.eflags = VCPU_EFLAGS_TF | VCPU_EFLAGS_IF |
        VCPU_EFLAGS_CF;
    fixture.cpu.data.dr0 = 0x1000u;
    fixture.cpu.data.dr7 = 0x00010101u;
    if (cpu_instruction_run(&fixture, write, sizeof(write), &after) !=
            LIB_STATUS_OK || fixture.fault.valid ||
        (after.data.dr6 & (1u | 0x00004000u)) != (1u | 0x00004000u) ||
        fixture.memory[0x1000u] != 0x5au) return 0;
    return 1;
}

static lib_bool debug_state_test_popf_completion(void)
{
    static const lib_u8 forms[][2] = {{0x9du,0u}, {0x66u,0x9du}};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_size profile;
    lib_u8 form, old_rf, popped_rf, old_tf, popped_tf;
    lib_u32 rf_failures = 0u, tf_failures = 0u;

    for (form = 0u; form < 2u; ++form)
    for (old_rf = 0u; old_rf < 2u; ++old_rf)
    for (popped_rf = 0u; popped_rf < 2u; ++popped_rf) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_u32 image = 0x02u | (popped_rf ? VCPU_EFLAGS_RF : 0u);

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.sp = 0x8000u;
        fixture.cpu.data.eflags = 0x02u | (old_rf ? VCPU_EFLAGS_RF : 0u);
        lib_memory_copy(fixture.memory + 0x8000u, &image, form ? 4u : 2u);
        if (cpu_instruction_run(&fixture, forms[form], form + 1u, &after) !=
                LIB_STATUS_OK || fixture.fault.valid ||
            (after.data.eflags & VCPU_EFLAGS_RF) !=
                (old_rf ? VCPU_EFLAGS_RF : 0u) ||
            after.data.eip != form + 1u || after.data.sp != 0x8000u +
                (form ? 4u : 2u)) ++rf_failures;
    }
    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form < (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80386 ?
        2u : 1u); ++form)
    for (old_tf = 0u; old_tf < 2u; ++old_tf)
    for (popped_tf = 0u; popped_tf < 2u; ++popped_tf) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_u16 handler[] = {0x0100u,0u};
        const lib_u32 image = 0x02u | (popped_tf ? VCPU_EFLAGS_TF : 0u);
        lib_u16 return_ip = 0u;

        cpu_instruction_prepare(&fixture, profiles[profile]);
        fixture.cpu.data.sp = 0x8000u;
        fixture.cpu.data.eflags = 0x02u | (old_tf ? VCPU_EFLAGS_TF : 0u);
        lib_memory_copy(fixture.memory + 4u, handler, sizeof(handler));
        fixture.memory[0x0100u] = 0x90u;
        lib_memory_copy(fixture.memory + 0x8000u, &image, form ? 4u : 2u);
        if (cpu_instruction_run(&fixture, forms[form], form + 1u, &after) !=
                LIB_STATUS_OK || fixture.fault.valid) {
            ++tf_failures;
            continue;
        }
        lib_memory_copy(&return_ip, fixture.memory + after.data.sp,
            sizeof(return_ip));
        if (old_tf ? (!fixture.delivered_exception.valid ||
                fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
                after.data.eip != 0x0100u || return_ip != form + 1u ||
                (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80386 &&
                    (after.data.dr6 & 0x00004000u) == 0u)) :
            (fixture.delivered_exception.valid || after.data.eip != form + 1u ||
                (after.data.eflags & VCPU_EFLAGS_TF) !=
                    (popped_tf ? VCPU_EFLAGS_TF : 0u))) ++tf_failures;
    }
    lib_c_printf("POPF completion RF cases=8 failures=%u; TF cases=24 failures=%u\n",
        (unsigned)rf_failures, (unsigned)tf_failures);
    return rf_failures == 0u && tf_failures == 0u;
}

static lib_bool debug_state_test_rf_images(void)
{
    static const struct {
        lib_u8 code[2];
        lib_u8 length;
        lib_u8 vector;
        lib_bool fault;
    } cases[] = {
        {{0xf6u,0xf1u},2u,0u,LIB_TRUE},
        {{0x0fu,0x0bu},2u,6u,LIB_TRUE},
        {{0x90u,0u},1u,1u,LIB_TRUE},
        {{0x90u,0u},1u,1u,LIB_FALSE},
        {{0xcdu,0x20u},2u,0x20u,LIB_FALSE}
    };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0,0,0x9au,0,0
    };
    lib_u8 mode, wide, old_rf;
    lib_size index;
    lib_u32 failures = 0u;

    for (mode = 0u; mode < 2u; ++mode)
    for (wide = 0u; wide <= mode; ++wide)
    for (old_rf = 0u; old_rf < 2u; ++old_rf)
    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_u8 vector = cases[index].vector;
        const lib_u16 real_gate[] = {0x0100u,0u};
        const lib_u8 gate[] = {0u,1u,8u,0u,0u,
            (lib_u8)(wide ? 0x8eu : 0x86u),0u,0u};
        lib_u32 image = 0u;
        lib_u32 flags = 0x02u | (old_rf ? VCPU_EFLAGS_RF : 0u);

        /* An execution breakpoint is suppressed by RF. Test that fault with
         * RF clear; the other faults cover both incoming RF states. */
        if (index == 2u && old_rf) continue;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.sp = 0x8000u;
        if (index == 2u) fixture.cpu.data.dr7 = 1u;
        if (index == 3u) flags |= VCPU_EFLAGS_TF;
        if (mode) {
            debug_state_enter_protected(&fixture, 0u, LIB_FALSE);
            fixture.cpu.data.gdtr.base = 0x300u;
            fixture.cpu.data.gdtr.limit = sizeof(gdt) - 1u;
            fixture.cpu.data.idtr.base = 0x400u;
            fixture.cpu.data.idtr.limit = 0x107u;
            lib_memory_copy(fixture.memory + 0x300u, gdt, sizeof(gdt));
            lib_memory_copy(fixture.memory + 0x400u + vector * 8u,
                gate, sizeof(gate));
        } else {
            lib_memory_copy(fixture.memory + vector * 4u,
                real_gate, sizeof(real_gate));
        }
        fixture.cpu.data.eflags = flags;
        fixture.cpu.data.cs.base = 0x2000u;
        fixture.cpu.data.dr0 = 0x2000u;
        lib_memory_copy(fixture.memory + 0x2000u, cases[index].code,
            cases[index].length);
        core_machine_cpu_execution_refresh(&fixture.execution);
        after = fixture.cpu;
        if (fixture.execution.stop_requested ||
            fixture.fault.valid || after.data.eip != 0x0100u ||
            after.data.sp != 0x8000u - (wide ? 12u : 6u)) {
            lib_c_printf("RF delivery mode=%u wide=%u old=%u case=%u ip=%x sp=%x fault=%u\n",
                mode, wide, old_rf, (unsigned)index, after.data.eip,
                after.data.sp, fixture.fault.valid);
            ++failures;
            continue;
        }
        lib_memory_copy(&image, fixture.memory + after.data.sp +
            (wide ? 8u : 4u), wide ? 4u : 2u);
        if ((wide && (image & VCPU_EFLAGS_RF) !=
                (cases[index].fault || (index == 4u && old_rf) ?
                    VCPU_EFLAGS_RF : 0u)) ||
            (after.data.eflags & VCPU_EFLAGS_RF) != 0u ||
            (cases[index].fault && (!fixture.delivered_exception.valid ||
                fixture.delivered_exception.eflags != flags))) {
            lib_c_printf("RF image mode=%u wide=%u old=%u case=%u image=%x live=%x diagnostic=%u/%x expected=%x\n",
                mode, wide, old_rf, (unsigned)index, image, after.data.eflags,
                fixture.delivered_exception.valid,
                fixture.delivered_exception.eflags, flags);
            ++failures;
        }
    }
    /* Failed delivery must retain the original state, not the RF fault image. */
    for (index = 0u; index < 2u; ++index)
    for (old_rf = 0u; old_rf < 2u; ++old_rf) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_u8 code[] = {0x0fu,0x0bu};
        const lib_u32 flags = 0x02u | (old_rf ? VCPU_EFLAGS_RF : 0u);

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        if (index == 1u && old_rf) continue;
        if (index == 1u) fixture.cpu.data.dr7 = 1u;
        fixture.cpu.data.eflags = flags;
        fixture.cpu.data.idtr.limit = 0u;
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) !=
                LIB_STATUS_OK || !core_machine_cpu_is_shutdown(&fixture.execution) ||
            fixture.fault.valid || !fixture.delivered_exception.valid ||
            fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
            after.data.eflags != flags || fixture.delivered_exception.eflags != flags)
            ++failures;
    }
    lib_c_printf("RF fault/trap/image/rollback failures=%u\n",
        (unsigned)failures);
    return failures == 0u;
}

static lib_bool debug_state_test_ordinary_transfer_rf(void)
{
    static const struct {
        lib_u8 code[5];
        lib_u8 length;
        lib_u16 ip;
        lib_u16 sp;
    } cases[] = {
        {{0x90u,0u,0u,0u,0u},1u,1u,0x8000u},
        {{0xe8u,0u,0u,0u,0u},3u,3u,0x7ffeu},
        {{0xe9u,0u,0u,0u,0u},3u,3u,0x8000u},
        {{0x9au,0u,1u,0u,0u},5u,0x0100u,0x7ffcu},
        {{0xeau,0u,1u,0u,0u},5u,0x0100u,0x8000u}
    };
    lib_size index;
    lib_u8 old_rf;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
    for (old_rf = 0u; old_rf < 2u; ++old_rf) {
        cpu_instruction_fixture fixture;
        t_cpu after;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.sp = 0x8000u;
        fixture.cpu.data.eflags = 2u | (old_rf ? VCPU_EFLAGS_RF : 0u);
        if (cpu_instruction_run(&fixture, cases[index].code,
                cases[index].length, &after) != LIB_STATUS_OK || fixture.fault.valid ||
            after.data.eip != cases[index].ip || after.data.sp != cases[index].sp ||
            (after.data.eflags & VCPU_EFLAGS_RF) != 0u)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool debug_state_test_iret_completion(void)
{
    lib_u8 wide, bits;

    for (wide = 0u; wide < 2u; ++wide)
    for (bits = 0u; bits < 16u; ++bits) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_u8 code[] = {0x66u,0xcfu};
        const lib_u16 handler[] = {0x0200u,0u};
        const lib_u32 image = 2u | ((bits & 2u) ? VCPU_EFLAGS_TF : 0u) |
            ((bits & 8u) ? VCPU_EFLAGS_RF : 0u);
        const lib_u32 frame32[] = {0x0100u,0u,image};
        const lib_u16 frame16[] = {0x0100u,0u,(lib_u16)image};
        const lib_u32 expected_rf = (wide ? bits & 8u : bits & 4u) ?
            VCPU_EFLAGS_RF : 0u;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.cpu.data.sp = 0x8000u;
        fixture.cpu.data.eflags = 2u | ((bits & 1u) ? VCPU_EFLAGS_TF : 0u) |
            ((bits & 4u) ? VCPU_EFLAGS_RF : 0u);
        lib_memory_copy(fixture.memory + 4u, handler, sizeof(handler));
        lib_memory_copy(fixture.memory + 0x8000u,
            wide ? (const void *)frame32 : (const void *)frame16,
            wide ? sizeof(frame32) : sizeof(frame16));
        if (cpu_instruction_run(&fixture, wide ? code : code + 1u,
                wide ? 2u : 1u, &after) != LIB_STATUS_OK || fixture.fault.valid)
            return LIB_FALSE;
        if (bits & 1u) {
            if (!fixture.delivered_exception.valid ||
                fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_DB ||
                fixture.delivered_exception.point.eip != 0x0100u ||
                after.data.eip != 0x0200u ||
                (after.data.eflags & (VCPU_EFLAGS_RF | VCPU_EFLAGS_TF)) != 0u)
                return LIB_FALSE;
        } else if (fixture.delivered_exception.valid || after.data.eip != 0x0100u ||
            (after.data.eflags & VCPU_EFLAGS_RF) != expected_rf ||
            (after.data.eflags & VCPU_EFLAGS_TF) != (image & VCPU_EFLAGS_TF))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool debug_state_test_ss_shadow(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const struct {
        lib_u8 code[5];
        lib_u8 length;
    } cases[] = {
        {{0x8eu,0xd0u,0u,0u,0u},2u},
        {{0x17u,0u,0u,0u,0u},1u},
        {{0x0fu,0xb2u,0x06u,0u,0x10u},5u}
    };
    lib_size profile, index;
    lib_u32 failures = 0u;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (index = 0u; index < (profile ? 3u : 2u); ++index) {
        cpu_instruction_fixture state;
        t_cpu after;
        const lib_u16 vector[] = {0x0100u,0u};
        const lib_u16 pointer[] = {0x1234u,0u};
        const lib_u16 selector = 0u;

        cpu_instruction_prepare(&state, profiles[profile]);
        core_machine_cpu_execution_load_segment(&state.execution,
            &state.cpu.data.cs, 0x0200u);
        state.cpu.data.sp = 0x8000u;
        state.cpu.data.eflags = 2u | VCPU_EFLAGS_TF;
        lib_memory_copy(state.memory + 4u, vector, sizeof(vector));
        lib_memory_copy(state.memory + 0x8000u, &selector, sizeof(selector));
        lib_memory_copy(state.memory + 0x1000u, pointer, sizeof(pointer));
        lib_memory_copy(state.memory + 0x2000u, cases[index].code,
            cases[index].length);
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (state.execution.stop_requested || state.fault.valid)
            return LIB_FALSE;
        if (index == 2u) {
            if (!state.delivered_exception.valid || after.data.eip != 0x0100u ||
                state.delivered_exception.point.eip != cases[index].length)
                ++failures;
        } else {
            if (state.delivered_exception.valid || after.data.eip != cases[index].length) {
                lib_c_printf("SS shadow profile=%u case=%u ip=%x trap=%u\n",
                    (unsigned)profiles[profile], (unsigned)index,
                    after.data.eip, state.delivered_exception.valid);
                ++failures;
                continue;
            }
            state.memory[0x2000u + cases[index].length] = 0x90u;
            core_machine_cpu_execution_invalidate_prefetch(&state.execution);
            core_machine_cpu_execution_refresh(&state.execution);
            if (!state.delivered_exception.valid || state.cpu.data.eip != 0x0100u ||
                state.delivered_exception.point.eip != cases[index].length + 1u)
                ++failures;
        }
    }
    lib_c_printf("SS/LSS single-step cases=5 failures=%u\n", (unsigned)failures);
    return failures == 0u;
}

static lib_bool debug_state_test_comparator_status(void)
{
    static const lib_u8 forms[][5] = {
        {0x90u,0u,0u,0u,0u}, {0xc6u,0x06u,0u,0x10u,0x5au}
    };
    lib_u8 form, enabled, matched, index, global;
    lib_u32 failures = 0u;

    for (form = 0u; form < 2u; ++form)
    for (global = 0u; global < 2u; ++global)
    for (enabled = 0u; enabled < 16u; ++enabled)
    for (matched = 0u; matched < 16u; ++matched) {
        cpu_instruction_fixture fixture;
        const lib_u16 vector[] = {0x0100u,0u};
        const lib_bool triggered = (enabled & matched) != 0u;
        const lib_u32 address = form ? 0x1000u : 0x2000u;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        core_machine_cpu_execution_load_segment(&fixture.execution,
            &fixture.cpu.data.cs, 0x0200u);
        fixture.cpu.data.sp = 0x8000u;
        fixture.cpu.data.eflags = 2u;
        fixture.cpu.data.dr6 = 0x00004000u;
        for (index = 0u; index < 4u; ++index) {
            *debug_state_dr(&fixture.cpu, index) = (matched & (1u << index)) ?
                address : address + 0x100u;
            if (enabled & (1u << index))
                fixture.cpu.data.dr7 |= 1u << (2u * index + global);
            if (form) fixture.cpu.data.dr7 |= 1u << (16u + 4u * index);
        }
        lib_memory_copy(fixture.memory + 4u, vector, sizeof(vector));
        lib_memory_copy(fixture.memory + 0x2000u, forms[form], form ? 5u : 1u);
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.fault.valid ||
            fixture.delivered_exception.valid != triggered ||
            fixture.cpu.data.dr6 != (0x00004000u | (triggered ? matched : 0u)) ||
            fixture.cpu.data.eip != (triggered ? 0x0100u : (form ? 5u : 1u)))
            ++failures;
    }
    lib_c_printf("Comparator cause/status cases=1024 failures=%u\n", (unsigned)failures);
    return failures == 0u;
}

static lib_bool debug_state_test_shadow_fault_boundary(void)
{
    static const lib_u8 forms[][2] = {{0x8eu,0xd0u},{0x17u,0u}};
    const lib_u16 handler[] = {0x0100u,0u};
    const lib_u8 write[] = {0xc6u,0x06u,0u,8u,0x5au};
    lib_u8 form, mode;

    for (form = 0u; form < 2u; ++form)
    for (mode = 0u; mode < 3u; ++mode) {
        cpu_instruction_fixture state;
        const lib_u8 length = form ? 1u : 2u;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        core_machine_cpu_execution_load_segment(&state.execution,
            &state.cpu.data.cs, 0x0200u);
        state.cpu.data.sp = 0x8000u;
        state.cpu.data.eflags = 2u;
        lib_memory_copy(state.memory + 4u, handler, sizeof(handler));
        lib_memory_copy(state.memory + 0x2000u, forms[form], length);
        if (mode == 0u) {
            state.memory[0x2000u + length] = 0x90u;
            state.memory[0x2001u + length] = 0x90u;
            state.cpu.data.dr0 = 0x2000u + length;
            state.cpu.data.dr7 = 1u;
        } else {
            lib_memory_copy(state.memory + 0x2000u + length,
                mode == 2u ? forms[form] : write, mode == 2u ? length : sizeof(write));
            if (mode == 2u) lib_memory_copy(state.memory + 0x2000u + 2u * length,
                write, sizeof(write));
            state.cpu.data.dr0 = 0x0800u;
            state.cpu.data.dr7 = 0x00010001u;
        }
        core_machine_cpu_execution_refresh(&state.execution);
        if (state.execution.stop_requested || state.delivered_exception.valid ||
            state.cpu.data.eip != length) return LIB_FALSE;
        core_machine_cpu_execution_refresh(&state.execution);
        if (mode == 0u) {
            if (state.execution.stop_requested || state.delivered_exception.valid ||
                state.cpu.data.eip != length + 1u || state.cpu.data.dr6 != 0u)
                return LIB_FALSE;
            state.cpu.data.dr0 = 0x2001u + length;
            core_machine_cpu_execution_refresh(&state.execution);
            if (!state.delivered_exception.valid || state.cpu.data.eip != 0x0100u ||
                state.delivered_exception.point.eip != length + 1u)
                return LIB_FALSE;
        } else {
            if (mode == 2u) {
                if (state.execution.stop_requested || state.delivered_exception.valid ||
                    state.cpu.data.eip != 2u * length) return LIB_FALSE;
                core_machine_cpu_execution_refresh(&state.execution);
            }
            if (state.execution.stop_requested || !state.delivered_exception.valid ||
                state.cpu.data.eip != 0x0100u || state.cpu.data.dr6 != 1u ||
                state.memory[0x0800u] != 0x5au ||
                state.delivered_exception.point.eip != length * (mode == 2u ? 2u : 1u) +
                    sizeof(write)) return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!debug_state_test_shadow_fault_boundary()) return 1;
    if (!debug_state_test_comparator_status()) return 1;
    if (!debug_state_test_ss_shadow()) return 1;
    if (!debug_state_test_iret_completion()) return 1;
    if (!debug_state_test_ordinary_transfer_rf()) return 1;
    if (!debug_state_test_rf_images()) return 1;
    if (!debug_state_test_popf_completion()) return 1;
    if (!debug_state_test_mov_dr()) {
        lib_c_printf("debug-state stage=mov-dr\n");
        return 1;
    }
    if (!debug_state_test_debug_exceptions()) {
        lib_c_printf("debug-state stage=exceptions\n");
        return 1;
    }
    if (!debug_state_test_data_breakpoints()) {
        lib_c_printf("debug-state stage=data-breakpoints\n");
        return 1;
    }
    lib_c_printf("M5:T539:S46:DEBUG-STATE:OK\n");
    return 0;
}

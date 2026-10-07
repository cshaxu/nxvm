#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"

static lib_u16 pushf_real_flags_image(core_machine_cpu_profile profile,
    lib_u16 flags)
{
    if (profile < CORE_MACHINE_CPU_PROFILE_80286)
        return (lib_u16)((flags & 0x0fd5u) | 0xf002u);
    return (lib_u16)((flags & ~VCPU_EFLAGS_RESERVED) | 0x02u);
}

static lib_u16 pushf_real_flags_known_mask(
    core_machine_cpu_profile profile)
{
    return profile < CORE_MACHINE_CPU_PROFILE_80286 ? 0x0fd5u : 0x7fd5u;
}

static lib_i32 pushf_step(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    state->cpu.data.eip = 0u;
    return cpu_instruction_run(state, code, bytes, after) == LIB_STATUS_OK &&
        !state->fault.valid;
}

static lib_i32 pushf_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static lib_i32 pushf_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static lib_i32 pushf_test_defaults(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 opcodes[] = {0x9cu,0x9du};
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
        VCPU_EFLAGS_OF | 0x8002u;

    for (lib_size profile = 0u; profile < sizeof(profiles) /
        sizeof(profiles[0]); ++profile) {
        for (lib_size form = 0u; form < sizeof(opcodes); ++form) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after = {0};
            lib_u16 image = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_NT |
                0x8000u;
            lib_u16 observed = 0u;
            lib_u16 mask = pushf_real_flags_known_mask(profiles[profile]);

            if (form == 0u && profiles[profile] < CORE_MACHINE_CPU_PROFILE_80286)
                mask |= 0xf000u;

            cpu_instruction_prepare(&state, profiles[profile]);
            state.cpu.data.esp = 0x12348000u;
            state.cpu.data.eflags = flags;
            if (form != 0u) lib_memory_copy(state.memory + 0x8000u,
                &image, sizeof(image));
            before = state.cpu;
            if (!pushf_step(&state, &opcodes[form], 1u, &after) ||
                after.data.eip != 1u ||
                after.data.esp != (form == 0u ? 0x12347ffeu :
                    0x12348002u) ||
                !pushf_gprs_same(&before, &after) ||
                !pushf_sregs_same(&before, &after)) return 0;
            if (form == 0u) {
                lib_memory_copy(&observed, state.memory + 0x7ffeu,
                    sizeof(observed));
                if ((observed & mask) !=
                    (pushf_real_flags_image(profiles[profile],
                        (lib_u16)flags) & mask) ||
                    (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80386 &&
                        (observed & 0x8000u) != 0u)) return 0;
            } else if ((after.data.eflags & mask) !=
                    (pushf_real_flags_image(profiles[profile],
                        profiles[profile] == CORE_MACHINE_CPU_PROFILE_80286 ?
                            (lib_u16)((image & ~(VCPU_EFLAGS_IOPL |
                                VCPU_EFLAGS_NT)) | (flags &
                                (VCPU_EFLAGS_IOPL | VCPU_EFLAGS_NT))) : image)
                        & mask) ||
                (after.data.eflags & 0xffff0000u) !=
                    (flags & 0xffff0000u)) return 0;
        }
    }
    return 1;
}

static lib_i32 pushf_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_bool check_memory)
{
    /* T337_REAL_UD_TERMINAL_CPU_OWNER: no board IVT delivery in this test. */
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after = {0};
    lib_u32 sentinel = 0xa55aa55au;
    lib_u32 low = 0u;
    lib_u32 high = 0u;

    cpu_instruction_prepare(&state, profile);
    state.cpu.data.esp = 0x8000u;
    state.cpu.data.idtr.limit = 0x17u;
    if (check_memory) {
        lib_memory_copy(state.memory + 0x7ffcu, &sentinel,
            sizeof(sentinel));
        lib_memory_copy(state.memory + 0x8000u, &sentinel,
            sizeof(sentinel));
    }
    before = state.cpu;
    if (cpu_instruction_run(&state, code, bytes, &after) !=
            LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
        !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
        lib_memory_compare(&before, &after, sizeof(before)) != 0) return 0;
    if (check_memory) {
        lib_memory_copy(&low, state.memory + 0x7ffcu, sizeof(low));
        lib_memory_copy(&high, state.memory + 0x8000u, sizeof(high));
        if (low != sentinel || high != sentinel) return 0;
    }
    return 1;
}

static lib_i32 pushf_test_attributes_and_rejects(void)
{
    static const lib_u8 prefixes[][2] = {
        {0x66u,0u}, {0x67u,0u}, {0x66u,0x67u}
    };
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };

    for (lib_u8 attribute = 0u; attribute != 3u; ++attribute) {
        for (lib_u8 opcode = 0x9cu; opcode <= 0x9du; ++opcode) {
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after = {0};
            lib_u8 code[] = {prefixes[attribute][0],opcode,0u};
            lib_u8 bytes = attribute == 2u ? 3u : 2u;
            lib_u8 width = attribute == 0u || attribute == 2u ? 4u : 2u;
            lib_u32 image = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF |
                VCPU_EFLAGS_IF;
            lib_u32 observed = 0u;

            if (attribute == 2u) {
                code[1] = prefixes[attribute][1];
                code[2] = opcode;
            }
            cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
            state.cpu.data.esp = 0x12348000u;
            state.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF |
                VCPU_EFLAGS_RF | VCPU_EFLAGS_VM | 0x02u;
            if (opcode == 0x9du)
                lib_memory_copy(state.memory + 0x8000u, &image, width);
            before = state.cpu;
            if (!pushf_step(&state, code, bytes, &after) ||
                after.data.eip != bytes || after.data.esp !=
                    (opcode == 0x9cu ? 0x12348000u - width :
                        0x12348000u + width) ||
                !pushf_gprs_same(&before, &after) ||
                !pushf_sregs_same(&before, &after)) return 0;
            if (opcode == 0x9cu) {
                lib_memory_copy(&observed, state.memory +
                    ((0x12348000u - width) & 0xffffu), width);
                if ((width == 4u ? observed !=
                        ((before.data.eflags & ~(VCPU_EFLAGS_RESERVED |
                            VCPU_EFLAGS_VM | VCPU_EFLAGS_RF)) | 0x02u) :
                        (observed & 0xffffu) !=
                            (((before.data.eflags & ~VCPU_EFLAGS_RESERVED) |
                                0x02u) & 0xffffu)) ||
                    after.data.eflags !=
                        (before.data.eflags & ~VCPU_EFLAGS_RF)) return 0;
            } else if (after.data.eflags != (width == 4u ?
                ((image & ~(VCPU_EFLAGS_RESERVED | VCPU_EFLAGS_RF |
                    VCPU_EFLAGS_VM)) | (before.data.eflags &
                    (VCPU_EFLAGS_RESERVED | VCPU_EFLAGS_VM |
                        VCPU_EFLAGS_RF)) | 0x02u) :
                ((image & ~(VCPU_EFLAGS_RESERVED | 0xffff0000u)) |
                    (before.data.eflags & (VCPU_EFLAGS_RESERVED |
                    0xffff0000u)) | 0x02u))) return 0;
        }
    }
    for (lib_size profile = 0u; profile < sizeof(legacy) /
        sizeof(legacy[0]); ++profile) {
        for (lib_u8 prefix = 0u; prefix != 3u; ++prefix) {
            for (lib_u8 opcode = 0x9cu; opcode <= 0x9du; ++opcode) {
                lib_u8 code[] = {prefixes[prefix][0],opcode,0u};
                lib_u8 bytes = prefix == 2u ? 3u : 2u;

                if (prefix == 2u) {
                    code[1] = prefixes[prefix][1];
                    code[2] = opcode;
                }
                if (!pushf_expect_ud(legacy[profile], code, bytes,
                    LIB_FALSE)) return 0;
            }
        }
    }
    return 1;
}

static lib_i32 pushf_test_lock(void)
{
    static const lib_u8 prefixes[][2] = {
        {0u,0u}, {0x66u,0u}, {0x67u,0u}, {0x66u,0x67u}
    };

    for (lib_u8 attribute = 0u; attribute != 4u; ++attribute) {
        for (lib_u8 opcode = 0x9cu; opcode <= 0x9du; ++opcode) {
            lib_u8 code[] = {0xf0u,opcode,0u,0u};
            lib_u8 bytes = attribute == 0u ? 2u :
                attribute == 3u ? 4u : 3u;

            if (attribute != 0u) {
                code[1] = prefixes[attribute][0];
                code[2] = opcode;
            }
            if (attribute == 3u) {
                code[2] = prefixes[attribute][1];
                code[3] = opcode;
            }
            if (!pushf_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, code,
                bytes, LIB_TRUE)) return 0;
        }
    }
    return 1;
}

static lib_i32 pushf_test_legacy_forms(void)
{
    static const lib_u8 forms[][2] = {
        {0x9cu,0u}, {0x66u,0x9cu}, {0x9du,0u}, {0x66u,0x9du}
    };
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;

    for (lib_u8 form = 0u; form != 4u; ++form) {
        cpu_instruction_fixture state;
        t_cpu after = {0};
        lib_u32 image = form < 2u ? 0u :
            VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF;
        lib_u8 bytes = form == 1u || form == 3u ? 2u : 1u;
        lib_u8 width = form == 1u || form == 3u ? 4u : 2u;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.esp = 0x8000u;
        state.cpu.data.eflags = flags;
        if (form >= 2u)
            lib_memory_copy(state.memory + 0x8000u, &image, width);
        if (!pushf_step(&state, forms[form], bytes, &after) ||
            after.data.eip != bytes ||
            after.data.esp != 0x8000u + (form < 2u ?
                (lib_u32)(0u - width) : width)) return 0;
        if (form < 2u) {
            lib_memory_copy(&image, state.memory + (lib_u16)after.data.esp,
                width);
            if (image != (form == 0u ?
                (((flags & ~VCPU_EFLAGS_RESERVED) | 0x02u) & 0xffffu) :
                ((flags & ~(VCPU_EFLAGS_RESERVED | VCPU_EFLAGS_VM |
                    VCPU_EFLAGS_RF)) | 0x02u))) return 0;
        } else if ((after.data.eflags &
            (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF)) != image ||
            (form == 2u && (after.data.eflags & 0xffff0000u) !=
                (flags & 0xffff0000u)) ||
            (form == 3u && (after.data.eflags &
                (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF |
                    (VCPU_EFLAGS_RESERVED & ~0x02u))) !=
                (flags & (VCPU_EFLAGS_VM | VCPU_EFLAGS_RF |
                    (VCPU_EFLAGS_RESERVED & ~0x02u))))) return 0;
    }
    return 1;
}

static void pushf_prepare_protected(cpu_instruction_fixture *state,
    core_machine_cpu_profile profile)
{
    cpu_instruction_prepare(state, profile);
    state->cpu.data.cr0 |= VCPU_CR0_PE;
    state->cpu.data.cs.flagValid = LIB_TRUE;
    state->cpu.data.cs.selector = 0x08u;
    state->cpu.data.cs.sregtype = SREG_CODE;
    state->cpu.data.cs.base = 0x2000u;
    state->cpu.data.cs.limit = 0xffffu;
    state->cpu.data.cs.seg.executable = LIB_TRUE;
    state->cpu.data.cs.seg.exec.readable = LIB_TRUE;
    state->cpu.data.cs.seg.exec.defsize = LIB_FALSE;
    state->cpu.data.ss.flagValid = LIB_TRUE;
    state->cpu.data.ss.selector = 0x18u;
    state->cpu.data.ss.sregtype = SREG_STACK;
    state->cpu.data.ss.base = 0x4000u;
    state->cpu.data.ss.limit = 0xffffu;
    state->cpu.data.ss.seg.data.writable = LIB_TRUE;
    state->cpu.data.ss.seg.data.big = LIB_FALSE;
    state->cpu.data.esp = 0x8000u;
}

static lib_status pushf_protected_run(cpu_instruction_fixture *state,
    const lib_u8 *code, lib_u8 bytes, t_cpu *after)
{
    lib_memory_copy(state->memory + state->cpu.data.cs.base, code, bytes);
    core_machine_cpu_execution_refresh(&state->execution);
    *after = state->cpu;
    return state->execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR :
        LIB_STATUS_OK;
}

static lib_i32 pushf_test_protected_iopl(void)
{
    static const lib_u8 popfw[] = {0x9du};

    for (lib_u8 pass = 0u; pass != 3u; ++pass) {
        cpu_instruction_fixture state;
        t_cpu after = {0};
        const lib_u32 iopl = pass == 2u ? VCPU_EFLAGS_IOPL : 0u;
        const lib_u32 image = VCPU_EFLAGS_IF | VCPU_EFLAGS_ZF |
            VCPU_EFLAGS_IOPL;
        const lib_u32 expected_if = pass == 1u ? 0u : VCPU_EFLAGS_IF;
        const lib_u32 expected_iopl = pass == 0u ? VCPU_EFLAGS_IOPL : iopl;

        pushf_prepare_protected(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.cs.dpl = pass == 0u ? 0u : 3u;
        state.cpu.data.ss.dpl = state.cpu.data.cs.dpl;
        state.cpu.data.eflags = VCPU_EFLAGS_CF | iopl;
        lib_memory_copy(state.memory + 0xc000u, &image, 2u);
        if (pushf_protected_run(&state, popfw, sizeof(popfw), &after) !=
                LIB_STATUS_OK || state.fault.valid ||
            (after.data.eflags & VCPU_EFLAGS_IF) != expected_if ||
            (after.data.eflags & VCPU_EFLAGS_IOPL) != expected_iopl ||
            (after.data.eflags & VCPU_EFLAGS_ZF) != VCPU_EFLAGS_ZF ||
            after.data.eip != 1u) {
            lib_c_fprintf(lib_c_stderr, "iopl pass=%u flags=%08x eip=%08x fault=%u\n",
                pass, after.data.eflags, after.data.eip,
                state.fault.valid);
            return 0;
        }
    }
    return 1;
}

static lib_i32 pushf_test_stack_faults(void)
{
    static const lib_u8 forms[][2] = {{0x9cu,0u}, {0x66u,0x9du}};

    for (lib_u8 pass = 0u; pass != 2u; ++pass) {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after = {0};
        lib_u32 image = VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF;
        lib_u32 observed = 0u;

        pushf_prepare_protected(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        if (pass) state.cpu.data.ss.limit = 0x7fffu;
        else state.cpu.data.ss.seg.data.writable = LIB_FALSE;
        lib_memory_copy(state.memory + 0xc000u, &image, sizeof(image));
        before = state.cpu;
        if (pushf_protected_run(&state, forms[pass], pass ? 2u : 1u,
            &after) != LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
            after.data.eip != 0u || after.data.esp != 0x8000u ||
            after.data.eflags != before.data.eflags) return 0;
        lib_memory_copy(&observed, state.memory + 0xc000u,
            sizeof(observed));
        if (observed != image) return 0;
    }
    return 1;
}

static void pushf_prepare_vm86(cpu_instruction_fixture *state,
    lib_u32 flags)
{
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0
    };
    const lib_u8 gate[] = {0u,1u,0x08u,0u,0u,0x8eu,0u,0u};
    lib_u8 tss[10] = {0};

    cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
    tss[5] = 0x90u;
    tss[8] = 0x10u;
    lib_memory_copy(state->memory + 0x300u, gdt, sizeof(gdt));
    lib_memory_copy(state->memory + 0x400u + 0x0du * 8u, gate,
        sizeof(gate));
    lib_memory_copy(state->memory + 0x500u, tss, sizeof(tss));
    state->memory[0x2100u] = 0xf4u;
    state->cpu.data.cr0 |= VCPU_CR0_PE;
    state->cpu.data.gdtr.flagValid = LIB_TRUE;
    state->cpu.data.gdtr.sregtype = SREG_GDTR;
    state->cpu.data.gdtr.base = 0x300u;
    state->cpu.data.gdtr.limit = 31u;
    state->cpu.data.idtr.flagValid = LIB_TRUE;
    state->cpu.data.idtr.sregtype = SREG_IDTR;
    state->cpu.data.idtr.base = 0x400u;
    state->cpu.data.idtr.limit = 0x6fu;
    state->cpu.data.cs.flagValid = LIB_TRUE;
    state->cpu.data.cs.selector = 0u;
    state->cpu.data.cs.base = 0u;
    state->cpu.data.cs.limit = 0xffffu;
    state->cpu.data.cs.dpl = 3u;
    state->cpu.data.cs.seg.exec.defsize = LIB_FALSE;
    state->cpu.data.ss.flagValid = LIB_TRUE;
    state->cpu.data.ss.selector = 0u;
    state->cpu.data.ss.base = 0u;
    state->cpu.data.ss.limit = 0xffffu;
    state->cpu.data.ss.dpl = 3u;
    state->cpu.data.ss.seg.data.big = LIB_FALSE;
    state->cpu.data.tr.flagValid = LIB_TRUE;
    state->cpu.data.tr.selector = 0x28u;
    state->cpu.data.tr.sregtype = SREG_TR;
    state->cpu.data.tr.base = 0x500u;
    state->cpu.data.tr.limit = 0x67u;
    state->cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    state->cpu.data.esp = 0x8000u;
    state->cpu.data.eflags = flags;
}

static lib_i32 pushf_test_vm86(void)
{
    static const lib_u8 forms[][2] = {
        {0x9cu,0u}, {0x9du,0u}, {0x66u,0x9cu}, {0x66u,0x9du}
    };

    for (lib_u8 form = 0u; form != 4u; ++form) {
        for (lib_u8 pass = 0u; pass != (form == 2u ? 1u : 4u);
            ++pass) {
            cpu_instruction_fixture state;
            t_cpu after = {0};
            lib_u32 flags = VCPU_EFLAGS_VM | VCPU_EFLAGS_CF |
                (pass == 0u ? VCPU_EFLAGS_IOPL :
                    ((lib_u32)(pass - 1u) << 12u)) |
                (form == 2u ? VCPU_EFLAGS_RF : 0u);
            const lib_u32 image = VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF |
                (form == 3u ? VCPU_EFLAGS_RF | VCPU_EFLAGS_VM : 0u);
            lib_u32 observed = 0u;
            lib_status status;

            pushf_prepare_vm86(&state, flags);
            if (form == 1u || form == 3u)
                lib_memory_copy(state.memory + 0x8000u, &image,
                    form == 1u ? 2u : 4u);
            status = cpu_instruction_run(&state, forms[form],
                form >= 2u ? 2u : 1u, &after);
            if (status != LIB_STATUS_OK || state.fault.valid) return 0;
            if (pass != 0u) {
                if (after.data.cs.selector != 0x08u ||
                    after.data.ss.selector != 0x10u ||
                    after.data.eip != 0x100u ||
                    (after.data.eflags & VCPU_EFLAGS_VM) != 0u) return 0;
            } else if (form == 0u &&
                (after.data.esp != 0x7ffeu || after.data.eip != 1u))
                return 0;
            else if (form == 1u &&
                (after.data.esp != 0x8002u || after.data.eip != 1u ||
                (after.data.eflags & (VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF)) !=
                    image)) return 0;
            else if (form == 2u) {
                lib_memory_copy(&observed, state.memory + 0x7ffcu,
                    sizeof(observed));
                if (after.data.eip != 2u || after.data.esp != 0x7ffcu ||
                    observed != ((flags & ~(VCPU_EFLAGS_VM |
                        VCPU_EFLAGS_RF | VCPU_EFLAGS_RESERVED)) | 0x02u))
                    return 0;
            }
            else if (form == 3u &&
                (after.data.eip != 2u || after.data.esp != 0x8004u ||
                (after.data.eflags & (VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL |
                    VCPU_EFLAGS_RF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF)) !=
                    (VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_ZF |
                        VCPU_EFLAGS_IF))) {
                lib_c_printf("VM86 POPFD flags=%x ip=%x sp=%x\n",
                    (unsigned)after.data.eflags, (unsigned)after.data.eip,
                    (unsigned)after.data.esp);
                return 0;
            }
        }
    }
    return 1;
}

static lib_bool pushf_test_286_real_preservation(void)
{
    const lib_u8 code[] = {0x9du};
    const lib_u32 privileged = VCPU_EFLAGS_NT | VCPU_EFLAGS_IOPL;
    lib_u8 old_bits, popped_bits;
    lib_bool passed = LIB_TRUE;

    for (old_bits = 0u; old_bits < 8u; ++old_bits)
    for (popped_bits = 0u; popped_bits < 8u; ++popped_bits) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        const lib_u16 popped = (lib_u16)(((lib_u32)popped_bits << 12u) | 0x0cd5u);
        lib_u32 expected;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);
        state.cpu.data.eflags = ((lib_u32)old_bits << 12u) | 0x02u;
        state.cpu.data.sp = 0x8000u;
        before = state.cpu;
        expected = (before.data.eflags & privileged) | (popped & 0x0fd5u);
        if (cpu_instruction_write(&state, 0x8000u, &popped, sizeof(popped),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, code, sizeof(code), &after) != LIB_STATUS_OK ||
            state.fault.valid || (after.data.eflags & 0x7fd5u) != expected ||
            after.data.sp != 0x8002u || after.data.eip != 1u ||
            !pushf_gprs_same(&before, &after) || !pushf_sregs_same(&before, &after)) {
            lib_c_printf("286 real POPF old=%u popped=%u flags=%x expected=%x\n",
                (unsigned)old_bits, (unsigned)popped_bits,
                (unsigned)after.data.eflags, (unsigned)expected);
            passed = LIB_FALSE;
        }
    }
    return passed;
}

static lib_bool pushf_test_protected_privilege_matrix(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 forms[][2] = {{0x9du,0u}, {0x66u,0x9du}};
    lib_u32 failures[2] = {0u,0u}, cases[2] = {0u,0u};
    lib_size profile;
    lib_u8 form, cpl, old_bits, popped_bits, if_bits;

    for (profile = 0u; profile < 2u; ++profile)
    for (form = 0u; form < (profile == 0u ? 1u : 2u); ++form)
    for (cpl = 0u; cpl < 4u; ++cpl)
    for (old_bits = 0u; old_bits < 8u; ++old_bits)
    for (popped_bits = 0u; popped_bits < 8u; ++popped_bits)
    for (if_bits = 0u; if_bits < 4u; ++if_bits) {
        cpu_instruction_fixture state;
        t_cpu before, after;
        const lib_u8 width = form == 0u ? 2u : 4u;
        const lib_u32 popped = ((lib_u32)popped_bits << 12u) | 0x0cd5u |
            ((if_bits & 2u) != 0u ? VCPU_EFLAGS_IF : 0u);
        lib_u32 expected;

        pushf_prepare_protected(&state, profiles[profile]);
        state.cpu.data.cs.dpl = cpl;
        state.cpu.data.cs.selector |= cpl;
        state.cpu.data.ss.dpl = cpl;
        state.cpu.data.ss.selector |= cpl;
        state.cpu.data.eflags = ((lib_u32)old_bits << 12u) | 0x02u |
            ((if_bits & 1u) != 0u ? VCPU_EFLAGS_IF : 0u);
        before = state.cpu;
        expected = popped;
        if (cpl != 0u)
            expected = (expected & ~VCPU_EFLAGS_IOPL) |
                (before.data.eflags & VCPU_EFLAGS_IOPL);
        if (cpl > (old_bits & 3u))
            expected = (expected & ~VCPU_EFLAGS_IF) |
                (before.data.eflags & VCPU_EFLAGS_IF);
        lib_memory_copy(state.memory + 0xc000u, &popped, width);
        ++cases[profile];
        if (pushf_protected_run(&state, forms[form], form + 1u, &after) !=
                LIB_STATUS_OK || state.fault.valid ||
            (after.data.eflags & 0x7fd5u) != expected ||
            after.data.sp != 0x8000u + width || after.data.eip != form + 1u ||
            !pushf_gprs_same(&before, &after) || !pushf_sregs_same(&before, &after))
            ++failures[profile];
    }
    for (profile = 0u; profile < 2u; ++profile)
        lib_c_printf("protected POPF profile=%u cases=%u failures=%u\n",
            (unsigned)profiles[profile], (unsigned)cases[profile],
            (unsigned)failures[profile]);
    return failures[0] == 0u && failures[1] == 0u;
}

static lib_bool pushf_test_starting_sp(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u16 starts[] = {0u,1u,2u,0xffffu};
    lib_size profile, start;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (start = 0u; start < sizeof(starts) / sizeof(starts[0]); ++start) {
        cpu_instruction_fixture state;
        t_cpu after;
        const lib_u8 opcode = 0x9cu;
        const lib_bool shutdown = profiles[profile] >= CORE_MACHINE_CPU_PROFILE_80286 &&
            starts[start] == 1u;
        const lib_u16 expected_sp = (lib_u16)(starts[start] - 2u);
        lib_u16 image = 0u;

        cpu_instruction_prepare(&state, profiles[profile]);
        state.cpu.data.sp = starts[start];
        state.cpu.data.eflags = 0x43u;
        state.cpu.data.cs.base = 0x2000u;
        state.memory[0x2000u] = opcode;
        core_machine_cpu_execution_refresh(&state.execution);
        after = state.cpu;
        if (shutdown) {
            if (!state.execution.shutdown_requested || !state.execution.stop_requested ||
                !state.delivered_exception.valid ||
                state.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
                after.data.eip != 0u || after.data.sp != starts[start])
                return LIB_FALSE;
        } else {
            lib_memory_copy(&image, state.memory + expected_sp, 1u);
            lib_memory_copy((lib_u8 *)&image + 1u,
                state.memory + (lib_u16)(expected_sp + 1u), 1u);
            if (state.execution.stop_requested || state.fault.valid ||
                after.data.eip != 1u || after.data.sp != expected_sp ||
                image != pushf_real_flags_image(profiles[profile], 0x43u)) {
                lib_c_printf("PUSHF SP profile=%u start=%x stop=%u ip=%x sp=%x image=%x\n",
                    (unsigned)profiles[profile], starts[start],
                    state.execution.stop_requested, after.data.eip, after.data.sp, image);
                return LIB_FALSE;
            }
        }
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!pushf_test_starting_sp()) return 1;
    lib_bool real_ok = pushf_test_286_real_preservation();
    lib_bool protected_ok = pushf_test_protected_privilege_matrix();
    lib_bool vm86_ok = pushf_test_vm86();

    if (!real_ok || !protected_ok || !vm86_ok) return 1;
    if (!pushf_test_defaults()) { lib_c_fprintf(lib_c_stderr, "%s", "defaults\n"); return 1; }
    if (!pushf_test_attributes_and_rejects()) {
        lib_c_fprintf(lib_c_stderr, "%s", "attributes\n"); return 1;
    }
    if (!pushf_test_lock()) { lib_c_fprintf(lib_c_stderr, "%s", "lock\n"); return 1; }
    if (!pushf_test_legacy_forms()) {
        lib_c_fprintf(lib_c_stderr, "%s", "legacy forms\n"); return 1;
    }
    if (!pushf_test_protected_iopl()) {
        lib_c_fprintf(lib_c_stderr, "%s", "protected iopl\n"); return 1;
    }
    if (!pushf_test_stack_faults()) {
        lib_c_fprintf(lib_c_stderr, "%s", "stack faults\n"); return 1;
    }
    lib_c_printf("%s\n", "M5:T316:S21:PUSHF-POPF:OK");
    lib_c_printf("%s\n", "M5:T316:S47:PUSHF-POPF:OK");
    lib_c_printf("%s\n", "M5:T401:S39:PUSHF-POPF-PROFILES:OK");
    lib_c_printf("%s\n", "M5:T539:S36:CPU-PUSHF-POPF:OK");
    return 0;
}

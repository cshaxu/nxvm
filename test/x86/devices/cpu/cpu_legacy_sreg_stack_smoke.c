#include "support/cpu_instruction_fixture.h"
#include <stdio.h>

/* T337_REAL_UD_TERMINAL_CPU_OWNER: terminal-UD assertions stay CPU-owned. */
static lib_i32 legacy_sreg_stack_test_lock(void)
{
    static const lib_u8 opcodes[] = {0x06u,0x07u,0x0eu,0x16u,0x17u,0x1eu,0x1fu};
    lib_u8 index;

    for (index = 0u; index != sizeof(opcodes); ++index)
    {
        cpu_instruction_fixture state;

        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_u8 code[] = {0xf0u, opcodes[index]};
        lib_u32 sentinel = 0xdeadbeefu;
        lib_u32 observed = 0u;
        lib_i32 failed;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        failed = 0;
        if (!failed)
        {
            state.cpu.data.esp = 0x12348000u;
            failed |= cpu_instruction_write(&state, 0x7ffcu, &sentinel, sizeof(sentinel), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            state.cpu.data.idtr.limit = 0x17u;
            before = state.cpu;
            failed |= cpu_instruction_write(&state, 0u, code, sizeof(code), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
                LIB_STATUS_OK;
            status = (core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
            after = state.cpu;
            failed |= status != LIB_STATUS_INTERNAL_ERROR || !state.execution.stop_requested || !state.fault.valid ||
                !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
                lib_memory_compare(&before, &after, sizeof(before)) != 0 ||
                cpu_instruction_read(&state, 0x7ffcu, &observed, sizeof(observed), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || observed != sentinel;
        }

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_sreg_stack_gprs_same_except_esp(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static lib_i32 legacy_sreg_stack_sregs_same(const t_cpu *before, const t_cpu *after)
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

static const t_cpu_data_sreg *legacy_sreg_stack_target(const t_cpu *cpu,
    lib_u8 target)
{
    if (target == 0u)
        return &cpu->data.es;
    if (target == 1u)
        return &cpu->data.ss;
    return &cpu->data.ds;
}

static lib_i32 legacy_sreg_stack_non_target_sregs_same(const t_cpu *before,
    const t_cpu *after, lib_u8 target)
{
    return (target == 0u || lib_memory_compare(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0) &&
        lib_memory_compare(&before->data.cs, &after->data.cs,
        sizeof(before->data.cs)) == 0 &&
        (target == 1u || lib_memory_compare(&before->data.ss, &after->data.ss,
        sizeof(before->data.ss)) == 0) &&
        (target == 2u || lib_memory_compare(&before->data.ds, &after->data.ds,
        sizeof(before->data.ds)) == 0) &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
        sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
        sizeof(before->data.gs)) == 0;
}

static lib_i32 legacy_sreg_stack_real_cache(const t_cpu_data_sreg *sreg,
    lib_u16 selector, lib_u8 target)
{
    return sreg->selector == selector && sreg->flagValid &&
        sreg->base == (lib_u32)selector << 4u && sreg->limit == 0xffffu &&
        sreg->sregtype == (target == 1u ? SREG_STACK : SREG_DATA) &&
        !sreg->seg.executable && sreg->seg.data.writable &&
        !sreg->seg.data.big && !sreg->seg.data.expdown;
}

static lib_i32 legacy_sreg_stack_protected_cache(const t_cpu_data_sreg *sreg,
    lib_u16 selector, lib_u8 target)
{
    return sreg->selector == selector && sreg->flagValid &&
        sreg->base == 0x5000u && sreg->limit == 0xffffu &&
        sreg->sregtype == (target == 1u ? SREG_STACK : SREG_DATA) &&
        !sreg->seg.executable && sreg->seg.data.writable &&
        !sreg->seg.data.big && !sreg->seg.data.expdown;
}

static lib_i32 legacy_sreg_stack_test_defaults(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 push_ops[] = {0x06u,0x0eu,0x16u,0x1eu};
    static const lib_u8 pop_ops[] = {0x07u,0x17u,0x1fu};
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    {
        lib_u8 form;

        for (form = 0u; form != sizeof(push_ops); ++form)
        {
            cpu_instruction_fixture state;

            t_cpu before;
            t_cpu after;
            lib_status status;
            lib_u32 image = 0u;
            lib_i32 failed = 0;
            cpu_instruction_prepare(&state, profiles[profile]);

            if (!failed)
            {
                state.cpu.data.esp = 0x00008000u;
                state.cpu.data.es.selector = 0x1111u;
                state.cpu.data.cs.selector = 0x2222u;
                state.cpu.data.ss.selector = 0x3333u;
                state.cpu.data.ds.selector = 0x4444u;
                before = state.cpu;
                failed |= cpu_instruction_write(&state, 0u, &push_ops[form], 1u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
                status = (core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
                after = state.cpu;
                failed |= status != LIB_STATUS_OK || state.fault.valid || after.data.eip != 1u ||
                    after.data.esp != 0x00007ffeu || after.data.eflags != before.data.eflags ||
                    !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                    !legacy_sreg_stack_sregs_same(&before, &after) ||
                    cpu_instruction_read(&state, 0x7ffeu, &image, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || image !=
                    (form == 0u ? 0x1111u : form == 1u ? 0x2222u : form == 2u ? 0x3333u : 0x4444u);
            }

            if (failed)
                return 0;
        }
        for (form = 0u; form != sizeof(pop_ops); ++form)
        {
            cpu_instruction_fixture state;

            t_cpu before;
            t_cpu after;
            lib_status status;
            lib_u16 selector = (lib_u16)(0x5555u + form);
            lib_i32 failed = 0;
            cpu_instruction_prepare(&state, profiles[profile]);

            if (!failed)
            {
                state.cpu.data.esp = 0x00008000u;
                failed |= cpu_instruction_write(&state, 0x8000u, &selector, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
                    cpu_instruction_write(&state, 0u, &pop_ops[form], 1u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
                before = state.cpu;
                status = (core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
                after = state.cpu;
                failed |= status != LIB_STATUS_OK || state.fault.valid || after.data.eip != 1u ||
                    after.data.esp != 0x00008002u || after.data.eflags != before.data.eflags ||
                    after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
                    after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
                    after.data.ebp != before.data.ebp || after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
                    !legacy_sreg_stack_non_target_sregs_same(&before, &after, form) ||
                    !legacy_sreg_stack_real_cache(legacy_sreg_stack_target(&after, form),
                    selector, form);
            }

            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 legacy_sreg_stack_test_attributes(void)
{
    static const lib_u8 prefixes[][2] = {
        {0x66u, 0u}, {0x67u, 0u}, {0x66u, 0x67u}
    };
    static const lib_u8 push_ops[] = {0x06u, 0x0eu, 0x16u, 0x1eu};
    static const lib_u8 push_selectors[] = {0x11u, 0x22u, 0x33u, 0x44u};
    static const lib_u8 pop_ops[] = {0x07u, 0x17u, 0x1fu};
    static const core_machine_cpu_profile legacy[] = {CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286};
    lib_u8 attribute;
    lib_u8 form;
    lib_u8 profile;

    for (attribute = 0u; attribute != sizeof(prefixes) / sizeof(prefixes[0]);
        ++attribute)
    {
        for (form = 0u; form != sizeof(push_ops); ++form)
        {
            cpu_instruction_fixture state;

            t_cpu before;
            t_cpu after;
            lib_status status;
            lib_u8 code[] = {prefixes[attribute][0], push_ops[form], 0u};
            lib_u8 bytes = attribute == 2u ? 3u : 2u;
            lib_u16 image = 0u;
            lib_u16 selector = push_selectors[form];
            lib_u8 width = attribute == 0u || attribute == 2u ? 4u : 2u;
            lib_u32 expected_esp = 0x12348000u - width;
            lib_i32 failed = 0;
            cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

            if (!failed)
            {
                if (attribute == 2u)
                {
                    code[1] = prefixes[attribute][1];
                    code[2] = push_ops[form];
                }
                state.cpu.data.esp = 0x12348000u;
                state.cpu.data.es.selector = 0x0011u;
                state.cpu.data.cs.selector = 0x0022u;
                state.cpu.data.ss.selector = 0x0033u;
                state.cpu.data.ds.selector = 0x0044u;
                before = state.cpu;
                failed |= cpu_instruction_write(&state, 0u, code, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
                status = (core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
                after = state.cpu;
                failed |= status != LIB_STATUS_OK ||
                    state.fault.valid || after.data.eip != bytes ||
                    after.data.esp != expected_esp ||
                    after.data.eflags != before.data.eflags ||
                    !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                    !legacy_sreg_stack_sregs_same(&before, &after) ||
                    cpu_instruction_read(&state, expected_esp & 0xffffu, &image, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                    LIB_STATUS_OK || image != selector;
            }

            if (failed)
                return 0;
        }
        for (form = 0u; form != sizeof(pop_ops); ++form)
        {
            cpu_instruction_fixture state;

            t_cpu before;
            t_cpu after;
            lib_status status;
            lib_u8 code[] = {prefixes[attribute][0], pop_ops[form], 0u};
            lib_u8 bytes = attribute == 2u ? 3u : 2u;
            lib_u8 width = attribute == 0u || attribute == 2u ? 4u : 2u;
            lib_u16 selector = (lib_u16)(0x1110u + attribute * 3u + form);
            lib_i32 failed = 0;
            cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

            if (!failed)
            {
                if (attribute == 2u)
                {
                    code[1] = prefixes[attribute][1];
                    code[2] = pop_ops[form];
                }
                state.cpu.data.esp = 0x12348000u;
                failed |= cpu_instruction_write(&state, 0x8000u, &selector, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || cpu_instruction_write(&state, 0u, code, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
                before = state.cpu;
                status = (core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
                after = state.cpu;
                failed |= status != LIB_STATUS_OK ||
                    state.fault.valid || after.data.eip != bytes ||
                    after.data.esp != 0x12348000u + width ||
                    after.data.eflags != before.data.eflags ||
                    !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                    !legacy_sreg_stack_non_target_sregs_same(&before, &after, form) ||
                    !legacy_sreg_stack_real_cache(legacy_sreg_stack_target(&after, form),
                    selector, form);
            }

            if (failed)
                return 0;
        }
    }
    for (profile = 0u; profile != sizeof(legacy)/sizeof(legacy[0]); ++profile)
    {
        for (attribute = 0u; attribute != sizeof(prefixes) / sizeof(prefixes[0]);
            ++attribute)
        {
            for (form = 0u; form != sizeof(push_ops) + sizeof(pop_ops); ++form)
            {
                cpu_instruction_fixture state;

                t_cpu before;
                t_cpu after;
                lib_status status;
                lib_u8 opcode = form < sizeof(push_ops) ? push_ops[form] :
                    pop_ops[form - sizeof(push_ops)];
                lib_u8 code[] = {prefixes[attribute][0], opcode, 0u};
                lib_u8 bytes = attribute == 2u ? 3u : 2u;
                lib_i32 failed = 0;
                cpu_instruction_prepare(&state, legacy[profile]);

                if (!failed)
                {
                    if (attribute == 2u)
                    {
                        code[1] = prefixes[attribute][1];
                        code[2] = opcode;
                    }
                    state.cpu.data.idtr.limit = 0x17u;
                    before = state.cpu;
                    failed |= cpu_instruction_write(&state, 0u, code, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
                    status = (core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
                    after = state.cpu;
                    failed |= status != LIB_STATUS_INTERNAL_ERROR ||
                        !state.fault.valid || !(state.fault.exception_mask & VCPUINS_EXCEPT_UD) ||
                        lib_memory_compare(&before, &after, sizeof(before)) != 0;
                }

                if (failed)
                    return 0;
            }
        }
    }
    return 1;
}

static lib_i32 legacy_sreg_stack_boot_protected(cpu_instruction_fixture *state)
{
    static const lib_u8 pointer[] = {0x37u,0u,0u,0x03u,0u,0u};
    static const lib_u8 gdt[] = {0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x92u,0,0,
        0xffu,0xffu,0,0x50u,0,0x98u,0,0,
        0xffu,0xffu,0,0x50u,0,0x12u,0,0};
    static const lib_u8 boot[] = {0x0fu,1u,0x16u,0,1u,0xb8u,1u,0,
        0x0fu,1u,0xf0u,0xb8u,0x10u,0,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0,0x8eu,0xd0u,0xbcu,0,0x80u,0xeau,0,0,8u,0};
    lib_memory_copy(state->memory + 0x100u, pointer, sizeof(pointer));
    lib_memory_copy(state->memory + 0x300u, gdt, sizeof(gdt));
    lib_memory_copy(state->memory, boot, sizeof(boot));
    for (lib_u8 step = 0u; step != 10u; ++step) {
        core_machine_cpu_execution_refresh(&state->execution);
        if (state->execution.stop_requested || state->fault.valid) return 0;
    }
    return state->cpu.data.cs.selector == 8u && state->cpu.data.eip == 0u;
}

static lib_i32 legacy_sreg_stack_test_protected_pop(void)
{
    static const lib_u8 opcodes[] = {0x07u,0x17u,0x1fu};
    lib_u8 form;

    for (form = 0u; form != 3u; ++form)
    {
        cpu_instruction_fixture state;

        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_u16 selector = 0x20u;
        lib_u8 access = 0u;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed)
            failed |= !legacy_sreg_stack_boot_protected(&state);
        if (!failed)
        {
            failed |= cpu_instruction_write(&state, 0xc000u, &selector, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || cpu_instruction_write(&state, 0x2000u, &opcodes[form], 1u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            before = state.cpu;
            state.cpu.data.eip = 0u;
            status = (core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
            after = state.cpu;
            failed |= status != LIB_STATUS_OK || state.fault.valid ||
                after.data.eip != 1u || after.data.esp != 0x00008002u ||
                after.data.eflags != before.data.eflags ||
                !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                !legacy_sreg_stack_non_target_sregs_same(&before, &after, form) ||
                !legacy_sreg_stack_protected_cache(
                legacy_sreg_stack_target(&after, form), selector, form) ||
                cpu_instruction_read(&state, 0x325u, &access, sizeof(access), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                access != 0x93u;
        }

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_sreg_stack_test_protected_null(void)
{
    static const lib_u8 opcodes[] = {0x07u, 0x1fu};
    lib_u8 form;

    for (form = 0u; form != sizeof(opcodes); ++form)
    {
        cpu_instruction_fixture state;

        t_cpu before;
        t_cpu after;
        lib_status status;
        lib_u16 selector = 0u;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed)
            failed |= !legacy_sreg_stack_boot_protected(&state);
        if (!failed)
        {
            failed |= cpu_instruction_write(&state, 0xc000u, &selector, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || cpu_instruction_write(&state, 0x2000u, &opcodes[form], 1u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            before = state.cpu;
            state.cpu.data.eip = 0u;
            status = (core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
            after = state.cpu;
            failed |= status != LIB_STATUS_OK || state.fault.valid ||
                after.data.eip != 1u || after.data.esp != 0x8002u ||
                after.data.eflags != before.data.eflags ||
                !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                !legacy_sreg_stack_non_target_sregs_same(&before, &after,
                form == 0u ? 0u : 2u) || legacy_sreg_stack_target(&after,
                form == 0u ? 0u : 2u)->selector != 0u ||
                legacy_sreg_stack_target(&after, form == 0u ? 0u : 2u)->flagValid;
        }

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_sreg_stack_test_protected_ss_null(void)
{
    cpu_instruction_fixture state;
    t_cpu before,after; lib_status status;
    lib_u16 selector=0u; lib_i32 failed = 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if(!failed) failed|=!legacy_sreg_stack_boot_protected(&state);
    if(!failed) {
        failed|=cpu_instruction_write(&state, 0xc000u, &selector, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA)!=LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x2000u, (lib_u8[]){0x17u}, 1u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA)!=LIB_STATUS_OK;
        before=state.cpu;
        state.cpu.data.eip = 0u;
        status=(core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
        after=state.cpu;
        failed|=status!=LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
            !(state.fault.exception_mask & VCPUINS_EXCEPT_DF) || after.data.eip!=0u ||
            after.data.esp!=before.data.esp || after.data.eflags!=before.data.eflags ||
            !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
            !legacy_sreg_stack_sregs_same(&before, &after);
    }
    return !failed;
}

static lib_i32 legacy_sreg_stack_test_protected_rejects(void)
{
    static const lib_u8 opcodes[] = {0x07u,0x17u,0x1fu};
    static const lib_u16 selectors[] = {0x28u,0x23u,0x30u};
    lib_u8 target,kind;
    for(target=0u;target!=3u;++target) for(kind=0u;kind!=3u;++kind) {
        cpu_instruction_fixture state;
        t_cpu before,after; lib_status status;
        lib_u16 selector=selectors[kind], observed=0u;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if(!failed) failed|=!legacy_sreg_stack_boot_protected(&state);
        if(!failed) {
            failed|=cpu_instruction_write(&state, 0xc000u, &selector, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA)!=LIB_STATUS_OK ||
                cpu_instruction_write(&state, 0x2000u, &opcodes[target], 1u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA)!=LIB_STATUS_OK;
            before=state.cpu;
            state.cpu.data.eip = 0u;
            status=(core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
            after=state.cpu;
            failed|=status!=LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
                !(state.fault.exception_mask & VCPUINS_EXCEPT_DF) || after.data.eip!=0u ||
                after.data.esp!=before.data.esp || after.data.eax!=before.data.eax || after.data.ecx!=before.data.ecx ||
                after.data.edx!=before.data.edx || after.data.ebx!=before.data.ebx || after.data.ebp!=before.data.ebp ||
                after.data.esi!=before.data.esi || after.data.edi!=before.data.edi || after.data.eflags!=before.data.eflags ||
                !legacy_sreg_stack_sregs_same(&before, &after) ||
                cpu_instruction_read(&state, 0xc000u, &observed, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE)!=LIB_STATUS_OK || observed!=selector;
        }
        if(failed)return 0;
    }return 1;
}

static lib_i32 legacy_sreg_stack_test_protected_stack_limits(void)
{
    static const lib_u8 opcodes[] = {0x06u,0x07u};
    lib_u8 form;
    for(form=0u;form!=2u;++form) {
        cpu_instruction_fixture state;
        t_cpu before,after; lib_status status;
        lib_u16 image=0xbe5au; lib_u32 candidate=form==0u?0xbffeu:0xc000u;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if(!failed) failed|=!legacy_sreg_stack_boot_protected(&state);
        if(!failed) {
            state.cpu.data.ss.limit=form==0u?0xffffu:0x7fffu;
            state.cpu.data.ss.seg.data.expdown=form==0u?LIB_TRUE:LIB_FALSE;
            failed|=cpu_instruction_write(&state, candidate, &image, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA)!=LIB_STATUS_OK ||
                cpu_instruction_write(&state, 0x2000u, &opcodes[form], 1u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA)!=LIB_STATUS_OK;
            before=state.cpu;
            state.cpu.data.eip = 0u;
            status=(core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
            after=state.cpu;
            failed|=status!=LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
                !(state.fault.exception_mask & VCPUINS_EXCEPT_DF) ||
                after.data.eip!=0u || after.data.esp!=before.data.esp ||
                after.data.eflags!=before.data.eflags || lib_memory_compare(&before.data.es,&after.data.es,sizeof(before.data.es))!=0 ||
                !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
                !legacy_sreg_stack_sregs_same(&before, &after) ||
                cpu_instruction_read(&state, candidate, &image, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE)!=LIB_STATUS_OK || image!=0xbe5au;
        }
        if(failed)return 0;
    }
    return 1;
}

static lib_i32 legacy_sreg_stack_test_fs_gs(void)
{
    static const lib_u8 opcodes[] = {0xa0u,0xa1u,0xa8u,0xa9u};
    lib_u8 form;
    for (form = 0u; form != sizeof(opcodes); ++form) {
        cpu_instruction_fixture state;   t_cpu before, after; lib_u16 image = (lib_u16)(0x5500u + form); lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        if (!failed) { state.cpu.data.esp=0x8000u; state.cpu.data.fs.selector=0x1111u; state.cpu.data.gs.selector=0x2222u; if (form & 1u) failed |= cpu_instruction_write(&state, 0x8000u, &image, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA)!=LIB_STATUS_OK; failed |= cpu_instruction_write(&state, 0u, (lib_u8[]){0x0fu,opcodes[form]}, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA)!=LIB_STATUS_OK; before=state.cpu; failed |= (core_machine_cpu_execution_refresh(&state.execution), state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK)!=LIB_STATUS_OK || state.execution.stop_requested; after=state.cpu; failed |= state.fault.valid || after.data.eip!=2u || after.data.eflags!=before.data.eflags || !legacy_sreg_stack_gprs_same_except_esp(&before,&after); if (!(form&1u)) { lib_u16 observed=0u; failed |= after.data.esp!=0x7ffeu || cpu_instruction_read(&state, 0x7ffeu, &observed, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE)!=LIB_STATUS_OK || observed!=(form==0u?0x1111u:0x2222u); } else failed |= after.data.esp!=0x8002u || (form==1u ? after.data.fs.selector : after.data.gs.selector)!=image; }  if (failed) return 0;
    } return 1;
}

typedef struct legacy_sreg_irq_fixture {
    cpu_instruction_fixture instruction;
    lib_bool pending;
    lib_u8 acknowledgements;
} legacy_sreg_irq_fixture;

static lib_status legacy_sreg_irq_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    legacy_sreg_irq_fixture *fixture = (legacy_sreg_irq_fixture *)opaque;
    return cpu_instruction_read(&fixture->instruction, address, destination, bytes,
        provenance, observe_only, reset_fetch);
}

static lib_status legacy_sreg_irq_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance)
{
    legacy_sreg_irq_fixture *fixture = (legacy_sreg_irq_fixture *)opaque;
    return cpu_instruction_write(&fixture->instruction, address, source, bytes, provenance);
}

static lib_bool legacy_sreg_irq_pending(void *opaque)
{
    return ((legacy_sreg_irq_fixture *)opaque)->pending;
}

static lib_status legacy_sreg_irq_acknowledge(void *opaque, lib_u8 *vector)
{
    legacy_sreg_irq_fixture *fixture = (legacy_sreg_irq_fixture *)opaque;
    if (!fixture->pending) return LIB_STATUS_INVALID_STATE;
    fixture->pending = LIB_FALSE;
    ++fixture->acknowledgements;
    *vector = 0x20u;
    return LIB_STATUS_OK;
}

/* The CPU receiver retains every original private-cache predicate. The board
 * receiver separately proves real PIC acknowledgement and the same IRQ frame. */
static lib_i32 legacy_sreg_stack_test_irq(void)
{
    static const lib_u8 codes[][3]={{0x17u,0x90u},{0x07u,0x90u},{0x1fu,0x90u},{0x06u,0x90u}};
    static const lib_u8 frame[] = {2u,1u,1u,1u};
    static const core_machine_cpu_bus_provider bus = {
        .read_memory = legacy_sreg_irq_read,
        .write_memory = legacy_sreg_irq_write,
        .interrupt_pending = legacy_sreg_irq_pending,
        .acknowledge_interrupt = legacy_sreg_irq_acknowledge
    };
    lib_u8 form;
    for (form = 0u; form != 4u; ++form) {
        legacy_sreg_irq_fixture fixture;
        cpu_instruction_fixture *state = &fixture.instruction;
        t_cpu before, after;
        lib_u16 ip = 0u, image = 0u;
        lib_i32 failed;
        cpu_instruction_prepare(state, CORE_MACHINE_CPU_PROFILE_80386);
        fixture.pending = LIB_TRUE;
        fixture.acknowledgements = 0u;
        state->execution.bus = &bus;
        state->execution.bus_context = &fixture;
        state->cpu.data.esp = 0x8000u;
        state->cpu.data.eflags |= VCPU_EFLAGS_IF;
        lib_memory_copy(state->memory, codes[form], 2u);
        state->memory[0x81u] = 1u;
        state->memory[0x100u] = 0xf4u;
        before = state->cpu;
        for (lib_u8 step = 0u; step != 3u && !state->cpu.data.flagHalt; ++step)
            core_machine_cpu_execution_refresh(&state->execution);
        after = state->cpu;
        failed = cpu_instruction_read(state, after.data.ss.base + (lib_u16)after.data.esp,
            &ip, 2u, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            cpu_instruction_read(state, 0x7ffeu, &image, 2u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK;
        failed |= state->fault.valid || state->execution.stop_requested ||
            !after.data.flagHalt || fixture.pending || fixture.acknowledgements != 1u ||
            after.data.eip != 0x101u || ip != frame[form] ||
            !legacy_sreg_stack_gprs_same_except_esp(&before, &after) ||
            (form == 3u && (after.data.esp != 0x7ff8u ||
            !legacy_sreg_stack_sregs_same(&before, &after) || image != before.data.es.selector)) ||
            (form != 3u && (after.data.esp != 0x7ffcu ||
            !legacy_sreg_stack_non_target_sregs_same(&before, &after, form == 0u ? 1u : form == 1u ? 0u : 2u) ||
            !legacy_sreg_stack_real_cache(legacy_sreg_stack_target(&after,
            form == 0u ? 1u : form == 1u ? 0u : 2u), 0u, form == 0u ? 1u : form == 1u ? 0u : 2u)));
        if (failed) return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!legacy_sreg_stack_test_defaults())
    {
        printf("LEGACY-SREG-STACK stage=defaults\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_attributes())
    {
        printf("LEGACY-SREG-STACK stage=attributes\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_protected_pop())
    {
        printf("LEGACY-SREG-STACK stage=protected-pop\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_protected_null())
    {
        printf("LEGACY-SREG-STACK stage=protected-null\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_protected_ss_null())
    {
        printf("LEGACY-SREG-STACK stage=protected-ss-null\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_protected_rejects())
    {
        printf("LEGACY-SREG-STACK stage=protected-rejects\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_protected_stack_limits())
    {
        printf("LEGACY-SREG-STACK stage=protected-stack-limits\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_irq())
    {
        printf("LEGACY-SREG-STACK stage=irq\n");
        return 1;
    }
    if (!legacy_sreg_stack_test_fs_gs())
        return 1;
    if (!legacy_sreg_stack_test_lock())
    {
        printf("LEGACY-SREG-STACK stage=lock\n");
        return 1;
    }
    printf("M5:T539:S26:CPU-LEGACY-SREG-STACK:OK\n");
    printf("M5:T401:S41:SREG-PUSH-POP-PROFILES:OK\n");
    return 0;
}

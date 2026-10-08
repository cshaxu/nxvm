#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* T337_REAL_UD_TERMINAL_CPU_OWNER: terminal-UD assertions stay CPU-owned. */
#define SEG_GDT_POINTER 0x0100u
#define SEG_GDT_ADDRESS 0x0300u
#define SEG_CODE_ADDRESS 0x2000u
#define SEG_DATA_ADDRESS 0x3000u

typedef struct segment_cpu {
    cpu_instruction_fixture chip;
    core_machine_cpu_fault_snapshot delivered;
} segment_cpu;

static void segment_fault(void *opaque, const core_machine_cpu_fault_snapshot *fault)
{
    ((segment_cpu *)opaque)->chip.fault = *fault;
}

static void segment_delivered(void *opaque, const core_machine_cpu_fault_snapshot *fault)
{
    ((segment_cpu *)opaque)->delivered = *fault;
}

static const core_machine_cpu_execution_diagnostic_provider segment_diagnostics = {
    .record_fault = segment_fault, .record_delivered_exception = segment_delivered
};

static lib_i32 segment_prepare(segment_cpu *state, core_machine_cpu_profile profile)
{
    lib_memory_set(state, 0, sizeof(*state));
    cpu_instruction_prepare(&state->chip, profile);
    core_machine_cpu_execution_context_bind_diagnostic_provider(
        &state->chip.execution, &segment_diagnostics, state);
    return 1;
}

/* Same finite instruction budget as the original board test, no board clock. */
static lib_status segment_execute(segment_cpu *state, lib_u32 instructions)
{
    for (lib_u32 step = 0u; step != instructions; ++step) {
        if (state->chip.cpu.data.flagHalt || state->chip.execution.stop_requested) break;
        core_machine_cpu_execution_refresh(&state->chip.execution);
    }
    return state->chip.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
}

static lib_i32 segment_write(segment_cpu *state, lib_u32 address,
    const void *bytes, lib_size byte_count)
{
    return state != LIB_NULL &&
        cpu_instruction_write(&state->chip, address, bytes, byte_count, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) ==
            LIB_STATUS_OK;
}

static lib_i32 segment_run_halt(segment_cpu *state, const lib_u8 *code,
    lib_size code_size, lib_u32 address, t_cpu *out_cpu)
{
    const lib_u32 budget = 96u;

    if (state == LIB_NULL || code == LIB_NULL ||
        out_cpu == LIB_NULL || !segment_write(state, address, code, code_size))
        return 0;
    state->chip.cpu.data.flagHalt = LIB_FALSE;
    state->chip.cpu.data.eip = address == 0u ? 0u : address - SEG_CODE_ADDRESS;
    if (segment_execute(state, budget) != LIB_STATUS_OK ||
        !state->chip.cpu.data.flagHalt) {
        lib_c_fprintf(lib_c_stderr,
            "M5:T301:SEGMENT-SELECTOR run address=%08x reason=%d detail=%08x\n",
            address, state->chip.execution.stop_requested, state->chip.fault.exception_mask);
        return 0;
    }
    *out_cpu = state->chip.cpu;
    return 1;
}

static lib_i32 segment_run_exception(segment_cpu *state, const lib_u8 *code,
    lib_size code_size, lib_u32 address, lib_u32 exception,
    t_cpu *out_cpu)
{
    const lib_u32 budget = 16u;

    if (state == LIB_NULL || code == LIB_NULL ||
        out_cpu == LIB_NULL || !segment_write(state, address, code, code_size))
        return 0;
    state->chip.cpu.data.flagHalt = LIB_FALSE;
    state->chip.cpu.data.eip = address == 0u ? 0u : address - SEG_CODE_ADDRESS;
    lib_status status = segment_execute(state, budget);
    if (X86_CPU_BIT_IS_SET(state->chip.cpu.data.cr0, VCPU_CR0_PE)) {
        if (status != LIB_STATUS_OK ||
            !core_machine_cpu_is_shutdown(&state->chip.execution) ||
            state->chip.execution.stop_requested || state->chip.fault.valid ||
            !state->delivered.valid ||
            state->delivered.exception_mask != VCPUINS_EXCEPT_SHUTDOWN)
            return 0;
    } else {
        if (status != LIB_STATUS_OK || state->chip.execution.stop_requested ||
            state->chip.fault.valid || !state->delivered.valid ||
            state->delivered.exception_mask != exception)
            return 0;
    }
    *out_cpu = state->chip.cpu;
    return 1;
}

static const t_cpu_data_sreg *segment_sreg(const t_cpu *cpu, lib_u8 target)
{
    if (cpu == LIB_NULL) return LIB_NULL;
    switch (target) {
    case 0u: return &cpu->data.es;
    case 1u: return &cpu->data.ds;
    case 2u: return &cpu->data.ss;
    case 3u: return &cpu->data.fs;
    case 4u: return &cpu->data.gs;
    default: return LIB_NULL;
    }
}

static lib_i32 segment_boot_protected(segment_cpu *state)
{
    static const lib_u8 gdt_pointer[] = {
        0x37u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0x40,0,
        0xff,0xff,0,0x30,0,0x92,0x40,0,
        0xff,0xff,0,0x30,0,0x12,0x40,0,
        0xff,0xff,0,0x30,0,0x98,0x40,0,
        0,0,0,0,0,0x80,0,0,
        0xff,0xff,0,0x00,0,0x89,0x40,0
    };
    static const lib_u8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const lib_u8 halt[] = { 0xf4u };
    const lib_u32 budget = 96u;
    lib_i32 installed;
    lib_status run_status;

    installed = segment_write(state, SEG_GDT_POINTER, gdt_pointer,
        sizeof(gdt_pointer));
    installed &= segment_write(state, SEG_GDT_ADDRESS, gdt, sizeof(gdt));
    installed &= segment_write(state, 0u, real_code, sizeof(real_code));
    installed &= segment_write(state, SEG_CODE_ADDRESS, halt, sizeof(halt));
    if (!installed) {
        lib_c_fprintf(lib_c_stderr,
            "M5:T301:SEGMENT-SELECTOR bootstrap-install-failed\n");
        return 0;
    }
    run_status = segment_execute(state, budget);
    if (run_status != LIB_STATUS_OK ||
        !state->chip.cpu.data.flagHalt) {
        lib_c_fprintf(lib_c_stderr,
            "M5:T301:SEGMENT-SELECTOR bootstrap status=%d reason=%d detail=%08x\n",
            run_status, state->chip.execution.stop_requested, state->chip.fault.exception_mask);
        return 0;
    }
    /* Deliberately leave later protected-mode rejection cases without a receiver. */
    state->chip.cpu.data.idtr.base = 0u;
    state->chip.cpu.data.idtr.limit = 0u;
    return 1;
}

static lib_i32 segment_boot_protected_286(segment_cpu *state)
{
    static const lib_u8 gdt_pointer[] = {
        0x37u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const lib_u8 idt_pointer[] = {
        0x6fu, 0x00u, 0x00u, 0x04u, 0x00u, 0x00u
    };
    static const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x30,0,0x12,0,0,
        0xff,0xff,0,0x30,0,0x98,0,0,
        0x0fu,0,0,0x50u,0,0x82u,0,0,
        0xff,0xff,0,0x00,0,0x89,0,0
    };
    static const lib_u8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const lib_u8 halt[] = { 0xf4u };
    const lib_u32 budget = 96u;
    lib_u8 idt[0x70u] = { 0u };

    idt[11u * 8u + 1u] = 0x01u;
    idt[11u * 8u + 2u] = 0x08u;
    idt[11u * 8u + 5u] = 0x86u;
    idt[12u * 8u + 1u] = 0x01u;
    idt[12u * 8u + 2u] = 0x08u;
    idt[12u * 8u + 5u] = 0x86u;
    idt[13u * 8u + 1u] = 0x01u;
    idt[13u * 8u + 2u] = 0x08u;
    idt[13u * 8u + 5u] = 0x86u;

    if (!segment_write(state, SEG_GDT_POINTER, gdt_pointer,
            sizeof(gdt_pointer)) || !segment_write(state, SEG_GDT_ADDRESS, gdt,
            sizeof(gdt)) || !segment_write(state, 0x0110u, idt_pointer,
            sizeof(idt_pointer)) || !segment_write(state, 0x0400u, idt,
            sizeof(idt)) || !segment_write(state, 0u, real_code,
            sizeof(real_code)) || !segment_write(state, SEG_CODE_ADDRESS, halt,
            sizeof(halt)) || !segment_write(state, SEG_CODE_ADDRESS + 0x100u,
            halt, sizeof(halt))) return 0;
    return segment_execute(state, budget) == LIB_STATUS_OK &&
        state->chip.cpu.data.flagHalt;
}

static lib_i32 segment_test_real_load_forms(void)
{
    static const lib_u8 mov_code[] = {
        0xb8u,0x34u,0x12u,0x8eu,0xe0u,0x8cu,0xe0u,0xf4u
    };
    static const lib_u8 les_code[] = {
        0xc4u,0x1eu,0x00u,0x04u,0xf4u
    };
    static const lib_u8 lfs_code[] = {
        0x66u,0x0fu,0xb4u,0x1eu,0x00u,0x04u,0xf4u
    };
    static const lib_u8 pop_code[] = { 0x66u,0x0fu,0xa9u,0xf4u };
    static const lib_u8 pointer16[] = { 0x78u,0x56u,0x56u,0x34u };
    static const lib_u8 pointer32_bytes[] = {
        0x78u,0x56u,0x34u,0x12u,0x56u,0x34u
    };
    static const lib_u8 pop_value[] = { 0x56u,0x34u,0xefu,0xbeu };
    segment_cpu state;
    t_cpu cpu;
    lib_i32 failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !segment_run_halt(&state, mov_code, sizeof(mov_code), 0u,
            &cpu) || cpu.data.fs.selector != 0x1234u ||
            cpu.data.fs.base != 0x12340u || (cpu.data.eax & 0xffffu) != 0x1234u;
    }
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        failed |= !segment_write(&state, 0x0400u, pointer16, sizeof(pointer16)) ||
            !segment_run_halt(&state, les_code, sizeof(les_code), 0u, &cpu) ||
            cpu.data.es.selector != 0x3456u || cpu.data.es.base != 0x34560u ||
            (cpu.data.ebx & 0xffffu) != 0x5678u;
    }
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        failed |= !segment_write(&state, 0x0400u, pointer32_bytes,
            sizeof(pointer32_bytes)) ||
            !segment_run_halt(&state, lfs_code, sizeof(lfs_code), 0u, &cpu) ||
            cpu.data.fs.selector != 0x3456u || cpu.data.fs.base != 0x34560u ||
            cpu.data.ebx != 0x12345678u;
    }
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        state.chip.cpu.data.esp = 0x0100u;
        failed |= !segment_write(&state, 0x0100u, pop_value, sizeof(pop_value)) ||
            !segment_run_halt(&state, pop_code, sizeof(pop_code), 0u, &cpu) ||
            cpu.data.gs.selector != 0x3456u || cpu.data.gs.base != 0x34560u ||
            cpu.data.esp != 0x0104u;
    }
    return failed;
}

static lib_i32 segment_test_80286_protected_legal_forms(void)
{
    static const lib_u8 les_code[] = { 0xc4u,0x1eu,0x00u,0x04u,0xf4u };
    static const lib_u8 lds_code[] = { 0xc5u,0x1eu,0x00u,0x04u,0xf4u };
    static const lib_u8 lar_code[] = {
        0xb8u,0x10u,0x00u,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const lib_u8 lsl_code[] = {
        0xb8u,0x10u,0x00u,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const lib_u8 verr_code[] = {
        0xb8u,0x10u,0x00u,0x0fu,0x00u,0xe0u,0xf4u
    };
    static const lib_u8 verw_code[] = {
        0xb8u,0x10u,0x00u,0x0fu,0x00u,0xe8u,0xf4u
    };
    static const lib_u8 mov_load_code[] = {
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0xf4u
    };
    static const lib_u8 mov_store_code[] = { 0x8cu,0xd8u,0xf4u };
    static const lib_u8 pop_load_code[] = {
        0xb8u,0x10u,0x00u,0x50u,0x1fu,0xf4u
    };
    static const lib_u8 push_store_code[] = { 0x06u,0xf4u };
    static const lib_u8 ldt_lar_code[] = {
        0xb8u,0x28u,0x00u,0x0fu,0x00u,0xd0u,
        0xb8u,0x0cu,0x00u,0x0fu,0x02u,0xc8u,0xf4u
    };
    static const lib_u8 ldt_lsl_code[] = {
        0xb8u,0x28u,0x00u,0x0fu,0x00u,0xd0u,
        0xb8u,0x0cu,0x00u,0x0fu,0x03u,0xc8u,0xf4u
    };
    static const lib_u8 ldt_verr_code[] = {
        0xb8u,0x28u,0x00u,0x0fu,0x00u,0xd0u,
        0xb8u,0x0cu,0x00u,0x0fu,0x00u,0xe0u,0xf4u
    };
    static const lib_u8 ldt_verw_code[] = {
        0xb8u,0x28u,0x00u,0x0fu,0x00u,0xd0u,
        0xb8u,0x0cu,0x00u,0x0fu,0x00u,0xe8u,0xf4u
    };
    static const lib_u8 lldt_memory_code[] = {
        0x0fu,0x00u,0x16u,0x00u,0x04u,0xf4u
    };
    static const lib_u8 sldt_memory_code[] = {
        0xb8u,0x28u,0x00u,0x0fu,0x00u,0xd0u,
        0x0fu,0x00u,0x06u,0x00u,0x04u,0xf4u
    };
    static const lib_u8 ltr_memory_code[] = {
        0x0fu,0x00u,0x1eu,0x00u,0x04u,0xf4u
    };
    static const lib_u8 str_memory_code[] = {
        0xb8u,0x30u,0x00u,0x0fu,0x00u,0xd8u,
        0x0fu,0x00u,0x0eu,0x00u,0x04u,0xf4u
    };
    static const lib_u8 ldt_descriptor[] = {
        0xffu,0xffu,0,0x70u,0,0x92u,0,0
    };
    static const lib_u8 pointer[] = { 0x78u,0x56u,0x10u,0x00u };
    const lib_u8 *codes[] = { les_code, lds_code, lar_code, lsl_code,
        verr_code, verw_code, mov_load_code, mov_store_code, pop_load_code,
        push_store_code, ldt_lar_code, ldt_lsl_code, ldt_verr_code,
        ldt_verw_code, lldt_memory_code, sldt_memory_code, ltr_memory_code,
        str_memory_code };
    const lib_size sizes[] = { sizeof(les_code), sizeof(lds_code),
        sizeof(lar_code), sizeof(lsl_code), sizeof(verr_code), sizeof(verw_code),
        sizeof(mov_load_code), sizeof(mov_store_code), sizeof(pop_load_code),
        sizeof(push_store_code), sizeof(ldt_lar_code), sizeof(ldt_lsl_code),
        sizeof(ldt_verr_code), sizeof(ldt_verw_code), sizeof(lldt_memory_code),
        sizeof(sldt_memory_code), sizeof(ltr_memory_code),
        sizeof(str_memory_code) };
    lib_size index;
    lib_i32 failed = 0;

    for (index = 0u; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        segment_cpu state;
        t_cpu cpu = {0};

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286) ||
            !segment_boot_protected_286(&state)) return 1;
        if (index < 2u) failed |= !segment_write(&state,
            SEG_DATA_ADDRESS + 0x0400u, pointer, sizeof(pointer));
        if (index >= 10u) failed |= !segment_write(&state, 0x5008u,
            ldt_descriptor, sizeof(ldt_descriptor));
        if (index == 14u || index == 16u) {
            lib_u16 selector = index == 14u ? 0x0028u : 0x0030u;

            failed |= !segment_write(&state, SEG_DATA_ADDRESS + 0x0400u,
                &selector, sizeof(selector));
        }
        failed |= !segment_run_halt(&state, codes[index], sizes[index],
            SEG_CODE_ADDRESS, &cpu);
        switch (index) {
        case 0u:
            failed |= cpu.data.es.selector != 0x0010u ||
                (cpu.data.ebx & 0xffffu) != 0x5678u;
            break;
        case 1u:
            failed |= cpu.data.ds.selector != 0x0010u ||
                (cpu.data.ebx & 0xffffu) != 0x5678u;
            break;
        case 2u:
            failed |= (cpu.data.eax & 0xffffu) != 0x9300u ||
                !X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
            break;
        case 3u:
            failed |= (cpu.data.eax & 0xffffu) != 0xffffu ||
                !X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
            break;
        case 6u:
            failed |= cpu.data.ds.selector != 0x0010u;
            break;
        case 7u:
            failed |= (cpu.data.eax & 0xffffu) != 0x0010u;
            break;
        case 8u:
            failed |= cpu.data.ds.selector != 0x0010u ||
                cpu.data.esp != 0x8000u;
            break;
        case 9u:
        {
            lib_u16 selector = 0u;

            failed |= cpu.data.esp != 0x7ffeu ||
                cpu_instruction_read(&state.chip, SEG_DATA_ADDRESS + 0x7ffeu, &selector, sizeof(selector), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                selector != 0x0010u;
            break;
        }
        case 10u:
            failed |= cpu.data.ldtr.selector != 0x0028u ||
                (cpu.data.ecx & 0xffffu) != 0x9200u ||
                !X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
            break;
        case 11u:
            failed |= cpu.data.ldtr.selector != 0x0028u ||
                (cpu.data.ecx & 0xffffu) != 0xffffu ||
                !X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
            break;
        case 14u:
            failed |= cpu.data.ldtr.selector != 0x0028u;
            break;
        case 15u:
        {
            lib_u16 selector = 0u;

            failed |= cpu.data.ldtr.selector != 0x0028u ||
                cpu_instruction_read(&state.chip, SEG_DATA_ADDRESS + 0x0400u, &selector, sizeof(selector), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || selector != 0x0028u;
            break;
        }
        case 16u:
            failed |= cpu.data.tr.selector != 0x0030u ||
                !cpu.data.tr.flagValid;
            break;
        case 17u:
        {
            lib_u16 selector = 0u;

            failed |= cpu.data.tr.selector != 0x0030u ||
                cpu_instruction_read(&state.chip, SEG_DATA_ADDRESS + 0x0400u, &selector, sizeof(selector), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                LIB_STATUS_OK || selector != 0x0030u;
            break;
        }
        default:
            failed |= !X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
            break;
        }
    }
    return failed;
}

static lib_i32 segment_test_80286_protected_cache_rejections(void)
{
    static const lib_u8 nonpresent_ds[] = {
        0xb8u,0x18u,0x00u,0x8eu,0xd8u
    };
    static const lib_u8 execute_only_ds[] = {
        0xb8u,0x20u,0x00u,0x8eu,0xd8u
    };
    static const lib_u8 null_ds[] = {
        0xb8u,0x00u,0x00u,0x8eu,0xd8u,0xf4u
    };
    static const lib_u8 null_ss[] = {
        0xb8u,0x00u,0x00u,0x8eu,0xd0u
    };
    const lib_u8 *codes[] = {
        nonpresent_ds, execute_only_ds, null_ds, null_ss
    };
    const lib_size sizes[] = {
        sizeof(nonpresent_ds), sizeof(execute_only_ds), sizeof(null_ds),
        sizeof(null_ss)
    };
    const lib_u32 exceptions[] = {
        VCPUINS_EXCEPT_NP, VCPUINS_EXCEPT_GP, 0u, VCPUINS_EXCEPT_GP
    };
    lib_u8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
        segment_cpu state;
        t_cpu before;
        t_cpu after;
        lib_i32 failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286) ||
            !segment_boot_protected_286(&state);

        if (!failed) {
            before = state.chip.cpu;
            failed |= !segment_write(&state, SEG_CODE_ADDRESS, codes[form],
                sizes[form]);
            state.chip.cpu.data.flagHalt = LIB_FALSE;
            state.chip.cpu.data.eip = 0u;
            failed |= segment_execute(&state, form == 2u ? 16u : 2u) != LIB_STATUS_OK;
            after = state.chip.cpu;
            if (form == 2u) {
                failed |= !state.chip.cpu.data.flagHalt ||
                    state.chip.fault.valid || after.data.ds.selector != 0u ||
                    after.data.ds.flagValid || after.data.eax != (lib_u32)(codes[form][1] | (codes[form][2] << 8u)) ||
                    lib_memory_compare(&after.data.es, &before.data.es,
                    sizeof(after.data.es)) != 0 ||
                    lib_memory_compare(&after.data.ss, &before.data.ss,
                    sizeof(after.data.ss)) != 0;
            } else {
                const t_cpu_data_sreg *target = form == 3u ? &after.data.ss :
                    &after.data.ds;
                const t_cpu_data_sreg *before_target = form == 3u ?
                    &before.data.ss : &before.data.ds;

                failed |= (state.chip.cpu.data.flagHalt || state.chip.execution.stop_requested) ||
                    state.chip.fault.valid ||
                    !state.delivered.valid ||
                    !X86_CPU_BIT_IS_SET(state.delivered.exception_mask,
                    exceptions[form]) || after.data.eip != 0x100u ||
                    lib_memory_compare(target, before_target, sizeof(*target)) != 0 ||
                    after.data.eax != (lib_u32)(codes[form][1] | (codes[form][2] << 8u)) ||
                    lib_memory_compare(&after.data.es, &before.data.es,
                    sizeof(after.data.es)) != 0;
            }
        }
        if (failed) return 1;
    }
    return 0;
}

typedef struct segment_lxs_form {
    lib_u8 first;
    lib_u8 second;
    lib_u8 bytes;
    lib_u8 target;
} segment_lxs_form;

static lib_i32 segment_test_lxs_success(const segment_lxs_form *form,
    lib_i32 protected_mode, lib_i32 pointer32)
{
    static const lib_u8 pointer16[] = { 0x78u,0x56u,0x10u,0x00u };
    static const lib_u8 pointer32_bytes[] = {
        0x78u,0x56u,0x34u,0x12u,0x10u,0x00u
    };
    lib_u8 code[8u] = {0};
    lib_u8 code_size = 0u;
    lib_u8 prefix = protected_mode ? !pointer32 : pointer32;
    lib_u32 address = protected_mode ? SEG_CODE_ADDRESS : 0u;
    lib_u32 pointer_address = protected_mode ? SEG_DATA_ADDRESS + 0x0400u :
        0x0400u;
    lib_u32 expected_offset = pointer32 ? 0x12345678u : 0x00005678u;
    segment_cpu state;
    t_cpu cpu;
    const t_cpu_data_sreg *sreg;
    lib_i32 failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed && protected_mode) failed = !segment_boot_protected(&state);
    if (!failed) {
        if (prefix) code[code_size++] = 0x66u;
        if (protected_mode) code[code_size++] = 0x67u;
        code[code_size++] = form->first;
        if (form->bytes == 2u) code[code_size++] = form->second;
        code[code_size++] = 0x1eu;
        code[code_size++] = 0x00u;
        code[code_size++] = 0x04u;
        code[code_size++] = 0xf4u;
        failed |= !segment_write(&state, pointer_address,
                pointer32 ? pointer32_bytes : pointer16,
                pointer32 ? sizeof(pointer32_bytes) : sizeof(pointer16)) ||
            !segment_run_halt(&state, code, code_size, address, &cpu);
        sreg = segment_sreg(&cpu, form->target);
        failed |= sreg == LIB_NULL || sreg->selector != 0x0010u ||
            (cpu.data.ebx & (pointer32 ? 0xffffffffu : 0xffffu)) !=
                expected_offset;
        if (!protected_mode && sreg != LIB_NULL)
            failed |= sreg->base != 0x0100u;
    }
    return failed;
}

static lib_i32 segment_test_lxs_memory_only(void)
{
    static const segment_lxs_form forms[] = {
        { 0xc4u,0u,1u,0u }, { 0xc5u,0u,1u,1u },
        { 0x0fu,0xb2u,2u,2u }, { 0x0fu,0xb4u,2u,3u },
        { 0x0fu,0xb5u,2u,4u }
    };
    lib_size index;
    lib_i32 protected_mode;
    lib_i32 pointer32;
    lib_i32 failed = 0;

    for (protected_mode = 0; protected_mode <= 1; ++protected_mode) {
        for (pointer32 = 0; pointer32 <= 1; ++pointer32) {
            for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index)
                failed |= segment_test_lxs_success(&forms[index], protected_mode,
                    pointer32);
        }
    }
    for (protected_mode = 0; protected_mode <= 1; ++protected_mode) {
        for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
            lib_u8 code[5u] = {0};
            lib_u8 code_size = 0u;
            lib_u32 address = protected_mode ? SEG_CODE_ADDRESS : 0u;
            segment_cpu state;
            t_cpu after;
            lib_i32 case_failed;

            if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 1;
            if (protected_mode && !segment_boot_protected(&state)) {
                return 1;
            }
            code[code_size++] = forms[index].first;
            if (forms[index].bytes == 2u) code[code_size++] = forms[index].second;
            code[code_size++] = 0xc0u;
            /* The fixture deliberately has no receiver.  Its terminal
             * exception frame is not the rejected instruction's state. */
            case_failed = !segment_run_exception(&state, code, code_size,
                address, VCPUINS_EXCEPT_UD, &after);
            failed |= case_failed;
        }
    }
    return failed;
}

static lib_i32 segment_test_lxs_fault_atomicity(void)
{
    static const segment_lxs_form forms[] = {
        { 0xc4u,0u,1u,0u }, { 0xc5u,0u,1u,1u },
        { 0x0fu,0xb2u,2u,2u }, { 0x0fu,0xb4u,2u,3u },
        { 0x0fu,0xb5u,2u,4u }
    };
    static const lib_u8 pointer[] = { 0x44u,0x33u,0x22u,0x11u,0x18u,0u };
    lib_size index;
    lib_i32 failed = 0;

    for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        lib_u8 code[8u] = {0};
        lib_u8 code_size = 0u;
        lib_u8 access = 0u;
        segment_cpu state;
        t_cpu before;
        t_cpu after;
        lib_u32 exception = forms[index].target == 2u ? VCPUINS_EXCEPT_SS :
            VCPUINS_EXCEPT_NP;

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        code[code_size++] = forms[index].first;
        if (forms[index].bytes == 2u) code[code_size++] = forms[index].second;
        code[code_size++] = 0x05u;
        code[code_size++] = 0x00u;
        code[code_size++] = 0x04u;
        code[code_size++] = 0x00u;
        code[code_size++] = 0x00u;
        before = state.chip.cpu;
        failed |= !segment_write(&state, SEG_DATA_ADDRESS + 0x0400u, pointer,
                sizeof(pointer)) || !segment_run_exception(&state, code,
                code_size, SEG_CODE_ADDRESS, exception, &after) ||
            before.data.eax != after.data.eax ||
            before.data.esp != after.data.esp ||
            before.data.eflags != after.data.eflags ||
            lib_memory_compare(&before.data.es, &after.data.es,
                sizeof(before.data.es)) != 0 ||
            lib_memory_compare(&before.data.ds, &after.data.ds,
                sizeof(before.data.ds)) != 0 ||
            lib_memory_compare(&before.data.ss, &after.data.ss,
                sizeof(before.data.ss)) != 0 ||
            lib_memory_compare(&before.data.fs, &after.data.fs,
                sizeof(before.data.fs)) != 0 ||
            lib_memory_compare(&before.data.gs, &after.data.gs,
                sizeof(before.data.gs)) != 0 ||
            !(core_machine_cpu_execution_read_linear(&state.chip.execution, SEG_GDT_ADDRESS + 29u, (lib_uptr)&access, sizeof(access)) == 0) || access != 0x12u;
    }
    return failed;
}

typedef struct segment_sreg_form {
    lib_u8 target;
    lib_u8 mov_modrm;
    lib_u8 pop_first;
    lib_u8 pop_second;
    lib_u8 pop_bytes;
} segment_sreg_form;

static lib_i32 segment_test_real_sreg_loads(void)
{
    static const segment_sreg_form forms[] = {
        { 0u,0xc0u,0x07u,0u,1u }, { 2u,0xd0u,0x17u,0u,1u },
        { 1u,0xd8u,0x1fu,0u,1u }, { 3u,0xe0u,0x0fu,0xa1u,2u },
        { 4u,0xe8u,0x0fu,0xa9u,2u }
    };
    static const lib_u8 stack_word[] = { 0x34u,0x12u,0,0 };
    static const lib_u8 stack_dword[] = { 0x34u,0x12u,0xefu,0xbeu };
    lib_size index;
    lib_i32 width32;
    lib_i32 failed = 0;

    for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        lib_u8 code[] = { 0xb8u,0x34u,0x12u,0x8eu,forms[index].mov_modrm,0xf4u };
        segment_cpu state;
        t_cpu cpu;
        const t_cpu_data_sreg *sreg;

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 1;
        failed |= !segment_run_halt(&state, code, sizeof(code), 0u, &cpu);
        sreg = segment_sreg(&cpu, forms[index].target);
        failed |= sreg == LIB_NULL || sreg->selector != 0x1234u ||
            sreg->base != 0x12340u;
    }
    for (width32 = 0; width32 <= 1; ++width32) {
        for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
            lib_u8 code[5u] = {0};
            lib_u8 code_size = 0u;
            segment_cpu state;
            t_cpu cpu;
            const t_cpu_data_sreg *sreg;

            if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 1;
            state.chip.cpu.data.esp = 0x0100u;
            if (width32) code[code_size++] = 0x66u;
            code[code_size++] = forms[index].pop_first;
            if (forms[index].pop_bytes == 2u)
                code[code_size++] = forms[index].pop_second;
            code[code_size++] = 0xf4u;
            failed |= !segment_write(&state, 0x0100u,
                    width32 ? stack_dword : stack_word,
                    width32 ? sizeof(stack_dword) : sizeof(stack_word)) ||
                !segment_run_halt(&state, code, code_size, 0u, &cpu);
            sreg = segment_sreg(&cpu, forms[index].target);
            failed |= sreg == LIB_NULL || sreg->selector != 0x1234u ||
                sreg->base != 0x12340u || cpu.data.esp !=
                    0x0100u + (width32 ? 4u : 2u);
        }
    }
    return failed;
}

static lib_i32 segment_test_protected_sreg_success(void)
{
    static const segment_sreg_form forms[] = {
        { 0u,0xc0u,0x07u,0u,1u }, { 2u,0xd0u,0x17u,0u,1u },
        { 1u,0xd8u,0x1fu,0u,1u }, { 3u,0xe0u,0x0fu,0xa1u,2u },
        { 4u,0xe8u,0x0fu,0xa9u,2u }
    };
    static const lib_u8 stack_word[] = { 0x10u,0,0,0 };
    static const lib_u8 stack_dword[] = { 0x10u,0,0xefu,0xbeu };
    lib_size index;
    lib_i32 width32;
    lib_i32 failed = 0;

    for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        lib_u8 code[] = { 0xb8u,0x10u,0,0,0,0x8eu,forms[index].mov_modrm,0xf4u };
        segment_cpu state;
        t_cpu cpu;
        const t_cpu_data_sreg *sreg;

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        failed |= !segment_run_halt(&state, code, sizeof(code), SEG_CODE_ADDRESS,
            &cpu);
        sreg = segment_sreg(&cpu, forms[index].target);
        failed |= sreg == LIB_NULL || sreg->selector != 0x0010u ||
            sreg->base != SEG_DATA_ADDRESS;
    }
    for (width32 = 0; width32 <= 1; ++width32) {
        for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
            lib_u8 code[5u] = {0};
            lib_u8 code_size = 0u;
            segment_cpu state;
            t_cpu cpu;
            const t_cpu_data_sreg *sreg;

            if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
                !segment_boot_protected(&state)) return 1;
            state.chip.cpu.data.esp = 0x8000u;
            if (!width32) code[code_size++] = 0x66u;
            code[code_size++] = forms[index].pop_first;
            if (forms[index].pop_bytes == 2u)
                code[code_size++] = forms[index].pop_second;
            code[code_size++] = 0xf4u;
            failed |= !segment_write(&state, SEG_DATA_ADDRESS + 0x8000u,
                    width32 ? stack_dword : stack_word,
                    width32 ? sizeof(stack_dword) : sizeof(stack_word)) ||
                !segment_run_halt(&state, code, code_size, SEG_CODE_ADDRESS,
                    &cpu);
            sreg = segment_sreg(&cpu, forms[index].target);
            failed |= sreg == LIB_NULL || sreg->selector != 0x0010u ||
                sreg->base != SEG_DATA_ADDRESS || cpu.data.esp !=
                    0x8000u + (width32 ? 4u : 2u);
        }
    }
    return failed;
}

typedef struct segment_sreg_failure {
    lib_u8 target;
    lib_u8 mov_modrm;
    lib_u16 selector;
    lib_u32 exception;
    lib_u32 access_address;
    lib_u8 access_value;
} segment_sreg_failure;

static lib_i32 segment_test_protected_sreg_failures(void)
{
    static const segment_sreg_failure failures[] = {
        { 1u,0xd8u,0x0018u,VCPUINS_EXCEPT_NP,SEG_GDT_ADDRESS + 29u,0x12u },
        { 2u,0xd0u,0x0018u,VCPUINS_EXCEPT_SS,SEG_GDT_ADDRESS + 29u,0x12u },
        { 3u,0xe0u,0x0020u,VCPUINS_EXCEPT_GP,SEG_GDT_ADDRESS + 37u,0x98u },
        { 4u,0xe8u,0x0013u,VCPUINS_EXCEPT_GP,SEG_GDT_ADDRESS + 21u,0x93u }
    };
    static const lib_u8 pop_fs[] = { 0x66u,0x0fu,0xa1u };
    static const lib_u8 pop_ss[] = { 0x66u,0x17u };
    static const lib_u8 selector_nonpresent[] = { 0x18u,0,0,0 };
    lib_size index;
    lib_i32 failed = 0;

    for (index = 0u; index < sizeof(failures) / sizeof(failures[0]); ++index) {
        lib_u8 code[] = { 0xb8u,0,0,0,0,0x8eu,failures[index].mov_modrm };
        segment_cpu state;
        t_cpu before;
        t_cpu after;
        const t_cpu_data_sreg *before_sreg;
        const t_cpu_data_sreg *after_sreg;
        lib_u8 access = 0u;
        lib_i32 case_failed;

        code[1u] = (lib_u8)failures[index].selector;
        code[2u] = (lib_u8)(failures[index].selector >> 8u);
        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        before = state.chip.cpu;
        case_failed = !segment_run_exception(&state, code, sizeof(code),
            SEG_CODE_ADDRESS, failures[index].exception, &after);
        before_sreg = segment_sreg(&before, failures[index].target);
        after_sreg = segment_sreg(&after, failures[index].target);
        case_failed |= before_sreg == LIB_NULL || after_sreg == LIB_NULL ||
            lib_memory_compare(before_sreg, after_sreg, sizeof(*before_sreg)) != 0 ||
            before.data.esp != after.data.esp ||
            before.data.eflags != after.data.eflags ||
            !(core_machine_cpu_execution_read_linear(&state.chip.execution, failures[index].access_address, (lib_uptr)&access, sizeof(access)) == 0) ||
            access != failures[index].access_value;
        if (case_failed) lib_c_fprintf(lib_c_stderr,
            "M5:T301:SEGMENT-SELECTOR mov-fail index=%u selector=%04x access=%02x esp=%08x/%08x flags=%08x/%08x\n",
            (unsigned)index, failures[index].selector, access, before.data.esp,
            after.data.esp, before.data.eflags, after.data.eflags);
        failed |= case_failed;
    }
    for (index = 0u; index < 2u; ++index) {
        const lib_u8 *code = index == 0u ? pop_fs : pop_ss;
        lib_size code_size = index == 0u ? sizeof(pop_fs) : sizeof(pop_ss);
        lib_u8 target = index == 0u ? 3u : 2u;
        lib_u32 exception = index == 0u ? VCPUINS_EXCEPT_NP : VCPUINS_EXCEPT_SS;
        segment_cpu state;
        t_cpu before;
        t_cpu after;
        const t_cpu_data_sreg *before_sreg;
        const t_cpu_data_sreg *after_sreg;
        lib_u8 access = 0u;
        lib_i32 case_failed;

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        state.chip.cpu.data.esp = 0x8000u;
        before = state.chip.cpu;
        case_failed = !segment_write(&state, SEG_DATA_ADDRESS + 0x8000u,
                selector_nonpresent, sizeof(selector_nonpresent)) ||
            !segment_run_exception(&state, code, code_size, SEG_CODE_ADDRESS,
                exception, &after);
        before_sreg = segment_sreg(&before, target);
        after_sreg = segment_sreg(&after, target);
        case_failed |= before_sreg == LIB_NULL || after_sreg == LIB_NULL ||
            lib_memory_compare(before_sreg, after_sreg, sizeof(*before_sreg)) != 0 ||
            before.data.esp != after.data.esp ||
            before.data.eflags != after.data.eflags ||
            !(core_machine_cpu_execution_read_linear(&state.chip.execution, SEG_GDT_ADDRESS + 29u, (lib_uptr)&access, sizeof(access)) == 0) || access != 0x12u;
        if (case_failed) lib_c_fprintf(lib_c_stderr,
            "M5:T301:SEGMENT-SELECTOR pop-fail index=%u access=%02x esp=%08x/%08x flags=%08x/%08x\n",
            (unsigned)index, access, before.data.esp, after.data.esp,
            before.data.eflags, after.data.eflags);
        failed |= case_failed;
    }
    return failed;
}

static lib_i32 segment_test_protected_selector_forms(void)
{
    static const lib_u8 lar_code[] = {
        0xb8u,0x10u,0x00u,0x00u,0x00u,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const lib_u8 lsl_code[] = {
        0xb8u,0x10u,0x00u,0x00u,0x00u,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const lib_u8 lar16_code[] = {
        0xb8u,0x10u,0x00u,0xcdu,0xabu,0x66u,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const lib_u8 arpl_code[] = {
        0x66u,0xb8u,0x01u,0x00u,0x66u,0xb9u,0x03u,0x00u,
        0x66u,0x63u,0xc8u,0xf4u
    };
    static const lib_u8 verr_code[] = {
        0xb8u,0x10u,0x00u,0x00u,0x00u,0x0fu,0x00u,0xe0u,0xf4u
    };
    static const lib_u8 verw_code[] = {
        0xb8u,0x10u,0x00u,0x00u,0x00u,0x0fu,0x00u,0xe8u,0xf4u
    };
    segment_cpu state;
    t_cpu cpu;
    lib_i32 failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, lar_code, sizeof(lar_code), SEG_CODE_ADDRESS,
            &cpu) || cpu.data.eax != 0x00409300u ||
        !X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, lsl_code, sizeof(lsl_code), SEG_CODE_ADDRESS,
            &cpu) || cpu.data.eax != 0x0000ffffu ||
        !X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, lar16_code, sizeof(lar16_code), SEG_CODE_ADDRESS,
            &cpu) || cpu.data.eax != 0xabcd9300u ||
        !X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, arpl_code, sizeof(arpl_code), SEG_CODE_ADDRESS,
            &cpu) || (cpu.data.eax & 0xffffu) != 0x0003u ||
        !X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, verr_code, sizeof(verr_code), SEG_CODE_ADDRESS,
            &cpu) || !X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, verw_code, sizeof(verw_code), SEG_CODE_ADDRESS,
            &cpu) || !X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF);
    return failed;
}

static lib_i32 segment_test_selector_query_edges(void)
{
    static const lib_u8 lsl16_code[] = {
        0xb8u,0x10u,0x00u,0xcdu,0xabu,0x66u,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const lib_u8 lar_invalid[] = {
        0xb8u,0x28u,0x00u,0xcdu,0xabu,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const lib_u8 lsl_invalid[] = {
        0xb8u,0x28u,0x00u,0xcdu,0xabu,0x66u,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const lib_u8 lar_nonpresent[] = {
        0xb8u,0x18u,0x00u,0xcdu,0xabu,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const lib_u8 lsl_nonpresent[] = {
        0xb8u,0x18u,0x00u,0xcdu,0xabu,0x66u,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const lib_u8 lar_tss[] = {
        0xb8u,0x30u,0x00u,0,0,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const lib_u8 lsl_tss[] = {
        0xb8u,0x30u,0x00u,0,0,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const lib_u8 lar_tss_rpl[] = {
        0xb8u,0x33u,0x00u,0xcdu,0xabu,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const lib_u8 lsl_tss_rpl[] = {
        0xb8u,0x33u,0x00u,0xcdu,0xabu,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const lib_u8 verr_type[] = {
        0xb8u,0x20u,0x00u,0,0,0x0fu,0x00u,0xe0u,0xf4u
    };
    static const lib_u8 verw_type[] = {
        0xb8u,0x20u,0x00u,0,0,0x0fu,0x00u,0xe8u,0xf4u
    };
    static const lib_u8 verr_nonpresent[] = {
        0xb8u,0x18u,0x00u,0,0,0x0fu,0x00u,0xe0u,0xf4u
    };
    static const lib_u8 verw_nonpresent[] = {
        0xb8u,0x18u,0x00u,0,0,0x0fu,0x00u,0xe8u,0xf4u
    };
    static const lib_u8 verr_memory[] = {
        0x67u,0x0fu,0x00u,0x26u,0x00u,0x04u,0xf4u
    };
    static const lib_u8 verw_memory[] = {
        0x67u,0x0fu,0x00u,0x2eu,0x00u,0x04u,0xf4u
    };
    static const lib_u8 data_selector[] = { 0x10u,0u };
    const lib_u8 *codes[] = { lsl16_code, lar_invalid, lsl_invalid,
        lar_nonpresent, lsl_nonpresent, lar_tss, lsl_tss, lar_tss_rpl,
        lsl_tss_rpl, verr_type, verw_type, verr_nonpresent, verw_nonpresent,
        verr_memory, verw_memory };
    const lib_size sizes[] = { sizeof(lsl16_code), sizeof(lar_invalid),
        sizeof(lsl_invalid), sizeof(lar_nonpresent), sizeof(lsl_nonpresent),
        sizeof(lar_tss), sizeof(lsl_tss), sizeof(lar_tss_rpl),
        sizeof(lsl_tss_rpl), sizeof(verr_type), sizeof(verw_type),
        sizeof(verr_nonpresent), sizeof(verw_nonpresent), sizeof(verr_memory),
        sizeof(verw_memory) };
    const lib_u32 expected_eax[] = { 0xabcdffffu,0xabcd0028u,0xabcd0028u,
        0xabcd0018u,0xabcd0018u,0x00408900u,0x0000ffffu,0xabcd0033u,
        0xabcd0033u,0x00000020u,0x00000020u,0x00000018u,0x00000018u,
        0x00000010u,0x00000010u };
    const lib_i32 expected_zf[] = { 1,0,0,0,0,1,1,0,0,0,0,0,0,1,1 };
    lib_size index;
    lib_i32 failed = 0;

    for (index = 0u; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        segment_cpu state;
        t_cpu before;
        t_cpu cpu;
        lib_u8 system_access = 0u;
        lib_u8 nonpresent_access = 0u;
        lib_i32 case_failed;

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        before = state.chip.cpu;
        if (index >= 13u)
            failed |= !segment_write(&state, SEG_DATA_ADDRESS + 0x0400u,
                data_selector, sizeof(data_selector));
        case_failed = !segment_run_halt(&state, codes[index], sizes[index],
            SEG_CODE_ADDRESS, &cpu) || cpu.data.eax != expected_eax[index] ||
            !!X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF) != expected_zf[index];
        case_failed |= cpu.data.esp != before.data.esp ||
            (cpu.data.eflags & ~VCPU_EFLAGS_ZF) !=
                (before.data.eflags & ~VCPU_EFLAGS_ZF) ||
            lib_memory_compare(&cpu.data.es, &before.data.es, sizeof(cpu.data.es)) != 0 ||
            lib_memory_compare(&cpu.data.ds, &before.data.ds, sizeof(cpu.data.ds)) != 0 ||
            lib_memory_compare(&cpu.data.ss, &before.data.ss, sizeof(cpu.data.ss)) != 0 ||
            lib_memory_compare(&cpu.data.fs, &before.data.fs, sizeof(cpu.data.fs)) != 0 ||
            lib_memory_compare(&cpu.data.gs, &before.data.gs, sizeof(cpu.data.gs)) != 0;
        case_failed |= !(core_machine_cpu_execution_read_linear(&state.chip.execution, SEG_GDT_ADDRESS + 45u, (lib_uptr)&system_access, sizeof(system_access)) == 0) || system_access != 0x80u ||
            !(core_machine_cpu_execution_read_linear(&state.chip.execution, SEG_GDT_ADDRESS + 29u, (lib_uptr)&nonpresent_access, sizeof(nonpresent_access)) == 0) || nonpresent_access != 0x12u;
        if (case_failed) lib_c_fprintf(lib_c_stderr,
            "M5:T301:SEGMENT-SELECTOR query-edge index=%u eax=%08x zf=%d/%d sys=%02x np=%02x\n",
            (unsigned)index, cpu.data.eax,
            !!X86_CPU_BIT_IS_SET(cpu.data.eflags, VCPU_EFLAGS_ZF), expected_zf[index],
            system_access, nonpresent_access);
        failed |= case_failed;
    }
    return failed;
}

static lib_i32 segment_test_rejected_forms(void)
{
    static const lib_u8 mov_cs[] = { 0x8eu,0xc8u };
    static const lib_u8 mov_from_fs[] = { 0x8cu,0xe0u };
    static const lib_u8 mov_to_fs[] = { 0x8eu,0xe0u };
    static const lib_u8 pop_fs[] = { 0x0fu,0xa1u };
    static const lib_u8 pop_gs[] = { 0x0fu,0xa9u };
    static const lib_u8 lss[] = { 0x0fu,0xb2u,0xc0u };
    static const lib_u8 lfs[] = { 0x0fu,0xb4u,0xc0u };
    static const lib_u8 lgs[] = { 0x0fu,0xb5u,0xc0u };
    static const lib_u8 lar_real[] = { 0x0fu,0x02u,0xc0u };
    static const lib_u8 lsl_real[] = { 0x0fu,0x03u,0xc0u };
    static const lib_u8 verr_real[] = { 0x0fu,0x00u,0xe0u };
    static const lib_u8 verw_real[] = { 0x0fu,0x00u,0xe8u };
    static const lib_u8 lar_66[] = { 0x66u,0x0fu,0x02u,0xc0u };
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    const lib_u8 *programs[] = { mov_cs, mov_from_fs, mov_to_fs, pop_fs,
        pop_gs, lss, lfs, lgs, lar_real, lsl_real, verr_real, verw_real,
        lar_66 };
    const lib_size sizes[] = { sizeof(mov_cs), sizeof(mov_from_fs),
        sizeof(mov_to_fs), sizeof(pop_fs), sizeof(pop_gs), sizeof(lss),
        sizeof(lfs), sizeof(lgs), sizeof(lar_real), sizeof(lsl_real),
        sizeof(verr_real), sizeof(verw_real), sizeof(lar_66) };
    const core_machine_cpu_profile maximum_profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_CPU_PROFILE_80386, CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_size profile_index;
    lib_size program_index;
    lib_i32 failed = 0;

    for (profile_index = 0u; profile_index < sizeof(profiles) / sizeof(profiles[0]);
         ++profile_index) {
        for (program_index = 0u; program_index < sizeof(programs) / sizeof(programs[0]);
             ++program_index) {
            segment_cpu state;

            if (profiles[profile_index] > maximum_profiles[program_index]) continue;
            if (!segment_prepare(&state, profiles[profile_index])) return 1;
            state.chip.cpu.data.idtr.limit = 0x17u;
            failed |= !segment_write(&state, 0u, programs[program_index],
                sizes[program_index]);
            failed |= segment_execute(&state, 8u) != LIB_STATUS_OK ||
                !core_machine_cpu_is_shutdown(&state.chip.execution) ||
                state.chip.execution.stop_requested || state.chip.fault.valid ||
                !state.delivered.valid ||
                state.delivered.exception_mask != VCPUINS_EXCEPT_SHUTDOWN;
        }
    }
    return failed;
}

static lib_i32 segment_test_pop_fault_atomicity(void)
{
    static const lib_u8 pop_fs[] = { 0x66u,0x0fu,0xa1u };
    static const lib_u8 selector[] = { 0x18u,0x00u,0,0 };
    const lib_u32 budget = 8u;
    segment_cpu state;
    t_cpu before;
    t_cpu after;
    lib_i32 failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed |= !segment_boot_protected(&state);
    if (!failed) {
        state.chip.cpu.data.esp = 0x8000u;
        before = state.chip.cpu;
        failed |= !segment_write(&state, SEG_DATA_ADDRESS + 0x8000u, selector,
            sizeof(selector)) || !segment_write(&state, SEG_CODE_ADDRESS, pop_fs,
            sizeof(pop_fs));
        state.chip.cpu.data.flagHalt = LIB_FALSE;
        state.chip.cpu.data.eip = 0u;
        failed |= segment_execute(&state, budget) != LIB_STATUS_OK ||
            !core_machine_cpu_is_shutdown(&state.chip.execution) ||
            state.chip.execution.stop_requested || state.chip.fault.valid ||
            !state.delivered.valid ||
            state.delivered.exception_mask != VCPUINS_EXCEPT_SHUTDOWN;
        after = state.chip.cpu;
        failed |= after.data.esp != before.data.esp ||
            lib_memory_compare(&after.data.fs, &before.data.fs, sizeof(after.data.fs)) != 0;
    }
    return failed;
}

static lib_i32 segment_test_vm86_pop_fs_gs_timing(void)
{
    static const lib_u8 opcodes[] = { 0xa1u, 0xa9u };
    static const lib_u16 selectors[] = { 0x1234u, 0x5678u };

    for (lib_size index = 0u; index < sizeof(opcodes) / sizeof(opcodes[0]);
        ++index) {
        const lib_u8 code[] = { 0x0fu, opcodes[index] };
        core_machine_cpu_timing_result timing;
        cpu_instruction_fixture state;
        t_cpu after;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        state.cpu.data.cr0 = VCPU_CR0_PE;
        state.cpu.data.eflags |= VCPU_EFLAGS_VM;
        state.cpu.data.esp = 0x0200u;
        if (cpu_instruction_write(&state, 0x0200u, &selectors[index],
                sizeof(selectors[index]), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
                LIB_STATUS_OK || cpu_instruction_run(&state, code, sizeof(code),
                &after) != LIB_STATUS_OK ||
            !core_machine_cpu_timing_select(&state.execution, &timing) ||
            timing.source_timing_unallocated || timing.ticks != 7u ||
            (opcodes[index] == 0xa1u ? after.data.fs.selector :
                after.data.gs.selector) != selectors[index]) return 1;
    }
    return 0;
}

static lib_i32 segment_test_metadata(void)
{
    core_machine_cpu_instruction_metadata verr =
        core_machine_cpu_instruction_metadata_get(CORE_MACHINE_CPU_INSTRUCTION_0F,
            0x00u, 0xe0u);
    core_machine_cpu_instruction_metadata verw =
        core_machine_cpu_instruction_metadata_get(CORE_MACHINE_CPU_INSTRUCTION_0F,
            0x00u, 0xe8u);
    core_machine_cpu_instruction_metadata reserved =
        core_machine_cpu_instruction_metadata_get(CORE_MACHINE_CPU_INSTRUCTION_0F,
            0x00u, 0xf0u);

    return !verr.valid || !verw.valid || reserved.valid ||
        verr.minimum_cpu != CORE_MACHINE_CPU_PROFILE_80286 ||
        verw.minimum_cpu != CORE_MACHINE_CPU_PROFILE_80286;
}

lib_i32 main(void)
{
    lib_i32 real_loads = segment_test_real_load_forms();
    lib_i32 protected_286 = segment_test_80286_protected_legal_forms();
    lib_i32 protected_286_rejections =
        segment_test_80286_protected_cache_rejections();
    lib_i32 lxs_memory_only = segment_test_lxs_memory_only();
    lib_i32 lxs_atomicity = segment_test_lxs_fault_atomicity();
    lib_i32 real_sregs = segment_test_real_sreg_loads();
    lib_i32 protected_sregs = segment_test_protected_sreg_success();
    lib_i32 protected_sreg_failures = segment_test_protected_sreg_failures();
    lib_i32 protected_forms = segment_test_protected_selector_forms();
    lib_i32 query_edges = segment_test_selector_query_edges();
    lib_i32 rejected = segment_test_rejected_forms();
    lib_i32 atomicity = segment_test_pop_fault_atomicity();
    lib_i32 vm86_pop = segment_test_vm86_pop_fs_gs_timing();
    lib_i32 metadata = segment_test_metadata();

    if (real_loads || protected_286 || protected_286_rejections || lxs_memory_only || lxs_atomicity || real_sregs || protected_sregs ||
        protected_sreg_failures || protected_forms || query_edges || rejected ||
        atomicity || vm86_pop || metadata) {
        lib_c_fprintf(lib_c_stderr,
            "M5:T301:SEGMENT-SELECTOR:FAIL real=%d protected-286=%d protected-286-reject=%d lxs=%d lxs-atomic=%d sreg-real=%d sreg-protected=%d sreg-fault=%d protected=%d query=%d rejected=%d atomic=%d vm86-pop=%d metadata=%d\n",
            real_loads, protected_286, protected_286_rejections, lxs_memory_only, lxs_atomicity, real_sregs, protected_sregs,
            protected_sreg_failures, protected_forms, query_edges, rejected,
            atomicity, vm86_pop, metadata);
        return 1;
    }
    lib_c_printf("M5:T301:SEGMENT-SELECTOR:OK\n");
    return 0;
}

#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define SEG_GDT_POINTER 0x0100u
#define SEG_GDT_ADDRESS 0x0300u
#define SEG_CODE_ADDRESS 0x2000u
#define SEG_DATA_ADDRESS 0x3000u

typedef struct segment_machine {
    core_machine *machine;
} segment_machine;

static C_VOID segment_reset(C_VOID *opaque)
{
    segment_machine *state = (segment_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider segment_provider = {
    segment_reset, STD_NULL
};

static C_INT segment_prepare(segment_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK) return 0;
    if (!test_core_machine_fixture_bind_freeze_reset(state->machine,
            &segment_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT segment_write(segment_machine *state, type_unsigned_32 address,
    const C_VOID *bytes, STD_SIZE_T byte_count)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, bytes, byte_count) ==
            TYPE_STATUS_OK;
}

static C_INT segment_run_halt(segment_machine *state, const type_unsigned_8 *code,
    STD_SIZE_T code_size, type_unsigned_32 address, t_cpu *out_cpu)
{
    const core_machine_run_budget budget = { 96u, 0u };
    core_machine_run_result result;

    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        out_cpu == STD_NULL || !segment_write(state, address, code, code_size))
        return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine,
        address == 0u ? 0u : address - SEG_CODE_ADDRESS);
    if (core_machine_run(state->machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        STD_FPRINTF(STD_STDERR,
            "M5:T301:SEGMENT-SELECTOR run address=%08x reason=%d detail=%08x\n",
            address, result.reason, result.detail);
        return 0;
    }
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT segment_run_ud(segment_machine *state, const type_unsigned_8 *code,
    STD_SIZE_T code_size, type_unsigned_32 address, t_cpu *out_cpu)
{
    const core_machine_run_budget budget = { 16u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;

    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        out_cpu == STD_NULL || !segment_write(state, address, code, code_size))
        return 0;
    if (!TYPE_GET_BIT(state->machine->executor_cpu.data.cr0, VCPU_CR0_PE) &&
        !test_core_machine_fixture_preflight_real_ud_terminal(state->machine))
        return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine,
        address == 0u ? 0u : address - SEG_CODE_ADDRESS);
    if (core_machine_run(state->machine, budget, &result) != TYPE_STATUS_FAULT ||
        result.reason != CORE_MACHINE_STOP_FAULT ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) !=
            TYPE_STATUS_OK || !diagnostic.first_fault.valid ||
        !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD))
        return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT segment_run_exception(segment_machine *state, const type_unsigned_8 *code,
    STD_SIZE_T code_size, type_unsigned_32 address, type_unsigned_32 exception,
    t_cpu *out_cpu)
{
    const core_machine_run_budget budget = { 16u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;

    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        out_cpu == STD_NULL || !segment_write(state, address, code, code_size))
        return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine,
        address == 0u ? 0u : address - SEG_CODE_ADDRESS);
    if (state->machine->cpu_profile >= CORE_MACHINE_CPU_PROFILE_80386 &&
        TYPE_GET_BIT(state->machine->executor_cpu.data.cr0, VCPU_CR0_PE) &&
        (exception == VCPUINS_EXCEPT_TS || exception == VCPUINS_EXCEPT_NP ||
            exception == VCPUINS_EXCEPT_SS || exception == VCPUINS_EXCEPT_GP)) {
        exception = VCPUINS_EXCEPT_DF;
    }
    if (core_machine_run(state->machine, budget, &result) != TYPE_STATUS_FAULT ||
        result.reason != CORE_MACHINE_STOP_FAULT ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) !=
            TYPE_STATUS_OK || !diagnostic.first_fault.valid ||
        !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, exception)) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static const t_cpu_data_sreg *segment_sreg(const t_cpu *cpu, type_unsigned_8 target)
{
    if (cpu == STD_NULL) return STD_NULL;
    switch (target) {
    case 0u: return &cpu->data.es;
    case 1u: return &cpu->data.ds;
    case 2u: return &cpu->data.ss;
    case 3u: return &cpu->data.fs;
    case 4u: return &cpu->data.gs;
    default: return STD_NULL;
    }
}

static C_INT segment_boot_protected(segment_machine *state)
{
    static const type_unsigned_8 gdt_pointer[] = {
        0x37u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0x40,0,
        0xff,0xff,0,0x30,0,0x92,0x40,0,
        0xff,0xff,0,0x30,0,0x12,0x40,0,
        0xff,0xff,0,0x30,0,0x98,0x40,0,
        0,0,0,0,0,0x80,0,0,
        0xff,0xff,0,0x00,0,0x89,0x40,0
    };
    static const type_unsigned_8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    const core_machine_run_budget budget = { 96u, 0u };
    core_machine_run_result result;
    C_INT installed;
    type_status run_status;

    installed = segment_write(state, SEG_GDT_POINTER, gdt_pointer,
        sizeof(gdt_pointer));
    installed &= segment_write(state, SEG_GDT_ADDRESS, gdt, sizeof(gdt));
    installed &= segment_write(state, 0u, real_code, sizeof(real_code));
    installed &= segment_write(state, SEG_CODE_ADDRESS, halt, sizeof(halt));
    if (!installed) {
        STD_FPRINTF(STD_STDERR,
            "M5:T301:SEGMENT-SELECTOR bootstrap-install-failed\n");
        return 0;
    }
    run_status = core_machine_run(state->machine, budget, &result);
    if (run_status != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        STD_FPRINTF(STD_STDERR,
            "M5:T301:SEGMENT-SELECTOR bootstrap status=%d reason=%d detail=%08x\n",
            run_status, result.reason, result.detail);
        return 0;
    }
    return 1;
}

static C_INT segment_boot_protected_286(segment_machine *state)
{
    static const type_unsigned_8 gdt_pointer[] = {
        0x37u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const type_unsigned_8 idt_pointer[] = {
        0x6fu, 0x00u, 0x00u, 0x04u, 0x00u, 0x00u
    };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x30,0,0x12,0,0,
        0xff,0xff,0,0x30,0,0x98,0,0,
        0x0fu,0,0,0x50u,0,0x82u,0,0,
        0xff,0xff,0,0x00,0,0x89,0,0
    };
    static const type_unsigned_8 real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0x0fu,0x01u,0x1eu,0x10u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    const core_machine_run_budget budget = { 96u, 0u };
    core_machine_run_result result;
    type_unsigned_8 idt[0x70u] = { 0u };

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
    return core_machine_run(state->machine, budget, &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT segment_test_real_load_forms(C_VOID)
{
    static const type_unsigned_8 mov_code[] = {
        0xb8u,0x34u,0x12u,0x8eu,0xe0u,0x8cu,0xe0u,0xf4u
    };
    static const type_unsigned_8 les_code[] = {
        0xc4u,0x1eu,0x00u,0x04u,0xf4u
    };
    static const type_unsigned_8 lfs_code[] = {
        0x66u,0x0fu,0xb4u,0x1eu,0x00u,0x04u,0xf4u
    };
    static const type_unsigned_8 pop_code[] = { 0x66u,0x0fu,0xa9u,0xf4u };
    static const type_unsigned_8 pointer16[] = { 0x78u,0x56u,0x56u,0x34u };
    static const type_unsigned_8 pointer32_bytes[] = {
        0x78u,0x56u,0x34u,0x12u,0x56u,0x34u
    };
    static const type_unsigned_8 pop_value[] = { 0x56u,0x34u,0xefu,0xbeu };
    segment_machine state;
    t_cpu cpu;
    C_INT failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !segment_run_halt(&state, mov_code, sizeof(mov_code), 0u,
            &cpu) || cpu.data.fs.selector != 0x1234u ||
            cpu.data.fs.base != 0x12340u || (cpu.data.eax & 0xffffu) != 0x1234u;
    }
    core_machine_destroy(state.machine);
    state.machine = STD_NULL;
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        failed |= !segment_write(&state, 0x0400u, pointer16, sizeof(pointer16)) ||
            !segment_run_halt(&state, les_code, sizeof(les_code), 0u, &cpu) ||
            cpu.data.es.selector != 0x3456u || cpu.data.es.base != 0x34560u ||
            (cpu.data.ebx & 0xffffu) != 0x5678u;
    }
    core_machine_destroy(state.machine);
    state.machine = STD_NULL;
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        failed |= !segment_write(&state, 0x0400u, pointer32_bytes,
            sizeof(pointer32_bytes)) ||
            !segment_run_halt(&state, lfs_code, sizeof(lfs_code), 0u, &cpu) ||
            cpu.data.fs.selector != 0x3456u || cpu.data.fs.base != 0x34560u ||
            cpu.data.ebx != 0x12345678u;
    }
    core_machine_destroy(state.machine);
    state.machine = STD_NULL;
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x0100u;
        failed |= !segment_write(&state, 0x0100u, pop_value, sizeof(pop_value)) ||
            !segment_run_halt(&state, pop_code, sizeof(pop_code), 0u, &cpu) ||
            cpu.data.gs.selector != 0x3456u || cpu.data.gs.base != 0x34560u ||
            cpu.data.esp != 0x0104u;
    }
    core_machine_destroy(state.machine);
    state.machine = STD_NULL;
    return failed;
}

static C_INT segment_test_80286_protected_legal_forms(C_VOID)
{
    static const type_unsigned_8 les_code[] = { 0xc4u,0x1eu,0x00u,0x04u,0xf4u };
    static const type_unsigned_8 lds_code[] = { 0xc5u,0x1eu,0x00u,0x04u,0xf4u };
    static const type_unsigned_8 lar_code[] = {
        0xb8u,0x10u,0x00u,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const type_unsigned_8 lsl_code[] = {
        0xb8u,0x10u,0x00u,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const type_unsigned_8 verr_code[] = {
        0xb8u,0x10u,0x00u,0x0fu,0x00u,0xe0u,0xf4u
    };
    static const type_unsigned_8 verw_code[] = {
        0xb8u,0x10u,0x00u,0x0fu,0x00u,0xe8u,0xf4u
    };
    static const type_unsigned_8 mov_load_code[] = {
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0xf4u
    };
    static const type_unsigned_8 mov_store_code[] = { 0x8cu,0xd8u,0xf4u };
    static const type_unsigned_8 pop_load_code[] = {
        0xb8u,0x10u,0x00u,0x50u,0x1fu,0xf4u
    };
    static const type_unsigned_8 push_store_code[] = { 0x06u,0xf4u };
    static const type_unsigned_8 ldt_lar_code[] = {
        0xb8u,0x28u,0x00u,0x0fu,0x00u,0xd0u,
        0xb8u,0x0cu,0x00u,0x0fu,0x02u,0xc8u,0xf4u
    };
    static const type_unsigned_8 ldt_lsl_code[] = {
        0xb8u,0x28u,0x00u,0x0fu,0x00u,0xd0u,
        0xb8u,0x0cu,0x00u,0x0fu,0x03u,0xc8u,0xf4u
    };
    static const type_unsigned_8 ldt_verr_code[] = {
        0xb8u,0x28u,0x00u,0x0fu,0x00u,0xd0u,
        0xb8u,0x0cu,0x00u,0x0fu,0x00u,0xe0u,0xf4u
    };
    static const type_unsigned_8 ldt_verw_code[] = {
        0xb8u,0x28u,0x00u,0x0fu,0x00u,0xd0u,
        0xb8u,0x0cu,0x00u,0x0fu,0x00u,0xe8u,0xf4u
    };
    static const type_unsigned_8 lldt_memory_code[] = {
        0x0fu,0x00u,0x16u,0x00u,0x04u,0xf4u
    };
    static const type_unsigned_8 sldt_memory_code[] = {
        0xb8u,0x28u,0x00u,0x0fu,0x00u,0xd0u,
        0x0fu,0x00u,0x06u,0x00u,0x04u,0xf4u
    };
    static const type_unsigned_8 ltr_memory_code[] = {
        0x0fu,0x00u,0x1eu,0x00u,0x04u,0xf4u
    };
    static const type_unsigned_8 str_memory_code[] = {
        0xb8u,0x30u,0x00u,0x0fu,0x00u,0xd8u,
        0x0fu,0x00u,0x0eu,0x00u,0x04u,0xf4u
    };
    static const type_unsigned_8 ldt_descriptor[] = {
        0xffu,0xffu,0,0x70u,0,0x92u,0,0
    };
    static const type_unsigned_8 pointer[] = { 0x78u,0x56u,0x10u,0x00u };
    const type_unsigned_8 *codes[] = { les_code, lds_code, lar_code, lsl_code,
        verr_code, verw_code, mov_load_code, mov_store_code, pop_load_code,
        push_store_code, ldt_lar_code, ldt_lsl_code, ldt_verr_code,
        ldt_verw_code, lldt_memory_code, sldt_memory_code, ltr_memory_code,
        str_memory_code };
    const STD_SIZE_T sizes[] = { sizeof(les_code), sizeof(lds_code),
        sizeof(lar_code), sizeof(lsl_code), sizeof(verr_code), sizeof(verw_code),
        sizeof(mov_load_code), sizeof(mov_store_code), sizeof(pop_load_code),
        sizeof(push_store_code), sizeof(ldt_lar_code), sizeof(ldt_lsl_code),
        sizeof(ldt_verr_code), sizeof(ldt_verw_code), sizeof(lldt_memory_code),
        sizeof(sldt_memory_code), sizeof(ltr_memory_code),
        sizeof(str_memory_code) };
    STD_SIZE_T index;
    C_INT failed = 0;

    for (index = 0u; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        segment_machine state;
        t_cpu cpu = {0};

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286) ||
            !segment_boot_protected_286(&state)) return 1;
        if (index < 2u) failed |= !segment_write(&state,
            SEG_DATA_ADDRESS + 0x0400u, pointer, sizeof(pointer));
        if (index >= 10u) failed |= !segment_write(&state, 0x5008u,
            ldt_descriptor, sizeof(ldt_descriptor));
        if (index == 14u || index == 16u) {
            type_unsigned_16 selector = index == 14u ? 0x0028u : 0x0030u;

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
                !TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF);
            break;
        case 3u:
            failed |= (cpu.data.eax & 0xffffu) != 0xffffu ||
                !TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF);
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
            type_unsigned_16 selector = 0u;

            failed |= cpu.data.esp != 0x7ffeu ||
                core_machine_memory_read(state.machine, SEG_DATA_ADDRESS + 0x7ffeu,
                    &selector, sizeof(selector)) != TYPE_STATUS_OK ||
                selector != 0x0010u;
            break;
        }
        case 10u:
            failed |= cpu.data.ldtr.selector != 0x0028u ||
                (cpu.data.ecx & 0xffffu) != 0x9200u ||
                !TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF);
            break;
        case 11u:
            failed |= cpu.data.ldtr.selector != 0x0028u ||
                (cpu.data.ecx & 0xffffu) != 0xffffu ||
                !TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF);
            break;
        case 14u:
            failed |= cpu.data.ldtr.selector != 0x0028u;
            break;
        case 15u:
        {
            type_unsigned_16 selector = 0u;

            failed |= cpu.data.ldtr.selector != 0x0028u ||
                core_machine_memory_read(state.machine,
                SEG_DATA_ADDRESS + 0x0400u, &selector, sizeof(selector)) !=
                TYPE_STATUS_OK || selector != 0x0028u;
            break;
        }
        case 16u:
            failed |= cpu.data.tr.selector != 0x0030u ||
                !cpu.data.tr.flagValid;
            break;
        case 17u:
        {
            type_unsigned_16 selector = 0u;

            failed |= cpu.data.tr.selector != 0x0030u ||
                core_machine_memory_read(state.machine,
                SEG_DATA_ADDRESS + 0x0400u, &selector, sizeof(selector)) !=
                TYPE_STATUS_OK || selector != 0x0030u;
            break;
        }
        default:
            failed |= !TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF);
            break;
        }
        core_machine_destroy(state.machine);
    }
    return failed;
}

static C_INT segment_test_80286_protected_cache_rejections(C_VOID)
{
    static const type_unsigned_8 nonpresent_ds[] = {
        0xb8u,0x18u,0x00u,0x8eu,0xd8u
    };
    static const type_unsigned_8 execute_only_ds[] = {
        0xb8u,0x20u,0x00u,0x8eu,0xd8u
    };
    static const type_unsigned_8 null_ds[] = {
        0xb8u,0x00u,0x00u,0x8eu,0xd8u,0xf4u
    };
    static const type_unsigned_8 null_ss[] = {
        0xb8u,0x00u,0x00u,0x8eu,0xd0u
    };
    const type_unsigned_8 *codes[] = {
        nonpresent_ds, execute_only_ds, null_ds, null_ss
    };
    const STD_SIZE_T sizes[] = {
        sizeof(nonpresent_ds), sizeof(execute_only_ds), sizeof(null_ds),
        sizeof(null_ss)
    };
    const type_unsigned_32 exceptions[] = {
        VCPUINS_EXCEPT_NP, VCPUINS_EXCEPT_GP, 0u, VCPUINS_EXCEPT_GP
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
        segment_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        C_INT failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286) ||
            !segment_boot_protected_286(&state);

        if (!failed) {
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !segment_write(&state, SEG_CODE_ADDRESS, codes[form],
                sizes[form]);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ form == 2u ? 16u : 2u, 0u },
                &result) != TYPE_STATUS_OK ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (form == 2u) {
                failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                    diagnostic.first_fault.valid || after.data.ds.selector != 0u ||
                    after.data.ds.flagValid || after.data.eax != before.data.eax ||
                    STD_MEMCMP(&after.data.es, &before.data.es,
                    sizeof(after.data.es)) != 0 ||
                    STD_MEMCMP(&after.data.ss, &before.data.ss,
                    sizeof(after.data.ss)) != 0;
            } else {
                const t_cpu_data_sreg *target = form == 3u ? &after.data.ss :
                    &after.data.ds;
                const t_cpu_data_sreg *before_target = form == 3u ?
                    &before.data.ss : &before.data.ds;

                failed |= result.reason != CORE_MACHINE_STOP_BUDGET ||
                    diagnostic.first_fault.valid ||
                    !diagnostic.last_delivered_exception.valid ||
                    !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                    exceptions[form]) || after.data.eip != 0x100u ||
                    STD_MEMCMP(target, before_target, sizeof(*target)) != 0 ||
                    after.data.eax != before.data.eax ||
                    STD_MEMCMP(&after.data.es, &before.data.es,
                    sizeof(after.data.es)) != 0;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

typedef struct segment_lxs_form {
    type_unsigned_8 first;
    type_unsigned_8 second;
    type_unsigned_8 bytes;
    type_unsigned_8 target;
} segment_lxs_form;

static C_INT segment_test_lxs_success(const segment_lxs_form *form,
    C_INT protected_mode, C_INT pointer32)
{
    static const type_unsigned_8 pointer16[] = { 0x78u,0x56u,0x10u,0x00u };
    static const type_unsigned_8 pointer32_bytes[] = {
        0x78u,0x56u,0x34u,0x12u,0x10u,0x00u
    };
    type_unsigned_8 code[8u] = {0};
    type_unsigned_8 code_size = 0u;
    type_unsigned_8 prefix = protected_mode ? !pointer32 : pointer32;
    type_unsigned_32 address = protected_mode ? SEG_CODE_ADDRESS : 0u;
    type_unsigned_32 pointer_address = protected_mode ? SEG_DATA_ADDRESS + 0x0400u :
        0x0400u;
    type_unsigned_32 expected_offset = pointer32 ? 0x12345678u : 0x00005678u;
    segment_machine state;
    t_cpu cpu;
    const t_cpu_data_sreg *sreg;
    C_INT failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

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
        failed |= sreg == STD_NULL || sreg->selector != 0x0010u ||
            (cpu.data.ebx & (pointer32 ? 0xffffffffu : 0xffffu)) !=
                expected_offset;
        if (!protected_mode && sreg != STD_NULL)
            failed |= sreg->base != 0x0100u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT segment_test_lxs_memory_only(C_VOID)
{
    static const segment_lxs_form forms[] = {
        { 0xc4u,0u,1u,0u }, { 0xc5u,0u,1u,1u },
        { 0x0fu,0xb2u,2u,2u }, { 0x0fu,0xb4u,2u,3u },
        { 0x0fu,0xb5u,2u,4u }
    };
    STD_SIZE_T index;
    C_INT protected_mode;
    C_INT pointer32;
    C_INT failed = 0;

    for (protected_mode = 0; protected_mode <= 1; ++protected_mode) {
        for (pointer32 = 0; pointer32 <= 1; ++pointer32) {
            for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index)
                failed |= segment_test_lxs_success(&forms[index], protected_mode,
                    pointer32);
        }
    }
    for (protected_mode = 0; protected_mode <= 1; ++protected_mode) {
        for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
            type_unsigned_8 code[5u] = {0};
            type_unsigned_8 code_size = 0u;
            type_unsigned_32 address = protected_mode ? SEG_CODE_ADDRESS : 0u;
            segment_machine state;
            t_cpu before;
            t_cpu after;
            const t_cpu_data_sreg *before_sreg;
            const t_cpu_data_sreg *after_sreg;

            if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 1;
            if (protected_mode && !segment_boot_protected(&state)) {
                core_machine_destroy(state.machine);
                return 1;
            }
            code[code_size++] = forms[index].first;
            if (forms[index].bytes == 2u) code[code_size++] = forms[index].second;
            code[code_size++] = 0xc0u;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !segment_run_ud(&state, code, code_size, address, &after);
            before_sreg = segment_sreg(&before, forms[index].target);
            after_sreg = segment_sreg(&after, forms[index].target);
            failed |= before_sreg == STD_NULL || after_sreg == STD_NULL ||
                STD_MEMCMP(before_sreg, after_sreg, sizeof(*before_sreg)) != 0 ||
                before.data.eax != after.data.eax || before.data.ebx != after.data.ebx ||
                before.data.esp != after.data.esp ||
                before.data.eflags != after.data.eflags;
            core_machine_destroy(state.machine);
        }
    }
    return failed;
}

static C_INT segment_test_lxs_fault_atomicity(C_VOID)
{
    static const segment_lxs_form forms[] = {
        { 0xc4u,0u,1u,0u }, { 0xc5u,0u,1u,1u },
        { 0x0fu,0xb2u,2u,2u }, { 0x0fu,0xb4u,2u,3u },
        { 0x0fu,0xb5u,2u,4u }
    };
    static const type_unsigned_8 pointer[] = { 0x44u,0x33u,0x22u,0x11u,0x18u,0u };
    STD_SIZE_T index;
    C_INT failed = 0;

    for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        type_unsigned_8 code[8u] = {0};
        type_unsigned_8 code_size = 0u;
        type_unsigned_8 access = 0u;
        segment_machine state;
        t_cpu before;
        t_cpu after;
        type_unsigned_32 exception = forms[index].target == 2u ? VCPUINS_EXCEPT_SS :
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
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !segment_write(&state, SEG_DATA_ADDRESS + 0x0400u, pointer,
                sizeof(pointer)) || !segment_run_exception(&state, code,
                code_size, SEG_CODE_ADDRESS, exception, &after) ||
            before.data.eax != after.data.eax ||
            before.data.esp != after.data.esp ||
            before.data.eflags != after.data.eflags ||
            STD_MEMCMP(&before.data.es, &after.data.es,
                sizeof(before.data.es)) != 0 ||
            STD_MEMCMP(&before.data.ds, &after.data.ds,
                sizeof(before.data.ds)) != 0 ||
            STD_MEMCMP(&before.data.ss, &after.data.ss,
                sizeof(before.data.ss)) != 0 ||
            STD_MEMCMP(&before.data.fs, &after.data.fs,
                sizeof(before.data.fs)) != 0 ||
            STD_MEMCMP(&before.data.gs, &after.data.gs,
                sizeof(before.data.gs)) != 0 ||
            !test_core_machine_fixture_read_linear(state.machine,
                SEG_GDT_ADDRESS + 29u, TYPE_REFERENCE_OF(access),
                sizeof(access)) || access != 0x12u;
        core_machine_destroy(state.machine);
    }
    return failed;
}

typedef struct segment_sreg_form {
    type_unsigned_8 target;
    type_unsigned_8 mov_modrm;
    type_unsigned_8 pop_first;
    type_unsigned_8 pop_second;
    type_unsigned_8 pop_bytes;
} segment_sreg_form;

static C_INT segment_test_real_sreg_loads(C_VOID)
{
    static const segment_sreg_form forms[] = {
        { 0u,0xc0u,0x07u,0u,1u }, { 2u,0xd0u,0x17u,0u,1u },
        { 1u,0xd8u,0x1fu,0u,1u }, { 3u,0xe0u,0x0fu,0xa1u,2u },
        { 4u,0xe8u,0x0fu,0xa9u,2u }
    };
    static const type_unsigned_8 stack_word[] = { 0x34u,0x12u,0,0 };
    static const type_unsigned_8 stack_dword[] = { 0x34u,0x12u,0xefu,0xbeu };
    STD_SIZE_T index;
    C_INT width32;
    C_INT failed = 0;

    for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        type_unsigned_8 code[] = { 0xb8u,0x34u,0x12u,0x8eu,forms[index].mov_modrm,0xf4u };
        segment_machine state;
        t_cpu cpu;
        const t_cpu_data_sreg *sreg;

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 1;
        failed |= !segment_run_halt(&state, code, sizeof(code), 0u, &cpu);
        sreg = segment_sreg(&cpu, forms[index].target);
        failed |= sreg == STD_NULL || sreg->selector != 0x1234u ||
            sreg->base != 0x12340u;
        core_machine_destroy(state.machine);
    }
    for (width32 = 0; width32 <= 1; ++width32) {
        for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
            type_unsigned_8 code[5u] = {0};
            type_unsigned_8 code_size = 0u;
            segment_machine state;
            t_cpu cpu;
            const t_cpu_data_sreg *sreg;

            if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386)) return 1;
            state.machine->executor_cpu.data.esp = 0x0100u;
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
            failed |= sreg == STD_NULL || sreg->selector != 0x1234u ||
                sreg->base != 0x12340u || cpu.data.esp !=
                    0x0100u + (width32 ? 4u : 2u);
            core_machine_destroy(state.machine);
        }
    }
    return failed;
}

static C_INT segment_test_protected_sreg_success(C_VOID)
{
    static const segment_sreg_form forms[] = {
        { 0u,0xc0u,0x07u,0u,1u }, { 2u,0xd0u,0x17u,0u,1u },
        { 1u,0xd8u,0x1fu,0u,1u }, { 3u,0xe0u,0x0fu,0xa1u,2u },
        { 4u,0xe8u,0x0fu,0xa9u,2u }
    };
    static const type_unsigned_8 stack_word[] = { 0x10u,0,0,0 };
    static const type_unsigned_8 stack_dword[] = { 0x10u,0,0xefu,0xbeu };
    STD_SIZE_T index;
    C_INT width32;
    C_INT failed = 0;

    for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        type_unsigned_8 code[] = { 0xb8u,0x10u,0,0,0,0x8eu,forms[index].mov_modrm,0xf4u };
        segment_machine state;
        t_cpu cpu;
        const t_cpu_data_sreg *sreg;

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        failed |= !segment_run_halt(&state, code, sizeof(code), SEG_CODE_ADDRESS,
            &cpu);
        sreg = segment_sreg(&cpu, forms[index].target);
        failed |= sreg == STD_NULL || sreg->selector != 0x0010u ||
            sreg->base != SEG_DATA_ADDRESS;
        core_machine_destroy(state.machine);
    }
    for (width32 = 0; width32 <= 1; ++width32) {
        for (index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
            type_unsigned_8 code[5u] = {0};
            type_unsigned_8 code_size = 0u;
            segment_machine state;
            t_cpu cpu;
            const t_cpu_data_sreg *sreg;

            if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
                !segment_boot_protected(&state)) return 1;
            state.machine->executor_cpu.data.esp = 0x8000u;
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
            failed |= sreg == STD_NULL || sreg->selector != 0x0010u ||
                sreg->base != SEG_DATA_ADDRESS || cpu.data.esp !=
                    0x8000u + (width32 ? 4u : 2u);
            core_machine_destroy(state.machine);
        }
    }
    return failed;
}

typedef struct segment_sreg_failure {
    type_unsigned_8 target;
    type_unsigned_8 mov_modrm;
    type_unsigned_16 selector;
    type_unsigned_32 exception;
    type_unsigned_32 access_address;
    type_unsigned_8 access_value;
} segment_sreg_failure;

static C_INT segment_test_protected_sreg_failures(C_VOID)
{
    static const segment_sreg_failure failures[] = {
        { 1u,0xd8u,0x0018u,VCPUINS_EXCEPT_NP,SEG_GDT_ADDRESS + 29u,0x12u },
        { 2u,0xd0u,0x0018u,VCPUINS_EXCEPT_SS,SEG_GDT_ADDRESS + 29u,0x12u },
        { 3u,0xe0u,0x0020u,VCPUINS_EXCEPT_GP,SEG_GDT_ADDRESS + 37u,0x98u },
        { 4u,0xe8u,0x0013u,VCPUINS_EXCEPT_GP,SEG_GDT_ADDRESS + 21u,0x93u }
    };
    static const type_unsigned_8 pop_fs[] = { 0x66u,0x0fu,0xa1u };
    static const type_unsigned_8 pop_ss[] = { 0x66u,0x17u };
    static const type_unsigned_8 selector_nonpresent[] = { 0x18u,0,0,0 };
    STD_SIZE_T index;
    C_INT failed = 0;

    for (index = 0u; index < sizeof(failures) / sizeof(failures[0]); ++index) {
        type_unsigned_8 code[] = { 0xb8u,0,0,0,0,0x8eu,failures[index].mov_modrm };
        segment_machine state;
        t_cpu before;
        t_cpu after;
        const t_cpu_data_sreg *before_sreg;
        const t_cpu_data_sreg *after_sreg;
        type_unsigned_8 access = 0u;
        C_INT case_failed;

        code[1u] = (type_unsigned_8)failures[index].selector;
        code[2u] = (type_unsigned_8)(failures[index].selector >> 8u);
        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        case_failed = !segment_run_exception(&state, code, sizeof(code),
            SEG_CODE_ADDRESS, failures[index].exception, &after);
        before_sreg = segment_sreg(&before, failures[index].target);
        after_sreg = segment_sreg(&after, failures[index].target);
        case_failed |= before_sreg == STD_NULL || after_sreg == STD_NULL ||
            STD_MEMCMP(before_sreg, after_sreg, sizeof(*before_sreg)) != 0 ||
            before.data.esp != after.data.esp ||
            before.data.eflags != after.data.eflags ||
            !test_core_machine_fixture_read_linear(state.machine,
                failures[index].access_address, TYPE_REFERENCE_OF(access),
                sizeof(access)) ||
            access != failures[index].access_value;
        if (case_failed) STD_FPRINTF(STD_STDERR,
            "M5:T301:SEGMENT-SELECTOR mov-fail index=%u selector=%04x access=%02x esp=%08x/%08x flags=%08x/%08x\n",
            (unsigned)index, failures[index].selector, access, before.data.esp,
            after.data.esp, before.data.eflags, after.data.eflags);
        failed |= case_failed;
        core_machine_destroy(state.machine);
    }
    for (index = 0u; index < 2u; ++index) {
        const type_unsigned_8 *code = index == 0u ? pop_fs : pop_ss;
        STD_SIZE_T code_size = index == 0u ? sizeof(pop_fs) : sizeof(pop_ss);
        type_unsigned_8 target = index == 0u ? 3u : 2u;
        type_unsigned_32 exception = index == 0u ? VCPUINS_EXCEPT_NP : VCPUINS_EXCEPT_SS;
        segment_machine state;
        t_cpu before;
        t_cpu after;
        const t_cpu_data_sreg *before_sreg;
        const t_cpu_data_sreg *after_sreg;
        type_unsigned_8 access = 0u;
        C_INT case_failed;

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        state.machine->executor_cpu.data.esp = 0x8000u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        case_failed = !segment_write(&state, SEG_DATA_ADDRESS + 0x8000u,
                selector_nonpresent, sizeof(selector_nonpresent)) ||
            !segment_run_exception(&state, code, code_size, SEG_CODE_ADDRESS,
                exception, &after);
        before_sreg = segment_sreg(&before, target);
        after_sreg = segment_sreg(&after, target);
        case_failed |= before_sreg == STD_NULL || after_sreg == STD_NULL ||
            STD_MEMCMP(before_sreg, after_sreg, sizeof(*before_sreg)) != 0 ||
            before.data.esp != after.data.esp ||
            before.data.eflags != after.data.eflags ||
            !test_core_machine_fixture_read_linear(state.machine,
                SEG_GDT_ADDRESS + 29u, TYPE_REFERENCE_OF(access),
                sizeof(access)) || access != 0x12u;
        if (case_failed) STD_FPRINTF(STD_STDERR,
            "M5:T301:SEGMENT-SELECTOR pop-fail index=%u access=%02x esp=%08x/%08x flags=%08x/%08x\n",
            (unsigned)index, access, before.data.esp, after.data.esp,
            before.data.eflags, after.data.eflags);
        failed |= case_failed;
        core_machine_destroy(state.machine);
    }
    return failed;
}

static C_INT segment_test_protected_selector_forms(C_VOID)
{
    static const type_unsigned_8 lar_code[] = {
        0xb8u,0x10u,0x00u,0x00u,0x00u,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const type_unsigned_8 lsl_code[] = {
        0xb8u,0x10u,0x00u,0x00u,0x00u,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const type_unsigned_8 lar16_code[] = {
        0xb8u,0x10u,0x00u,0xcdu,0xabu,0x66u,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const type_unsigned_8 arpl_code[] = {
        0x66u,0xb8u,0x01u,0x00u,0x66u,0xb9u,0x03u,0x00u,
        0x66u,0x63u,0xc8u,0xf4u
    };
    static const type_unsigned_8 verr_code[] = {
        0xb8u,0x10u,0x00u,0x00u,0x00u,0x0fu,0x00u,0xe0u,0xf4u
    };
    static const type_unsigned_8 verw_code[] = {
        0xb8u,0x10u,0x00u,0x00u,0x00u,0x0fu,0x00u,0xe8u,0xf4u
    };
    segment_machine state;
    t_cpu cpu;
    C_INT failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, lar_code, sizeof(lar_code), SEG_CODE_ADDRESS,
            &cpu) || cpu.data.eax != 0x00409300u ||
        !TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF);
    core_machine_destroy(state.machine);
    state.machine = STD_NULL;
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, lsl_code, sizeof(lsl_code), SEG_CODE_ADDRESS,
            &cpu) || cpu.data.eax != 0x0000ffffu ||
        !TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF);
    core_machine_destroy(state.machine);
    state.machine = STD_NULL;
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, lar16_code, sizeof(lar16_code), SEG_CODE_ADDRESS,
            &cpu) || cpu.data.eax != 0xabcd9300u ||
        !TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF);
    core_machine_destroy(state.machine);
    state.machine = STD_NULL;
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, arpl_code, sizeof(arpl_code), SEG_CODE_ADDRESS,
            &cpu) || (cpu.data.eax & 0xffffu) != 0x0003u ||
        !TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF);
    core_machine_destroy(state.machine);
    state.machine = STD_NULL;
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, verr_code, sizeof(verr_code), SEG_CODE_ADDRESS,
            &cpu) || !TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF);
    core_machine_destroy(state.machine);
    state.machine = STD_NULL;
    if (!failed) failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    if (!failed) failed |= !segment_boot_protected(&state) ||
        !segment_run_halt(&state, verw_code, sizeof(verw_code), SEG_CODE_ADDRESS,
            &cpu) || !TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF);
    core_machine_destroy(state.machine);
    state.machine = STD_NULL;
    return failed;
}

static C_INT segment_test_selector_query_edges(C_VOID)
{
    static const type_unsigned_8 lsl16_code[] = {
        0xb8u,0x10u,0x00u,0xcdu,0xabu,0x66u,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const type_unsigned_8 lar_invalid[] = {
        0xb8u,0x28u,0x00u,0xcdu,0xabu,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const type_unsigned_8 lsl_invalid[] = {
        0xb8u,0x28u,0x00u,0xcdu,0xabu,0x66u,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const type_unsigned_8 lar_nonpresent[] = {
        0xb8u,0x18u,0x00u,0xcdu,0xabu,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const type_unsigned_8 lsl_nonpresent[] = {
        0xb8u,0x18u,0x00u,0xcdu,0xabu,0x66u,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const type_unsigned_8 lar_tss[] = {
        0xb8u,0x30u,0x00u,0,0,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const type_unsigned_8 lsl_tss[] = {
        0xb8u,0x30u,0x00u,0,0,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const type_unsigned_8 lar_tss_rpl[] = {
        0xb8u,0x33u,0x00u,0xcdu,0xabu,0x0fu,0x02u,0xc0u,0xf4u
    };
    static const type_unsigned_8 lsl_tss_rpl[] = {
        0xb8u,0x33u,0x00u,0xcdu,0xabu,0x0fu,0x03u,0xc0u,0xf4u
    };
    static const type_unsigned_8 verr_type[] = {
        0xb8u,0x20u,0x00u,0,0,0x0fu,0x00u,0xe0u,0xf4u
    };
    static const type_unsigned_8 verw_type[] = {
        0xb8u,0x20u,0x00u,0,0,0x0fu,0x00u,0xe8u,0xf4u
    };
    static const type_unsigned_8 verr_nonpresent[] = {
        0xb8u,0x18u,0x00u,0,0,0x0fu,0x00u,0xe0u,0xf4u
    };
    static const type_unsigned_8 verw_nonpresent[] = {
        0xb8u,0x18u,0x00u,0,0,0x0fu,0x00u,0xe8u,0xf4u
    };
    static const type_unsigned_8 verr_memory[] = {
        0x67u,0x0fu,0x00u,0x26u,0x00u,0x04u,0xf4u
    };
    static const type_unsigned_8 verw_memory[] = {
        0x67u,0x0fu,0x00u,0x2eu,0x00u,0x04u,0xf4u
    };
    static const type_unsigned_8 data_selector[] = { 0x10u,0u };
    const type_unsigned_8 *codes[] = { lsl16_code, lar_invalid, lsl_invalid,
        lar_nonpresent, lsl_nonpresent, lar_tss, lsl_tss, lar_tss_rpl,
        lsl_tss_rpl, verr_type, verw_type, verr_nonpresent, verw_nonpresent,
        verr_memory, verw_memory };
    const STD_SIZE_T sizes[] = { sizeof(lsl16_code), sizeof(lar_invalid),
        sizeof(lsl_invalid), sizeof(lar_nonpresent), sizeof(lsl_nonpresent),
        sizeof(lar_tss), sizeof(lsl_tss), sizeof(lar_tss_rpl),
        sizeof(lsl_tss_rpl), sizeof(verr_type), sizeof(verw_type),
        sizeof(verr_nonpresent), sizeof(verw_nonpresent), sizeof(verr_memory),
        sizeof(verw_memory) };
    const type_unsigned_32 expected_eax[] = { 0xabcdffffu,0xabcd0028u,0xabcd0028u,
        0xabcd0018u,0xabcd0018u,0x00408900u,0x0000ffffu,0xabcd0033u,
        0xabcd0033u,0x00000020u,0x00000020u,0x00000018u,0x00000018u,
        0x00000010u,0x00000010u };
    const C_INT expected_zf[] = { 1,0,0,0,0,1,1,0,0,0,0,0,0,1,1 };
    STD_SIZE_T index;
    C_INT failed = 0;

    for (index = 0u; index < sizeof(codes) / sizeof(codes[0]); ++index) {
        segment_machine state;
        t_cpu before;
        t_cpu cpu;
        type_unsigned_8 system_access = 0u;
        type_unsigned_8 nonpresent_access = 0u;
        C_INT case_failed;

        if (!segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !segment_boot_protected(&state)) return 1;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        if (index >= 13u)
            failed |= !segment_write(&state, SEG_DATA_ADDRESS + 0x0400u,
                data_selector, sizeof(data_selector));
        case_failed = !segment_run_halt(&state, codes[index], sizes[index],
            SEG_CODE_ADDRESS, &cpu) || cpu.data.eax != expected_eax[index] ||
            !!TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF) != expected_zf[index];
        case_failed |= cpu.data.esp != before.data.esp ||
            (cpu.data.eflags & ~VCPU_EFLAGS_ZF) !=
                (before.data.eflags & ~VCPU_EFLAGS_ZF) ||
            STD_MEMCMP(&cpu.data.es, &before.data.es, sizeof(cpu.data.es)) != 0 ||
            STD_MEMCMP(&cpu.data.ds, &before.data.ds, sizeof(cpu.data.ds)) != 0 ||
            STD_MEMCMP(&cpu.data.ss, &before.data.ss, sizeof(cpu.data.ss)) != 0 ||
            STD_MEMCMP(&cpu.data.fs, &before.data.fs, sizeof(cpu.data.fs)) != 0 ||
            STD_MEMCMP(&cpu.data.gs, &before.data.gs, sizeof(cpu.data.gs)) != 0;
        case_failed |= !test_core_machine_fixture_read_linear(state.machine,
                SEG_GDT_ADDRESS + 45u, TYPE_REFERENCE_OF(system_access),
                sizeof(system_access)) || system_access != 0x80u ||
            !test_core_machine_fixture_read_linear(state.machine,
                SEG_GDT_ADDRESS + 29u, TYPE_REFERENCE_OF(nonpresent_access),
                sizeof(nonpresent_access)) || nonpresent_access != 0x12u;
        if (case_failed) STD_FPRINTF(STD_STDERR,
            "M5:T301:SEGMENT-SELECTOR query-edge index=%u eax=%08x zf=%d/%d sys=%02x np=%02x\n",
            (unsigned)index, cpu.data.eax,
            !!TYPE_GET_BIT(cpu.data.eflags, VCPU_EFLAGS_ZF), expected_zf[index],
            system_access, nonpresent_access);
        failed |= case_failed;
        core_machine_destroy(state.machine);
    }
    return failed;
}

static C_INT segment_test_rejected_forms(C_VOID)
{
    static const type_unsigned_8 mov_cs[] = { 0x8eu,0xc8u };
    static const type_unsigned_8 mov_from_fs[] = { 0x8cu,0xe0u };
    static const type_unsigned_8 mov_to_fs[] = { 0x8eu,0xe0u };
    static const type_unsigned_8 pop_fs[] = { 0x0fu,0xa1u };
    static const type_unsigned_8 pop_gs[] = { 0x0fu,0xa9u };
    static const type_unsigned_8 lss[] = { 0x0fu,0xb2u,0xc0u };
    static const type_unsigned_8 lfs[] = { 0x0fu,0xb4u,0xc0u };
    static const type_unsigned_8 lgs[] = { 0x0fu,0xb5u,0xc0u };
    static const type_unsigned_8 lar_real[] = { 0x0fu,0x02u,0xc0u };
    static const type_unsigned_8 lsl_real[] = { 0x0fu,0x03u,0xc0u };
    static const type_unsigned_8 verr_real[] = { 0x0fu,0x00u,0xe0u };
    static const type_unsigned_8 verw_real[] = { 0x0fu,0x00u,0xe8u };
    static const type_unsigned_8 lar_66[] = { 0x66u,0x0fu,0x02u,0xc0u };
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    const type_unsigned_8 *programs[] = { mov_cs, mov_from_fs, mov_to_fs, pop_fs,
        pop_gs, lss, lfs, lgs, lar_real, lsl_real, verr_real, verw_real,
        lar_66 };
    const STD_SIZE_T sizes[] = { sizeof(mov_cs), sizeof(mov_from_fs),
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
    STD_SIZE_T profile_index;
    STD_SIZE_T program_index;
    C_INT failed = 0;

    for (profile_index = 0u; profile_index < sizeof(profiles) / sizeof(profiles[0]);
         ++profile_index) {
        for (program_index = 0u; program_index < sizeof(programs) / sizeof(programs[0]);
             ++program_index) {
            core_machine_run_result result;
            core_machine_cpu_diagnostic diagnostic;
            segment_machine state;

            if (profiles[profile_index] > maximum_profiles[program_index]) continue;
            if (!segment_prepare(&state, profiles[profile_index])) return 1;
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            failed |= !segment_write(&state, 0u, programs[program_index],
                sizes[program_index]);
            failed |= core_machine_run(state.machine,
                (const core_machine_run_budget){ 8u, 0u }, &result) !=
                    TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT;
            failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                    TYPE_STATUS_OK || !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD);
            core_machine_destroy(state.machine);
        }
    }
    return failed;
}

static C_INT segment_test_pop_fault_atomicity(C_VOID)
{
    static const type_unsigned_8 pop_fs[] = { 0x66u,0x0fu,0xa1u };
    static const type_unsigned_8 selector[] = { 0x18u,0x00u,0,0 };
    const core_machine_run_budget budget = { 8u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    segment_machine state;
    t_cpu before;
    t_cpu after;
    C_INT failed = !segment_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) failed |= !segment_boot_protected(&state);
    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x8000u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !segment_write(&state, SEG_DATA_ADDRESS + 0x8000u, selector,
            sizeof(selector)) || !segment_write(&state, SEG_CODE_ADDRESS, pop_fs,
            sizeof(pop_fs));
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            diagnostic.first_fault.exception_code != 0u;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.esp != before.data.esp ||
            STD_MEMCMP(&after.data.fs, &before.data.fs, sizeof(after.data.fs)) != 0;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT segment_test_metadata(C_VOID)
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

C_INT main(C_VOID)
{
    C_INT real_loads = segment_test_real_load_forms();
    C_INT protected_286 = segment_test_80286_protected_legal_forms();
    C_INT protected_286_rejections =
        segment_test_80286_protected_cache_rejections();
    C_INT lxs_memory_only = segment_test_lxs_memory_only();
    C_INT lxs_atomicity = segment_test_lxs_fault_atomicity();
    C_INT real_sregs = segment_test_real_sreg_loads();
    C_INT protected_sregs = segment_test_protected_sreg_success();
    C_INT protected_sreg_failures = segment_test_protected_sreg_failures();
    C_INT protected_forms = segment_test_protected_selector_forms();
    C_INT query_edges = segment_test_selector_query_edges();
    C_INT rejected = segment_test_rejected_forms();
    C_INT atomicity = segment_test_pop_fault_atomicity();
    C_INT metadata = segment_test_metadata();

    if (real_loads || protected_286 || protected_286_rejections || lxs_memory_only || lxs_atomicity || real_sregs || protected_sregs ||
        protected_sreg_failures || protected_forms || query_edges || rejected ||
        atomicity || metadata) {
        STD_FPRINTF(STD_STDERR,
            "M5:T301:SEGMENT-SELECTOR:FAIL real=%d protected-286=%d protected-286-reject=%d lxs=%d lxs-atomic=%d sreg-real=%d sreg-protected=%d sreg-fault=%d protected=%d query=%d rejected=%d atomic=%d metadata=%d\n",
            real_loads, protected_286, protected_286_rejections, lxs_memory_only, lxs_atomicity, real_sregs, protected_sregs,
            protected_sreg_failures, protected_forms, query_edges, rejected,
            atomicity, metadata);
        return 1;
    }
    STD_PRINTF("M5:T301:SEGMENT-SELECTOR:OK\n");
    return 0;
}

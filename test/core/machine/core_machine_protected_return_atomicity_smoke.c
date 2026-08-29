#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define ATOMIC_GDT_POINTER 0x0100u
#define ATOMIC_IDT_POINTER 0x0110u
#define ATOMIC_GDT_BASE 0x0300u
#define ATOMIC_IDT_BASE 0x0400u
#define ATOMIC_KERNEL_BASE 0x2000u
#define ATOMIC_KERNEL_STACK_BASE 0x3000u
#define ATOMIC_USER_CODE_ACCESS (ATOMIC_GDT_BASE + 29u)
#define ATOMIC_NONPRESENT_CODE_ACCESS (ATOMIC_GDT_BASE + 45u)
#define ATOMIC_NONPRESENT_STACK_ACCESS (ATOMIC_GDT_BASE + 53u)
#define ATOMIC_BAD_ENTRY_ACCESS (ATOMIC_GDT_BASE + 61u)
#define ATOMIC_CONFORM_CODE_ACCESS (ATOMIC_GDT_BASE + 69u)

typedef struct atomic_machine {
    core_machine *machine;
} atomic_machine;

typedef struct atomic_return_case {
    const C_CHAR *name;
    type_unsigned_16 cs;
    type_unsigned_16 ss;
    type_unsigned_8 vector;
    type_unsigned_16 error_code;
    type_unsigned_32 exception_mask;
    C_INT delivered;
} atomic_return_case;

static C_VOID atomic_reset(C_VOID *opaque)
{
    atomic_machine *state = (atomic_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider atomic_provider = {
    atomic_reset, STD_NULL
};

static C_INT atomic_prepare(atomic_machine *state, core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &atomic_provider, state, &state->machine)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT atomic_write(atomic_machine *state, type_unsigned_32 address,
    const type_unsigned_8 *bytes, STD_SIZE_T byte_count)
{
    return core_machine_memory_write(state->machine, address, bytes, byte_count) ==
        TYPE_STATUS_OK;
}

static C_VOID atomic_set_gate(type_unsigned_8 *idt, type_unsigned_8 vector, type_unsigned_16 offset)
{
    type_unsigned_16 index = (type_unsigned_16)vector * 8u;

    idt[index] = (type_unsigned_8)offset;
    idt[index + 1u] = (type_unsigned_8)(offset >> 8u);
    idt[index + 2u] = 0x08u;
    idt[index + 5u] = 0x86u;
}

static C_INT atomic_install(atomic_machine *state)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x47u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u };
    static const type_unsigned_8 idt_pointer[] = { 0x6fu, 0x00u, 0x00u, 0x04u, 0x00u, 0x00u };
    static const type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x40,0,0xfa,0,0,
        0xff,0xff,0,0x50,0,0xf2,0,0,
        0xff,0xff,0,0x40,0,0x7a,0,0,
        0xff,0xff,0,0x50,0,0x72,0,0,
        0xff,0xff,0,0x60,0,0xf2,0,0,
        0xff,0xff,0,0x70,0,0xfe,0,0
    };
    type_unsigned_8 idt[0x70u] = {0};
    static const type_unsigned_8 real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0x0f,0x01,0x1e,0x10,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x10,0x00,0x8e,0xd8,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    static const type_unsigned_8 initial_halt[] = { 0xf4 };
    type_unsigned_8 fault_handlers[0x21u] = {0};

    atomic_set_gate(idt, 11u, 0x0110u);
    atomic_set_gate(idt, 12u, 0x0120u);
    atomic_set_gate(idt, 13u, 0x0100u);
    fault_handlers[0u] = 0xf4u;
    fault_handlers[0x10u] = 0xf4u;
    fault_handlers[0x20u] = 0xf4u;
    return atomic_write(state, ATOMIC_GDT_POINTER, gdt_pointer, sizeof(gdt_pointer)) &&
        atomic_write(state, ATOMIC_IDT_POINTER, idt_pointer, sizeof(idt_pointer)) &&
        atomic_write(state, ATOMIC_GDT_BASE, gdt, sizeof(gdt)) &&
        atomic_write(state, ATOMIC_IDT_BASE, idt, sizeof(idt)) &&
        atomic_write(state, 0u, real_code, sizeof(real_code)) &&
        atomic_write(state, ATOMIC_KERNEL_BASE, initial_halt, sizeof(initial_halt)) &&
        atomic_write(state, ATOMIC_KERNEL_BASE + 0x100u, fault_handlers,
            sizeof(fault_handlers));
}

static STD_SIZE_T atomic_return_code(type_unsigned_8 *bytes, C_INT use_iret,
    const atomic_return_case *test)
{
    static const type_unsigned_8 retf[] = {
        0xb8,0,0,0x50,0xb8,0x00,0xa0,0x50,
        0xb8,0,0,0x50,0xb8,0x00,0x00,0x50,0xcb
    };
    static const type_unsigned_8 iret[] = {
        0xb8,0,0,0x50,0xb8,0x00,0xa0,0x50,
        0xb8,0x02,0x02,0x50,0xb8,0,0,0x50,
        0xb8,0x00,0x00,0x50,0xcf
    };

    if (use_iret) {
        STD_MEMCPY(bytes, iret, sizeof(iret));
        bytes[1u] = (type_unsigned_8)test->ss;
        bytes[2u] = (type_unsigned_8)(test->ss >> 8u);
        bytes[13u] = (type_unsigned_8)test->cs;
        bytes[14u] = (type_unsigned_8)(test->cs >> 8u);
        return sizeof(iret);
    }
    STD_MEMCPY(bytes, retf, sizeof(retf));
    bytes[1u] = (type_unsigned_8)test->ss;
    bytes[2u] = (type_unsigned_8)(test->ss >> 8u);
    bytes[9u] = (type_unsigned_8)test->cs;
    bytes[10u] = (type_unsigned_8)(test->cs >> 8u);
    return sizeof(retf);
}

static type_unsigned_16 atomic_fault_stop_ip(type_unsigned_8 vector)
{
    switch (vector) {
    case 11u: return 0x0111u;
    case 12u: return 0x0121u;
    case 13u: return 0x0101u;
    default: return 0u;
    }
}

static C_INT atomic_test_outer_return(const atomic_return_case *test, C_INT use_iret)
{
    static const type_unsigned_32 access_addresses[] = {
        ATOMIC_USER_CODE_ACCESS,
        ATOMIC_NONPRESENT_CODE_ACCESS,
        ATOMIC_NONPRESENT_STACK_ACCESS,
        ATOMIC_BAD_ENTRY_ACCESS,
        ATOMIC_CONFORM_CODE_ACCESS
    };
    static const type_unsigned_8 expected_access[] = { 0xfau, 0x7au, 0x72u, 0xf2u, 0xfeu };
    atomic_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic;
    const core_machine_run_budget budget = { 128u, 0u };
    t_cpu_data before;
    t_cpu cpu;
    type_unsigned_8 code[21u];
    type_unsigned_8 access[sizeof(access_addresses) / sizeof(access_addresses[0])];
    type_unsigned_16 frame[4u] = {0};
    type_unsigned_16 expected_sp = use_iret ? 0x7feeu : 0x7ff0u;
    type_unsigned_16 expected_return_ip = use_iret ? 20u : 16u;
    STD_SIZE_T index;
    C_INT install_failed;
    C_INT boot_failed;
    C_INT run_failed;
    C_INT access_failed = 0;
    C_INT state_failed;
    C_INT failed = !atomic_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        install_failed = !atomic_install(&state);
        boot_failed = core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= install_failed || boot_failed;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before = cpu.data;
        failed |= !atomic_write(&state, ATOMIC_KERNEL_BASE, code,
            atomic_return_code(code, use_iret, test));
        if (test->delivered) {
            run_failed = core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || !diagnostic.last_delivered_exception.valid ||
                !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                    test->exception_mask) ||
                diagnostic.last_delivered_exception.exception_code != test->error_code;
        } else {
            run_failed = core_machine_run(state.machine, budget, &result) != TYPE_STATUS_FAULT ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                result.detail != 0x00001000u;
        }
        failed |= run_failed;
        for (index = 0u; index < sizeof(access); ++index) {
            access_failed |= !test_core_machine_fixture_read_linear(state.machine,
                access_addresses[index], TYPE_REFERENCE_OF(access[index]),
                sizeof(access[index])) ||
                access[index] != expected_access[index];
        }
        failed |= access_failed;
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        if (test->delivered) {
            state_failed = core_machine_memory_read(state.machine,
                ATOMIC_KERNEL_STACK_BASE + expected_sp, frame, sizeof(frame)) != TYPE_STATUS_OK;
            state_failed |= cpu.data.eip != atomic_fault_stop_ip(test->vector) ||
                cpu.data.sp != expected_sp ||
                cpu.data.flags != before.flags ||
                STD_MEMCMP(&cpu.data.cs, &before.cs, sizeof(before.cs)) != 0 ||
                STD_MEMCMP(&cpu.data.ss, &before.ss, sizeof(before.ss)) != 0 ||
                frame[0u] != test->error_code || frame[1u] != expected_return_ip ||
                frame[2u] != before.cs.selector || frame[3u] != before.flags;
        } else {
            state_failed = cpu.data.eip != expected_return_ip ||
                cpu.data.sp != (use_iret ? 0x7ff6u : 0x7ff8u) ||
                cpu.data.flags != before.flags ||
                STD_MEMCMP(&cpu.data.cs, &before.cs, sizeof(before.cs)) != 0 ||
                STD_MEMCMP(&cpu.data.ss, &before.ss, sizeof(before.ss)) != 0;
        }
        failed |= state_failed;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T293 %s return=%s fail=%d/%d/%d/%d/%d ip=%04x sp=%04x flags=%04x/%04x cs=%d ss=%d access=%02x/%02x/%02x/%02x frame=%04x/%04x/%04x/%04x\n",
                test->name, use_iret ? "iret" : "retf", install_failed, boot_failed,
                run_failed, access_failed, state_failed,
                cpu.data.ip, cpu.data.sp, cpu.data.flags, before.flags,
                STD_MEMCMP(&cpu.data.cs, &before.cs, sizeof(before.cs)),
                STD_MEMCMP(&cpu.data.ss, &before.ss, sizeof(before.ss)),
                access[0u], access[1u], access[2u], access[3u],
                frame[0u], frame[1u], frame[2u], frame[3u]);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT atomic_test_outer_retf_frame(C_INT operand16, C_INT address_prefix,
    C_INT wide_new_stack)
{
    static const type_unsigned_8 retf32[] = { 0xcau,0x04u,0x00u };
    static const type_unsigned_8 retf32_address[] = { 0x67u,0xcau,0x04u,0x00u };
    static const type_unsigned_8 retf16[] = { 0x66u,0xcau,0x04u,0x00u };
    static const type_unsigned_32 frame32[] = { 0x0010u,0x001bu,0u,0x1000u,0x0023u };
    static const type_unsigned_16 frame16[] = { 0x0010u,0x001bu,0u,0u,0x1000u,0x0023u };
    const core_machine_run_budget boot_budget = { 128u, 0u };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 stack_flags = 0x40u;
    const type_unsigned_8 *program = operand16 ? retf16 :
        (address_prefix ? retf32_address : retf32);
    const STD_SIZE_T program_bytes = operand16 ? sizeof(retf16) :
        (address_prefix ? sizeof(retf32_address) : sizeof(retf32));
    atomic_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    C_INT failed = !atomic_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !atomic_install(&state) ||
            core_machine_run(state.machine, boot_budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        cpu.data.cs.seg.exec.defsize = TYPE_TRUE;
        cpu.data.esp = 0x12348000u;
        state.machine->executor_cpu = cpu;
        if (!failed && wide_new_stack) {
            failed |= !atomic_write(&state, ATOMIC_GDT_BASE + 38u, &stack_flags,
                sizeof(stack_flags));
        }
        failed |= !atomic_write(&state, ATOMIC_KERNEL_BASE, program, program_bytes) ||
            !atomic_write(&state, ATOMIC_KERNEL_STACK_BASE + 0x8000u,
                operand16 ? (const C_VOID *)frame16 : (const C_VOID *)frame32,
                operand16 ? sizeof(frame16) : sizeof(frame32)) ||
            core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid || cpu.data.eip != 0x0010u ||
            cpu.data.cs.selector != 0x001bu || cpu.data.cs.dpl != 3u ||
            cpu.data.ss.selector != 0x0023u || cpu.data.ss.dpl != 3u ||
            cpu.data.esp != (wide_new_stack ? 0x00001004u : 0x12341004u);
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT atomic_test_outer_retf32_nonpresent_stack(C_VOID)
{
    static const type_unsigned_8 retf[] = { 0x66u,0xcau,0x04u,0x00u };
    static const type_unsigned_32 frame[] = { 0x0010u,0x001bu,0u,0x1000u,0x0033u };
    const core_machine_run_budget boot_budget = { 128u, 0u };
    const core_machine_run_budget budget = { 128u, 0u };
    atomic_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 code_access = 0u;
    type_unsigned_8 stack_access = 0u;
    C_INT failed = !atomic_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !atomic_install(&state) ||
            core_machine_run(state.machine, boot_budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.esp = 0x12348000u;
        state.machine->executor_cpu = before;
        failed |= !atomic_write(&state, ATOMIC_KERNEL_BASE, retf, sizeof(retf)) ||
            !atomic_write(&state, ATOMIC_KERNEL_STACK_BASE + 0x8000u,
                (const type_unsigned_8 *)frame,
                sizeof(frame)) ||
            !test_core_machine_fixture_read_linear(state.machine,
                ATOMIC_USER_CODE_ACCESS, TYPE_REFERENCE_OF(code_access), 1u) ||
            !test_core_machine_fixture_read_linear(state.machine,
                ATOMIC_NONPRESENT_STACK_ACCESS, TYPE_REFERENCE_OF(stack_access), 1u) ||
            core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_SS) ||
            diagnostic.last_delivered_exception.exception_code != 0x0030u ||
            after.data.eip != atomic_fault_stop_ip(12u) || after.data.sp != 0x7ff8u ||
            after.data.flags != before.data.flags ||
            STD_MEMCMP(&after.data.cs, &before.data.cs, sizeof(before.data.cs)) != 0 ||
            STD_MEMCMP(&after.data.ss, &before.data.ss, sizeof(before.data.ss)) != 0 ||
            !test_core_machine_fixture_read_linear(state.machine,
                ATOMIC_USER_CODE_ACCESS, TYPE_REFERENCE_OF(code_access), 1u) ||
            code_access != 0xfau ||
            !test_core_machine_fixture_read_linear(state.machine,
                ATOMIC_NONPRESENT_STACK_ACCESS, TYPE_REFERENCE_OF(stack_access), 1u) ||
            stack_access != 0x72u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT atomic_test_outer_iret_frame(C_INT operand16, C_INT address_prefix,
    C_INT wide_new_stack, C_INT restricted_flags)
{
    static const type_unsigned_8 iret32[] = { 0xcfu };
    static const type_unsigned_8 iret32_address[] = { 0x67u,0xcfu };
    static const type_unsigned_8 iret16[] = { 0x66u,0xcfu };
    type_unsigned_32 frame32[] = { 0x0010u,0x001bu,0u,0x1000u,0x0023u };
    type_unsigned_16 frame16[] = { 0x0010u,0x001bu,0u,0x1000u,0x0023u };
    const core_machine_run_budget boot_budget = { 128u, 0u };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_8 stack_flags = 0x40u;
    const type_unsigned_8 *program = operand16 ? iret16 :
        (address_prefix ? iret32_address : iret32);
    const STD_SIZE_T program_bytes = operand16 ? sizeof(iret16) :
        (address_prefix ? sizeof(iret32_address) : sizeof(iret32));
    const type_unsigned_32 return_flags = restricted_flags ? 0x00013003u :
        (operand16 ? 0x00003203u : 0x00013203u);
    const type_unsigned_32 expected_flags = restricted_flags ? 0x00010203u :
        return_flags;
    atomic_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    C_INT failed = !atomic_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !atomic_install(&state) ||
            core_machine_run(state.machine, boot_budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        cpu.data.cs.seg.exec.defsize = TYPE_TRUE;
        cpu.data.eflags = restricted_flags ? 0x00000202u : 0x00000002u;
        if (restricted_flags) {
            cpu.data.cs.selector = 0x0009u;
            cpu.data.cs.dpl = 1u;
            cpu.data.ss.selector = 0x0011u;
            cpu.data.ss.dpl = 1u;
        }
        cpu.data.esp = 0x12348000u;
        state.machine->executor_cpu = cpu;
        frame32[2u] = return_flags;
        frame16[2u] = (type_unsigned_16)return_flags;
        if (!failed && wide_new_stack) {
            failed |= !atomic_write(&state, ATOMIC_GDT_BASE + 38u, &stack_flags,
                sizeof(stack_flags));
        }
        failed |= !atomic_write(&state, ATOMIC_KERNEL_BASE, program, program_bytes) ||
            !atomic_write(&state, ATOMIC_KERNEL_STACK_BASE + 0x8000u,
                operand16 ? (const C_VOID *)frame16 : (const C_VOID *)frame32,
                operand16 ? sizeof(frame16) : sizeof(frame32)) ||
            core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid || cpu.data.eip != 0x0010u ||
            cpu.data.cs.selector != 0x001bu || cpu.data.cs.dpl != 3u ||
            cpu.data.ss.selector != 0x0023u || cpu.data.ss.dpl != 3u ||
            cpu.data.eflags != expected_flags ||
            cpu.data.esp != (wide_new_stack ? 0x00001000u : 0x12341000u);
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT atomic_test_outer_iret32_failure(C_INT target_limit)
{
    static const type_unsigned_8 iret[] = { 0x66u,0xcfu };
    static const type_unsigned_32 frame[] = {
        0x0010u,0x001bu,0x00000203u,0x1000u,0x0033u
    };
    const core_machine_run_budget boot_budget = { 128u, 0u };
    const core_machine_run_budget budget = { 128u, 0u };
    const type_unsigned_8 zero = 0u;
    const type_unsigned_8 code_access = 0xf8u;
    atomic_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 current_code_access = 0u;
    type_unsigned_8 stack_access = 0u;
    C_INT failed = !atomic_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !atomic_install(&state) ||
            core_machine_run(state.machine, boot_budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.esp = 0x12348000u;
        state.machine->executor_cpu = before;
        if (target_limit) {
            failed |= !atomic_write(&state, ATOMIC_GDT_BASE + 24u, &zero, 1u) ||
                !atomic_write(&state, ATOMIC_GDT_BASE + 25u, &zero, 1u) ||
                !atomic_write(&state, ATOMIC_USER_CODE_ACCESS, &code_access, 1u);
        }
        failed |= !atomic_write(&state, ATOMIC_KERNEL_BASE, iret, sizeof(iret)) ||
            !atomic_write(&state, ATOMIC_KERNEL_STACK_BASE + 0x8000u,
                (const type_unsigned_8 *)frame, sizeof(frame)) ||
            core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                target_limit ? VCPUINS_EXCEPT_GP : VCPUINS_EXCEPT_SS) ||
            diagnostic.last_delivered_exception.exception_code !=
                (target_limit ? 0u : 0x0030u) ||
            after.data.eip != atomic_fault_stop_ip(target_limit ? 13u : 12u) ||
            after.data.sp != 0x7ff8u || after.data.flags != before.data.flags ||
            STD_MEMCMP(&after.data.cs, &before.data.cs, sizeof(before.data.cs)) != 0 ||
            STD_MEMCMP(&after.data.ss, &before.data.ss, sizeof(before.data.ss)) != 0 ||
            !test_core_machine_fixture_read_linear(state.machine,
                ATOMIC_USER_CODE_ACCESS, TYPE_REFERENCE_OF(current_code_access), 1u) ||
            current_code_access != (target_limit ? code_access : 0xfau) ||
            !test_core_machine_fixture_read_linear(state.machine,
                ATOMIC_NONPRESENT_STACK_ACCESS, TYPE_REFERENCE_OF(stack_access), 1u) ||
            stack_access != 0x72u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

C_INT main(C_VOID)
{
    static const atomic_return_case cases[] = {
        { "nonpresent-cs", 0x002bu, 0x0023u, 11u, 0x0028u, VCPUINS_EXCEPT_NP, 1 },
        { "nonpresent-ss", 0x001bu, 0x0033u, 12u, 0x0030u, VCPUINS_EXCEPT_SS, 1 },
        { "inaccessible-entry", 0x003bu, 0x0023u, 13u, 0x0038u, VCPUINS_EXCEPT_GP, 1 },
        { "conforming-cs", 0x0043u, 0x0023u, 13u, 0x0040u, VCPUINS_EXCEPT_GP, 1 }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        if (atomic_test_outer_return(&cases[index], 0) ||
            atomic_test_outer_return(&cases[index], 1)) return 1;
    }
    if (atomic_test_outer_retf_frame(1, 0, 0) ||
        atomic_test_outer_retf_frame(0, 0, 1) ||
        atomic_test_outer_retf_frame(0, 1, 1) ||
        atomic_test_outer_retf32_nonpresent_stack() ||
        atomic_test_outer_iret_frame(1, 0, 0, 0) ||
        atomic_test_outer_iret_frame(0, 0, 1, 0) ||
        atomic_test_outer_iret_frame(0, 1, 1, 0) ||
        atomic_test_outer_iret_frame(0, 0, 0, 1) ||
        atomic_test_outer_iret32_failure(0) ||
        atomic_test_outer_iret32_failure(1)) return 1;
    STD_PRINTF("M5:T306:S4:OUTER-RETF:OK\n");
    STD_PRINTF("M5:T306:S5:OUTER-IRET:OK\n");
    STD_PRINTF("M5:T293:S2:PROTECTED-RETURN-ATOMICITY:OK\n");
    return 0;
}

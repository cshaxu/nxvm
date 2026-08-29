#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TEST_GDT_POINTER 0x0100u
#define TEST_GDT_ADDRESS 0x0300u
#define TEST_PROTECTED_CODE 0x0040u
#define TEST_PAGE_DIRECTORY 0x1000u
#define TEST_PAGE_TABLE 0x2000u
#define TEST_PAGE_TABLE_SECOND 0x3000u
#define TEST_PAGE_DIRECTORY_RELOAD 0x4000u
#define TEST_DATA_PHYSICAL 0x5000u
#define TEST_STACK_PHYSICAL 0x6000u
#define TEST_RELOAD_DATA_PHYSICAL 0xb000u
#define TEST_PERMISSION_CODE 0x7000u
#define TEST_CROSS_CODE_PHYSICAL 0x8000u
#define TEST_CROSS_DATA_FIRST 0x9000u
#define TEST_CROSS_DATA_SECOND 0xa000u
#define TEST_DATA_LINEAR 0x00403000u
#define TEST_CODE_SELECTOR 0x0008u
#define TEST_DATA_SELECTOR 0x0010u
#define TEST_USER_CODE_SELECTOR 0x001bu
#define TEST_USER_DATA_SELECTOR 0x0023u

#define TEST_PAGE_PRESENT 0x00000001u
#define TEST_PAGE_WRITABLE 0x00000002u
#define TEST_PAGE_US 0x00000004u
#define TEST_PAGE_ACCESSED 0x00000020u
#define TEST_PAGE_DIRTY 0x00000040u
#define TEST_80386_FORMER_WP_BIT 0x00010000u

typedef struct paging_machine {
    core_machine *machine;
} paging_machine;

typedef struct paging_trace_probe {
    core_machine_trace_event events[4096];
    type_unsigned_32 count;
} paging_trace_probe;

static C_VOID paging_trace(C_VOID *opaque, const core_machine_trace_event *event)
{
    paging_trace_probe *probe = (paging_trace_probe *)opaque;

    if (probe != STD_NULL && event != STD_NULL && probe->count < 4096u) {
        probe->events[probe->count++] = *event;
    }
}

static C_INT paging_has_provenance_pair(const paging_trace_probe *probe,
    core_machine_transaction_kind kind,
    core_machine_cpu_memory_access_provenance provenance)
{
    type_unsigned_32 index;

    if (probe == STD_NULL) return 0;
    for (index = 0u; index + 1u < probe->count; ++index) {
        if (probe->events[index].type == CORE_MACHINE_TRACE_TRANSACTION_BEGIN &&
            probe->events[index + 1u].type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
            (probe->events[index].detail & 0xffu) ==
                CORE_MACHINE_TRANSACTION_OWNER_CPU &&
            ((probe->events[index].detail >> 8u) & 0xffu) == kind &&
            (probe->events[index].detail >> 16u) == provenance) return 1;
    }
    return 0;
}

static C_VOID paging_reset(C_VOID *opaque)
{
    paging_machine *state = (paging_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider paging_provider = {
    paging_reset,
    STD_NULL
};

static C_INT paging_prepare(paging_machine *state,
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
            &paging_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT paging_write_u32(core_machine *machine, type_unsigned_32 address,
    type_unsigned_32 value)
{
    return core_machine_memory_write(machine, address, &value, sizeof(value)) ==
        TYPE_STATUS_OK;
}

static C_INT paging_read_u32(core_machine *machine, type_unsigned_32 address,
    type_unsigned_32 *out_value)
{
    return core_machine_memory_read(machine, address, out_value,
        sizeof(*out_value)) == TYPE_STATUS_OK;
}

static C_INT paging_install_gdt(core_machine *machine)
{
    static const type_unsigned_8 gdt_pointer[] = {
        0x27u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const type_unsigned_8 gdt[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0x9au, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0x92u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0xfau, 0x8fu, 0x00u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0xf2u, 0x8fu, 0x00u
    };

    return core_machine_memory_write(machine, TEST_GDT_POINTER, gdt_pointer,
        sizeof(gdt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TEST_GDT_ADDRESS, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK;
}

static C_INT paging_install_tables(core_machine *machine, type_unsigned_32 code_entry,
    type_unsigned_32 data_entry, type_unsigned_32 stack_entry)
{
    return paging_write_u32(machine, TEST_PAGE_DIRECTORY,
               TEST_PAGE_TABLE | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE) &&
        paging_write_u32(machine, TEST_PAGE_TABLE, code_entry) &&
        paging_write_u32(machine, TEST_PAGE_TABLE + 3u * 4u, data_entry) &&
        paging_write_u32(machine, TEST_PAGE_TABLE + 4u * 4u, stack_entry);
}

static C_INT paging_write_bootstrap(core_machine *machine,
    const type_unsigned_8 *protected_code, STD_SIZE_T protected_code_size)
{
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd8u,
        0x8eu, 0xd0u,
        0xeau, 0x40u, 0x00u, 0x08u, 0x00u
    };

    return paging_install_gdt(machine) &&
        core_machine_memory_write(machine, 0u, real_code, sizeof(real_code)) ==
            TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TEST_PROTECTED_CODE, protected_code,
            protected_code_size) == TYPE_STATUS_OK;
}

static C_INT paging_run(core_machine *machine, C_INT expect_fault,
    core_machine_run_result *out_result,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = { 128u, 0u };

    return core_machine_run(machine, budget, out_result) ==
            (expect_fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) &&
        out_result->reason == (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) &&
        core_machine_get_cpu_diagnostic(machine, out_diagnostic) == TYPE_STATUS_OK;
}

static C_INT paging_expect_fault(const core_machine_cpu_diagnostic *diagnostic,
    type_unsigned_32 exception, type_unsigned_32 code, type_unsigned_32 point_linear)
{
    return diagnostic->first_fault.valid &&
        TYPE_GET_BIT(diagnostic->first_fault.exception_mask, exception) &&
        diagnostic->first_fault.exception_code == code &&
        diagnostic->first_fault.point.cs == TEST_CODE_SELECTOR &&
        diagnostic->first_fault.point.linear_pc == point_linear;
}

static C_INT paging_test_delivered_page_fault(C_VOID)
{
    static const type_unsigned_8 protected_code[] = {
        0xbcu, 0x00u, 0x50u,
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0xf4u
    };
    static const type_unsigned_8 faulting_code[] = { 0xa1u, 0x00u, 0x90u };
    static const type_unsigned_8 hlt[] = { 0xf4u };
    const type_unsigned_8 gate[] = { 0x00u, 0x01u, 0x08u, 0x00u,
        0x00u, 0x8eu, 0x00u, 0x00u };
    const type_unsigned_32 code_entry = TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    const type_unsigned_32 data_entry = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 stack_entry = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_32 frame[4] = { 0u, 0u, 0u, 0u };
    C_INT failed = !paging_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !paging_install_tables(state.machine, code_entry, data_entry,
            stack_entry) || !paging_write_bootstrap(state.machine, protected_code,
            sizeof(protected_code));
        failed |= !paging_run(state.machine, 0, &result, &diagnostic);
    }
    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x0500u + 14u * 8u,
            gate, sizeof(gate)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x0080u, faulting_code, sizeof(faulting_code)) !=
            TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x0100u,
            hlt, sizeof(hlt)) != TYPE_STATUS_OK;
        state.machine->executor_cpu.data.idtr.base = 0x0500u;
        state.machine->executor_cpu.data.idtr.limit = 14u * 8u + 7u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0x0080u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
            &result) != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_PF) || diagnostic.last_delivered_exception.exception_code != 0u ||
            after.data.eip != 0x0100u || after.data.cr2 != 0x00009000u ||
            after.data.esp != 0x00004ff0u || !test_core_machine_fixture_read_linear(
                state.machine, after.data.ss.base + after.data.esp,
                TYPE_REFERENCE_OF(frame), sizeof(frame)) || frame[0] != 0u ||
            frame[1] != 0x0080u || frame[2] != TEST_CODE_SELECTOR ||
            frame[3] != before.data.eflags || after.data.eax != before.data.eax ||
            after.data.ebx != before.data.ebx || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi || after.data.edi != before.data.edi;
    }
    if (!failed) {
        failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
            &result) != TYPE_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            state.machine->executor_cpu.data.eip != 0x0101u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT paging_test_valid_path(C_VOID)
{
    static const type_unsigned_8 protected_code[] = {
        0xbcu, 0x00u, 0x50u,
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0x0fu, 0x20u, 0xc1u,
        0x0fu, 0x20u, 0xd2u,
        0xbbu, 0x00u, 0x30u,
        0xb8u, 0x34u, 0x12u,
        0x89u, 0x07u,
        0x50u,
        0x5eu,
        0xf4u
    };
    const type_unsigned_32 code_entry = TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    const type_unsigned_32 data_entry = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 stack_entry = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_16 data = 0u;
    type_unsigned_32 pde = 0u;
    type_unsigned_32 pte_code = 0u;
    type_unsigned_32 pte_data = 0u;
    type_unsigned_32 pte_stack = 0u;
    type_unsigned_32 pre_cr0 = 0u;
    type_unsigned_32 pre_cr2 = 0u;
    type_unsigned_32 pre_cr3 = 0u;
    type_unsigned_32 pre_ecx = 0u;
    type_unsigned_32 pre_edx = 0u;
    t_cpu cpu;
    paging_trace_probe trace = {{{0}}, 0u};
    const core_machine_trace_provider trace_provider = { paging_trace, &trace };
    C_INT ran;
    C_INT registers;
    C_INT data_ok;
    C_INT entries_read;
    C_INT entries_ok;
    C_INT provenance_ok;
    C_INT reset_ok;
    C_INT failed = !paging_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= core_machine_set_trace_provider(state.machine,
            &trace_provider) != TYPE_STATUS_OK;
        failed |= !paging_install_tables(state.machine, code_entry, data_entry,
            stack_entry);
        failed |= !paging_write_bootstrap(state.machine, protected_code,
            sizeof(protected_code));
        ran = paging_run(state.machine, 0, &result, &diagnostic);
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        pre_cr0 = cpu.data.cr0;
        pre_cr2 = cpu.data.cr2;
        pre_cr3 = cpu.data.cr3;
        pre_ecx = cpu.data.ecx;
        pre_edx = cpu.data.edx;
        registers = !diagnostic.first_fault.valid &&
            pre_cr0 == (VCPU_CR0_PE | VCPU_CR0_PG) && pre_cr2 == 0u &&
            pre_cr3 == TEST_PAGE_DIRECTORY && pre_ecx ==
                (VCPU_CR0_PE | VCPU_CR0_PG) && pre_edx == 0u;
        data_ok = core_machine_memory_read(state.machine, TEST_DATA_PHYSICAL,
            &data, sizeof(data)) == TYPE_STATUS_OK && data == 0x1234u;
        entries_read = paging_read_u32(state.machine, TEST_PAGE_DIRECTORY, &pde) &&
            paging_read_u32(state.machine, TEST_PAGE_TABLE, &pte_code) &&
            paging_read_u32(state.machine, TEST_PAGE_TABLE + 3u * 4u,
                &pte_data) && paging_read_u32(state.machine,
                TEST_PAGE_TABLE + 4u * 4u, &pte_stack);
        entries_ok = (pde & TEST_PAGE_ACCESSED) != 0u &&
            (pte_code & TEST_PAGE_ACCESSED) != 0u &&
            (pte_data & (TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY)) ==
                (TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY) &&
            (pte_stack & (TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY)) ==
                (TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY);
        provenance_ok = paging_has_provenance_pair(&trace,
                CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ,
                CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ) &&
            paging_has_provenance_pair(&trace,
                CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE,
                CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE);
        reset_ok = core_machine_reset(state.machine) == TYPE_STATUS_OK &&
            ((cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine)),
            cpu.data.cr0 == 0u && cpu.data.cr2 == 0u && cpu.data.cr3 == 0u);
        failed |= !ran || !registers || !data_ok || !entries_read ||
            !entries_ok || !provenance_ok || !reset_ok;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T258 valid result=%d/%d fault=%d/%08x cr0=%08x cr2=%08x cr3=%08x ecx=%08x edx=%08x data=%04x pde=%08x pte=%08x/%08x/%08x\n",
                result.executed, result.reason, diagnostic.first_fault.valid,
                diagnostic.first_fault.exception_mask, pre_cr0, pre_cr2,
                pre_cr3, pre_ecx, pre_edx, data, pde, pte_code, pte_data,
                pte_stack);
            STD_FPRINTF(STD_STDERR,
                "T258 valid checks ran=%d regs=%d data=%d reads=%d entries=%d provenance=%d reset=%d\n",
                ran, registers, data_ok, entries_read, entries_ok, provenance_ok,
                reset_ok);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT paging_test_fault(type_unsigned_32 code_entry, type_unsigned_32 data_entry,
    type_unsigned_32 stack_entry, const type_unsigned_8 *protected_code,
    STD_SIZE_T protected_code_size, type_unsigned_32 expected_code,
    type_unsigned_32 expected_point, type_unsigned_32 expected_cr2)
{
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    C_INT failed = !paging_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !paging_install_tables(state.machine, code_entry, data_entry,
            stack_entry);
        failed |= !paging_write_bootstrap(state.machine, protected_code,
            protected_code_size);
        failed |= !paging_run(state.machine, 1, &result, &diagnostic);
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !paging_expect_fault(&diagnostic, VCPUINS_EXCEPT_PF,
            expected_code, expected_point);
        failed |= cpu.data.cr2 != expected_cr2 ||
            diagnostic.first_fault.cr2 != expected_cr2;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T258 fault result=%d/%d diag=%d/%08x/%08x point=%04x:%08x cr2=%08x expected=%08x/%08x\n",
                result.executed, result.reason, diagnostic.first_fault.valid,
                diagnostic.first_fault.exception_mask,
                diagnostic.first_fault.exception_code,
                diagnostic.first_fault.point.cs,
                diagnostic.first_fault.point.linear_pc, cpu.data.cr2,
                expected_point, expected_cr2);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT paging_test_page_faults(C_VOID)
{
    static const type_unsigned_8 enable_only[] = {
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u
    };
    static const type_unsigned_8 data_read[] = {
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0xbbu, 0x00u, 0x30u,
        0x8bu, 0x07u
    };
    static const type_unsigned_8 stack_write[] = {
        0xbcu, 0x00u, 0x50u,
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0x50u
    };
    const type_unsigned_32 code = TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    const type_unsigned_32 data = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 stack = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    C_INT failed = 0;

    failed |= paging_test_fault(0u, data, stack, enable_only,
        sizeof(enable_only), 0u, TEST_PROTECTED_CODE + sizeof(enable_only),
        TEST_PROTECTED_CODE + sizeof(enable_only));
    failed |= paging_test_fault(code, 0u, stack, data_read, sizeof(data_read),
        0u, TEST_PROTECTED_CODE + 21u, 0x3000u);
    failed |= paging_test_fault(code, data, 0u, stack_write,
        sizeof(stack_write), 0x02u, TEST_PROTECTED_CODE + 21u, 0x4ffeu);
    return failed;
}

static C_INT paging_test_control_gate(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, STD_SIZE_T code_size)
{
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !paging_prepare(&state, profile);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, code, code_size) !=
            TYPE_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        failed |= !paging_run(state.machine, 1, &result, &diagnostic);
        failed |= !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD);
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT paging_test_control_forms(C_VOID)
{
    static const type_unsigned_8 write_reserved_cr1[] = {
        0x66u, 0xb8u, 0x34u, 0x12u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xc8u
    };
    static const type_unsigned_8 pg_without_pe[] = {
        0x66u, 0xb8u, 0x00u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u
    };
    static const type_unsigned_8 unaligned_cr3[] = {
        0x66u, 0xb8u, 0x34u, 0x12u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u
    };
    static const type_unsigned_8 read_cr0[] = { 0x0fu, 0x20u, 0xc0u };
    C_INT failed = 0;

    failed |= paging_test_control_gate(CORE_MACHINE_CPU_PROFILE_80386,
        write_reserved_cr1, sizeof(write_reserved_cr1));
    failed |= paging_test_control_gate(CORE_MACHINE_CPU_PROFILE_80386,
        pg_without_pe, sizeof(pg_without_pe));
    failed |= paging_test_control_gate(CORE_MACHINE_CPU_PROFILE_80386,
        unaligned_cr3, sizeof(unaligned_cr3));
    failed |= paging_test_control_gate(CORE_MACHINE_CPU_PROFILE_80286,
        read_cr0, sizeof(read_cr0));
    failed |= paging_test_control_gate(CORE_MACHINE_CPU_PROFILE_80186,
        read_cr0, sizeof(read_cr0));
    return failed;
}

static C_INT paging_test_invlpg_real_case(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, STD_SIZE_T code_size)
{
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT failed = !paging_prepare(&state, profile);

    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x11223344u;
        state.machine->executor_cpu.data.ecx = 0x55667788u;
        state.machine->executor_cpu.data.edx = 0x99aabbccu;
        state.machine->executor_cpu.data.ebx = 0xddeeff00u;
        state.machine->executor_cpu.data.esi = 0x13579bdfu;
        state.machine->executor_cpu.data.edi = 0x2468ace0u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine, 0u, code, code_size) !=
                TYPE_STATUS_OK || !paging_run(state.machine, 1, &result,
                &diagnostic);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            diagnostic.first_fault.exception_code != 0u ||
            diagnostic.first_fault.point.cs != 0u ||
            diagnostic.first_fault.point.linear_pc != 0u || STD_MEMCMP(&after,
                &before, sizeof(after)) != 0;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT paging_test_invlpg_rejection(C_VOID)
{
    static const type_unsigned_8 invlpg[] = {
        0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u
    };
    static const type_unsigned_8 operand_size[] = {
        0x66u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u
    };
    static const type_unsigned_8 address_size[] = {
        0x67u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u
    };
    static const type_unsigned_8 combined_size[] = {
        0x66u, 0x67u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u
    };
    static const type_unsigned_8 locked[] = {
        0xf0u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u
    };
    if (paging_test_invlpg_real_case(CORE_MACHINE_CPU_PROFILE_80186, invlpg,
            sizeof(invlpg))) return 1;
    if (paging_test_invlpg_real_case(CORE_MACHINE_CPU_PROFILE_80286, invlpg,
            sizeof(invlpg))) return 2;
    if (paging_test_invlpg_real_case(CORE_MACHINE_CPU_PROFILE_80386,
            invlpg, sizeof(invlpg))) return 3;
    if (paging_test_invlpg_real_case(CORE_MACHINE_CPU_PROFILE_80386,
            operand_size, sizeof(operand_size))) return 4;
    if (paging_test_invlpg_real_case(CORE_MACHINE_CPU_PROFILE_80386,
            address_size, sizeof(address_size))) return 5;
    if (paging_test_invlpg_real_case(CORE_MACHINE_CPU_PROFILE_80386,
            combined_size, sizeof(combined_size))) return 6;
    if (paging_test_invlpg_real_case(CORE_MACHINE_CPU_PROFILE_80386,
            locked, sizeof(locked))) return 7;
    return 0;
}

static C_INT paging_test_cr0_mutable_controls(C_VOID)
{
    static const type_unsigned_8 write_mutable[] = {
        0x66u, 0xb8u, 0x1eu, 0x00u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xc0u,
        0x0fu, 0x20u, 0xc1u,
        0xf4u
    };
    const type_unsigned_32 mutable = VCPU_CR0_MP | VCPU_CR0_EM | VCPU_CR0_TS |
        VCPU_CR0_ET;
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu cpu;
    C_INT failed = !paging_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine, 0u, write_mutable,
            sizeof(write_mutable)) != TYPE_STATUS_OK || !paging_run(state.machine,
            0, &result, &diagnostic);
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid || cpu.data.cr0 != mutable ||
            cpu.data.eax != mutable || cpu.data.ecx != mutable ||
            cpu.data.eip != sizeof(write_mutable) || cpu.data.eflags != 0x02u ||
            cpu.data.ebx != 0u || cpu.data.edx != 0u || cpu.data.esp != 0u ||
            cpu.data.ebp != 0u || cpu.data.esi != 0u || cpu.data.edi != 0u ||
            STD_MEMCMP(&cpu.data.es, &before.data.es, sizeof(cpu.data.es)) != 0 ||
            STD_MEMCMP(&cpu.data.cs, &before.data.cs, sizeof(cpu.data.cs)) != 0 ||
            STD_MEMCMP(&cpu.data.ss, &before.data.ss, sizeof(cpu.data.ss)) != 0 ||
            STD_MEMCMP(&cpu.data.ds, &before.data.ds, sizeof(cpu.data.ds)) != 0 ||
            STD_MEMCMP(&cpu.data.fs, &before.data.fs, sizeof(cpu.data.fs)) != 0 ||
            STD_MEMCMP(&cpu.data.gs, &before.data.gs, sizeof(cpu.data.gs)) != 0;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT paging_test_cr3_directory_reload(C_VOID)
{
    static const type_unsigned_8 protected_code[] = {
        0xbcu, 0x00u, 0x50u,
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0xbbu, 0x00u, 0x30u,
        0x66u, 0x67u, 0x8bu, 0x03u,
        0x66u, 0xb9u, 0x00u, 0x40u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd9u,
        0x66u, 0x67u, 0x8bu, 0x13u,
        0xf4u
    };
    const type_unsigned_32 present_writable = TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 first_value = 0x11223344u;
    const type_unsigned_32 second_value = 0x55667788u;
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    C_INT failed = !paging_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= !paging_write_u32(state.machine, TEST_PAGE_DIRECTORY,
                TEST_PAGE_TABLE | present_writable) ||
            !paging_write_u32(state.machine, TEST_PAGE_TABLE,
                present_writable) ||
            !paging_write_u32(state.machine, TEST_PAGE_TABLE + 3u * 4u,
                TEST_DATA_PHYSICAL | present_writable) ||
            !paging_write_u32(state.machine, TEST_PAGE_DIRECTORY_RELOAD,
                TEST_PAGE_TABLE_SECOND | present_writable) ||
            !paging_write_u32(state.machine, TEST_PAGE_TABLE_SECOND,
                present_writable) ||
            !paging_write_u32(state.machine, TEST_PAGE_TABLE_SECOND + 3u * 4u,
                TEST_RELOAD_DATA_PHYSICAL | present_writable) ||
            core_machine_memory_write(state.machine, TEST_DATA_PHYSICAL,
                &first_value, sizeof(first_value)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, TEST_RELOAD_DATA_PHYSICAL,
                &second_value, sizeof(second_value)) != TYPE_STATUS_OK ||
            !paging_write_bootstrap(state.machine, protected_code,
                sizeof(protected_code));
    }
    if (!failed) {
        failed |= !paging_run(state.machine, 0, &result, &diagnostic);
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid || cpu.data.cr0 !=
            (VCPU_CR0_PE | VCPU_CR0_PG) || cpu.data.cr3 !=
            TEST_PAGE_DIRECTORY_RELOAD || cpu.data.eax != first_value ||
            cpu.data.ecx != TEST_PAGE_DIRECTORY_RELOAD || cpu.data.edx !=
            second_value || cpu.data.ebx != 0x00003000u || cpu.data.eip !=
            TEST_PROTECTED_CODE + sizeof(protected_code) ||
            cpu.data.eflags != 0x00000002u ||
            cpu.data.esp != 0x00005000u || cpu.data.ebp != 0u ||
            cpu.data.esi != 0u || cpu.data.edi != 0u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT paging_permission_read(core_machine *machine,
    type_unsigned_32 physical, C_VOID *out_data, STD_SIZE_T bytes);
static C_INT paging_permission_prepare(paging_machine *state,
    const type_unsigned_8 *program, STD_SIZE_T program_size,
    type_unsigned_32 pde_code, type_unsigned_32 pde_data,
    type_unsigned_32 pte_code, type_unsigned_32 pte_data,
    type_unsigned_32 pte_stack, C_INT user, C_INT set_reserved_cr0_bit,
    type_unsigned_32 *out_program_eip);

static C_INT paging_test_no_stale_translation(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0x66u, 0x67u, 0x8bu, 0x03u,
        0x66u, 0x67u, 0x8bu, 0x13u
    };
    const type_unsigned_32 entry = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 replacement = TEST_RELOAD_DATA_PHYSICAL |
        TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    const type_unsigned_32 first_value = 0x10203040u;
    const type_unsigned_32 second_value = 0x50607080u;
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    type_unsigned_32 pte = 0u;
    C_INT failed = !paging_permission_prepare(&state, program, sizeof(program),
        TEST_PAGE_TABLE | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
        TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
        TEST_PERMISSION_CODE | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
        entry, TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
        0, 0, STD_NULL);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, TEST_DATA_PHYSICAL,
                &first_value, sizeof(first_value)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, TEST_RELOAD_DATA_PHYSICAL,
                &second_value, sizeof(second_value)) != TYPE_STATUS_OK ||
            core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_BUDGET || core_machine_get_cpu_diagnostic(
                state.machine, &diagnostic) != TYPE_STATUS_OK;
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid || cpu.data.eax != first_value ||
            cpu.data.eip != 4u || cpu.data.edx != 0u || cpu.data.cr3 !=
            TEST_PAGE_DIRECTORY || cpu.data.eflags != 0x00000002u;
    }
    if (!failed) {
        failed |= !paging_write_u32(state.machine, TEST_PAGE_TABLE_SECOND +
                3u * 4u, replacement) || core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
        cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid || cpu.data.eax != first_value ||
            cpu.data.edx != second_value || cpu.data.eip != 8u ||
            cpu.data.cr3 != TEST_PAGE_DIRECTORY || cpu.data.eflags !=
                0x00000002u || !paging_permission_read(state.machine,
                TEST_PAGE_TABLE_SECOND + 3u * 4u, &pte, sizeof(pte)) ||
            pte != (replacement | TEST_PAGE_ACCESSED);
    }
    core_machine_destroy(state.machine);
    return failed;
}

typedef enum paging_permission_access {
    PAGING_PERMISSION_FETCH,
    PAGING_PERMISSION_READ,
    PAGING_PERMISSION_WRITE,
    PAGING_PERMISSION_STACK
} paging_permission_access;

static C_INT paging_permission_install(core_machine *machine,
    type_unsigned_32 pde_code, type_unsigned_32 pde_data, type_unsigned_32 pte_code,
    type_unsigned_32 pte_data, type_unsigned_32 pte_stack)
{
    return core_machine_memory_write_physical(&machine->executor_memory,
               TEST_PAGE_DIRECTORY, TYPE_REFERENCE_OF(pde_code),
               sizeof(pde_code)) == TYPE_STATUS_OK &&
        core_machine_memory_write_physical(&machine->executor_memory,
            TEST_PAGE_DIRECTORY + 4u, TYPE_REFERENCE_OF(pde_data),
            sizeof(pde_data)) == TYPE_STATUS_OK &&
        core_machine_memory_write_physical(&machine->executor_memory,
            TEST_PAGE_TABLE + 7u * 4u, TYPE_REFERENCE_OF(pte_code), sizeof(pte_code)) ==
                TYPE_STATUS_OK &&
        core_machine_memory_write_physical(&machine->executor_memory,
            TEST_PAGE_TABLE + 4u * 4u, TYPE_REFERENCE_OF(pte_stack),
            sizeof(pte_stack)) == TYPE_STATUS_OK &&
        core_machine_memory_write_physical(&machine->executor_memory,
            TEST_PAGE_TABLE_SECOND + 3u * 4u, TYPE_REFERENCE_OF(pte_data),
            sizeof(pte_data)) == TYPE_STATUS_OK;
}

static C_INT paging_permission_read(core_machine *machine, type_unsigned_32 physical,
    C_VOID *out_data, STD_SIZE_T bytes)
{
    return machine != STD_NULL && core_machine_memory_read_physical(
        &machine->executor_memory, physical, (type_virtual_address)out_data,
        bytes) == TYPE_STATUS_OK;
}

static C_INT paging_permission_prepare(paging_machine *state,
    const type_unsigned_8 *program, STD_SIZE_T program_size, type_unsigned_32 pde_code,
    type_unsigned_32 pde_data, type_unsigned_32 pte_code, type_unsigned_32 pte_data,
    type_unsigned_32 pte_stack, C_INT user, C_INT set_reserved_cr0_bit,
    type_unsigned_32 *out_program_eip)
{
    static const type_unsigned_8 enable_paging[] = {
        0xbcu, 0x00u, 0x50u,
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0xf4u
    };
    type_unsigned_8 protected_code[64u] = {0};
    type_unsigned_16 data = 0x1234u;
    type_unsigned_16 stack = 0xaaaau;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 initial_pde = TEST_PAGE_TABLE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    type_unsigned_32 initial_pte = TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    C_INT failed = program_size > sizeof(protected_code) -
        sizeof(enable_paging) || !paging_prepare(state,
        CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        STD_MEMCPY(protected_code, enable_paging, sizeof(enable_paging));
        STD_MEMCPY(protected_code + sizeof(enable_paging), program, program_size);
        failed |= !paging_permission_install(state->machine, initial_pde,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
            initial_pte, TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
            TEST_PAGE_WRITABLE, TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
            TEST_PAGE_WRITABLE);
        failed |= !paging_write_u32(state->machine, TEST_PAGE_TABLE, initial_pte);
        failed |= !paging_write_bootstrap(state->machine, protected_code,
            sizeof(enable_paging) + program_size);
        failed |= core_machine_memory_write(state->machine, TEST_PERMISSION_CODE,
            program, program_size) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state->machine, TEST_DATA_PHYSICAL,
            &data, sizeof(data)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state->machine, TEST_STACK_PHYSICAL +
                0xffeu, &stack, sizeof(stack)) != TYPE_STATUS_OK;
        failed |= !paging_run(state->machine, 0, &result, &diagnostic);
        if (user) {
            state->machine->executor_cpu.data.cs.selector =
                TEST_USER_CODE_SELECTOR;
            state->machine->executor_cpu.data.cs.dpl = 3u;
            failed |= core_machine_cpu_execution_load_segment(
                &state->machine->executor_cpu_execution,
                &state->machine->executor_cpu.data.ds,
                TEST_USER_DATA_SELECTOR) != 0 ||
                core_machine_cpu_execution_load_segment(
                &state->machine->executor_cpu_execution,
                &state->machine->executor_cpu.data.ss,
                TEST_USER_DATA_SELECTOR) != 0;
        } else {
            state->machine->executor_cpu.data.ds.limit = 0xffffffffu;
            state->machine->executor_cpu.data.ss.limit = 0xffffffffu;
        }
        failed |= !paging_permission_install(state->machine, pde_code, pde_data,
            pte_code, pte_data, pte_stack);
        state->machine->executor_cpu.data.cs.base = TEST_PERMISSION_CODE;
        state->machine->executor_cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG |
            (set_reserved_cr0_bit ? TEST_80386_FORMER_WP_BIT : 0u);
        state->machine->executor_cpu.data.eax = 0xfacebeefu;
        state->machine->executor_cpu.data.ebx = TEST_DATA_LINEAR;
        state->machine->executor_cpu.data.esp = 0x00005000u;
        test_core_machine_fixture_resume_after_halt_at(state->machine,
            0u);
        if (out_program_eip != STD_NULL) *out_program_eip =
            0u;
    }
    return !failed;
}

static C_INT paging_permission_expect_fault(paging_machine *state,
    type_unsigned_32 program_eip, type_unsigned_32 expected_code, type_unsigned_32 expected_cr2,
    type_unsigned_32 pde_address, type_unsigned_32 pde_initial, type_unsigned_32 pte_address,
    type_unsigned_32 pte_initial, paging_permission_access access)
{
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    type_unsigned_32 pde = 0u;
    type_unsigned_32 pte = 0u;
    type_unsigned_16 data = 0u;
    const core_machine_run_budget budget = { 32u, 0u };
    C_INT failed = core_machine_run(state->machine, budget, &result) !=
        TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) !=
            TYPE_STATUS_OK;

    cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
        diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_PF) ||
        diagnostic.first_fault.exception_code != expected_code ||
        diagnostic.first_fault.point.linear_pc != TEST_PERMISSION_CODE + program_eip ||
        cpu.data.cr2 != expected_cr2 ||
        diagnostic.first_fault.cr2 != expected_cr2 || cpu.data.eip != program_eip ||
        cpu.data.ebx != TEST_DATA_LINEAR || cpu.data.esp != 0x00005000u ||
        cpu.data.eflags != 0x00000002u;
    if (access == PAGING_PERMISSION_READ) failed |= cpu.data.eax != 0xfacebeefu;
    failed |= !paging_permission_read(state->machine, pde_address, &pde,
        sizeof(pde)) || !paging_permission_read(state->machine, pte_address,
            &pte, sizeof(pte)) ||
        pde != pde_initial || pte != pte_initial;
    if (access == PAGING_PERMISSION_WRITE) {
        failed |= !paging_permission_read(state->machine, TEST_DATA_PHYSICAL,
            &data, sizeof(data)) || data != 0x1234u;
    } else if (access == PAGING_PERMISSION_STACK) {
        failed |= !paging_permission_read(state->machine, TEST_STACK_PHYSICAL +
            0xffeu, &data, sizeof(data)) || data != 0xaaaau;
    }
    if (failed) {
        STD_FPRINTF(STD_STDERR,
            "T311 fault access=%u result=%u/%u diag=%x/%x eip=%x cr2=%x eax=%x ebx=%x esp=%x flags=%x pde=%x/%x pte=%x/%x\n",
            (unsigned)access, (unsigned)result.executed, (unsigned)result.reason,
            diagnostic.first_fault.exception_mask,
            diagnostic.first_fault.exception_code, cpu.data.eip, cpu.data.cr2,
            cpu.data.eax, cpu.data.ebx, cpu.data.esp, cpu.data.eflags, pde,
            pde_initial, pte, pte_initial);
    }
    return !failed;
}

static C_INT paging_permission_expect_success(paging_machine *state,
    paging_permission_access access, type_unsigned_32 pde_address, type_unsigned_32 pte_address)
{
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    type_unsigned_32 pde = 0u;
    type_unsigned_32 pte = 0u;
    type_unsigned_16 data = 0u;
    const core_machine_run_budget budget = { 1u, 0u };
    C_INT failed = core_machine_run(state->machine, budget, &result) !=
        TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) !=
            TYPE_STATUS_OK;

    cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    failed |= diagnostic.first_fault.valid || !paging_permission_read(
        state->machine, pde_address, &pde, sizeof(pde)) ||
        !paging_permission_read(state->machine, pte_address, &pte,
            sizeof(pte)) || (pde & TEST_PAGE_ACCESSED) == 0u ||
        (pte & TEST_PAGE_ACCESSED) == 0u;
    if (access == PAGING_PERMISSION_READ) {
        failed |= cpu.data.eax != 0xface1234u ||
            (pte & TEST_PAGE_DIRTY) != 0u;
    } else if (access == PAGING_PERMISSION_WRITE) {
        failed |= !paging_permission_read(state->machine, TEST_DATA_PHYSICAL,
            &data, sizeof(data)) || data != 0xbeefu ||
            (pte & TEST_PAGE_DIRTY) == 0u;
    } else if (access == PAGING_PERMISSION_STACK) {
        failed |= cpu.data.esp != 0x00004ffeu ||
            !paging_permission_read(state->machine, TEST_STACK_PHYSICAL +
                0xffeu, &data, sizeof(data)) ||
            data != 0xbeefu || (pte & TEST_PAGE_DIRTY) == 0u;
    }
    if (failed) {
        STD_FPRINTF(STD_STDERR,
            "T311 success access=%u result=%u/%u fault=%d eax=%x esp=%x pde=%x pte=%x\n",
            (unsigned)access, (unsigned)result.executed, (unsigned)result.reason,
            diagnostic.first_fault.valid, cpu.data.eax, cpu.data.esp, pde, pte);
    }
    return !failed;
}

static C_INT paging_test_permissions(C_VOID)
{
    static const type_unsigned_8 fetch[] = { 0x90u };
    static const type_unsigned_8 read[] = { 0x67u, 0x8bu, 0x03u };
    static const type_unsigned_8 write[] = { 0x67u, 0x89u, 0x03u };
    static const type_unsigned_8 stack[] = { 0x50u };
    const type_unsigned_32 code_user = TEST_PERMISSION_CODE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE | TEST_PAGE_US;
    const type_unsigned_32 data_user = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE | TEST_PAGE_US;
    const type_unsigned_32 stack_user = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE | TEST_PAGE_US;
    const type_unsigned_32 pde_code = TEST_PAGE_TABLE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE | TEST_PAGE_US;
    const type_unsigned_32 pde_data = TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE | TEST_PAGE_US;
    paging_machine state;
    type_unsigned_32 eip = 0u;
    C_INT failed = 0;

    if (!paging_permission_prepare(&state, fetch, sizeof(fetch), pde_code,
            pde_data, TEST_PERMISSION_CODE | TEST_PAGE_PRESENT |
            TEST_PAGE_WRITABLE, data_user,
            stack_user, 1, 0, &eip) || !paging_permission_expect_fault(&state,
            eip, 0x05u, TEST_PERMISSION_CODE, TEST_PAGE_DIRECTORY,
            pde_code | TEST_PAGE_ACCESSED,
            TEST_PAGE_TABLE + 7u * 4u, TEST_PERMISSION_CODE |
            TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, PAGING_PERMISSION_FETCH))
        failed = 1;
    core_machine_destroy(state.machine);

    if (!paging_permission_prepare(&state, read, sizeof(read), pde_code,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
            code_user, data_user, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_fault(&state, eip, 0x05u, TEST_DATA_LINEAR,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT |
            TEST_PAGE_WRITABLE, TEST_PAGE_TABLE_SECOND + 3u * 4u, data_user,
            PAGING_PERMISSION_READ)) failed = 1;
    core_machine_destroy(state.machine);

    if (!paging_permission_prepare(&state, read, sizeof(read), pde_code,
            pde_data, code_user, TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
            TEST_PAGE_WRITABLE, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_fault(&state, eip, 0x05u, TEST_DATA_LINEAR,
            TEST_PAGE_DIRECTORY + 4u, pde_data, TEST_PAGE_TABLE_SECOND + 3u * 4u,
            TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
            PAGING_PERMISSION_READ)) failed = 1;
    core_machine_destroy(state.machine);

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_US, code_user,
            data_user, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_fault(&state, eip, 0x07u, TEST_DATA_LINEAR,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT |
            TEST_PAGE_US, TEST_PAGE_TABLE_SECOND + 3u * 4u, data_user,
            PAGING_PERMISSION_WRITE)) failed = 1;
    core_machine_destroy(state.machine);

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            pde_data, code_user, TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
            TEST_PAGE_US, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_fault(&state, eip, 0x07u, TEST_DATA_LINEAR,
            TEST_PAGE_DIRECTORY + 4u, pde_data, TEST_PAGE_TABLE_SECOND + 3u * 4u,
            TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT | TEST_PAGE_US,
            PAGING_PERMISSION_WRITE)) failed = 1;
    core_machine_destroy(state.machine);

    if (!paging_permission_prepare(&state, stack, sizeof(stack), pde_code,
            pde_data, code_user, data_user, TEST_STACK_PHYSICAL |
            TEST_PAGE_PRESENT | TEST_PAGE_US, 1, 0, &eip) ||
        !paging_permission_expect_fault(&state, eip, 0x07u, 0x00004ffeu,
            TEST_PAGE_DIRECTORY, pde_code | TEST_PAGE_ACCESSED,
            TEST_PAGE_TABLE + 4u * 4u, TEST_STACK_PHYSICAL |
            TEST_PAGE_PRESENT | TEST_PAGE_US, PAGING_PERMISSION_STACK)) failed = 1;
    core_machine_destroy(state.machine);

    if (!paging_permission_prepare(&state, read, sizeof(read), pde_code,
            pde_data, code_user, data_user, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_success(&state, PAGING_PERMISSION_READ,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u)) failed = 1;
    core_machine_destroy(state.machine);

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            pde_data, code_user, data_user, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_success(&state, PAGING_PERMISSION_WRITE,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u)) failed = 1;
    core_machine_destroy(state.machine);

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_US, code_user,
            data_user, stack_user, 0, 0, &eip) ||
        !paging_permission_expect_success(&state, PAGING_PERMISSION_WRITE,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u)) failed = 1;
    core_machine_destroy(state.machine);

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_US, code_user,
            data_user, stack_user, 0, 1, &eip) ||
        !paging_permission_expect_success(&state, PAGING_PERMISSION_WRITE,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u))
        failed = 1;
    core_machine_destroy(state.machine);

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            pde_data, code_user, TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
            TEST_PAGE_US, stack_user, 0, 1, &eip) ||
        !paging_permission_expect_success(&state, PAGING_PERMISSION_WRITE,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u))
        failed = 1;
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT paging_cross_prepare(paging_machine *state, const type_unsigned_8 *program,
    STD_SIZE_T program_size, type_unsigned_32 second_entry,
    C_INT set_reserved_cr0_bit)
{
    const type_unsigned_32 code_entry = TEST_PERMISSION_CODE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 data_entry = TEST_CROSS_DATA_FIRST | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 stack_entry = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 pde_code = TEST_PAGE_TABLE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 pde_data = TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;

    return paging_permission_prepare(state, program, program_size, pde_code,
        pde_data, code_entry, data_entry, stack_entry, 0, set_reserved_cr0_bit,
        STD_NULL) && paging_write_u32(state->machine,
        TEST_PAGE_TABLE_SECOND + 4u * 4u, second_entry);
}

static C_INT paging_cross_entries(core_machine *machine, type_unsigned_32 pde_address,
    type_unsigned_32 first_address, type_unsigned_32 second_address, type_unsigned_32 expected_pde,
    type_unsigned_32 expected_first, type_unsigned_32 expected_second)
{
    type_unsigned_32 pde = 0u;
    type_unsigned_32 first = 0u;
    type_unsigned_32 second = 0u;

    return paging_permission_read(machine, pde_address, &pde, sizeof(pde)) &&
        paging_permission_read(machine, first_address, &first, sizeof(first)) &&
        paging_permission_read(machine, second_address, &second, sizeof(second)) &&
        pde == expected_pde && first == expected_first && second == expected_second;
}

static C_INT paging_cross_run(paging_machine *state, C_INT expect_fault,
    core_machine_run_result *out_result,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = { 1u, 0u };

    return core_machine_run(state->machine, budget, out_result) ==
            (expect_fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) &&
        out_result->reason == (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_BUDGET) && core_machine_get_cpu_diagnostic(
                state->machine, out_diagnostic) == TYPE_STATUS_OK;
}

static C_INT paging_test_cross_data(C_VOID)
{
    static const type_unsigned_8 read[] = { 0x67u, 0x8bu, 0x03u };
    static const type_unsigned_8 write[] = { 0x66u, 0x67u, 0x89u, 0x03u };
    const type_unsigned_32 pde = TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 first = TEST_CROSS_DATA_FIRST | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 second = TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    type_unsigned_16 word = 0u;
    type_unsigned_8 byte = 0u;
    C_INT failed = 0;

    if (!paging_cross_prepare(&state, read, sizeof(read), second, 0)) return 1;
    word = 0x3412u;
    failed |= core_machine_memory_write(state.machine, TEST_CROSS_DATA_FIRST +
        0xfffu, &word, sizeof(word)) != TYPE_STATUS_OK;
    state.machine->executor_cpu.data.ebx = TEST_DATA_LINEAR + 0xfffu;
    failed |= !paging_cross_run(&state, 0, &result, &diagnostic);
    cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
    failed |= diagnostic.first_fault.valid || cpu.data.eax != 0xface3412u ||
        !paging_cross_entries(state.machine, TEST_PAGE_DIRECTORY + 4u,
            TEST_PAGE_TABLE_SECOND + 3u * 4u, TEST_PAGE_TABLE_SECOND + 4u * 4u,
            pde | TEST_PAGE_ACCESSED, first | TEST_PAGE_ACCESSED,
            second | TEST_PAGE_ACCESSED);
    core_machine_destroy(state.machine);

    if (!paging_cross_prepare(&state, write, sizeof(write), second, 0)) return 1;
    state.machine->executor_cpu.data.ebx = TEST_DATA_LINEAR + 0xfffu;
    failed |= !paging_cross_run(&state, 0, &result, &diagnostic);
    failed |= diagnostic.first_fault.valid || !paging_cross_entries(state.machine,
        TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u,
        TEST_PAGE_TABLE_SECOND + 4u * 4u, pde | TEST_PAGE_ACCESSED,
        first | TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY, second |
        TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY) || !paging_permission_read(
        state.machine, TEST_CROSS_DATA_FIRST + 0xfffu, &byte, sizeof(byte)) ||
        byte != 0xefu || !paging_permission_read(state.machine,
        TEST_CROSS_DATA_SECOND, &word, sizeof(word)) || word != 0xcebeu;
    core_machine_destroy(state.machine);

    return failed;
}

static C_INT paging_test_cross_stack(C_VOID)
{
    static const type_unsigned_8 push[] = { 0x50u };
    static const type_unsigned_8 pop[] = { 0x58u };
    const type_unsigned_32 pde = TEST_PAGE_TABLE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 first = TEST_CROSS_DATA_FIRST | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 second = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    type_unsigned_8 low = 0u;
    type_unsigned_8 high = 0u;
    C_INT failed;

    if (!paging_cross_prepare(&state, push, sizeof(push),
            TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, 0))
        return 1;
    failed = !paging_write_u32(state.machine, TEST_PAGE_TABLE + 3u * 4u, first) ||
        !paging_write_u32(state.machine, TEST_PAGE_TABLE + 4u * 4u, second);
    state.machine->executor_cpu.data.esp = 0x00004001u;
    failed |= !paging_cross_run(&state, 0, &result, &diagnostic);
    cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
    failed |= diagnostic.first_fault.valid || cpu.data.esp != 0x00003fffu ||
        !paging_cross_entries(state.machine, TEST_PAGE_DIRECTORY,
            TEST_PAGE_TABLE + 3u * 4u, TEST_PAGE_TABLE + 4u * 4u,
            pde | TEST_PAGE_ACCESSED, first | TEST_PAGE_ACCESSED |
            TEST_PAGE_DIRTY, second | TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY) ||
        !paging_permission_read(state.machine, TEST_CROSS_DATA_FIRST + 0xfffu,
            &low, sizeof(low)) || !paging_permission_read(state.machine,
            TEST_STACK_PHYSICAL, &high, sizeof(high)) || low != 0xefu ||
        high != 0xbeu;
    core_machine_destroy(state.machine);

    if (!paging_cross_prepare(&state, pop, sizeof(pop),
            TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, 0))
        return 1;
    failed |= !paging_write_u32(state.machine, TEST_PAGE_TABLE + 3u * 4u, first) ||
        !paging_write_u32(state.machine, TEST_PAGE_TABLE + 4u * 4u, second) ||
        core_machine_memory_write(state.machine, TEST_CROSS_DATA_FIRST + 0xfffu,
            &(type_unsigned_8){ 0x12u }, 1u) != TYPE_STATUS_OK ||
        core_machine_memory_write(state.machine, TEST_STACK_PHYSICAL,
            &(type_unsigned_8){ 0x34u }, 1u) != TYPE_STATUS_OK;
    state.machine->executor_cpu.data.esp = 0x00003fffu;
    failed |= !paging_cross_run(&state, 0, &result, &diagnostic);
    cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
    failed |= diagnostic.first_fault.valid || cpu.data.esp != 0x00004001u ||
        cpu.data.eax != 0xface3412u || !paging_cross_entries(state.machine,
        TEST_PAGE_DIRECTORY, TEST_PAGE_TABLE + 3u * 4u,
        TEST_PAGE_TABLE + 4u * 4u, pde | TEST_PAGE_ACCESSED,
        first | TEST_PAGE_ACCESSED, second | TEST_PAGE_ACCESSED);
    core_machine_destroy(state.machine);

    if (!paging_cross_prepare(&state, push, sizeof(push),
            TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, 0))
        return 1;
    failed |= !paging_write_u32(state.machine, TEST_PAGE_TABLE + 3u * 4u, first) ||
        !paging_write_u32(state.machine, TEST_PAGE_TABLE + 4u * 4u,
            TEST_STACK_PHYSICAL | TEST_PAGE_WRITABLE);
    state.machine->executor_cpu.data.esp = 0x00004001u;
    failed |= !paging_cross_run(&state, 1, &result, &diagnostic);
    cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
    failed |= !paging_expect_fault(&diagnostic, VCPUINS_EXCEPT_PF, 0x02u,
        TEST_PERMISSION_CODE) || cpu.data.cr2 != 0x00004000u ||
        cpu.data.esp != 0x00004001u || cpu.data.eax != 0xfacebeefu ||
        cpu.data.eflags != 0x02u || !paging_cross_entries(state.machine,
        TEST_PAGE_DIRECTORY, TEST_PAGE_TABLE + 3u * 4u,
        TEST_PAGE_TABLE + 4u * 4u, pde | TEST_PAGE_ACCESSED, first,
        TEST_STACK_PHYSICAL | TEST_PAGE_WRITABLE);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT paging_test_cross_fetch(C_VOID)
{
    static const type_unsigned_8 first[] = { 0x66u };
    static const type_unsigned_8 second[] = { 0xb8u, 0x34u, 0x12u, 0x00u, 0x00u };
    const type_unsigned_32 pde = TEST_PAGE_TABLE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 code_first = TEST_PERMISSION_CODE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const type_unsigned_32 code_second = TEST_CROSS_CODE_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu cpu;
    C_INT failed;

    if (!paging_cross_prepare(&state, first, sizeof(first),
            TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, 0))
        return 1;
    failed = !paging_write_u32(state.machine, TEST_PAGE_TABLE + 8u * 4u,
        code_second) || core_machine_memory_write(state.machine,
        TEST_PERMISSION_CODE + 0xfffu, first, sizeof(first)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state.machine, TEST_CROSS_CODE_PHYSICAL,
            second, sizeof(second)) != TYPE_STATUS_OK;
    state.machine->executor_cpu.data.eip = 0x0fffu;
    failed |= !paging_cross_run(&state, 0, &result, &diagnostic);
    failed |= diagnostic.first_fault.valid || !paging_cross_entries(state.machine,
        TEST_PAGE_DIRECTORY, TEST_PAGE_TABLE + 7u * 4u,
        TEST_PAGE_TABLE + 8u * 4u, pde | TEST_PAGE_ACCESSED,
        code_first | TEST_PAGE_ACCESSED, code_second | TEST_PAGE_ACCESSED);
    core_machine_destroy(state.machine);

    if (!paging_cross_prepare(&state, first, sizeof(first),
            TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, 0))
        return 1;
    failed |= !paging_write_u32(state.machine, TEST_PAGE_TABLE + 8u * 4u,
        TEST_CROSS_CODE_PHYSICAL | TEST_PAGE_WRITABLE) ||
        core_machine_memory_write(state.machine, TEST_PERMISSION_CODE + 0xfffu,
            first, sizeof(first)) != TYPE_STATUS_OK;
    state.machine->executor_cpu.data.eip = 0x0fffu;
    failed |= !paging_cross_run(&state, 1, &result, &diagnostic);
    cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine);
    failed |= !paging_expect_fault(&diagnostic, VCPUINS_EXCEPT_PF, 0u,
        TEST_PERMISSION_CODE + 0x0fffu) || cpu.data.cr2 !=
        TEST_PERMISSION_CODE + 0x1000u || cpu.data.eip != 0x0fffu ||
        !paging_cross_entries(state.machine, TEST_PAGE_DIRECTORY,
            TEST_PAGE_TABLE + 7u * 4u, TEST_PAGE_TABLE + 8u * 4u,
            pde | TEST_PAGE_ACCESSED,
            code_first, TEST_CROSS_CODE_PHYSICAL | TEST_PAGE_WRITABLE);
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT paging_test_cross_page(C_VOID)
{
    return paging_test_cross_data() || paging_test_cross_stack() ||
        paging_test_cross_fetch();
}

C_INT main(C_VOID)
{
    const C_INT valid = paging_test_valid_path();
    const C_INT delivered = paging_test_delivered_page_fault();
    const C_INT faults = paging_test_page_faults();
    const C_INT controls = paging_test_control_forms();
    const C_INT invlpg = paging_test_invlpg_rejection();
    const C_INT cr0_controls = paging_test_cr0_mutable_controls();
    const C_INT cr3_reload = paging_test_cr3_directory_reload();
    const C_INT no_stale_translation = paging_test_no_stale_translation();
    const C_INT permissions = paging_test_permissions();
    const C_INT cross_page = paging_test_cross_page();

    if (valid || delivered || faults || controls || invlpg || cr0_controls ||
        cr3_reload || no_stale_translation || permissions || cross_page) {
        STD_FPRINTF(STD_STDERR,
            "M5:T258:S2:I386-PAGING:FAIL valid=%d delivered=%d faults=%d controls=%d invlpg=%d cr0=%d cr3=%d stale=%d permissions=%d cross=%d\n",
            valid, delivered, faults, controls, invlpg, cr0_controls,
            cr3_reload, no_stale_translation, permissions, cross_page);
        return 1;
    }
    STD_PRINTF("M5:T258:S2:I386-PAGING:OK\n");
    STD_PRINTF("M5:T258:S3:I386-PAGING:CORPUS:OK\n");
    STD_PRINTF("M5:T311:S3:PAGING-PERMISSIONS:OK\n");
    STD_PRINTF("M5:T311:S4:CROSS-PAGE:OK\n");
    STD_PRINTF("M5:T325:S1:CR0-PAGING-CONTROL:OK\n");
    STD_PRINTF("M5:T325:S2:CR2-CR3-TRANSLATION:OK\n");
    STD_PRINTF("M5:T325:S3:PAGING-CLOSURE:OK\n");
    return 0;
}

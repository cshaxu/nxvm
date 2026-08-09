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
#define TEST_DATA_PHYSICAL 0x5000u
#define TEST_STACK_PHYSICAL 0x6000u
#define TEST_CODE_SELECTOR 0x0008u
#define TEST_DATA_SELECTOR 0x0010u

#define TEST_PAGE_PRESENT 0x00000001u
#define TEST_PAGE_WRITABLE 0x00000002u
#define TEST_PAGE_ACCESSED 0x00000020u
#define TEST_PAGE_DIRTY 0x00000040u

typedef struct paging_machine {
    core_machine *machine;
} paging_machine;

static C_VOID paging_reset(C_VOID *opaque)
{
    paging_machine *state = (paging_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider paging_provider = {
    paging_reset,
    STD_NULL,
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
    if (core_machine_bind_execution_provider(state->machine, &paging_provider,
            state) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT paging_write_u32(core_machine *machine, uint32_t address,
    uint32_t value)
{
    return core_machine_memory_write(machine, address, &value, sizeof(value)) ==
        TYPE_STATUS_OK;
}

static C_INT paging_read_u32(core_machine *machine, uint32_t address,
    uint32_t *out_value)
{
    return core_machine_memory_read(machine, address, out_value,
        sizeof(*out_value)) == TYPE_STATUS_OK;
}

static C_INT paging_install_gdt(core_machine *machine)
{
    static const uint8_t gdt_pointer[] = {
        0x17u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const uint8_t gdt[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0x9au, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0x92u, 0x00u, 0x00u
    };

    return core_machine_memory_write(machine, TEST_GDT_POINTER, gdt_pointer,
        sizeof(gdt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, TEST_GDT_ADDRESS, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK;
}

static C_INT paging_install_tables(core_machine *machine, uint32_t code_entry,
    uint32_t data_entry, uint32_t stack_entry)
{
    return paging_write_u32(machine, TEST_PAGE_DIRECTORY,
               TEST_PAGE_TABLE | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE) &&
        paging_write_u32(machine, TEST_PAGE_TABLE, code_entry) &&
        paging_write_u32(machine, TEST_PAGE_TABLE + 3u * 4u, data_entry) &&
        paging_write_u32(machine, TEST_PAGE_TABLE + 4u * 4u, stack_entry);
}

static C_INT paging_write_bootstrap(core_machine *machine,
    const uint8_t *protected_code, STD_SIZE_T protected_code_size)
{
    static const uint8_t real_code[] = {
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
    uint32_t exception, uint32_t code, uint32_t point_linear)
{
    return diagnostic->first_fault.valid &&
        TYPE_GET_BIT(diagnostic->first_fault.exception_mask, exception) &&
        diagnostic->first_fault.exception_code == code &&
        diagnostic->first_fault.point.cs == TEST_CODE_SELECTOR &&
        diagnostic->first_fault.point.linear_pc == point_linear;
}

static C_INT paging_test_valid_path(C_VOID)
{
    static const uint8_t protected_code[] = {
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
    const uint32_t code_entry = TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    const uint32_t data_entry = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const uint32_t stack_entry = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    uint16_t data = 0u;
    uint32_t pde = 0u;
    uint32_t pte_code = 0u;
    uint32_t pte_data = 0u;
    uint32_t pte_stack = 0u;
    uint32_t pre_cr0 = 0u;
    uint32_t pre_cr2 = 0u;
    uint32_t pre_cr3 = 0u;
    uint32_t pre_ecx = 0u;
    uint32_t pre_edx = 0u;
    t_cpu cpu;
    C_INT ran;
    C_INT registers;
    C_INT data_ok;
    C_INT entries_read;
    C_INT entries_ok;
    C_INT reset_ok;
    C_INT failed = !paging_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
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
        reset_ok = core_machine_reset(state.machine) == TYPE_STATUS_OK &&
            ((cpu = test_core_machine_fixture_capture_cpu_after_run(state.machine)),
            cpu.data.cr0 == 0u && cpu.data.cr2 == 0u && cpu.data.cr3 == 0u);
        failed |= !ran || !registers || !data_ok || !entries_read ||
            !entries_ok || !reset_ok;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T258 valid result=%d/%d fault=%d/%08x cr0=%08x cr2=%08x cr3=%08x ecx=%08x edx=%08x data=%04x pde=%08x pte=%08x/%08x/%08x\n",
                result.executed, result.reason, diagnostic.first_fault.valid,
                diagnostic.first_fault.exception_mask, pre_cr0, pre_cr2,
                pre_cr3, pre_ecx, pre_edx, data, pde, pte_code, pte_data,
                pte_stack);
            STD_FPRINTF(STD_STDERR,
                "T258 valid checks ran=%d regs=%d data=%d reads=%d entries=%d reset=%d\n",
                ran, registers, data_ok, entries_read, entries_ok, reset_ok);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT paging_test_fault(uint32_t code_entry, uint32_t data_entry,
    uint32_t stack_entry, const uint8_t *protected_code,
    STD_SIZE_T protected_code_size, uint32_t expected_code,
    uint32_t expected_point, uint32_t expected_cr2)
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
    static const uint8_t enable_only[] = {
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u
    };
    static const uint8_t data_read[] = {
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0xbbu, 0x00u, 0x30u,
        0x8bu, 0x07u
    };
    static const uint8_t stack_write[] = {
        0xbcu, 0x00u, 0x50u,
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0x50u
    };
    const uint32_t code = TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    const uint32_t data = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const uint32_t stack = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
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
    const uint8_t *code, STD_SIZE_T code_size)
{
    paging_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !paging_prepare(&state, profile);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, code, code_size) !=
            TYPE_STATUS_OK;
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
    static const uint8_t write_reserved_cr1[] = {
        0x66u, 0xb8u, 0x34u, 0x12u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xc8u
    };
    static const uint8_t pg_without_pe[] = {
        0x66u, 0xb8u, 0x00u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u
    };
    static const uint8_t unaligned_cr3[] = {
        0x66u, 0xb8u, 0x34u, 0x12u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u
    };
    static const uint8_t read_cr0[] = { 0x0fu, 0x20u, 0xc0u };
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

C_INT main(C_VOID)
{
    const C_INT valid = paging_test_valid_path();
    const C_INT faults = paging_test_page_faults();
    const C_INT controls = paging_test_control_forms();

    if (valid || faults || controls) {
        STD_FPRINTF(STD_STDERR,
            "M5:T258:S2:I386-PAGING:FAIL valid=%d faults=%d controls=%d\n",
            valid, faults, controls);
        return 1;
    }
    STD_PRINTF("M5:T258:S2:I386-PAGING:OK\n");
    STD_PRINTF("M5:T258:S3:I386-PAGING:CORPUS:OK\n");
    return 0;
}

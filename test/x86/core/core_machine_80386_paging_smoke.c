#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86/core/device_support_interface.h"

#include "x86/core/debug_interface.h"
#include "x86/core/machine.h"

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
    lib_status reset_status;
} paging_machine;

typedef struct paging_trace_probe {
    core_machine_trace_event events[4096];
    lib_u32 count;
} paging_trace_probe;

static void paging_trace(void *opaque, const core_machine_trace_event *event)
{
    paging_trace_probe *probe = (paging_trace_probe *)opaque;

    if (probe != LIB_NULL && event != LIB_NULL && probe->count < 4096u) {
        probe->events[probe->count++] = *event;
    }
}

static lib_i32 paging_has_provenance_pair(const paging_trace_probe *probe,
    core_machine_transaction_kind kind,
    core_machine_cpu_memory_access_provenance provenance)
{
    lib_u32 index;

    if (probe == LIB_NULL) return 0;
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

static void paging_reset(void *opaque)
{
    paging_machine *state = (paging_machine *)opaque;

    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    if (state != LIB_NULL) state->reset_status =
        core_machine_cpu_debug_patch_registers(
            state->machine->executor_cpu_execution, &entry);
}

static const core_machine_execution_provider paging_provider = {
    paging_reset,
    LIB_NULL
};

static lib_i32 paging_prepare(paging_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_executor_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_neutral_create(&config, &state->machine) != LIB_STATUS_OK ||
        state->machine == LIB_NULL) return 0;
    if (core_machine_bind_execution_provider(state->machine,
            &paging_provider, state) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        core_machine_reset(state->machine) != LIB_STATUS_OK ||
        state->reset_status != LIB_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 paging_write_u32(core_machine *machine, lib_u32 address,
    lib_u32 value)
{
    return core_machine_memory_write(machine, address, &value, sizeof(value)) ==
        LIB_STATUS_OK;
}

static lib_i32 paging_read_u32(core_machine *machine, lib_u32 address,
    lib_u32 *out_value)
{
    return core_machine_memory_inspect(machine, address, out_value,
        sizeof(*out_value)) == LIB_STATUS_OK;
}

static lib_i32 paging_install_gdt(core_machine *machine)
{
    static const lib_u8 gdt_pointer[] = {
        0x27u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const lib_u8 gdt[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0x9au, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0x92u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0xfau, 0x8fu, 0x00u,
        0xffu, 0xffu, 0x00u, 0x00u, 0x00u, 0xf2u, 0x8fu, 0x00u
    };

    return core_machine_memory_write(machine, TEST_GDT_POINTER, gdt_pointer,
        sizeof(gdt_pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, TEST_GDT_ADDRESS, gdt,
            sizeof(gdt)) == LIB_STATUS_OK;
}

static lib_i32 paging_install_tables(core_machine *machine, lib_u32 code_entry,
    lib_u32 data_entry, lib_u32 stack_entry)
{
    return paging_write_u32(machine, TEST_PAGE_DIRECTORY,
               TEST_PAGE_TABLE | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE) &&
        paging_write_u32(machine, TEST_PAGE_TABLE, code_entry) &&
        paging_write_u32(machine, TEST_PAGE_TABLE + 3u * 4u, data_entry) &&
        paging_write_u32(machine, TEST_PAGE_TABLE + 4u * 4u, stack_entry);
}

static lib_i32 paging_write_bootstrap(core_machine *machine,
    const lib_u8 *protected_code, lib_size protected_code_size)
{
    static const lib_u8 real_code[] = {
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
            LIB_STATUS_OK &&
        core_machine_memory_write(machine, TEST_PROTECTED_CODE, protected_code,
            protected_code_size) == LIB_STATUS_OK;
}

static lib_i32 paging_run(core_machine *machine, lib_i32 expect_fault,
    core_machine_run_result *out_result,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = { 128u, 0u };

    return core_machine_run(machine, budget, out_result) == LIB_STATUS_OK &&
        out_result->reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
        (!expect_fault || out_result->detail == VCPUINS_EXCEPT_SHUTDOWN) &&
        core_machine_get_cpu_diagnostic(machine, out_diagnostic) == LIB_STATUS_OK;
}

/* Re-enter a halted fixture through the CPU-owned prepared-entry boundary. */
static lib_i32 paging_resume_at(core_machine *machine, lib_u16 cs,
    lib_u16 ds, lib_u16 ss, lib_u16 ip, lib_u32 flags)
{
    core_machine_debug_cpu_snapshot cpu;
    core_machine_cpu_prepared_entry *entry = LIB_NULL;
    core_machine_entry_plan_state state;

    if (core_machine_debug_capture_cpu_snapshot(machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK) return 0;
    state = (core_machine_entry_plan_state) {
        .cs = cs, .ds = ds, .es = cpu.es.selector, .ss = ss,
        .ip = ip, .sp = (lib_u16)cpu.esp,
        .eax = cpu.eax, .ebx = cpu.ebx, .ecx = cpu.ecx, .edx = cpu.edx,
        .esi = cpu.esi, .edi = cpu.edi, .ebp = cpu.ebp, .eflags = flags
    };
    if (core_machine_cpu_prepare_entry(machine->executor_cpu_execution,
            &state, &entry) != LIB_STATUS_OK || entry == LIB_NULL) return 0;
    core_machine_cpu_finish_entry(entry, LIB_TRUE);
    return 1;
}

static lib_i32 paging_test_delivered_page_fault(void)
{
    static const lib_u8 protected_code[] = {
        0x0fu, 0x01u, 0x1eu, 0x00u, 0x06u,
        0xbcu, 0x00u, 0x50u,
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0xf4u
    };
    static const lib_u8 faulting_code[] = { 0xa1u, 0x00u, 0x90u };
    static const lib_u8 hlt[] = { 0xf4u };
    static const lib_u8 idt_pointer[] = {
        0x77u, 0x00u, 0x00u, 0x05u, 0x00u, 0x00u
    };
    const lib_u8 gate[] = { 0x00u, 0x01u, 0x08u, 0x00u,
        0x00u, 0x8eu, 0x00u, 0x00u };
    const lib_u32 code_entry = TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    const lib_u32 data_entry = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 stack_entry = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    paging_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_u32 frame[4] = { 0u, 0u, 0u, 0u };
    lib_i32 failed = !paging_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        if (!failed) failed |= !paging_install_tables(state.machine, code_entry, data_entry,
            stack_entry) || !paging_write_bootstrap(state.machine, protected_code,
            sizeof(protected_code));
        if (!failed) failed |= core_machine_memory_write(state.machine, 0x0600u,
            idt_pointer, sizeof(idt_pointer)) != LIB_STATUS_OK;
        if (!failed) failed |= !paging_run(state.machine, 0, &result, &diagnostic);
    }
    if (!failed) {
        if (!failed) failed |= core_machine_memory_write(state.machine, 0x0500u + 14u * 8u,
            gate, sizeof(gate)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x0080u, faulting_code, sizeof(faulting_code)) !=
            LIB_STATUS_OK || core_machine_memory_write(state.machine, 0x0100u,
            hlt, sizeof(hlt)) != LIB_STATUS_OK;
        if (!failed) failed |= !paging_resume_at(state.machine, TEST_CODE_SELECTOR,
            TEST_DATA_SELECTOR, TEST_DATA_SELECTOR, 0x0080u,
            CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_IF);
        if (!failed) failed |= core_machine_debug_write_register(state.machine,
            CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_CF |
            CORE_MACHINE_DEBUG_EFLAGS_IF) != LIB_STATUS_OK;
        if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        if (!failed) failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
            &result) != LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed) failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_PF) || diagnostic.last_delivered_exception.exception_code != 0u ||
            after.eip != 0x0100u || after.cr2 != 0x00009000u ||
            after.esp != 0x00004ff0u || core_machine_debug_read_linear(
                state.machine, after.ss.base + after.esp,
                frame, sizeof(frame)) != LIB_STATUS_OK || frame[0] != 0u ||
            frame[1] != 0x0080u || frame[2] != TEST_CODE_SELECTOR ||
            frame[3] != (before.eflags | 0x00010000u) || after.eax != before.eax ||
            after.ebx != before.ebx || after.ecx != before.ecx ||
            after.edx != before.edx || after.ebp != before.ebp ||
            after.esi != before.esi || after.edi != before.edi;
    }
    if (!failed) {
        if (!failed) failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
            &result) != LIB_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            after.eip != 0x0101u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 paging_test_valid_path(void)
{
    static const lib_u8 protected_code[] = {
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
    const lib_u32 code_entry = TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    const lib_u32 data_entry = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 stack_entry = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    paging_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_u16 data = 0u;
    lib_u32 pde = 0u;
    lib_u32 pte_code = 0u;
    lib_u32 pte_data = 0u;
    lib_u32 pte_stack = 0u;
    lib_u32 pre_cr0 = 0u;
    lib_u32 pre_cr2 = 0u;
    lib_u32 pre_cr3 = 0u;
    lib_u32 pre_ecx = 0u;
    lib_u32 pre_edx = 0u;
    core_machine_debug_cpu_snapshot cpu = {0};
    paging_trace_probe trace = {{{0}}, 0u};
    const core_machine_trace_provider trace_provider = { paging_trace, &trace };
    lib_i32 ran = 0;
    lib_i32 registers = 0;
    lib_i32 data_ok = 0;
    lib_i32 entries_read = 0;
    lib_i32 entries_ok = 0;
    lib_i32 provenance_ok = 0;
    lib_i32 reset_ok = 0;
    lib_i32 failed = !paging_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        if (!failed) failed |= core_machine_set_trace_provider(state.machine,
            &trace_provider) != LIB_STATUS_OK;
        if (!failed) failed |= !paging_install_tables(state.machine, code_entry, data_entry,
            stack_entry);
        if (!failed) failed |= !paging_write_bootstrap(state.machine, protected_code,
            sizeof(protected_code));
        if (!failed) {
            ran = paging_run(state.machine, 0, &result, &diagnostic);
            failed |= !ran;
        }
        if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
        pre_cr0 = cpu.cr0;
        pre_cr2 = cpu.cr2;
        pre_cr3 = cpu.cr3;
        pre_ecx = cpu.ecx;
        pre_edx = cpu.edx;
        if (!failed) {
            registers = !diagnostic.first_fault.valid &&
                pre_cr0 == (VCPU_CR0_PE | VCPU_CR0_PG) && pre_cr2 == 0u &&
                pre_cr3 == TEST_PAGE_DIRECTORY && pre_ecx ==
                    (VCPU_CR0_PE | VCPU_CR0_PG) && pre_edx == 0u;
            failed |= !registers;
        }
        if (!failed) {
            data_ok = core_machine_memory_read(state.machine, TEST_DATA_PHYSICAL,
                &data, sizeof(data)) == LIB_STATUS_OK && data == 0x1234u;
            failed |= !data_ok;
        }
        if (!failed) {
            entries_read = paging_read_u32(state.machine, TEST_PAGE_DIRECTORY, &pde) &&
                paging_read_u32(state.machine, TEST_PAGE_TABLE, &pte_code) &&
                paging_read_u32(state.machine, TEST_PAGE_TABLE + 3u * 4u,
                    &pte_data) && paging_read_u32(state.machine,
                    TEST_PAGE_TABLE + 4u * 4u, &pte_stack);
            failed |= !entries_read;
        }
        if (!failed) {
            entries_ok = (pde & TEST_PAGE_ACCESSED) != 0u &&
                (pte_code & TEST_PAGE_ACCESSED) != 0u &&
                (pte_data & (TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY)) ==
                    (TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY) &&
                (pte_stack & (TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY)) ==
                    (TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY);
            failed |= !entries_ok;
        }
        if (!failed) {
            provenance_ok = paging_has_provenance_pair(&trace,
                    CORE_MACHINE_TRANSACTION_CPU_MEMORY_READ,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ) &&
                paging_has_provenance_pair(&trace,
                    CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE);
            failed |= !provenance_ok;
        }
        if (!failed) {
            reset_ok = core_machine_reset(state.machine) == LIB_STATUS_OK &&
                state.reset_status == LIB_STATUS_OK &&
                core_machine_debug_capture_cpu_snapshot(state.machine,
                    CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) == LIB_STATUS_OK &&
                cpu.cr0 == 0u && cpu.cr2 == 0u && cpu.cr3 == 0u;
            failed |= !reset_ok;
        }
        if (!failed) failed |= !ran || !registers || !data_ok || !entries_read ||
            !entries_ok || !provenance_ok || !reset_ok;
        if (failed) {
            lib_c_fprintf(lib_c_stderr,
                "valid result=%llu/%d fault=%d/%08x cr0=%08x cr2=%08x cr3=%08x ecx=%08x edx=%08x data=%04x pde=%08x pte=%08x/%08x/%08x\n",
                result.executed, result.reason, diagnostic.first_fault.valid,
                diagnostic.first_fault.exception_mask, pre_cr0, pre_cr2,
                pre_cr3, pre_ecx, pre_edx, data, pde, pte_code, pte_data,
                pte_stack);
            lib_c_fprintf(lib_c_stderr,
                "valid checks ran=%d regs=%d data=%d reads=%d entries=%d provenance=%d reset=%d\n",
                ran, registers, data_ok, entries_read, entries_ok, provenance_ok,
                reset_ok);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 paging_test_fault(lib_u32 code_entry, lib_u32 data_entry,
    lib_u32 stack_entry, const lib_u8 *protected_code,
    lib_size protected_code_size)
{
    paging_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot cpu = {0};
    lib_i32 failed = !paging_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        if (!failed) failed |= !paging_install_tables(state.machine, code_entry, data_entry,
            stack_entry);
        if (!failed) failed |= !paging_write_bootstrap(state.machine, protected_code,
            protected_code_size);
        if (!failed) failed |= !paging_run(state.machine, 1, &result, &diagnostic);
        if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
        if (!failed) failed |= result.detail != VCPUINS_EXCEPT_SHUTDOWN;
        if (failed) {
            lib_c_fprintf(lib_c_stderr,
                "receiverless-fault result=%llu/%d diag=%d/%08x/%08x point=%04x:%08x cr2=%08x\n",
                result.executed, result.reason, diagnostic.first_fault.valid,
                diagnostic.first_fault.exception_mask,
                diagnostic.first_fault.exception_code,
                diagnostic.first_fault.point.cs,
                diagnostic.first_fault.point.linear_pc, cpu.cr2);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 paging_test_page_faults(void)
{
    static const lib_u8 enable_only[] = {
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u
    };
    static const lib_u8 data_read[] = {
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0xbbu, 0x00u, 0x30u,
        0x8bu, 0x07u
    };
    static const lib_u8 stack_write[] = {
        0xbcu, 0x00u, 0x50u,
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0x50u
    };
    const lib_u32 code = TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    const lib_u32 data = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 stack = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    lib_i32 failed = 0;

    if (!failed) failed |= paging_test_fault(0u, data, stack, enable_only,
        sizeof(enable_only));
    if (!failed) failed |= paging_test_fault(code, 0u, stack, data_read,
        sizeof(data_read));
    if (!failed) failed |= paging_test_fault(code, data, 0u, stack_write,
        sizeof(stack_write));
    return failed;
}

static lib_i32 paging_test_cr3_directory_reload(void)
{
    static const lib_u8 protected_code[] = {
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
    const lib_u32 present_writable = TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 first_value = 0x11223344u;
    const lib_u32 second_value = 0x55667788u;
    paging_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot cpu = {0};
    lib_i32 failed = !paging_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        if (!failed) failed |= !paging_write_u32(state.machine, TEST_PAGE_DIRECTORY,
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
                &first_value, sizeof(first_value)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, TEST_RELOAD_DATA_PHYSICAL,
                &second_value, sizeof(second_value)) != LIB_STATUS_OK ||
            !paging_write_bootstrap(state.machine, protected_code,
                sizeof(protected_code));
    }
    if (!failed) {
        if (!failed) failed |= !paging_run(state.machine, 0, &result, &diagnostic);
        if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
        if (!failed) failed |= diagnostic.first_fault.valid || cpu.cr0 !=
            (VCPU_CR0_PE | VCPU_CR0_PG) || cpu.cr3 !=
            TEST_PAGE_DIRECTORY_RELOAD || cpu.eax != first_value ||
            cpu.ecx != TEST_PAGE_DIRECTORY_RELOAD || cpu.edx !=
            second_value || cpu.ebx != 0x00003000u || cpu.eip !=
            TEST_PROTECTED_CODE + sizeof(protected_code) ||
            cpu.eflags != 0x00000002u ||
            cpu.esp != 0x00005000u || cpu.ebp != 0u ||
            cpu.esi != 0u || cpu.edi != 0u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 paging_permission_read(core_machine *machine,
    lib_u32 physical, void *out_data, lib_size bytes);
static lib_i32 paging_permission_prepare(paging_machine *state,
    const lib_u8 *program, lib_size program_size,
    lib_u32 pde_code, lib_u32 pde_data,
    lib_u32 pte_code, lib_u32 pte_data,
    lib_u32 pte_stack, lib_i32 user, lib_i32 set_reserved_cr0_bit,
    lib_u32 *out_program_eip);

static lib_i32 paging_test_no_stale_translation(void)
{
    static const lib_u8 program[] = {
        0x66u, 0x67u, 0x8bu, 0x03u,
        0x66u, 0x67u, 0x8bu, 0x13u
    };
    const lib_u32 entry = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 replacement = TEST_RELOAD_DATA_PHYSICAL |
        TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    const lib_u32 first_value = 0x10203040u;
    const lib_u32 second_value = 0x50607080u;
    paging_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot cpu = {0};
    lib_u32 pte = 0u;
    lib_i32 failed = !paging_permission_prepare(&state, program, sizeof(program),
        TEST_PAGE_TABLE | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
        TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
        TEST_PERMISSION_CODE | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
        entry, TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
        0, 0, LIB_NULL);

    if (!failed) {
        if (!failed) failed |= core_machine_memory_write(state.machine, TEST_DATA_PHYSICAL,
                &first_value, sizeof(first_value)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, TEST_RELOAD_DATA_PHYSICAL,
                &second_value, sizeof(second_value)) != LIB_STATUS_OK ||
            core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != LIB_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_BUDGET || core_machine_get_cpu_diagnostic(
                state.machine, &diagnostic) != LIB_STATUS_OK;
        if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
        if (!failed) failed |= diagnostic.first_fault.valid || cpu.eax != first_value ||
            cpu.eip != 4u || cpu.edx != 0x00000300u || cpu.cr3 !=
            TEST_PAGE_DIRECTORY || cpu.eflags != 0x00000002u;
    }
    if (!failed) {
        if (!failed) failed |= !paging_write_u32(state.machine, TEST_PAGE_TABLE_SECOND +
                3u * 4u, replacement) || core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
        if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
        if (!failed) failed |= diagnostic.first_fault.valid || cpu.eax != first_value ||
            cpu.edx != second_value || cpu.eip != 8u ||
            cpu.cr3 != TEST_PAGE_DIRECTORY || cpu.eflags !=
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

static lib_i32 paging_permission_install(core_machine *machine,
    lib_u32 pde_code, lib_u32 pde_data, lib_u32 pte_code,
    lib_u32 pte_data, lib_u32 pte_stack)
{
    return core_machine_memory_write_physical(&machine->executor_memory,
               TEST_PAGE_DIRECTORY, CORE_MACHINE_REFERENCE_OF(pde_code),
               sizeof(pde_code)) == LIB_STATUS_OK &&
        core_machine_memory_write_physical(&machine->executor_memory,
            TEST_PAGE_DIRECTORY + 4u, CORE_MACHINE_REFERENCE_OF(pde_data),
            sizeof(pde_data)) == LIB_STATUS_OK &&
        core_machine_memory_write_physical(&machine->executor_memory,
            TEST_PAGE_TABLE + 7u * 4u, CORE_MACHINE_REFERENCE_OF(pte_code), sizeof(pte_code)) ==
                LIB_STATUS_OK &&
        core_machine_memory_write_physical(&machine->executor_memory,
            TEST_PAGE_TABLE + 4u * 4u, CORE_MACHINE_REFERENCE_OF(pte_stack),
            sizeof(pte_stack)) == LIB_STATUS_OK &&
        core_machine_memory_write_physical(&machine->executor_memory,
            TEST_PAGE_TABLE_SECOND + 3u * 4u, CORE_MACHINE_REFERENCE_OF(pte_data),
            sizeof(pte_data)) == LIB_STATUS_OK;
}

static lib_i32 paging_permission_read(core_machine *machine, lib_u32 physical,
    void *out_data, lib_size bytes)
{
    return machine != LIB_NULL && core_machine_memory_inspect(machine, physical, out_data,
        bytes) == LIB_STATUS_OK;
}

static lib_i32 paging_permission_prepare(paging_machine *state,
    const lib_u8 *program, lib_size program_size, lib_u32 pde_code,
    lib_u32 pde_data, lib_u32 pte_code, lib_u32 pte_data,
    lib_u32 pte_stack, lib_i32 user, lib_i32 set_reserved_cr0_bit,
    lib_u32 *out_program_eip)
{
    static const lib_u8 enable_paging[] = {
        0xbcu, 0x00u, 0x50u,
        0x66u, 0xb8u, 0x00u, 0x10u, 0x00u, 0x00u,
        0x0fu, 0x22u, 0xd8u,
        0x66u, 0xb8u, 0x01u, 0x00u, 0x00u, 0x80u,
        0x0fu, 0x22u, 0xc0u,
        0xf4u
    };
    lib_u8 protected_code[64u] = {0};
    lib_u16 data = 0x1234u;
    lib_u16 stack = 0xaaaau;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    lib_u32 initial_pde = TEST_PAGE_TABLE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    lib_u32 initial_pte = TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE;
    core_machine_debug_cpu_snapshot prepared = {0};
    lib_i32 failed;

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
    failed = program_size > sizeof(protected_code) -
        sizeof(enable_paging) || !paging_prepare(state,
        CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        lib_memory_copy(protected_code, enable_paging, sizeof(enable_paging));
        lib_memory_copy(protected_code + sizeof(enable_paging), program, program_size);
        if (!failed) failed |= !paging_permission_install(state->machine, initial_pde,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
            initial_pte, TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
            TEST_PAGE_WRITABLE, TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
            TEST_PAGE_WRITABLE);
        if (!failed) failed |= !paging_write_u32(state->machine, TEST_PAGE_TABLE, initial_pte);
        if (!failed) failed |= !paging_write_bootstrap(state->machine, protected_code,
            sizeof(enable_paging) + program_size);
        if (!failed) failed |= core_machine_memory_write(state->machine, TEST_PERMISSION_CODE,
            program, program_size) != LIB_STATUS_OK;
        if (!failed) failed |= core_machine_memory_write(state->machine, TEST_DATA_PHYSICAL,
            &data, sizeof(data)) != LIB_STATUS_OK ||
            core_machine_memory_write(state->machine, TEST_STACK_PHYSICAL +
                0xffeu, &stack, sizeof(stack)) != LIB_STATUS_OK;
        if (!failed) failed |= !paging_run(state->machine, 0, &result, &diagnostic);
        /* Load the intended caches from real descriptors before installing the
         * restrictive page permissions whose effects are under test. */
        const lib_u8 code_descriptor[] = {
            0xffu, 0xffu, 0x00u, 0x70u, 0x00u,
            user ? 0xfau : 0x9au, 0x00u, 0x00u
        };
        const lib_u8 data_descriptor[] = {
            0xffu, 0xffu, 0x00u, 0x00u, 0x00u,
            user ? 0xf2u : 0x92u, 0x8fu, 0x00u
        };
        if (!failed) failed |= core_machine_debug_write_register(state->machine,
            CORE_MACHINE_DEBUG_CR0, VCPU_CR0_PE) != LIB_STATUS_OK;
        if (!failed) failed |= core_machine_memory_write(state->machine, TEST_GDT_ADDRESS +
            (user ? 24u : 8u), code_descriptor, sizeof(code_descriptor)) !=
            LIB_STATUS_OK || core_machine_memory_write(state->machine,
            TEST_GDT_ADDRESS + (user ? 32u : 16u), data_descriptor,
            sizeof(data_descriptor)) != LIB_STATUS_OK;
        if (!failed) failed |= core_machine_debug_write_register(state->machine,
            CORE_MACHINE_DEBUG_EAX, 0xfacebeefu) != LIB_STATUS_OK ||
            core_machine_debug_write_register(state->machine,
            CORE_MACHINE_DEBUG_EBX, TEST_DATA_LINEAR) != LIB_STATUS_OK ||
            core_machine_debug_write_register(state->machine,
            CORE_MACHINE_DEBUG_ESP, 0x00005000u) != LIB_STATUS_OK;
        if (!failed) failed |= !paging_resume_at(state->machine,
            user ? TEST_USER_CODE_SELECTOR : TEST_CODE_SELECTOR,
            user ? TEST_USER_DATA_SELECTOR : TEST_DATA_SELECTOR,
            user ? TEST_USER_DATA_SELECTOR : TEST_DATA_SELECTOR, 0u, 0u);
        if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state->machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &prepared) != LIB_STATUS_OK ||
            prepared.cs.base != TEST_PERMISSION_CODE ||
            prepared.cs.limit != 0xffffu || prepared.cs.dpl != (user ? 3u : 0u) ||
            prepared.ds.limit != 0xffffffffu || prepared.ss.limit != 0xffffffffu ||
            prepared.eip != 0u;
        if (!failed) failed |= !paging_permission_install(state->machine, pde_code, pde_data,
            pte_code, pte_data, pte_stack);
        if (!failed) failed |= core_machine_debug_write_register(state->machine,
            CORE_MACHINE_DEBUG_CR0, VCPU_CR0_PE | VCPU_CR0_PG |
            (set_reserved_cr0_bit ? TEST_80386_FORMER_WP_BIT : 0u)) != LIB_STATUS_OK;
        if (!failed && out_program_eip != LIB_NULL) *out_program_eip =
            0u;
    }
    return !failed;
}

static lib_i32 paging_permission_expect_fault(paging_machine *state,
    lib_u32 program_eip, lib_u32 pde_address, lib_u32 pde_initial,
    lib_u32 pte_address,
    lib_u32 pte_initial, paging_permission_access access)
{
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot cpu = {0};
    lib_u32 pde = 0u;
    lib_u32 pte = 0u;
    const lib_u32 expected_pde = pde_initial | TEST_PAGE_ACCESSED;
    lib_u16 data = 0u;
    const core_machine_run_budget budget = { 32u, 0u };
    lib_i32 failed = core_machine_run(state->machine, budget, &result) !=
        LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) !=
            LIB_STATUS_OK;

    /* These receiverless probes must terminate through shutdown.  The
     * delivered-page-fault case above owns exact frame/code/CR2 coverage. */
    if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state->machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
    if (!failed) failed |= result.detail != VCPUINS_EXCEPT_SHUTDOWN ||
        cpu.eip != program_eip ||
        cpu.ebx != TEST_DATA_LINEAR || cpu.esp != 0x00005000u ||
        cpu.eflags != 0x00000002u;
    if (access == PAGING_PERMISSION_READ) failed |= cpu.eax != 0xfacebeefu;
    if (!failed) failed |= !paging_permission_read(state->machine, pde_address, &pde,
        sizeof(pde)) || !paging_permission_read(state->machine, pte_address,
            &pte, sizeof(pte)) ||
        pde != expected_pde || pte != pte_initial;
    if (access == PAGING_PERMISSION_WRITE) {
        if (!failed) failed |= !paging_permission_read(state->machine, TEST_DATA_PHYSICAL,
            &data, sizeof(data)) || data != 0x1234u;
    } else if (access == PAGING_PERMISSION_STACK) {
        if (!failed) failed |= !paging_permission_read(state->machine, TEST_STACK_PHYSICAL +
            0xffeu, &data, sizeof(data)) || data != 0xaaaau;
    }
    if (failed) {
        lib_c_fprintf(lib_c_stderr,
            "fault access=%u result=%u/%u diag=%x/%x eip=%x cr2=%x eax=%x ebx=%x esp=%x flags=%x pde=%x/%x pte=%x/%x\n",
            (unsigned)access, (unsigned)result.executed, (unsigned)result.reason,
            diagnostic.first_fault.exception_mask,
            diagnostic.first_fault.exception_code, cpu.eip, cpu.cr2,
            cpu.eax, cpu.ebx, cpu.esp, cpu.eflags, pde,
            expected_pde, pte, pte_initial);
    }
    return !failed;
}

static lib_i32 paging_permission_expect_success(paging_machine *state,
    paging_permission_access access, lib_u32 pde_address, lib_u32 pte_address)
{
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot cpu = {0};
    lib_u32 pde = 0u;
    lib_u32 pte = 0u;
    lib_u16 data = 0u;
    const core_machine_run_budget budget = { 1u, 0u };
    lib_i32 failed = core_machine_run(state->machine, budget, &result) !=
        LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) !=
            LIB_STATUS_OK;

    if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state->machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
    if (!failed) failed |= diagnostic.first_fault.valid || !paging_permission_read(
        state->machine, pde_address, &pde, sizeof(pde)) ||
        !paging_permission_read(state->machine, pte_address, &pte,
            sizeof(pte)) || (pde & TEST_PAGE_ACCESSED) == 0u ||
        (pte & TEST_PAGE_ACCESSED) == 0u;
    if (access == PAGING_PERMISSION_READ) {
        if (!failed) failed |= cpu.eax != 0xface1234u ||
            (pte & TEST_PAGE_DIRTY) != 0u;
    } else if (access == PAGING_PERMISSION_WRITE) {
        if (!failed) failed |= !paging_permission_read(state->machine, TEST_DATA_PHYSICAL,
            &data, sizeof(data)) || data != 0xbeefu ||
            (pte & TEST_PAGE_DIRTY) == 0u;
    } else if (access == PAGING_PERMISSION_STACK) {
        if (!failed) failed |= cpu.esp != 0x00004ffeu ||
            !paging_permission_read(state->machine, TEST_STACK_PHYSICAL +
                0xffeu, &data, sizeof(data)) ||
            data != 0xbeefu || (pte & TEST_PAGE_DIRTY) == 0u;
    }
    if (failed) {
        lib_c_fprintf(lib_c_stderr,
            "success access=%u result=%u/%u fault=%d eax=%x esp=%x pde=%x pte=%x\n",
            (unsigned)access, (unsigned)result.executed, (unsigned)result.reason,
            diagnostic.first_fault.valid, cpu.eax, cpu.esp, pde, pte);
    }
    return !failed;
}

static lib_i32 paging_test_permissions(void)
{
    static const lib_u8 fetch[] = { 0x90u };
    static const lib_u8 read[] = { 0x67u, 0x8bu, 0x03u };
    static const lib_u8 write[] = { 0x67u, 0x89u, 0x03u };
    static const lib_u8 stack[] = { 0x50u };
    const lib_u32 code_user = TEST_PERMISSION_CODE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE | TEST_PAGE_US;
    const lib_u32 data_user = TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE | TEST_PAGE_US;
    const lib_u32 stack_user = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE | TEST_PAGE_US;
    const lib_u32 pde_code = TEST_PAGE_TABLE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE | TEST_PAGE_US;
    const lib_u32 pde_data = TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE | TEST_PAGE_US;
    paging_machine state;
    lib_u32 eip = 0u;
    lib_i32 failed = 0;

    if (!paging_permission_prepare(&state, fetch, sizeof(fetch), pde_code,
            pde_data, TEST_PERMISSION_CODE | TEST_PAGE_PRESENT |
            TEST_PAGE_WRITABLE, data_user,
            stack_user, 1, 0, &eip) || !paging_permission_expect_fault(&state,
            eip, TEST_PAGE_DIRECTORY,
            pde_code | TEST_PAGE_ACCESSED,
            TEST_PAGE_TABLE + 7u * 4u, TEST_PERMISSION_CODE |
            TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, PAGING_PERMISSION_FETCH))
        failed = 1;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_permission_prepare(&state, read, sizeof(read), pde_code,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
            code_user, data_user, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_fault(&state, eip, TEST_PAGE_DIRECTORY + 4u,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT |
            TEST_PAGE_WRITABLE, TEST_PAGE_TABLE_SECOND + 3u * 4u, data_user,
            PAGING_PERMISSION_READ)) failed = 1;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_permission_prepare(&state, read, sizeof(read), pde_code,
            pde_data, code_user, TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
            TEST_PAGE_WRITABLE, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_fault(&state, eip, TEST_PAGE_DIRECTORY + 4u,
            pde_data, TEST_PAGE_TABLE_SECOND + 3u * 4u,
            TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE,
            PAGING_PERMISSION_READ)) failed = 1;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_US, code_user,
            data_user, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_fault(&state, eip, TEST_PAGE_DIRECTORY + 4u,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT |
            TEST_PAGE_US, TEST_PAGE_TABLE_SECOND + 3u * 4u, data_user,
            PAGING_PERMISSION_WRITE)) failed = 1;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            pde_data, code_user, TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
            TEST_PAGE_US, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_fault(&state, eip, TEST_PAGE_DIRECTORY + 4u,
            pde_data, TEST_PAGE_TABLE_SECOND + 3u * 4u,
            TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT | TEST_PAGE_US,
            PAGING_PERMISSION_WRITE)) failed = 1;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_permission_prepare(&state, stack, sizeof(stack), pde_code,
            pde_data, code_user, data_user, TEST_STACK_PHYSICAL |
            TEST_PAGE_PRESENT | TEST_PAGE_US, 1, 0, &eip) ||
        !paging_permission_expect_fault(&state, eip, TEST_PAGE_DIRECTORY,
            pde_code | TEST_PAGE_ACCESSED,
            TEST_PAGE_TABLE + 4u * 4u, TEST_STACK_PHYSICAL |
            TEST_PAGE_PRESENT | TEST_PAGE_US, PAGING_PERMISSION_STACK)) failed = 1;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_permission_prepare(&state, read, sizeof(read), pde_code,
            pde_data, code_user, data_user, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_success(&state, PAGING_PERMISSION_READ,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u)) failed = 1;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            pde_data, code_user, data_user, stack_user, 1, 0, &eip) ||
        !paging_permission_expect_success(&state, PAGING_PERMISSION_WRITE,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u)) failed = 1;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_US, code_user,
            data_user, stack_user, 0, 0, &eip) ||
        !paging_permission_expect_success(&state, PAGING_PERMISSION_WRITE,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u)) failed = 1;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_US, code_user,
            data_user, stack_user, 0, 1, &eip) ||
        !paging_permission_expect_success(&state, PAGING_PERMISSION_WRITE,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u))
        failed = 1;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_permission_prepare(&state, write, sizeof(write), pde_code,
            pde_data, code_user, TEST_DATA_PHYSICAL | TEST_PAGE_PRESENT |
            TEST_PAGE_US, stack_user, 0, 1, &eip) ||
        !paging_permission_expect_success(&state, PAGING_PERMISSION_WRITE,
            TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u))
        failed = 1;
    core_machine_destroy(state.machine);
    if (failed) return 1;
    return failed;
}

static lib_i32 paging_cross_prepare(paging_machine *state, const lib_u8 *program,
    lib_size program_size, lib_u32 second_entry,
    lib_i32 set_reserved_cr0_bit)
{
    const lib_u32 code_entry = TEST_PERMISSION_CODE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 data_entry = TEST_CROSS_DATA_FIRST | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 stack_entry = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 pde_code = TEST_PAGE_TABLE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 pde_data = TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;

    return paging_permission_prepare(state, program, program_size, pde_code,
        pde_data, code_entry, data_entry, stack_entry, 0, set_reserved_cr0_bit,
        LIB_NULL) && paging_write_u32(state->machine,
        TEST_PAGE_TABLE_SECOND + 4u * 4u, second_entry);
}

static lib_i32 paging_cross_entries(core_machine *machine, lib_u32 pde_address,
    lib_u32 first_address, lib_u32 second_address, lib_u32 expected_pde,
    lib_u32 expected_first, lib_u32 expected_second)
{
    lib_u32 pde = 0u;
    lib_u32 first = 0u;
    lib_u32 second = 0u;

    return paging_permission_read(machine, pde_address, &pde, sizeof(pde)) &&
        paging_permission_read(machine, first_address, &first, sizeof(first)) &&
        paging_permission_read(machine, second_address, &second, sizeof(second)) &&
        pde == expected_pde && first == expected_first && second == expected_second;
}

static lib_i32 paging_cross_run(paging_machine *state, lib_i32 expect_fault,
    core_machine_run_result *out_result,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = { 1u, 0u };

    return core_machine_run(state->machine, budget, out_result) == LIB_STATUS_OK &&
        out_result->reason == (expect_fault ? CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT :
            CORE_MACHINE_STOP_BUDGET) &&
        (!expect_fault || out_result->detail == VCPUINS_EXCEPT_SHUTDOWN) &&
        core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) == LIB_STATUS_OK;
}

static lib_i32 paging_test_cross_data(void)
{
    static const lib_u8 read[] = { 0x67u, 0x8bu, 0x03u };
    static const lib_u8 write[] = { 0x66u, 0x67u, 0x89u, 0x03u };
    const lib_u32 pde = TEST_PAGE_TABLE_SECOND | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 first = TEST_CROSS_DATA_FIRST | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 second = TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    paging_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot cpu = {0};
    lib_u16 word = 0u;
    lib_u8 byte = 0u;
    lib_i32 failed = 0;

    if (!paging_cross_prepare(&state, read, sizeof(read), second, 0)) {
        core_machine_destroy(state.machine);
        return 1;
    }
    word = 0x3412u;
    if (!failed) failed |= core_machine_memory_write(state.machine, TEST_CROSS_DATA_FIRST +
        0xfffu, &word, sizeof(word)) != LIB_STATUS_OK;
    if (!failed) failed |= core_machine_debug_write_register(state.machine,
        CORE_MACHINE_DEBUG_EBX, TEST_DATA_LINEAR + 0xfffu) != LIB_STATUS_OK;
    if (!failed) failed |= !paging_cross_run(&state, 0, &result, &diagnostic);
    if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
    if (!failed) failed |= diagnostic.first_fault.valid || cpu.eax != 0xface3412u ||
        !paging_cross_entries(state.machine, TEST_PAGE_DIRECTORY + 4u,
            TEST_PAGE_TABLE_SECOND + 3u * 4u, TEST_PAGE_TABLE_SECOND + 4u * 4u,
            pde | TEST_PAGE_ACCESSED, first | TEST_PAGE_ACCESSED,
            second | TEST_PAGE_ACCESSED);
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_cross_prepare(&state, write, sizeof(write), second, 0)) {
        core_machine_destroy(state.machine);
        return 1;
    }
    if (!failed) failed |= core_machine_debug_write_register(state.machine,
        CORE_MACHINE_DEBUG_EBX, TEST_DATA_LINEAR + 0xfffu) != LIB_STATUS_OK;
    if (!failed) failed |= !paging_cross_run(&state, 0, &result, &diagnostic);
    if (!failed) failed |= diagnostic.first_fault.valid || !paging_cross_entries(state.machine,
        TEST_PAGE_DIRECTORY + 4u, TEST_PAGE_TABLE_SECOND + 3u * 4u,
        TEST_PAGE_TABLE_SECOND + 4u * 4u, pde | TEST_PAGE_ACCESSED,
        first | TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY, second |
        TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY) || !paging_permission_read(
        state.machine, TEST_CROSS_DATA_FIRST + 0xfffu, &byte, sizeof(byte)) ||
        byte != 0xefu || !paging_permission_read(state.machine,
        TEST_CROSS_DATA_SECOND, &word, sizeof(word)) || word != 0xcebeu;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    return failed;
}

static lib_i32 paging_test_cross_stack(void)
{
    static const lib_u8 push[] = { 0x50u };
    static const lib_u8 pop[] = { 0x58u };
    const lib_u32 pde = TEST_PAGE_TABLE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 first = TEST_CROSS_DATA_FIRST | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 second = TEST_STACK_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    paging_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot cpu = {0};
    lib_u8 low = 0u;
    lib_u8 high = 0u;
    lib_i32 failed;

    if (!paging_cross_prepare(&state, push, sizeof(push),
            TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, 0)) {
        core_machine_destroy(state.machine);
        return 1;
    }
    failed = !paging_write_u32(state.machine, TEST_PAGE_TABLE + 3u * 4u, first) ||
        !paging_write_u32(state.machine, TEST_PAGE_TABLE + 4u * 4u, second);
    if (!failed) failed |= core_machine_debug_write_register(state.machine,
        CORE_MACHINE_DEBUG_ESP, 0x00004001u) != LIB_STATUS_OK;
    if (!failed) failed |= !paging_cross_run(&state, 0, &result, &diagnostic);
    if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
    if (!failed) failed |= diagnostic.first_fault.valid || cpu.esp != 0x00003fffu ||
        !paging_cross_entries(state.machine, TEST_PAGE_DIRECTORY,
            TEST_PAGE_TABLE + 3u * 4u, TEST_PAGE_TABLE + 4u * 4u,
            pde | TEST_PAGE_ACCESSED, first | TEST_PAGE_ACCESSED |
            TEST_PAGE_DIRTY, second | TEST_PAGE_ACCESSED | TEST_PAGE_DIRTY) ||
        !paging_permission_read(state.machine, TEST_CROSS_DATA_FIRST + 0xfffu,
            &low, sizeof(low)) || !paging_permission_read(state.machine,
            TEST_STACK_PHYSICAL, &high, sizeof(high)) || low != 0xefu ||
        high != 0xbeu;
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_cross_prepare(&state, pop, sizeof(pop),
            TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, 0)) {
        core_machine_destroy(state.machine);
        return 1;
    }
    if (!failed) failed |= !paging_write_u32(state.machine, TEST_PAGE_TABLE + 3u * 4u, first) ||
        !paging_write_u32(state.machine, TEST_PAGE_TABLE + 4u * 4u, second) ||
        core_machine_memory_write(state.machine, TEST_CROSS_DATA_FIRST + 0xfffu,
            &(lib_u8){ 0x12u }, 1u) != LIB_STATUS_OK ||
        core_machine_memory_write(state.machine, TEST_STACK_PHYSICAL,
            &(lib_u8){ 0x34u }, 1u) != LIB_STATUS_OK;
    if (!failed) failed |= core_machine_debug_write_register(state.machine,
        CORE_MACHINE_DEBUG_ESP, 0x00003fffu) != LIB_STATUS_OK;
    if (!failed) failed |= !paging_cross_run(&state, 0, &result, &diagnostic);
    if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
    if (!failed) failed |= diagnostic.first_fault.valid || cpu.esp != 0x00004001u ||
        cpu.eax != 0xface3412u || !paging_cross_entries(state.machine,
        TEST_PAGE_DIRECTORY, TEST_PAGE_TABLE + 3u * 4u,
        TEST_PAGE_TABLE + 4u * 4u, pde | TEST_PAGE_ACCESSED,
        first | TEST_PAGE_ACCESSED, second | TEST_PAGE_ACCESSED);
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_cross_prepare(&state, push, sizeof(push),
            TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, 0)) {
        core_machine_destroy(state.machine);
        return 1;
    }
    if (!failed) failed |= !paging_write_u32(state.machine, TEST_PAGE_TABLE + 3u * 4u, first) ||
        !paging_write_u32(state.machine, TEST_PAGE_TABLE + 4u * 4u,
            TEST_STACK_PHYSICAL | TEST_PAGE_WRITABLE);
    if (!failed) failed |= core_machine_debug_write_register(state.machine,
        CORE_MACHINE_DEBUG_ESP, 0x00004001u) != LIB_STATUS_OK;
    if (!failed) failed |= !paging_cross_run(&state, 1, &result, &diagnostic);
    if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
    if (!failed) failed |= cpu.esp != 0x00004001u || cpu.eax != 0xfacebeefu ||
        cpu.eflags != 0x02u || !paging_cross_entries(state.machine,
        TEST_PAGE_DIRECTORY, TEST_PAGE_TABLE + 3u * 4u,
        TEST_PAGE_TABLE + 4u * 4u, pde | TEST_PAGE_ACCESSED, first,
        TEST_STACK_PHYSICAL | TEST_PAGE_WRITABLE);
    core_machine_destroy(state.machine);
    if (failed) return 1;
    return failed;
}

static lib_i32 paging_test_cross_fetch(void)
{
    static const lib_u8 first[] = { 0x66u };
    static const lib_u8 second[] = { 0xb8u, 0x34u, 0x12u, 0x00u, 0x00u };
    const lib_u32 pde = TEST_PAGE_TABLE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 code_first = TEST_PERMISSION_CODE | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    const lib_u32 code_second = TEST_CROSS_CODE_PHYSICAL | TEST_PAGE_PRESENT |
        TEST_PAGE_WRITABLE;
    paging_machine state;
    core_machine_run_result result = {0};
    core_machine_cpu_diagnostic diagnostic = {0};
    core_machine_debug_cpu_snapshot cpu = {0};
    lib_i32 failed;

    if (!paging_cross_prepare(&state, first, sizeof(first),
            TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, 0)) {
        core_machine_destroy(state.machine);
        return 1;
    }
    failed = !paging_write_u32(state.machine, TEST_PAGE_TABLE + 8u * 4u,
        code_second) || core_machine_memory_write(state.machine,
        TEST_PERMISSION_CODE + 0xfffu, first, sizeof(first)) != LIB_STATUS_OK ||
        core_machine_memory_write(state.machine, TEST_CROSS_CODE_PHYSICAL,
            second, sizeof(second)) != LIB_STATUS_OK;
    if (!failed) failed |= core_machine_debug_write_register(state.machine,
        CORE_MACHINE_DEBUG_EIP, 0x0fffu) != LIB_STATUS_OK;
    if (!failed) failed |= !paging_cross_run(&state, 0, &result, &diagnostic);
    if (!failed) failed |= diagnostic.first_fault.valid || !paging_cross_entries(state.machine,
        TEST_PAGE_DIRECTORY, TEST_PAGE_TABLE + 7u * 4u,
        TEST_PAGE_TABLE + 8u * 4u, pde | TEST_PAGE_ACCESSED,
        code_first | TEST_PAGE_ACCESSED, code_second | TEST_PAGE_ACCESSED);
    core_machine_destroy(state.machine);
    if (failed) return 1;

    if (!paging_cross_prepare(&state, first, sizeof(first),
            TEST_CROSS_DATA_SECOND | TEST_PAGE_PRESENT | TEST_PAGE_WRITABLE, 0)) {
        core_machine_destroy(state.machine);
        return 1;
    }
    if (!failed) failed |= !paging_write_u32(state.machine, TEST_PAGE_TABLE + 8u * 4u,
        TEST_CROSS_CODE_PHYSICAL | TEST_PAGE_WRITABLE) ||
        core_machine_memory_write(state.machine, TEST_PERMISSION_CODE + 0xfffu,
            first, sizeof(first)) != LIB_STATUS_OK;
    if (!failed) failed |= core_machine_debug_write_register(state.machine,
        CORE_MACHINE_DEBUG_EIP, 0x0fffu) != LIB_STATUS_OK;
    if (!failed) failed |= !paging_cross_run(&state, 1, &result, &diagnostic);
    if (!failed) failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
        CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK;
    if (!failed) failed |= cpu.eip != 0x0fffu ||
        !paging_cross_entries(state.machine, TEST_PAGE_DIRECTORY,
            TEST_PAGE_TABLE + 7u * 4u, TEST_PAGE_TABLE + 8u * 4u,
            pde | TEST_PAGE_ACCESSED,
            code_first | TEST_PAGE_ACCESSED, TEST_CROSS_CODE_PHYSICAL | TEST_PAGE_WRITABLE);
    core_machine_destroy(state.machine);
    if (failed) return 1;
    return failed;
}

static lib_i32 paging_test_cross_page(void)
{
    return paging_test_cross_data() || paging_test_cross_stack() ||
        paging_test_cross_fetch();
}

lib_i32 main(void)
{
    const lib_i32 valid = paging_test_valid_path();
    const lib_i32 delivered = paging_test_delivered_page_fault();
    const lib_i32 faults = paging_test_page_faults();
    const lib_i32 cr3_reload = paging_test_cr3_directory_reload();
    const lib_i32 no_stale_translation = paging_test_no_stale_translation();
    const lib_i32 permissions = paging_test_permissions();
    const lib_i32 cross_page = paging_test_cross_page();

    if (valid || delivered || faults ||
        cr3_reload || no_stale_translation || permissions || cross_page) {
        lib_c_fprintf(lib_c_stderr,
            "I386-PAGING:FAIL valid=%d delivered=%d faults=%d cr3=%d stale=%d permissions=%d cross=%d\n",
            valid, delivered, faults,
            cr3_reload, no_stale_translation, permissions, cross_page);
        return 1;
    }
    lib_c_printf("I386-PAGING:OK\n");
    lib_c_printf("I386-PAGING:CORPUS:OK\n");
    lib_c_printf("PAGING-PERMISSIONS:OK\n");
    lib_c_printf("CROSS-PAGE:OK\n");
    lib_c_printf("CR0-PAGING-CONTROL:OK\n");
    lib_c_printf("CR2-CR3-TRANSLATION:OK\n");
    lib_c_printf("PAGING-CLOSURE:OK\n");
    return 0;
}

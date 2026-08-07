#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"

#define GDT_PTR 0x0100u
#define IDT_PTR 0x0110u
#define GDT_BASE 0x0300u
#define IDT_BASE 0x0400u
#define TSS_BASE 0x0600u
#define KERNEL_BASE 0x2000u
#define USER_CODE_BASE 0x4000u
#define USER_DATA_BASE 0x5000u

typedef struct privilege_machine {
    core_machine *machine;
    t_cpu *cpu;
    core_machine_cpu_execution_context *execution;
} privilege_machine;

static C_VOID privilege_reset(C_VOID *opaque)
{
    privilege_machine *state = (privilege_machine *)opaque;

    if (state == STD_NULL || state->cpu == STD_NULL ||
        state->execution == STD_NULL) return;
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.cs, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.ds, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.es, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.ss, 0u);
    state->cpu->data.eip = 0u;
}

static const core_machine_execution_provider privilege_provider = {
    privilege_reset, STD_NULL, STD_NULL
};

static C_INT privilege_prepare(privilege_machine *state,
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
    state->cpu = core_machine_configuration_cpu_borrow(state->machine);
    state->execution = core_machine_configuration_cpu_execution_borrow(state->machine);
    if (state->cpu == STD_NULL || state->execution == STD_NULL ||
        core_machine_bind_execution_provider(state->machine, &privilege_provider,
            state) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT write_bytes(core_machine *machine, uint32_t address,
    const uint8_t *bytes, STD_SIZE_T count)
{
    return core_machine_memory_write(machine, address, bytes, count) ==
        TYPE_STATUS_OK;
}

static C_INT privilege_install(privilege_machine *state, C_INT fault_delivery)
{
    static const uint8_t gdt_pointer[] = { 0x2fu, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u };
    static const uint8_t idt_pointer[] = { 0x97u, 0x01u, 0x00u, 0x04u, 0x00u, 0x00u };
    static const uint8_t gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x40,0,0xfa,0,0,
        0xff,0xff,0,0x50,0,0xf2,0,0,
        0x2b,0,0,0x06,0,0x81,0,0
    };
    uint8_t idt[0x198u] = {0};
    static const uint8_t real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0x0f,0x01,0x1e,0x10,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x28,0x00,0x0f,0x00,0xd8,
        0xb8,0x10,0x00,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    static const uint8_t kernel_entry[] = {
        0xb8,0x10,0x00,0x8e,0xd8,
        0xb8,0x23,0x00,0x50,
        0xb8,0x00,0xa0,0x50,
        0xb8,0x02,0x02,0x50,
        0xb8,0x1b,0x00,0x50,
        0xb8,0x00,0x00,0x50,
        0xb8,0x23,0x00,0x8e,0xd8,
        0xcf
    };
    static const uint8_t kernel_handler[] = {
        0xb8,0x11,0x11,0xa3,0x00,0x00,0xcf
    };
    static const uint8_t kernel_stop[] = { 0xf4 };
    static const uint8_t kernel_fault[] = {
        0xb8,0x33,0x33,0xa3,0x04,0x00,0xf4
    };
    static const uint8_t user_code[] = {
        0xcd,0x30,0xb8,0x22,0x22,0xa3,0x02,0x00,0xcd,0x31
    };
    static const uint8_t user_fault_code[] = { 0xcd,0x32 };
    uint16_t sp0 = 0x9000u;
    uint16_t ss0 = 0x0010u;

    idt[0x180u] = 0x00u;
    idt[0x181u] = 0x01u;
    idt[0x182u] = 0x08u;
    idt[0x185u] = 0xe6u;
    idt[0x188u] = 0x10u;
    idt[0x189u] = 0x01u;
    idt[0x18au] = 0x08u;
    idt[0x18du] = 0xe6u;
    if (fault_delivery) {
        idt[0x68u] = 0x20u;
        idt[0x69u] = 0x01u;
        idt[0x6au] = 0x08u;
        idt[0x6du] = 0x86u;
        idt[0x190u] = 0x10u;
        idt[0x191u] = 0x01u;
        idt[0x192u] = 0x08u;
        idt[0x195u] = 0x86u;
    }
    return write_bytes(state->machine, GDT_PTR, gdt_pointer, sizeof(gdt_pointer)) &&
        write_bytes(state->machine, IDT_PTR, idt_pointer, sizeof(idt_pointer)) &&
        write_bytes(state->machine, GDT_BASE, gdt, sizeof(gdt)) &&
        write_bytes(state->machine, IDT_BASE, idt, sizeof(idt)) &&
        write_bytes(state->machine, TSS_BASE + 2u, (const uint8_t *)&sp0, sizeof(sp0)) &&
        write_bytes(state->machine, TSS_BASE + 4u, (const uint8_t *)&ss0, sizeof(ss0)) &&
        write_bytes(state->machine, 0u, real_code, sizeof(real_code)) &&
        write_bytes(state->machine, KERNEL_BASE, kernel_entry, sizeof(kernel_entry)) &&
        write_bytes(state->machine, KERNEL_BASE + 0x100u, kernel_handler,
            sizeof(kernel_handler)) &&
        write_bytes(state->machine, KERNEL_BASE + 0x110u, kernel_stop,
            sizeof(kernel_stop)) &&
        write_bytes(state->machine, KERNEL_BASE + 0x120u, kernel_fault,
            sizeof(kernel_fault)) &&
        write_bytes(state->machine, USER_CODE_BASE,
            fault_delivery ? user_fault_code : user_code,
            fault_delivery ? sizeof(user_fault_code) : sizeof(user_code));
}

static C_INT privilege_test_fault_delivery(core_machine_cpu_profile profile)
{
    privilege_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    uint16_t marker = 0u;
    const core_machine_run_budget budget = { 1024u, 0u };
    C_INT failed = !privilege_prepare(&state, profile);

    if (!failed) {
        failed |= !privilege_install(&state, 1);
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_memory_read(state.machine, USER_DATA_BASE + 4u,
            &marker, sizeof(marker)) != TYPE_STATUS_OK || marker != 0x3333u;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 1u ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_GP) ||
            diagnostic.last_delivered_exception.exception_code != 0x0192u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

int main(void)
{
    privilege_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    uint16_t markers[2] = {0u, 0u};
    const core_machine_run_budget budget = { 1024u, 0u };
    C_INT failed = !privilege_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286);

    if (!failed) {
        failed |= !privilege_install(&state, 0);
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_memory_read(state.machine, USER_DATA_BASE, markers,
            sizeof(markers)) != TYPE_STATUS_OK || markers[0] != 0x1111u ||
            markers[1] != 0x2222u;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 0u;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T259 result=%u markers=%04x/%04x fault=%d delivered=%d/%u cs=%04x sp=%04x\n",
                (unsigned)result.reason, markers[0], markers[1],
                diagnostic.first_fault.valid,
                diagnostic.last_delivered_exception.valid,
                diagnostic.delivered_exception_count, state.cpu->data.cs.selector,
                state.cpu->data.sp);
        }
    }
    core_machine_destroy(state.machine);
    failed |= privilege_test_fault_delivery(CORE_MACHINE_CPU_PROFILE_80286);
    failed |= privilege_test_fault_delivery(CORE_MACHINE_CPU_PROFILE_80386);
    if (failed) return 1;
    STD_PRINTF("M5:T259:S2:PROTECTED-PRIVILEGE:OK\n");
    STD_PRINTF("M5:T259:S3:PROTECTED-PRIVILEGE:CORPUS:OK\n");
    return 0;
}

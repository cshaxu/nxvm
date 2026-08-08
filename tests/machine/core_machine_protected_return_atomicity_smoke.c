#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"

#define ATOMIC_GDT_POINTER 0x0100u
#define ATOMIC_IDT_POINTER 0x0110u
#define ATOMIC_GDT_BASE 0x0300u
#define ATOMIC_IDT_BASE 0x0400u
#define ATOMIC_KERNEL_BASE 0x2000u
#define ATOMIC_USER_CODE_ACCESS (ATOMIC_GDT_BASE + 29u)

typedef struct atomic_machine {
    core_machine *machine;
    t_cpu *cpu;
    core_machine_cpu_execution_context *execution;
} atomic_machine;

static C_VOID atomic_reset(C_VOID *opaque)
{
    atomic_machine *state = (atomic_machine *)opaque;

    if (state == STD_NULL || state->cpu == STD_NULL || state->execution == STD_NULL) return;
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

static const core_machine_execution_provider atomic_provider = {
    atomic_reset, STD_NULL, STD_NULL
};

static C_INT atomic_prepare(atomic_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK) return 0;
    state->cpu = core_machine_configuration_cpu_borrow(state->machine);
    state->execution = core_machine_configuration_cpu_execution_borrow(state->machine);
    if (state->cpu == STD_NULL || state->execution == STD_NULL ||
        core_machine_bind_execution_provider(state->machine, &atomic_provider, state) !=
            TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT atomic_write(atomic_machine *state, uint32_t address,
    const uint8_t *bytes, STD_SIZE_T byte_count)
{
    return core_machine_memory_write(state->machine, address, bytes, byte_count) ==
        TYPE_STATUS_OK;
}

static C_INT atomic_install(atomic_machine *state, C_INT use_iret)
{
    static const uint8_t gdt_pointer[] = { 0x1fu, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u };
    static const uint8_t idt_pointer[] = { 0x6fu, 0x00u, 0x00u, 0x04u, 0x00u, 0x00u };
    static const uint8_t gdt[] = {
        0,0,0,0,0,0,0,0,
        0xff,0xff,0,0x20,0,0x9a,0,0,
        0xff,0xff,0,0x30,0,0x92,0,0,
        0xff,0xff,0,0x40,0,0xfa,0,0
    };
    uint8_t idt[0x70u] = {0};
    static const uint8_t real_code[] = {
        0x0f,0x01,0x16,0x00,0x01,
        0x0f,0x01,0x1e,0x10,0x01,
        0xb8,0x01,0x00,0x0f,0x01,0xf0,
        0xb8,0x10,0x00,0x8e,0xd8,0x8e,0xd0,0xbc,0x00,0x80,
        0xea,0x00,0x00,0x08,0x00
    };
    static const uint8_t retf_code[] = {
        0xb8,0x00,0x00,0x50,0xb8,0x00,0xa0,0x50,
        0xb8,0x1b,0x00,0x50,0xb8,0x00,0x00,0x50,0xcb
    };
    static const uint8_t iret_code[] = {
        0xb8,0x00,0x00,0x50,0xb8,0x00,0xa0,0x50,
        0xb8,0x02,0x02,0x50,0xb8,0x1b,0x00,0x50,
        0xb8,0x00,0x00,0x50,0xcf
    };
    static const uint8_t fault_handler[] = { 0xf4 };
    const uint8_t *kernel_code = use_iret ? iret_code : retf_code;
    STD_SIZE_T kernel_code_bytes = use_iret ? sizeof(iret_code) : sizeof(retf_code);

    idt[0x68u] = 0x00u;
    idt[0x69u] = 0x01u;
    idt[0x6au] = 0x08u;
    idt[0x6du] = 0x86u;
    return atomic_write(state, ATOMIC_GDT_POINTER, gdt_pointer, sizeof(gdt_pointer)) &&
        atomic_write(state, ATOMIC_IDT_POINTER, idt_pointer, sizeof(idt_pointer)) &&
        atomic_write(state, ATOMIC_GDT_BASE, gdt, sizeof(gdt)) &&
        atomic_write(state, ATOMIC_IDT_BASE, idt, sizeof(idt)) &&
        atomic_write(state, 0u, real_code, sizeof(real_code)) &&
        atomic_write(state, ATOMIC_KERNEL_BASE, kernel_code, kernel_code_bytes) &&
        atomic_write(state, ATOMIC_KERNEL_BASE + 0x100u, fault_handler,
            sizeof(fault_handler));
}

static C_INT atomic_test_outer_return(C_INT use_iret)
{
    atomic_machine state;
    core_machine_run_result result;
    const core_machine_run_budget budget = { 128u, 0u };
    uint8_t accessed = 0u;
    uint8_t initial_accessed = 0u;
    uint16_t expected_sp = use_iret ? 0x7feeu : 0x7ff0u;
    C_INT failed = !atomic_prepare(&state);

    if (!failed) {
        failed |= !atomic_install(&state, use_iret);
        failed |= core_machine_memory_read(state.machine, ATOMIC_USER_CODE_ACCESS,
            &initial_accessed, sizeof(initial_accessed)) != TYPE_STATUS_OK ||
            initial_accessed != 0xfau;
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_memory_read(state.machine, ATOMIC_USER_CODE_ACCESS,
            &accessed, sizeof(accessed)) != TYPE_STATUS_OK || accessed != 0xfau;
        failed |= state.cpu->data.cs.selector != 0x0008u ||
            state.cpu->data.ss.selector != 0x0010u || state.cpu->data.sp != expected_sp;
        if (failed) {
            STD_FPRINTF(STD_STDERR,
                "T293 return=%s reason=%u access=%02x/%02x cs=%04x ss=%04x sp=%04x expected=%04x\n",
                use_iret ? "iret" : "retf", (unsigned)result.reason, initial_accessed, accessed,
                state.cpu->data.cs.selector,
                state.cpu->data.ss.selector, state.cpu->data.sp, expected_sp);
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

C_INT main(C_VOID)
{
    if (atomic_test_outer_return(0) || atomic_test_outer_return(1)) return 1;
    STD_PRINTF("M5:T293:S1:PROTECTED-RETURN-ATOMICITY:OK\n");
    return 0;
}

#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

/* Private pending-event and whole-cache invariants belong to the CPU owner;
 * the board interrupt corpus separately checks PIC, frames and transactions. */
static void cpu_interrupt_prepare(cpu_bus_fixture *fixture, lib_u8 vector,
    lib_u8 gate_type)
{
    const lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0xfau,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0
    };
    const lib_u8 gate[] = {0u,1u,0x0bu,0u,0u,gate_type,0u,0u};
    t_cpu *cpu = &fixture->cpu;

    cpu_bus_prepare(fixture, CORE_MACHINE_CPU_PROFILE_80386);
    lib_memory_copy(fixture->memory + 0x300u, gdt, sizeof(gdt));
    lib_memory_copy(fixture->memory + 0x400u + vector * 8u, gate, sizeof(gate));
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = 0x300u;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.idtr.flagValid = LIB_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = 0x400u;
    cpu->data.idtr.limit = 0x187u;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = 0x0bu;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = 0x2000u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = 3u;
    cpu->data.cs.seg.accessed = LIB_FALSE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.defsize = LIB_TRUE;
    cpu->data.cs.seg.exec.conform = LIB_FALSE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = 0x10u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffffffu;
    cpu->data.ss.dpl = 0u;
    cpu->data.ss.seg.accessed = LIB_FALSE;
    cpu->data.ss.seg.executable = LIB_FALSE;
    cpu->data.ss.seg.data.big = LIB_TRUE;
    cpu->data.ss.seg.data.expdown = LIB_FALSE;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.eip = 0u;
    cpu->data.esp = 0x8000u;
    cpu->data.eflags = 0x302u;
    cpu->data.flagHalt = LIB_FALSE;
}

/* Preserve the full private-cache comparisons from the board UD corpus;
 * a copied public snapshot deliberately does not expose cache bookkeeping. */
static lib_i32 cpu_ud_cache_preservation(void)
{
    static const lib_u8 forms[][3] = {
        {0xf1u, 0u, 0u}, {0x0fu, 0x01u, 0xf8u},
        {0x0fu, 0x25u, 0xc0u}, {0x62u, 0xc0u, 0u},
        {0xf0u, 0x90u, 0u}
    };

    for (lib_size index = 0u; index < sizeof(forms) / sizeof(forms[0]); ++index) {
        for (lib_u8 reject = 0u; reject != 2u; ++reject) {
            cpu_bus_fixture fixture;
            t_cpu before;

            cpu_interrupt_prepare(&fixture, 6u, reject ? 0x80u : 0x8eu);
            /* The original UD cases use ring-zero 16-bit protected code. */
            fixture.memory[0x30du] = 0x9au;
            fixture.memory[0x30eu] = 0u;
            fixture.memory[0x432u] = 0x08u;
            fixture.cpu.data.cs.selector = 0x08u;
            fixture.cpu.data.cs.dpl = 0u;
            fixture.cpu.data.cs.seg.exec.defsize = LIB_FALSE;
            fixture.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF |
                VCPU_EFLAGS_DF;
            lib_memory_copy(fixture.memory, forms[index], sizeof(forms[index]));
            before = fixture.cpu;
            core_machine_cpu_execution_refresh(&fixture.execution);
            if (lib_memory_compare(&fixture.cpu.data.es, &before.data.es,
                    sizeof(before.data.es)) ||
                lib_memory_compare(&fixture.cpu.data.ss, &before.data.ss,
                    sizeof(before.data.ss)) ||
                lib_memory_compare(&fixture.cpu.data.ds, &before.data.ds,
                    sizeof(before.data.ds)) ||
                lib_memory_compare(&fixture.cpu.data.fs, &before.data.fs,
                    sizeof(before.data.fs)) ||
                lib_memory_compare(&fixture.cpu.data.gs, &before.data.gs,
                    sizeof(before.data.gs))) return 1;
            if (reject) {
                if (!fixture.faults || fixture.fault.exception_mask != VCPUINS_EXCEPT_UD ||
                    fixture.cpu.data.eip != before.data.eip ||
                    fixture.cpu.data.esp != before.data.esp ||
                    fixture.cpu.data.eflags != before.data.eflags) return 1;
            } else if (fixture.faults || fixture.cpu.data.eip != 0x100u ||
                fixture.cpu.data.esp != before.data.esp - 12u) return 1;
        }
    }
    return 0;
}

static lib_i32 cpu_interrupt_pending_and_rollback(void)
{
    for (lib_u8 reject = 0u; reject != 2u; ++reject) {
        cpu_bus_fixture fixture;
        t_cpu before;

        cpu_interrupt_prepare(&fixture, 2u, reject ? 0x80u : 0x8eu);
        fixture.memory[0] = 0x90u;
        fixture.cpu.data.eflags = 0x202u;
        if (!core_machine_cpu_request_nmi(&fixture.execution)) return 1;
        before = fixture.cpu;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.nmi_pending != reject) return 1;
        if (reject) {
            if (!fixture.faults || fixture.cpu.data.esp != before.data.esp ||
                fixture.cpu.data.eflags != before.data.eflags ||
                lib_memory_compare(&fixture.cpu.data.cs, &before.data.cs,
                    sizeof(before.data.cs)) ||
                lib_memory_compare(&fixture.cpu.data.ss, &before.data.ss,
                    sizeof(before.data.ss))) return 1;
        } else if (fixture.faults || fixture.cpu.data.esp != 0x7ff4u ||
            fixture.cpu.data.eip != 0x100u) return 1;
    }
    for (lib_u8 failure = 0u; failure != 4u; ++failure) {
        cpu_bus_fixture fixture;
        t_cpu before;
        const lib_u8 code[] = {0x0fu, 0x01u, 0xf0u};
        const lib_u8 gate = failure == 0u ? 0x80u :
            failure == 1u ? 0x0eu : 0x8eu;

        cpu_interrupt_prepare(&fixture, 0x0du, gate);
        lib_memory_copy(fixture.memory, code, sizeof(code));
        if (failure == 2u) fixture.memory[0x30du] = 0x7au;
        if (failure == 3u) fixture.cpu.data.ss.limit = 0x7ffeu;
        before = fixture.cpu;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (!fixture.faults || !(fixture.fault.exception_mask & VCPUINS_EXCEPT_DF) ||
            fixture.fault.exception_code != 0u ||
            fixture.cpu.data.eip != before.data.eip ||
            fixture.cpu.data.esp != before.data.esp ||
            fixture.cpu.data.eflags != before.data.eflags ||
            lib_memory_compare(&fixture.cpu.data.cs, &before.data.cs,
                sizeof(before.data.cs)) ||
            lib_memory_compare(&fixture.cpu.data.ss, &before.data.ss,
                sizeof(before.data.ss))) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    if (cpu_ud_cache_preservation() || cpu_interrupt_pending_and_rollback()) return 1;
    lib_c_printf("%s\n", "M5:T539:S91:CPU-EXECUTION-FAULT-EVENT:OK");
    return 0;
}

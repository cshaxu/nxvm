#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

static lib_i32 cpu_80186_lgdt_gate(void)
{
    static const lib_u8 program[] = { 0x0fu, 0x01u, 0x16u, 0x00u, 0x01u };
    static const lib_u8 gdtr[] = { 0x17u, 0u, 0u, 0x03u, 0u, 0u };
    static const lib_u8 gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0x30u, 0u, 0x92u, 0u, 0u
    };
    cpu_bus_fixture fixture;

    cpu_bus_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80186);
    fixture.cpu.data.eip = 0u;
    fixture.cpu.data.idtr.limit = 0x03ffu;
    lib_memory_copy(fixture.memory, program, sizeof(program));
    lib_memory_copy(fixture.memory + 0x0100u, gdtr, sizeof(gdtr));
    lib_memory_copy(fixture.memory + 0x0300u, gdt, sizeof(gdt));
    core_machine_cpu_execution_refresh(&fixture.execution);
    return fixture.faults != 0u || fixture.execution.stop_requested ||
        fixture.delivered_exceptions != 1u ||
        !fixture.delivered_exception.valid ||
        fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_UD ||
        fixture.delivered_exception.point.eip != 0u;
}

static lib_i32 cpu_paging_prepare(cpu_bus_fixture *fixture,
    core_machine_cpu_profile profile, const lib_u8 *code, lib_size bytes)
{
    cpu_bus_prepare(fixture, profile);
    core_machine_cpu_state_reset(&fixture->execution);
    if (bytes > sizeof(fixture->memory) ||
        core_machine_cpu_execution_load_segment(&fixture->execution,
            &fixture->cpu.data.cs, 0u) ||
        core_machine_cpu_execution_load_segment(&fixture->execution,
            &fixture->cpu.data.ds, 0u) ||
        core_machine_cpu_execution_load_segment(&fixture->execution,
            &fixture->cpu.data.es, 0u) ||
        core_machine_cpu_execution_load_segment(&fixture->execution,
            &fixture->cpu.data.ss, 0u)) return 0;
    fixture->cpu.data.eip = 0u;
    fixture->cpu.data.idtr.limit = 0x03ffu;
    lib_memory_copy(fixture->memory, code, bytes);
    return 1;
}

static lib_i32 cpu_paging_control_gate(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_size bytes)
{
    cpu_bus_fixture fixture;

    if (!cpu_paging_prepare(&fixture, profile, code, bytes)) return 1;
    for (lib_u32 step = 0u; step < 128u &&
            fixture.delivered_exceptions == 0u; ++step)
        core_machine_cpu_execution_refresh(&fixture.execution);
    return fixture.faults != 0u || fixture.execution.stop_requested ||
        fixture.delivered_exceptions != 1u ||
        !fixture.delivered_exception.valid ||
        fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_UD;
}

static lib_i32 cpu_paging_control_forms(void)
{
    static const lib_u8 write_reserved_cr1[] = {
        0x66u, 0xb8u, 0x34u, 0x12u, 0x00u, 0x00u, 0x0fu, 0x22u, 0xc8u
    };
    static const lib_u8 pg_without_pe[] = {
        0x66u, 0xb8u, 0x00u, 0x00u, 0x00u, 0x80u, 0x0fu, 0x22u, 0xc0u
    };
    static const lib_u8 unaligned_cr3[] = {
        0x66u, 0xb8u, 0x34u, 0x12u, 0x00u, 0x00u, 0x0fu, 0x22u, 0xd8u
    };
    static const lib_u8 read_cr0[] = { 0x0fu, 0x20u, 0xc0u };
    lib_i32 failed = 0;

    failed |= cpu_paging_control_gate(CORE_MACHINE_CPU_PROFILE_80386,
        write_reserved_cr1, sizeof(write_reserved_cr1));
    failed |= cpu_paging_control_gate(CORE_MACHINE_CPU_PROFILE_80386,
        pg_without_pe, sizeof(pg_without_pe));
    failed |= cpu_paging_control_gate(CORE_MACHINE_CPU_PROFILE_80386,
        unaligned_cr3, sizeof(unaligned_cr3));
    failed |= cpu_paging_control_gate(CORE_MACHINE_CPU_PROFILE_80286,
        read_cr0, sizeof(read_cr0));
    failed |= cpu_paging_control_gate(CORE_MACHINE_CPU_PROFILE_80186,
        read_cr0, sizeof(read_cr0));
    return failed;
}

static lib_i32 cpu_paging_invlpg_case(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_size bytes)
{
    cpu_bus_fixture fixture;
    t_cpu before;

    if (!cpu_paging_prepare(&fixture, profile, code, bytes)) return 1;
    fixture.cpu.data.eax = 0x11223344u;
    fixture.cpu.data.ecx = 0x55667788u;
    fixture.cpu.data.edx = 0x99aabbccu;
    fixture.cpu.data.ebx = 0xddeeff00u;
    fixture.cpu.data.esi = 0x13579bdfu;
    fixture.cpu.data.edi = 0x2468ace0u;
    fixture.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
    before = fixture.cpu;
    core_machine_cpu_execution_refresh(&fixture.execution);
    return fixture.faults != 0u || fixture.execution.stop_requested ||
        fixture.delivered_exceptions != 1u ||
        !fixture.delivered_exception.valid ||
        fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_UD ||
        fixture.delivered_exception.exception_code != 0u ||
        fixture.delivered_exception.point.cs != 0u ||
        fixture.delivered_exception.point.linear_pc != 0u ||
        fixture.cpu.data.eax != before.data.eax ||
        fixture.cpu.data.ecx != before.data.ecx ||
        fixture.cpu.data.edx != before.data.edx ||
        fixture.cpu.data.ebx != before.data.ebx ||
        fixture.cpu.data.esi != before.data.esi ||
        fixture.cpu.data.edi != before.data.edi;
}

static lib_i32 cpu_paging_invlpg_rejection(void)
{
    static const lib_u8 invlpg[] = { 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u };
    static const lib_u8 operand_size[] = { 0x66u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u };
    static const lib_u8 address_size[] = { 0x67u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u };
    static const lib_u8 combined_size[] = { 0x66u, 0x67u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u };
    static const lib_u8 locked[] = { 0xf0u, 0x0fu, 0x01u, 0x3eu, 0x00u, 0x20u };
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80186, invlpg, sizeof(invlpg))) return 1;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80286, invlpg, sizeof(invlpg))) return 2;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80386, invlpg, sizeof(invlpg))) return 3;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80386, operand_size, sizeof(operand_size))) return 4;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80386, address_size, sizeof(address_size))) return 5;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80386, combined_size, sizeof(combined_size))) return 6;
    if (cpu_paging_invlpg_case(CORE_MACHINE_CPU_PROFILE_80386, locked, sizeof(locked))) return 7;
    return 0;
}

static lib_i32 cpu_paging_cr0_mutable_controls(void)
{
    static const lib_u8 write_mutable[] = {
        0x66u, 0xb8u, 0x1eu, 0x00u, 0x00u, 0x00u, 0x0fu, 0x22u, 0xc0u,
        0x0fu, 0x20u, 0xc1u, 0xf4u
    };
    const lib_u32 mutable = VCPU_CR0_MP | VCPU_CR0_EM | VCPU_CR0_TS | VCPU_CR0_ET;
    cpu_bus_fixture fixture;
    t_cpu before;
    t_cpu cpu;
    lib_i32 failed = !cpu_paging_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386,
        write_mutable, sizeof(write_mutable));

    if (!failed) {
        before = fixture.cpu;
        for (lib_u32 step = 0u; step < 4u && fixture.faults == 0u; ++step)
            core_machine_cpu_execution_refresh(&fixture.execution);
        cpu = fixture.cpu;
        failed |= fixture.faults != 0u || !cpu.data.flagHalt || cpu.data.cr0 != mutable ||
            cpu.data.eax != mutable || cpu.data.ecx != mutable ||
            cpu.data.eip != sizeof(write_mutable) || cpu.data.eflags != 0x02u ||
            cpu.data.ebx != 0u || cpu.data.edx != 0x00000300u || cpu.data.esp != 0u ||
            cpu.data.ebp != 0u || cpu.data.esi != 0u || cpu.data.edi != 0u ||
            lib_memory_compare(&cpu.data.es, &before.data.es, sizeof(cpu.data.es)) != 0 ||
            lib_memory_compare(&cpu.data.cs, &before.data.cs, sizeof(cpu.data.cs)) != 0 ||
            lib_memory_compare(&cpu.data.ss, &before.data.ss, sizeof(cpu.data.ss)) != 0 ||
            lib_memory_compare(&cpu.data.ds, &before.data.ds, sizeof(cpu.data.ds)) != 0 ||
            lib_memory_compare(&cpu.data.fs, &before.data.fs, sizeof(cpu.data.fs)) != 0 ||
            lib_memory_compare(&cpu.data.gs, &before.data.gs, sizeof(cpu.data.gs)) != 0;
    }
    core_machine_cpu_execution_finalize(&fixture.execution);
    return failed;
}

lib_i32 main(void)
{
    lib_i32 result = 0;

    result |= cpu_80186_lgdt_gate();
    result |= cpu_paging_control_forms();
    result |= cpu_paging_cr0_mutable_controls();
    result |= cpu_paging_invlpg_rejection();
    if (result != 0) return 1;
    lib_c_printf("%s\n", "CPU-EXECUTION-PAGING:OK");
    return 0;
}

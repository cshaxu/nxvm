#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

typedef struct alignment_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 reads;
    lib_u32 writes;
    lib_u32 acknowledgements;
    lib_bool interrupt;
} alignment_fixture;

static lib_status alignment_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    alignment_fixture *fixture = opaque;
    if (address >= 0x9000u && address < 0xa000u) ++fixture->reads;
    return cpu_instruction_read(&fixture->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
}

static lib_status alignment_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    alignment_fixture *fixture = opaque;
    if (address >= 0x9000u && address < 0xa000u) ++fixture->writes;
    return cpu_instruction_write(&fixture->instruction, address, source, bytes, provenance, cache_control);
}

static lib_bool alignment_pending(void *opaque)
{
    return ((alignment_fixture *)opaque)->interrupt;
}

static lib_status alignment_acknowledge(void *opaque, lib_u8 *vector)
{
    alignment_fixture *fixture = opaque;
    fixture->interrupt = LIB_FALSE;
    ++fixture->acknowledgements;
    *vector = 0x20u;
    return LIB_STATUS_OK;
}

static const x86_core_cpu_bus_provider alignment_bus = {
    .read_memory = alignment_read,
    .write_memory = alignment_write,
    .interrupt_pending = alignment_pending,
    .acknowledge_interrupt = alignment_acknowledge
};

static void alignment_prepare(alignment_fixture *fixture, lib_bool vm86)
{
    static const lib_u8 descriptors[] = {
        0xffu,0xffu,0u,0u,0u,0x9bu,0u,0u,
        0xffu,0xffu,0u,0u,0u,0x93u,0u,0u,
        0xffu,0xffu,0u,0u,0u,0xfbu,0u,0u,
        0xffu,0xffu,0u,0u,0u,0xf3u,0u,0u
    };
    static const lib_u8 gate[] = {0u,1u,8u,0u,0u,0x8eu,0u,0u};
    const lib_u32 esp0 = 0x7000u;
    const lib_u16 ss0 = 0x10u;
    cpu_instruction_fixture *instruction = &fixture->instruction;
    t_cpu *cpu = &instruction->cpu;

    lib_memory_set(fixture, 0, sizeof(*fixture));
    cpu_instruction_prepare_with_bus(instruction, X86_CORE_CPU_PROFILE_80486SX,
        &alignment_bus, fixture);
    cpu->data.cr0 |= VCPU_CR0_PE | VCPU_CR0_AM;
    cpu->data.eflags = VCPU_EFLAGS_AC | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_CF |
        (vm86 ? VCPU_EFLAGS_VM : 0u);
    cpu->data.cs.selector = vm86 ? 0u : 0x1bu;
    cpu->data.ds.selector = vm86 ? 0u : 0x23u;
    cpu->data.es.selector = vm86 ? 0u : 0x23u;
    cpu->data.ss.selector = vm86 ? 0u : 0x23u;
    cpu->data.cs.dpl = 3u;
    cpu->data.ds.dpl = 3u;
    cpu->data.es.dpl = 3u;
    cpu->data.ss.dpl = 3u;
    cpu->data.gdtr.base = 0x4001u;
    cpu->data.gdtr.limit = 0x27u;
    cpu->data.idtr.base = 0x5001u;
    cpu->data.idtr.limit = 0x7ffu;
    cpu->data.tr.selector = 0x28u;
    cpu->data.tr.base = 0x6001u;
    cpu->data.tr.limit = 0x67u;
    cpu->data.tr.flagValid = LIB_TRUE;
    cpu->data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    cpu->data.esp = 0x8000u;
    cpu->data.cr2 = 0x12345678u;
    lib_memory_copy(instruction->memory + 0x4009u, descriptors, sizeof(descriptors));
    lib_memory_copy(instruction->memory + 0x5089u, gate, sizeof(gate));
    lib_memory_copy(instruction->memory + 0x5069u, gate, sizeof(gate));
    lib_memory_copy(instruction->memory + 0x5071u, gate, sizeof(gate));
    lib_memory_copy(instruction->memory + 0x6005u, &esp0, sizeof(esp0));
    lib_memory_copy(instruction->memory + 0x6009u, &ss0, sizeof(ss0));
}

typedef struct alignment_form {
    lib_u8 code[7u];
    lib_u8 bytes;
    lib_u8 displacement;
    lib_u8 width;
    lib_bool pseudo;
} alignment_form;

static const alignment_form alignment_forms[] = {
    {{0xa1u,0u,0x90u},3u,1u,2u,LIB_FALSE},
    {{0x66u,0xa1u,0u,0x90u},4u,2u,4u,LIB_FALSE},
    {{0x50u},1u,0u,2u,LIB_FALSE},
    {{0x58u},1u,0u,2u,LIB_FALSE},
    {{0xadu},1u,0u,2u,LIB_FALSE},
    {{0xabu},1u,0u,2u,LIB_FALSE},
    {{0xafu},1u,0u,2u,LIB_FALSE},
    {{0xa5u},1u,0u,2u,LIB_FALSE},
    {{0xa7u},1u,0u,2u,LIB_FALSE},
    {{0x66u,0x62u,0x06u,0u,0x90u},5u,3u,4u,LIB_FALSE},
    {{0x66u,0xc5u,0x06u,0u,0x90u},5u,3u,4u,LIB_FALSE},
    {{0x0fu,0x01u,0x06u,0u,0x90u},5u,3u,4u,LIB_TRUE},
    {{0x0fu,0x01u,0x0eu,0u,0x90u},5u,3u,4u,LIB_TRUE},
    {{0xa0u,0u,0x90u},3u,1u,1u,LIB_FALSE},
    {{0x8du,0x06u,0u,0x90u},4u,2u,1u,LIB_FALSE},
    {{0x66u,0x50u},2u,0u,4u,LIB_FALSE},
    {{0x66u,0x58u},2u,0u,4u,LIB_FALSE},
    {{0x66u,0xadu},2u,0u,4u,LIB_FALSE},
    {{0x66u,0xabu},2u,0u,4u,LIB_FALSE},
    {{0x66u,0xafu},2u,0u,4u,LIB_FALSE},
    {{0x66u,0xa5u},2u,0u,4u,LIB_FALSE},
    {{0x66u,0xa7u},2u,0u,4u,LIB_FALSE},
    {{0x62u,0x06u,0u,0x90u},4u,2u,2u,LIB_FALSE},
    {{0xc5u,0x06u,0u,0x90u},4u,2u,2u,LIB_FALSE}
};

static lib_bool alignment_case(lib_size form, lib_u8 residue, lib_bool vm86)
{
    const alignment_form *definition = &alignment_forms[form];
    alignment_fixture fixture;
    cpu_instruction_fixture *instruction = &fixture.instruction;
    lib_u8 code[7u];
    lib_u8 before[8u];
    t_cpu after;
    const lib_u16 address = 0x9000u + residue;
    const lib_u16 selector = 0x23u;
    const lib_u8 opcode = definition->code[definition->code[0] == 0x66u ? 1u : 0u];
    lib_u32 stacked_error = 1u;
    const lib_bool fault = definition->pseudo ? (residue != 2u) :
        (residue & (definition->width - 1u)) != 0u;

    alignment_prepare(&fixture, vm86);
    lib_memory_copy(code, definition->code, sizeof(code));
    if (definition->displacement != 0u) code[definition->displacement] = residue;
    instruction->cpu.data.esi = address;
    instruction->cpu.data.edi = address;
    if (opcode == 0x50u) instruction->cpu.data.esp = address + definition->width;
    if (opcode == 0x58u) instruction->cpu.data.esp = address;
    if (opcode == 0xc5u)
        lib_memory_copy(instruction->memory + address + definition->width, &selector, 2u);
    lib_memory_copy(before, instruction->memory + address, sizeof(before));
    if (cpu_instruction_run(instruction, code, definition->bytes, &after) != LIB_STATUS_OK ||
        instruction->fault.valid || x86_core_cpu_is_shutdown(&instruction->execution) ||
        instruction->delivered_exception.valid != fault)
        return LIB_FALSE;
    if (!fault) return after.data.eip == definition->bytes;
    lib_memory_copy(&stacked_error, instruction->memory + after.data.esp, 4u);
    return instruction->delivered_exception.exception_mask == VCPUINS_EXCEPT_AC &&
        instruction->delivered_exception.exception_code == 0u &&
        instruction->delivered_exception.point.eip == 0u &&
        after.data.eip == 0x100u && after.data.cs.selector == 8u &&
        after.data.ss.selector == 0x10u && after.data.cr2 == 0x12345678u &&
        stacked_error == 0u && fixture.reads == 0u && fixture.writes == 0u &&
        lib_memory_compare(before, instruction->memory + address, sizeof(before)) == 0;
}

static lib_bool alignment_access_order(void)
{
    for (lib_u8 scenario = 0u; scenario < 7u; ++scenario) {
        alignment_fixture fixture;
        cpu_instruction_fixture *instruction = &fixture.instruction;
        const lib_bool pair = scenario >= 4u && scenario < 6u;
        const lib_u32 exception = scenario == 2u ? VCPUINS_EXCEPT_GP :
            scenario == 1u || scenario == 3u ? VCPUINS_EXCEPT_AC : VCPUINS_EXCEPT_PF;
        const lib_u32 error = exception == VCPUINS_EXCEPT_PF ?
            (scenario == 0u ? 6u : 4u) : 0u;
        lib_u8 code[5u] = {0xa5u};
        lib_u8 bytes = 1u;
        lib_u32 entry = 0x2007u;
        t_cpu after;

        alignment_prepare(&fixture, LIB_FALSE);
        instruction->cpu.data.cr0 |= VCPU_CR0_PG;
        instruction->cpu.data.cr3 = 0x1000u;
        instruction->cpu.data.esi = scenario == 2u ? 0x9001u : 0x9000u;
        instruction->cpu.data.edi = scenario == 1u || scenario == 3u ? 0xa001u : 0xa000u;
        if (scenario == 2u) instruction->cpu.data.es.seg.data.writable = LIB_FALSE;
        if (scenario == 3u) code[0] = 0xa7u;
        if (pair) {
            const lib_u8 pair_code[] = {0x66u, scenario == 4u ? 0x62u : 0xc5u,
                0x06u, 0xfcu, 0x9fu};
            lib_memory_copy(code, pair_code, sizeof(code));
            bytes = sizeof(code);
        }
        if (scenario == 6u) {
            code[0] = 0xf3u;
            code[1] = 0xa5u;
            bytes = 2u;
            instruction->cpu.data.ecx = 0u;
            instruction->cpu.data.esi = 0x9001u;
            instruction->cpu.data.edi = 0xa001u;
        }
        lib_memory_copy(instruction->memory + 0x1000u, &entry, 4u);
        for (lib_u32 page = 0u; page < 16u; ++page) {
            const lib_bool absent = page == (scenario == 1u || scenario == 3u ? 9u : 10u);
            entry = (page << 12u) | (absent ? 0u : 7u);
            lib_memory_copy(instruction->memory + 0x2000u + page * 4u, &entry, 4u);
        }
        if (cpu_instruction_run(instruction, code, bytes, &after) != LIB_STATUS_OK ||
            instruction->fault.valid || fixture.reads != 0u || fixture.writes != 0u ||
            instruction->delivered_exception.valid != (scenario != 6u))
            goto failed;
        if (scenario == 6u) {
            if (after.data.eip != 2u || after.data.esi != 0x9001u ||
                after.data.edi != 0xa001u || after.data.ecx != 0u)
                goto failed;
        } else if (instruction->delivered_exception.exception_mask != exception ||
            instruction->delivered_exception.exception_code != error ||
            instruction->delivered_exception.point.eip != 0u ||
            after.data.cr2 != (exception == VCPUINS_EXCEPT_PF ? 0xa000u : 0x12345678u))
            goto failed;
        continue;
failed:
        lib_c_printf("CPU-ALIGNMENT-ORDER scenario=%u exception=%x error=%u reads=%u writes=%u\n",
            scenario, instruction->delivered_exception.exception_mask,
            instruction->delivered_exception.exception_code, fixture.reads, fixture.writes);
        return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool alignment_immediate_fetch_order(void)
{
    static const lib_u8 opcodes[] = {0xc0u, 0xc1u, 0xa4u, 0xacu};

    for (lib_size form = 0u; form < sizeof(opcodes) / sizeof(opcodes[0]); ++form) {
        alignment_fixture fixture;
        cpu_instruction_fixture *instruction = &fixture.instruction;
        lib_u8 code[6u] = {0u};
        lib_u8 bytes = 0u;
        lib_u32 entry = 0x2007u;
        t_cpu after;
        const lib_bool extended = form >= 2u;

        alignment_prepare(&fixture, LIB_FALSE);
        instruction->cpu.data.cr0 |= VCPU_CR0_PG;
        instruction->cpu.data.cr3 = 0x1000u;
        if (extended) code[bytes++] = 0x0fu;
        code[bytes++] = opcodes[form];
        code[bytes++] = extended ? 0x06u : 0x26u;
        code[bytes++] = 0u;
        code[bytes++] = 0x90u;
        code[bytes++] = 1u;
        instruction->cpu.data.eip = 0x1001u - bytes;
        lib_memory_copy(instruction->memory + instruction->cpu.data.eip, code, bytes);
        lib_memory_copy(instruction->memory + 0x1000u, &entry, 4u);
        for (lib_u32 page = 0u; page < 16u; ++page) {
            entry = (page << 12u) | (page == 1u ? 0u : page == 9u ? 5u : 7u);
            lib_memory_copy(instruction->memory + 0x2000u + page * 4u, &entry, 4u);
        }
        x86_core_cpu_execution_refresh(&instruction->execution);
        after = instruction->cpu;
        if (instruction->execution.stop_requested || instruction->fault.valid ||
            !instruction->delivered_exception.valid ||
            instruction->delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
            instruction->delivered_exception.exception_code != 4u ||
            instruction->delivered_exception.point.eip != 0x1001u - bytes ||
            after.data.cr2 != 0x1000u || fixture.reads != 0u || fixture.writes != 0u) {
            lib_c_printf("CPU-IMMEDIATE-FETCH form=%zu exception=%x error=%u cr2=%x\n",
                form, instruction->delivered_exception.exception_mask,
                instruction->delivered_exception.exception_code, after.data.cr2);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool alignment_system_references(void)
{
    static const lib_u8 code[][5u] = {
        {0x90u}, {0x8eu,0xd8u}, {0x0fu,0x01u,0x16u,0x01u,0x90u}
    };
    static const lib_u8 lengths[] = {1u,2u,5u};
    static const lib_u8 table[] = {0x27u,0u,1u,0x40u,0u,0u};

    for (lib_u8 scenario = 0u; scenario < 3u; ++scenario) {
        alignment_fixture fixture;
        cpu_instruction_fixture *instruction = &fixture.instruction;
        alignment_prepare(&fixture, LIB_FALSE);
        instruction->cpu.data.eip = 1u;
        instruction->cpu.data.eax = 0x23u;
        if (scenario == 2u) {
            instruction->cpu.data.cs.selector = 8u;
            instruction->cpu.data.cs.dpl = 0u;
            instruction->cpu.data.ss.selector = 0x10u;
            instruction->cpu.data.ss.dpl = 0u;
            lib_memory_copy(instruction->memory + 0x9001u, table, sizeof(table));
        }
        lib_memory_copy(instruction->memory + 1u, code[scenario], lengths[scenario]);
        x86_core_cpu_execution_refresh(&instruction->execution);
        if (instruction->execution.stop_requested || instruction->fault.valid ||
            instruction->delivered_exception.valid ||
            instruction->cpu.data.eip != 1u + lengths[scenario] ||
            (scenario == 1u && instruction->cpu.data.ds.selector != 0x23u) ||
            (scenario == 2u && (instruction->cpu.data.gdtr.base != 0x4001u ||
                instruction->cpu.data.gdtr.limit != 0x27u))) {
            lib_c_printf("CPU-ALIGNMENT-SYSTEM scenario=%u\n", scenario);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool alignment_same_level_interrupt(void)
{
    for (lib_u8 source = 0u; source < 4u; ++source)
    for (lib_u8 gate32 = 0u; gate32 < 2u; ++gate32)
    for (lib_u8 residue = 0u; residue < 4u; ++residue)
    for (lib_u8 enabled = 0u; enabled < 2u; ++enabled)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        alignment_fixture fixture;
        lib_u8 gate[] = {0u, 2u, 0x18u, 0u, 0u, gate32 ? 0xeeu : 0xe6u, 0u, 0u};
        lib_u8 code[4], bytes = 0u;
        lib_u32 frame[6], flags;
        const lib_u32 old_sp = 0x9010u + residue;
        const lib_u8 width = gate32 ? 4u : 2u;
        const lib_bool fault = enabled && (residue & (width - 1u)) != 0u;
        const lib_bool hardware = source >= 2u;
        cpu_instruction_fixture *instruction = &fixture.instruction;

        alignment_prepare(&fixture, LIB_FALSE);
        instruction->cpu.data.eflags |= 2u;
        if (hardware) instruction->cpu.data.eflags |= VCPU_EFLAGS_IF;
        if (!enabled) instruction->cpu.data.eflags &= ~VCPU_EFLAGS_AC;
        instruction->cpu.data.esp = old_sp;
        flags = instruction->cpu.data.eflags;
        lib_memory_copy(instruction->memory + (source == 1u ? 0x5031u :
            source == 3u ? 0x5011u : 0x5101u),
            gate, sizeof(gate));
        lib_memory_set(instruction->memory + 0x9000u, 0xa5u, 32u);
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = hardware ? 0x90u : source == 0u ? 0xcdu : 0x0fu;
        if (!hardware) code[bytes++] = source == 0u ? 0x20u : 0x0bu;
        fixture.interrupt = source == 2u;
        if (source == 3u && !x86_core_cpu_request_nmi(&instruction->execution))
            return LIB_FALSE;
        lib_memory_copy(instruction->memory, code, bytes);
        x86_core_cpu_execution_refresh(&instruction->execution);
        if (instruction->execution.stop_requested || instruction->fault.valid ||
            fixture.acknowledgements != (source == 2u ? 1u : 0u)) return LIB_FALSE;
        if (fault) {
            if (!instruction->delivered_exception.valid ||
                instruction->delivered_exception.exception_mask != VCPUINS_EXCEPT_AC ||
                instruction->delivered_exception.exception_code != 0u ||
                instruction->delivered_exception.point.eip != (hardware ? bytes : 0u) ||
                instruction->cpu.data.eip != 0x100u || instruction->cpu.data.esp != 0x6fe8u ||
                fixture.writes != 0u) return LIB_FALSE;
            for (lib_u8 index = 0u; index < 32u; ++index)
                if (instruction->memory[0x9000u + index] != 0xa5u) return LIB_FALSE;
            lib_memory_copy(frame, instruction->memory + 0x6fe8u, sizeof(frame));
            if (frame[0] != 0u || frame[1] != (hardware ? bytes : 0u) || frame[2] != 0x1bu ||
                frame[3] != (flags | VCPU_EFLAGS_RF) || frame[4] != old_sp ||
                frame[5] != 0x23u) return LIB_FALSE;
        } else {
            lib_u16 word_frame[3];

            if ((source != 1u ? instruction->delivered_exception.valid :
                    !instruction->delivered_exception.valid ||
                    instruction->delivered_exception.exception_mask != VCPUINS_EXCEPT_UD ||
                    instruction->delivered_exception.point.eip != 0u) ||
                instruction->cpu.data.eip != 0x200u ||
                instruction->cpu.data.cs.selector != 0x1bu ||
                instruction->cpu.data.esp != old_sp - 3u * width || fixture.writes == 0u)
                return LIB_FALSE;
            for (lib_u32 address = 0x9000u; address < 0x9020u; ++address)
                if ((address < old_sp - 3u * width || address >= old_sp) &&
                    instruction->memory[address] != 0xa5u) return LIB_FALSE;
            if (gate32) {
                lib_memory_copy(frame, instruction->memory + old_sp - 12u, 12u);
                if (frame[0] != (source != 1u ? bytes : 0u) || frame[1] != 0x1bu ||
                    frame[2] != (flags | (source == 1u ? VCPU_EFLAGS_RF : 0u)))
                    return LIB_FALSE;
            } else {
                lib_memory_copy(word_frame, instruction->memory + old_sp - 6u, sizeof(word_frame));
                if (word_frame[0] != (source != 1u ? bytes : 0u) || word_frame[1] != 0x1bu ||
                    word_frame[2] != (lib_u16)flags) return LIB_FALSE;
            }
        }
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!alignment_same_level_interrupt()) {
        lib_c_printf("Same-level interrupt alignment failed\n"); return 1;
    }
    if (!alignment_system_references()) return 1;
    if (!alignment_immediate_fetch_order()) return 1;
    if (!alignment_access_order()) return 1;
    for (lib_size form = 0u; form < sizeof(alignment_forms) / sizeof(alignment_forms[0]); ++form)
        for (lib_u8 residue = 0u; residue < 4u; ++residue)
            for (lib_u8 vm86 = 0u; vm86 < 2u; ++vm86)
                if (!alignment_case(form, residue, vm86)) {
                    lib_c_printf("CPU-ALIGNMENT-ACCESS form=%zu residue=%u vm86=%u\n",
                        form, residue, vm86);
                    return 1;
                }
    lib_c_printf("CPU-ALIGNMENT-ACCESS:OK\n");
    return 0;
}

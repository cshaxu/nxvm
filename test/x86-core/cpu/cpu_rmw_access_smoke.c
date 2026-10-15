#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

typedef struct rmw_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 reads;
    lib_u32 writes;
    lib_u8 failure;
} rmw_fixture;

static lib_status rmw_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    rmw_fixture *fixture = opaque;
    if (address >= 0x9000u && address < 0xb000u) {
        ++fixture->reads;
        if (fixture->failure == 5u) return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_read(&fixture->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch, cache_control);
}

static lib_status rmw_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    rmw_fixture *fixture = opaque;
    if (address >= 0x9000u && address < 0xb000u) {
        ++fixture->writes;
        if (fixture->failure == 6u) return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_write(&fixture->instruction, address, source, bytes,
        provenance, cache_control);
}

static const x86_core_cpu_bus_provider rmw_bus = {
    .read_memory = rmw_read,
    .write_memory = rmw_write,
    .interrupt_pending = cpu_instruction_interrupt_pending
};

typedef struct rmw_form {
    lib_u8 opcode;
    lib_u8 reg;
    lib_bool extended;
    lib_bool byte;
    lib_u8 immediate; /* 0: none, 1: byte, 2: operand-sized. */
    lib_bool modifies;
} rmw_form;

static const rmw_form rmw_forms[] = {
    {0x00u,0u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0x01u,0u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0x08u,0u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0x09u,0u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0x10u,0u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0x11u,0u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0x18u,0u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0x19u,0u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0x20u,0u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0x21u,0u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0x28u,0u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0x29u,0u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0x30u,0u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0x31u,0u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0x86u,0u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0x87u,0u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0x80u,0u,LIB_FALSE,LIB_TRUE, 1u,LIB_TRUE},
    {0x81u,0u,LIB_FALSE,LIB_FALSE,2u,LIB_TRUE},
    {0x83u,0u,LIB_FALSE,LIB_FALSE,1u,LIB_TRUE},
    {0xf6u,2u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0xf7u,2u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0xf6u,3u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0xf7u,3u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0xfeu,0u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0xffu,0u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0xfeu,1u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0xffu,1u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0xc0u,4u,LIB_FALSE,LIB_TRUE, 1u,LIB_TRUE},
    {0xc1u,4u,LIB_FALSE,LIB_FALSE,1u,LIB_TRUE},
    {0xd0u,4u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0xd1u,4u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0xd2u,4u,LIB_FALSE,LIB_TRUE, 0u,LIB_TRUE},
    {0xd3u,4u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    {0xa4u,0u,LIB_TRUE, LIB_FALSE,1u,LIB_TRUE},
    {0xa5u,0u,LIB_TRUE, LIB_FALSE,0u,LIB_TRUE},
    {0xacu,0u,LIB_TRUE, LIB_FALSE,1u,LIB_TRUE},
    {0xadu,0u,LIB_TRUE, LIB_FALSE,0u,LIB_TRUE},
    {0xabu,0u,LIB_TRUE, LIB_FALSE,0u,LIB_TRUE},
    {0xb3u,0u,LIB_TRUE, LIB_FALSE,0u,LIB_TRUE},
    {0xbbu,0u,LIB_TRUE, LIB_FALSE,0u,LIB_TRUE},
    {0xbau,5u,LIB_TRUE, LIB_FALSE,1u,LIB_TRUE},
    {0xbau,6u,LIB_TRUE, LIB_FALSE,1u,LIB_TRUE},
    {0xbau,7u,LIB_TRUE, LIB_FALSE,1u,LIB_TRUE},
    {0xc0u,0u,LIB_TRUE, LIB_TRUE, 0u,LIB_TRUE},
    {0xc1u,0u,LIB_TRUE, LIB_FALSE,0u,LIB_TRUE},
    {0xb0u,0u,LIB_TRUE, LIB_TRUE, 0u,LIB_TRUE},
    {0xb1u,0u,LIB_TRUE, LIB_FALSE,0u,LIB_TRUE},
    {0x63u,0u,LIB_FALSE,LIB_FALSE,0u,LIB_TRUE},
    /* Read-only controls must not inherit destination-write permission. */
    {0x38u,0u,LIB_FALSE,LIB_TRUE, 0u,LIB_FALSE},
    {0x39u,0u,LIB_FALSE,LIB_FALSE,0u,LIB_FALSE},
    {0x80u,7u,LIB_FALSE,LIB_TRUE, 1u,LIB_FALSE},
    {0x81u,7u,LIB_FALSE,LIB_FALSE,2u,LIB_FALSE},
    {0x83u,7u,LIB_FALSE,LIB_FALSE,1u,LIB_FALSE},
    {0xf6u,0u,LIB_FALSE,LIB_TRUE, 1u,LIB_FALSE},
    {0xf7u,0u,LIB_FALSE,LIB_FALSE,2u,LIB_FALSE},
    {0xa3u,0u,LIB_TRUE, LIB_FALSE,0u,LIB_FALSE},
    {0xbau,4u,LIB_TRUE, LIB_FALSE,1u,LIB_FALSE}
};

static lib_bool rmw_case(const rmw_form *form, lib_u8 width, lib_u8 scenario,
    lib_u8 count)
{
    static const lib_u8 descriptor[] = {0xffu,0xffu,0u,0u,0u,0xfbu,0u,0u};
    static const lib_u8 gate[] = {0u,1u,0x0bu,0u,0u,0x8eu,0u,0u};
    rmw_fixture fixture;
    cpu_instruction_fixture *instruction = &fixture.instruction;
    t_cpu after;
    lib_u8 code[10u] = {0u};
    lib_u8 bytes = 0u;
    const lib_u16 address = scenario == 3u ? 0x9fffu : 0x9000u;
    const lib_bool fault = scenario == 2u || (form->modifies && scenario != 0u);
    const lib_u32 exception = scenario == 4u ? VCPUINS_EXCEPT_GP : VCPUINS_EXCEPT_PF;
    const lib_u32 error = scenario == 4u ? 0u :
        (scenario == 2u ? 0u : 1u) | 4u | (form->modifies ? 2u : 0u);
    const lib_u32 accumulator = form->opcode == 0x63u ? 3u : 1u;
    lib_u32 entry = 0x2007u;
    lib_status status;

    if (scenario == 3u && width == 1u) return LIB_TRUE;
    if (scenario == 6u && !form->modifies) return LIB_TRUE;
    lib_memory_set(&fixture, 0, sizeof(fixture));
    fixture.failure = scenario;
    cpu_instruction_prepare_with_bus(instruction, X86_CORE_CPU_PROFILE_80486SX,
        &rmw_bus, &fixture);
    instruction->cpu.data.cr0 = VCPU_CR0_PE | VCPU_CR0_PG;
    instruction->cpu.data.cr3 = 0x1000u;
    instruction->cpu.data.cs.selector = 0x0bu;
    instruction->cpu.data.cs.dpl = 3u;
    instruction->cpu.data.ds.selector = 0x13u;
    instruction->cpu.data.ds.dpl = 3u;
    instruction->cpu.data.ss.selector = 0x13u;
    instruction->cpu.data.ss.dpl = 3u;
    instruction->cpu.data.ds.seg.data.writable = scenario != 4u;
    instruction->cpu.data.esp = 0x8000u;
    instruction->cpu.data.eax = accumulator;
    instruction->cpu.data.ecx = count;
    instruction->cpu.data.eflags = 0x8d7u;
    instruction->cpu.data.gdtr.base = 0x4000u;
    instruction->cpu.data.gdtr.limit = 15u;
    instruction->cpu.data.idtr.base = 0x5000u;
    instruction->cpu.data.idtr.limit = 0x7ffu;
    lib_memory_copy(instruction->memory + 0x4008u, descriptor, sizeof(descriptor));
    lib_memory_copy(instruction->memory + 0x5068u, gate, sizeof(gate));
    lib_memory_copy(instruction->memory + 0x5070u, gate, sizeof(gate));
    lib_memory_copy(instruction->memory + 0x1000u, &entry, sizeof(entry));
    for (lib_u32 page = 0u; page < 16u; ++page) {
        entry = (page << 12u) | 7u;
        if (page == 9u && scenario == 1u) entry &= ~2u;
        if (page == 9u && scenario == 2u) entry &= ~1u;
        if (page == 10u && scenario == 3u) entry &= ~2u;
        lib_memory_copy(instruction->memory + 0x2000u + page * 4u, &entry, 4u);
    }
    lib_memory_set(instruction->memory + address, 0x55, width);
    if (width == 4u) code[bytes++] = 0x66u;
    if (form->extended) code[bytes++] = 0x0fu;
    code[bytes++] = form->opcode;
    code[bytes++] = (lib_u8)(0x06u | (form->reg << 3u));
    code[bytes++] = (lib_u8)address;
    code[bytes++] = (lib_u8)(address >> 8u);
    if (form->immediate != 0u) {
        code[bytes++] = count;
        if (form->immediate == 2u) bytes += width - 1u;
    }
    status = cpu_instruction_run(instruction, code, bytes, &after);
    if (scenario >= 5u) {
        if (status != LIB_STATUS_INTERNAL_ERROR || !instruction->fault.valid ||
            instruction->fault.exception_mask != VCPUINS_EXCEPT_CE ||
            instruction->delivered_exception.valid || after.data.eip != 0u ||
            after.data.eax != accumulator || after.data.ecx != count ||
            after.data.eflags != 0x8d7u || fixture.reads != 1u ||
            fixture.writes != (scenario == 6u ? 1u : 0u))
            return LIB_FALSE;
        for (lib_u8 byte = 0u; byte < width; ++byte)
            if (instruction->memory[address + byte] != 0x55u) return LIB_FALSE;
        return LIB_TRUE;
    }
    if (status != LIB_STATUS_OK ||
        instruction->fault.valid || x86_core_cpu_is_shutdown(&instruction->execution) ||
        instruction->delivered_exception.valid != fault)
        return LIB_FALSE;
    if (!fault && (count & 31u) == 0u) {
        for (lib_u8 byte = 0u; byte < width; ++byte)
            if (instruction->memory[address + byte] != 0x55u) return LIB_FALSE;
        return after.data.eip == bytes && after.data.eflags == 0x8d7u &&
            fixture.reads != 0u && fixture.writes == 0u;
    }
    if (!fault)
        return after.data.eip == bytes && fixture.reads != 0u &&
            (form->modifies ? fixture.writes != 0u : fixture.writes == 0u);
    if (instruction->delivered_exception.exception_mask != exception ||
        instruction->delivered_exception.exception_code != error ||
        instruction->delivered_exception.point.eip != 0u ||
        instruction->delivered_exception.eflags != 0x8d7u ||
        after.data.eax != accumulator || after.data.ecx != count ||
        fixture.reads != 0u || fixture.writes != 0u)
        return LIB_FALSE;
    for (lib_u8 byte = 0u; byte < width; ++byte)
        if (instruction->memory[address + byte] != 0x55u) return LIB_FALSE;
    return LIB_TRUE;
}

int main(void)
{
    for (lib_size form = 0u; form < sizeof(rmw_forms) / sizeof(rmw_forms[0]); ++form) {
        const rmw_form *base = &rmw_forms[form];
        const lib_bool arithmetic = !base->extended && base->modifies &&
            (base->opcode == 0x80u || base->opcode == 0x81u || base->opcode == 0x83u);
        const lib_bool shift = !base->extended &&
            (base->opcode == 0xc0u || base->opcode == 0xc1u ||
             (base->opcode >= 0xd0u && base->opcode <= 0xd3u));
        const lib_bool test = !base->extended && !base->modifies &&
            (base->opcode == 0xf6u || base->opcode == 0xf7u);
        const lib_u8 variants = arithmetic ? 7u : shift ? 8u : test ? 2u : 1u;

        for (lib_u8 variant = 0u; variant < variants; ++variant) {
            rmw_form selected = *base;
            if (arithmetic || shift || test) selected.reg = variant;
            for (lib_u8 width = base->byte ? 1u : 2u;
                    width <= (base->byte ? 1u : 4u); width *= 2u)
                for (lib_u8 scenario = 0u; scenario < 7u; ++scenario)
                    if (!rmw_case(&selected, width, scenario, 1u)) {
                        lib_c_printf("CPU-RMW-ACCESS form=%zu reg=%u width=%u scenario=%u\n",
                            form, selected.reg, width, scenario);
                        return 1;
                    }
            if ((shift && base->opcode != 0xd0u && base->opcode != 0xd1u) ||
                (base->extended && (base->opcode == 0xa4u || base->opcode == 0xa5u ||
                    base->opcode == 0xacu || base->opcode == 0xadu)))
                for (lib_u8 width = base->byte ? 1u : 2u;
                        width <= (base->byte ? 1u : 4u); width *= 2u)
                    for (lib_u8 scenario = 0u; scenario < 5u; ++scenario)
                        if (!rmw_case(&selected, width, scenario, 0u) ||
                            !rmw_case(&selected, width, scenario, 32u)) {
                            lib_c_printf("CPU-RMW-ZERO form=%zu reg=%u width=%u scenario=%u\n",
                                form, selected.reg, width, scenario);
                            return 1;
                        }
        }
    }
    lib_c_printf("CPU-RMW-ACCESS:OK\n");
    return 0;
}

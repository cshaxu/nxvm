#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* Full CPU storage rollback remains with the CPU, not a Core receiver. */
static lib_i32 fpu_state_reject(const lib_u8 *code, lib_u8 bytes,
    x86_core_cpu_profile profile, x86_fpu_profile fpu_profile,
    lib_u32 exception)
{
    cpu_instruction_fixture state;
    t_cpu before, after;
    x86_fpu *fpu = LIB_NULL;
    lib_i32 failed;

    cpu_instruction_prepare(&state, profile);
    failed = x86_fpu_create(fpu_profile, &fpu) != LIB_STATUS_OK;
    if (!failed) {
        x86_core_cpu_execution_context_bind_fpu(&state.execution, fpu);
        if (exception == VCPUINS_EXCEPT_UD)
            failed |= !cpu_instruction_expect_real_fault(&state, code, bytes, 6u);
        else {
            before = state.cpu;
            failed |= cpu_instruction_run(&state, code, bytes, &after) !=
                LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
                (state.fault.exception_mask & exception) == 0u ||
                lib_memory_compare(&before, &after, sizeof(before)) != 0;
        }
    }
    x86_fpu_destroy(fpu);
    return failed;
}

static void fpu_state_command(void *owner, lib_u8 opcode, lib_u8 modrm)
{
    (void)owner; (void)opcode; (void)modrm;
}

static lib_i32 fpu_state_success(const lib_u8 *code, lib_u8 bytes,
    x86_core_cpu_profile profile, x86_fpu_profile fpu_profile)
{
    const x86_core_cpu_bus_provider bus = {
        .read_memory = cpu_instruction_read, .write_memory = cpu_instruction_write,
        .interrupt_pending = cpu_instruction_interrupt_pending,
        .extension_command = fpu_state_command
    };
    cpu_instruction_fixture state;
    t_cpu before, after;
    x86_fpu *fpu = LIB_NULL;
    lib_i32 failed;

    cpu_instruction_prepare_with_bus(&state, profile, &bus, &state);
    failed = x86_fpu_create(fpu_profile, &fpu) != LIB_STATUS_OK;
    if (!failed) {
        x86_core_cpu_execution_context_bind_fpu(&state.execution, fpu);
        state.cpu.data.cr0 = 0u;
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
            state.fault.valid || after.data.eip != bytes ||
        after.data.eax != before.data.eax ||
        after.data.ebx != before.data.ebx ||
        after.data.ecx != before.data.ecx ||
        after.data.edx != before.data.edx ||
        after.data.esp != before.data.esp ||
        after.data.ebp != before.data.ebp ||
        after.data.esi != before.data.esi ||
        after.data.edi != before.data.edi ||
        after.data.eflags != before.data.eflags ||
        lib_memory_compare(&after.data.es, &before.data.es, sizeof(after.data.es)) != 0 ||
        lib_memory_compare(&after.data.cs, &before.data.cs, sizeof(after.data.cs)) != 0 ||
        lib_memory_compare(&after.data.ss, &before.data.ss, sizeof(after.data.ss)) != 0 ||
        lib_memory_compare(&after.data.ds, &before.data.ds, sizeof(after.data.ds)) != 0 ||
        lib_memory_compare(&after.data.fs, &before.data.fs, sizeof(after.data.fs)) != 0 ||
        lib_memory_compare(&after.data.gs, &before.data.gs, sizeof(after.data.gs)) != 0;
    }
    x86_fpu_destroy(fpu);
    return failed;
}

static lib_i32 fpu_state_vm86(void)
{
    const lib_u8 esc[] = {0xd8u, 0xc0u};
    cpu_instruction_fixture state;
    t_cpu before, after;
    cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80386);
    state.cpu.data.cr0 |= VCPU_CR0_PE;
    state.cpu.data.eflags = X86_CORE_CPU_DEBUG_EFLAGS_VM | VCPU_EFLAGS_IOPL |
        X86_CORE_CPU_DEBUG_EFLAGS_IF | X86_CORE_CPU_DEBUG_EFLAGS_CF;
    state.cpu.data.cs.selector = 0u; state.cpu.data.cs.base = 0u;
    state.cpu.data.cs.limit = 0xffffu; state.cpu.data.cs.dpl = 3u;
    state.cpu.data.cs.flagValid = LIB_TRUE;
    state.cpu.data.ds.selector = 0u; state.cpu.data.ds.base = 0u;
    state.cpu.data.ds.limit = 0xffffu; state.cpu.data.ds.dpl = 3u;
    state.cpu.data.ds.flagValid = LIB_TRUE;
    state.cpu.data.ss.selector = 0u; state.cpu.data.ss.base = 0u;
    state.cpu.data.ss.limit = 0xffffu; state.cpu.data.ss.dpl = 3u;
    state.cpu.data.ss.flagValid = LIB_TRUE;
    before = state.cpu;
    if (cpu_instruction_run(&state, esc, sizeof(esc), &after) != LIB_STATUS_OK ||
        state.fault.valid || after.data.eip != sizeof(esc) ||
        after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
        after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
        after.data.esp != before.data.esp || after.data.ebp != before.data.ebp ||
        after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
        after.data.eflags != before.data.eflags) return 1;
    return lib_memory_compare(&after.data.es, &before.data.es, sizeof(after.data.es)) != 0 ||
        lib_memory_compare(&after.data.cs, &before.data.cs, sizeof(after.data.cs)) != 0 ||
        lib_memory_compare(&after.data.ss, &before.data.ss, sizeof(after.data.ss)) != 0 ||
        lib_memory_compare(&after.data.ds, &before.data.ds, sizeof(after.data.ds)) != 0 ||
        lib_memory_compare(&after.data.fs, &before.data.fs, sizeof(after.data.fs)) != 0 ||
        lib_memory_compare(&after.data.gs, &before.data.gs, sizeof(after.data.gs)) != 0;
}

typedef struct fpu_sx_probe {
    cpu_instruction_fixture instruction;
    lib_u32 operand_address;
    lib_u32 operand_reads;
    lib_u32 commands;
    lib_u32 test_reads;
} fpu_sx_probe;

static lib_status fpu_sx_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    fpu_sx_probe *probe = opaque;

    if (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA &&
        address == probe->operand_address) ++probe->operand_reads;
    return cpu_instruction_read(&probe->instruction, address, destination, bytes,
        provenance, observe_only, reset_fetch, cache_control);
}

static lib_status fpu_sx_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_u8 cache_control)
{
    fpu_sx_probe *probe = opaque;

    return cpu_instruction_write(&probe->instruction, address, source, bytes,
        provenance, cache_control);
}

static void fpu_sx_command(void *opaque, lib_u8 opcode, lib_u8 modrm)
{
    fpu_sx_probe *probe = opaque;

    (void)opcode;
    (void)modrm;
    ++probe->commands;
}

static lib_bool fpu_sx_test_asserted(void *opaque)
{
    fpu_sx_probe *probe = opaque;

    ++probe->test_reads;
    return LIB_TRUE;
}

static lib_bool fpu_sx_expect_exception(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, lib_u8 mode, lib_u8 vector)
{
    t_cpu before, after;
    lib_u32 frame[10] = {0};
    const lib_bool vm86 = mode == 5u;
    const lib_u8 error_words = vector == 13u ? 1u : 0u;
    const lib_u8 frame_bytes = (lib_u8)(((vm86 ? 9u : 3u) + error_words) * 4u);
    const lib_u32 *saved = frame + error_words;
    const lib_u8 cpl = vm86 ? 3u : (lib_u8)(mode - 1u);
    lib_status status;

    if (mode == 0u)
        return cpu_instruction_expect_real_fault(fixture, code, bytes, vector);
    before = fixture->cpu;
    lib_memory_copy(fixture->memory + 0x400u + vector * 8u,
        fixture->memory + 0x468u, 8u);
    status = cpu_instruction_run(fixture, code, bytes, &after);
    if (status != LIB_STATUS_OK ||
        fixture->fault.valid || !fixture->delivered_exception.valid ||
        fixture->delivered_exception.exception_mask != (UINT32_C(1) << vector) ||
        fixture->delivered_exception.exception_code != 0u ||
        fixture->delivered_exception.point.eip != before.data.eip ||
        fixture->delivered_exception.eflags != before.data.eflags ||
        after.data.eip != 0x300u || after.data.cs.selector != (vm86 ? 8u : 8u | cpl) ||
        after.data.esp != (vm86 ? 0x1000u : before.data.esp) - frame_bytes ||
        after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
        after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
        after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
        after.data.edi != before.data.edi) {
        lib_c_fprintf(lib_c_stderr,
            "SX exception delivery: status=%u fault=%x delivered=%x eip=%x cs=%x esp=%x expected=%x\n",
            (lib_u32)status, fixture->fault.exception_mask,
            fixture->delivered_exception.exception_mask, after.data.eip,
            (lib_u32)after.data.cs.selector, after.data.esp,
            (lib_u32)((vm86 ? 0x1000u : before.data.esp) - frame_bytes));
        return LIB_FALSE;
    }
    lib_memory_copy(frame, fixture->memory + after.data.ss.base + after.data.esp,
        frame_bytes);
    if ((error_words && frame[0] != 0u) || saved[0] != before.data.eip ||
        saved[1] != before.data.cs.selector ||
        saved[2] != (before.data.eflags | VCPU_EFLAGS_RF)) return LIB_FALSE;
    return !vm86 || (saved[3] == before.data.esp && saved[4] == before.data.ss.selector &&
        saved[5] == before.data.es.selector && saved[6] == before.data.ds.selector &&
        saved[7] == before.data.fs.selector && saved[8] == before.data.gs.selector);
}

static lib_bool fpu_state_sx_escapes(void)
{
    const x86_core_cpu_bus_provider bus = {
        .read_memory = fpu_sx_read, .write_memory = fpu_sx_write,
        .interrupt_pending = cpu_instruction_interrupt_pending,
        .extension_command = fpu_sx_command
    };

    for (lib_u8 opcode = 0xd8u; opcode <= 0xdfu; ++opcode)
    for (lib_u8 controls = 0u; controls < 16u; ++controls)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 memory = 0u; memory < 2u; ++memory)
    for (lib_u8 mode = 0u; mode < 6u; ++mode) {
        fpu_sx_probe probe;
        lib_u8 code[8], bytes = 0u;
        const lib_bool address32 = code32 != ((prefixes & 2u) != 0u);

        cpu_instruction_prepare_with_bus(&probe.instruction,
            X86_CORE_CPU_PROFILE_80486SX, &bus, &probe);
        if (mode != 0u)
            cpu_instruction_prepare_protected_exception(&probe.instruction,
                (lib_u8)(mode - 1u));
        probe.operand_reads = probe.commands = 0u;
        probe.operand_address = address32 ? 0x7ffffffeu : 0xfffeu;
        probe.instruction.cpu.data.cr0 = VCPU_CR0_ET | (mode ? VCPU_CR0_PE : 0u) |
            ((controls & 1u) ? VCPU_CR0_EM : 0u) |
            ((controls & 2u) ? VCPU_CR0_TS : 0u) |
            ((controls & 4u) ? VCPU_CR0_MP : 0u) |
            ((controls & 8u) ? VCPU_CR0_NE : 0u);
        probe.instruction.cpu.data.cs.seg.exec.defsize = code32;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = opcode;
        code[bytes++] = memory ? (address32 ? 0x05u : 0x06u) : 0xc0u;
        if (memory) {
            code[bytes++] = 0xfeu;
            code[bytes++] = 0xffu;
            if (address32) {
                code[bytes++] = 0xffu;
                code[bytes++] = 0x7fu;
            }
            /* EM/TS win over the operand; otherwise its segment is checked. */
            probe.instruction.cpu.data.ds.limit = 0u;
        }
        lib_bool accepted;
        if ((controls & 3u) != 0u || memory) {
            accepted = fpu_sx_expect_exception(&probe.instruction, code, bytes,
                mode, (controls & 3u) != 0u ? 7u : 13u);
        } else {
            t_cpu after;
            accepted = cpu_instruction_run(&probe.instruction, code, bytes,
                &after) == LIB_STATUS_OK && !probe.instruction.fault.valid &&
                !probe.instruction.delivered_exception.valid &&
                after.data.eip == bytes;
        }
        if (!accepted ||
            probe.operand_reads != 0u || probe.commands != 0u) {
            lib_c_fprintf(lib_c_stderr,
                "SX ESC: opcode=%x controls=%u code32=%u prefixes=%u memory=%u mode=%u\n",
                (lib_u32)opcode, (lib_u32)controls, (lib_u32)code32,
                (lib_u32)prefixes, (lib_u32)memory, (lib_u32)mode);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool fpu_state_sx_absent_operands(void)
{
    /* Standard memory forms, grouped by their independently expected widths.
     * FNSTENV/FNSAVE exercise both operand-size layouts below. */
    static const struct {
        lib_u8 opcode, group, bytes;
        lib_bool write;
    } forms[] = {
        {0xd8u,0u,4u,LIB_FALSE}, {0xd9u,0u,4u,LIB_FALSE},
        {0xd9u,2u,4u,LIB_TRUE}, {0xd9u,3u,4u,LIB_TRUE},
        {0xd9u,4u,14u,LIB_FALSE}, {0xd9u,5u,2u,LIB_FALSE},
        {0xd9u,6u,14u,LIB_TRUE}, {0xd9u,7u,2u,LIB_TRUE},
        {0xdau,7u,4u,LIB_FALSE}, {0xdbu,0u,4u,LIB_FALSE},
        {0xdbu,2u,4u,LIB_TRUE}, {0xdbu,3u,4u,LIB_TRUE},
        {0xdbu,5u,10u,LIB_FALSE}, {0xdbu,7u,10u,LIB_TRUE},
        {0xdcu,0u,8u,LIB_FALSE}, {0xddu,0u,8u,LIB_FALSE},
        {0xddu,2u,8u,LIB_TRUE}, {0xddu,3u,8u,LIB_TRUE},
        {0xddu,4u,94u,LIB_FALSE}, {0xddu,6u,94u,LIB_TRUE},
        {0xddu,7u,2u,LIB_TRUE}, {0xdeu,7u,2u,LIB_FALSE},
        {0xdfu,0u,2u,LIB_FALSE}, {0xdfu,2u,2u,LIB_TRUE},
        {0xdfu,3u,2u,LIB_TRUE}, {0xdfu,4u,10u,LIB_FALSE},
        {0xdfu,5u,8u,LIB_FALSE}, {0xdfu,6u,10u,LIB_TRUE},
        {0xdfu,7u,8u,LIB_TRUE}, {0xd9u,1u,0u,LIB_FALSE},
        {0xdbu,1u,0u,LIB_FALSE}, {0xdbu,4u,0u,LIB_FALSE},
        {0xdbu,6u,0u,LIB_FALSE}, {0xddu,1u,0u,LIB_FALSE},
        {0xddu,5u,0u,LIB_FALSE}, {0xdfu,1u,0u,LIB_FALSE}
    };
    for (lib_size form = 0u; form < sizeof(forms) / sizeof(forms[0]); ++form)
    for (lib_u8 arithmetic = 0u;
        arithmetic < ((forms[form].opcode & 1u) ? 1u : 8u); ++arithmetic)
    for (lib_u8 mode = 0u; mode < 6u; ++mode)
    for (lib_u8 operand32 = 0u; operand32 < 2u; ++operand32)
    for (lib_u8 address32 = 0u; address32 < 2u; ++address32) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u8 code[9], size = 0u, bytes = forms[form].bytes;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (mode != 0u)
            cpu_instruction_prepare_protected_exception(&fixture, mode - 1u);
        fixture.cpu.data.ds = fixture.cpu.data.ss;
        fixture.cpu.data.ds.sregtype = SREG_DATA;
        fixture.cpu.data.cr0 &= ~(VCPU_CR0_EM | VCPU_CR0_TS);
        lib_memory_set(fixture.memory + 0x7ffu, 0x5au, 110u);
        if (operand32) code[size++] = 0x66u;
        if (address32) code[size++] = 0x67u;
        code[size++] = forms[form].opcode;
        const lib_u8 group = (forms[form].opcode & 1u) ?
            forms[form].group : arithmetic;
        code[size++] = (lib_u8)((group << 3u) |
            (address32 ? 5u : 6u));
        code[size++] = 0u;
        code[size++] = 8u;
        if (address32) { code[size++] = 0u; code[size++] = 0u; }
        if (operand32) {
            if (bytes == 14u) bytes = 28u;
            else if (bytes == 94u) bytes = 108u;
        }
        if (cpu_instruction_run(&fixture, code, size, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            after.data.eip != size || fixture.memory[0x7ffu] != 0x5au ||
            fixture.memory[0x800u + bytes] != 0x5au) return LIB_FALSE;
        for (lib_u8 index = 0u; index < bytes; ++index)
            if (fixture.memory[0x800u + index] !=
                (forms[form].write ? 0xffu : 0x5au)) return LIB_FALSE;
        if (!forms[form].write && fixture.writes != 0u) return LIB_FALSE;
        if (forms[form].write && mode == 0u) {
            /* Full-range validation must precede the first store word. */
            cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
            fixture.cpu.data.cr0 = VCPU_CR0_ET;
            fixture.cpu.data.ds.limit = 0x800u + bytes - 2u;
            lib_memory_set(fixture.memory + 0x800u, 0x5au, 108u);
            if (!cpu_instruction_expect_real_fault(&fixture, code, size, 13u))
                return LIB_FALSE;
            for (lib_u8 index = 0u; index < bytes; ++index)
                if (fixture.memory[0x800u + index] != 0x5au) return LIB_FALSE;
        }
    }
    {
        const lib_u8 status[] = {0xdfu,0xe0u};
        cpu_instruction_fixture fixture;
        t_cpu after;
        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cr0 = VCPU_CR0_ET;
        fixture.cpu.data.eax = 0x12345678u;
        if (cpu_instruction_run(&fixture, status, sizeof(status), &after) !=
            LIB_STATUS_OK || after.data.eax != 0x1234ffffu ||
            fixture.fault.valid || fixture.delivered_exception.valid)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool fpu_state_sx_wait(void)
{
    const x86_core_cpu_bus_provider bus = {
        .read_memory = fpu_sx_read, .write_memory = fpu_sx_write,
        .interrupt_pending = cpu_instruction_interrupt_pending,
        .extension_command = fpu_sx_command,
        .wait_test_asserted = fpu_sx_test_asserted
    };

    for (lib_u8 mode = 0u; mode < 6u; ++mode)
    for (lib_u8 controls = 0u; controls < 16u; ++controls)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        fpu_sx_probe probe;
        t_cpu before, after;
        lib_u8 code[3], bytes = 0u;

        cpu_instruction_prepare_with_bus(&probe.instruction,
            X86_CORE_CPU_PROFILE_80486SX, &bus, &probe);
        probe.operand_address = LIB_UINT32_MAX;
        probe.operand_reads = 0u;
        probe.commands = 0u;
        probe.test_reads = 0u;
        if (mode != 0u) cpu_instruction_prepare_protected_exception(
            &probe.instruction, mode == 5u ? 4u : (lib_u8)(mode - 1u));
        probe.instruction.cpu.data.cr0 |=
            ((controls & 1u) ? VCPU_CR0_EM : 0u) |
            ((controls & 2u) ? VCPU_CR0_TS : 0u) |
            ((controls & 4u) ? VCPU_CR0_MP : 0u) |
            ((controls & 8u) ? VCPU_CR0_NE : 0u);
        probe.instruction.cpu.data.cs.seg.exec.defsize = code32;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0x9bu;
        before = probe.instruction.cpu;
        if ((controls & 6u) == 6u) {
            if (!fpu_sx_expect_exception(&probe.instruction, code, bytes, mode, 7u))
                return LIB_FALSE;
        } else {
            if (cpu_instruction_run(&probe.instruction, code, bytes, &after) != LIB_STATUS_OK ||
                probe.instruction.fault.valid || probe.instruction.delivered_exception.valid)
                return LIB_FALSE;
            before.data.eip = bytes;
            if (lib_memory_compare(&before, &after, sizeof(before)) != 0)
                return LIB_FALSE;
        }
        if (probe.test_reads != 0u || probe.commands != 0u ||
            probe.instruction.execution.wait_state != CPU_WAIT_STATE_NONE)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool fpu_state_sx_lock_priority(void)
{
    const lib_u8 opcodes[] = {0x9bu, 0xd8u, 0xd9u, 0xdau, 0xdbu, 0xdcu, 0xddu, 0xdeu, 0xdfu};
    const x86_core_cpu_bus_provider bus = {
        .read_memory = fpu_sx_read, .write_memory = fpu_sx_write,
        .interrupt_pending = cpu_instruction_interrupt_pending,
        .extension_command = fpu_sx_command
    };

    for (lib_size operation = 0u; operation < sizeof(opcodes) / sizeof(opcodes[0]); ++operation)
    for (lib_u8 memory = 0u; memory < 2u; ++memory)
    for (lib_u8 controls = 0u; controls < 16u; ++controls)
    for (lib_u8 mode = 0u; mode < 6u; ++mode)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        fpu_sx_probe probe;
        lib_u8 code[9], bytes = 0u;
        const lib_u8 opcode = opcodes[operation];
        const lib_bool address32 = code32 != ((prefixes & 2u) != 0u);

        if (opcode == 0x9bu && memory) continue;

        cpu_instruction_prepare_with_bus(&probe.instruction,
            X86_CORE_CPU_PROFILE_80486SX, &bus, &probe);
        probe.operand_address = memory ? (address32 ? 0x7ffffffeu : 0xfffeu) : LIB_UINT32_MAX;
        probe.operand_reads = 0u;
        probe.commands = 0u;
        probe.test_reads = 0u;
        if (mode != 0u) cpu_instruction_prepare_protected_exception(
            &probe.instruction, mode == 5u ? 4u : (lib_u8)(mode - 1u));
        probe.instruction.cpu.data.cr0 |=
            ((controls & 1u) ? VCPU_CR0_EM : 0u) |
            ((controls & 2u) ? VCPU_CR0_TS : 0u) |
            ((controls & 4u) ? VCPU_CR0_MP : 0u) |
            ((controls & 8u) ? VCPU_CR0_NE : 0u);
        probe.instruction.cpu.data.cs.seg.exec.defsize = code32;
        if (memory) probe.instruction.cpu.data.ds.limit = 0u;
        code[bytes++] = 0xf0u;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = opcode;
        if (opcode != 0x9bu) {
            code[bytes++] = memory ? (address32 ? 0x05u : 0x06u) : 0xc0u;
            if (memory)
                for (lib_u8 offset = 0u; offset < (address32 ? 4u : 2u); ++offset)
                    code[bytes++] = (lib_u8)(probe.operand_address >> (offset * 8u));
        }
        if (!fpu_sx_expect_exception(&probe.instruction, code, bytes, mode, 6u) ||
            probe.operand_reads != 0u || probe.commands != 0u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    static const lib_u8 attributes[][4] = {
        {0x66u,0x9bu}, {0x67u,0x9bu}, {0x66u,0x67u,0x9bu},
        {0x66u,0xdbu,0xe3u}, {0x67u,0xdbu,0xe3u}, {0x66u,0x67u,0xdbu,0xe3u}
    };
    static const lib_u8 attribute_sizes[] = {2u,2u,3u,3u,3u,4u};
    static const lib_u8 locks[][5] = {
        {0xf0u,0x9bu}, {0xf0u,0x66u,0x9bu}, {0xf0u,0x67u,0x9bu},
        {0xf0u,0x66u,0x67u,0x9bu}, {0xf0u,0xdbu,0xe3u},
        {0xf0u,0x66u,0xdbu,0xe3u}, {0xf0u,0x67u,0xdbu,0xe3u},
        {0xf0u,0x66u,0x67u,0xdbu,0xe3u}
    };
    static const lib_u8 lock_sizes[] = {2u,3u,3u,4u,3u,4u,4u,5u};
    static const lib_u8 fninit[] = {0xdbu,0xe3u};
    static const lib_u8 wait[] = {0x9bu};
    static const lib_u8 escapes[][2] = {
        {0xd8u,0xc0u}, {0xd9u,0xc0u}, {0xdau,0xc0u}, {0xdbu,0xe3u},
        {0xdcu,0xc0u}, {0xddu,0xc0u}, {0xdeu,0xc0u}, {0xdfu,0xc0u}
    };
    x86_core_cpu_profile profile;
    lib_size index;
    lib_i32 failed = 0;
    for (profile = X86_CORE_CPU_PROFILE_8086;
         profile <= X86_CORE_CPU_PROFILE_80386; ++profile) {
        failed |= fpu_state_success(wait, sizeof(wait), profile, X86_FPU_PROFILE_NONE);
        failed |= fpu_state_success(fninit, sizeof(fninit), profile, X86_FPU_PROFILE_NONE);
        for (index = 0u; index < sizeof(escapes) / sizeof(escapes[0]); ++index)
            failed |= fpu_state_success(escapes[index], sizeof(escapes[index]),
                profile, X86_FPU_PROFILE_NONE);
    }
    failed |= fpu_state_success(fninit, sizeof(fninit), X86_CORE_CPU_PROFILE_8086,
        X86_FPU_PROFILE_8087);
    for (index = 0u; index < sizeof(attribute_sizes); ++index)
        failed |= fpu_state_success(attributes[index], attribute_sizes[index],
            X86_CORE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE);
    for (profile = X86_CORE_CPU_PROFILE_8086;
         profile <= X86_CORE_CPU_PROFILE_80286; ++profile) {
        for (index = 0u; index < sizeof(attribute_sizes); ++index)
            failed |= fpu_state_reject(attributes[index], attribute_sizes[index],
                profile, X86_FPU_PROFILE_NONE, VCPUINS_EXCEPT_UD);
    }
    for (index = 0u; index < sizeof(lock_sizes); ++index)
        failed |= fpu_state_reject(locks[index], lock_sizes[index],
            X86_CORE_CPU_PROFILE_80386, X86_FPU_PROFILE_NONE, VCPUINS_EXCEPT_UD);
    failed |= fpu_state_reject(fninit, sizeof(fninit), X86_CORE_CPU_PROFILE_80386,
        X86_FPU_PROFILE_8087, VCPUINS_EXCEPT_FPU_UNSUPPORTED);
    failed |= fpu_state_vm86();
    failed |= !fpu_state_sx_escapes();
    failed |= !fpu_state_sx_absent_operands();
    failed |= fpu_state_success(fninit, sizeof(fninit),
        X86_CORE_CPU_PROFILE_80486SX, X86_FPU_PROFILE_NONE);
    failed |= !fpu_state_sx_wait();
    failed |= !fpu_state_sx_lock_priority();
    if (failed) return 1;
    lib_c_printf("%s\n", "FPU interface CPU full rollback/cache state: PASS");
    return 0;
}

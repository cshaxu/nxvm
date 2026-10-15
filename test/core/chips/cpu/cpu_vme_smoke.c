#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

static lib_u32 *vme_gpr(t_cpu *cpu, lib_u8 index)
{
    switch (index) {
    case 0u: return &cpu->data.eax;
    case 1u: return &cpu->data.ecx;
    case 2u: return &cpu->data.edx;
    case 3u: return &cpu->data.ebx;
    case 4u: return &cpu->data.esp;
    case 5u: return &cpu->data.ebp;
    case 6u: return &cpu->data.esi;
    default: return &cpu->data.edi;
    }
}

static lib_bool vme_control(void)
{
    for (lib_u8 protected_mode = 0u; protected_mode < 2u; ++protected_mode)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 reg = 0u; reg < 8u; ++reg)
    for (lib_u8 write = 0u; write < 2u; ++write)
    for (lib_u32 value = 0u; value < 2u; ++value) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u8 code[5], bytes = 0u;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        if (protected_mode) cpu_instruction_prepare_protected_exception(&fixture, 0u);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.cr4 = write ? 1u - value : value;
        *vme_gpr(&fixture.cpu, reg) = write ? value : 0x12345678u;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0x0fu;
        code[bytes++] = write ? 0x22u : 0x20u;
        code[bytes++] = (lib_u8)(0xe0u | reg);
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            after.data.eip != bytes || after.data.cr4 != value ||
            *vme_gpr(&after, reg) != value || after.data.eflags != 2u)
            return LIB_FALSE;
    }
    for (lib_u8 bit = 1u; bit < 32u; ++bit) {
        cpu_instruction_fixture fixture;
        const lib_u8 code[] = {0x0fu, 0x22u, 0xe0u};
        core_machine_debug_register_patch patch = {
            .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CR4) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS) |
                CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX),
            .values = {[CORE_MACHINE_DEBUG_CR4] = UINT32_C(1) << bit,
                [CORE_MACHINE_DEBUG_EFLAGS] = LIB_UINT32_MAX,
                [CORE_MACHINE_DEBUG_EAX] = 0x12345678u}
        };
        t_cpu before;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        before = fixture.cpu;
        if (core_machine_cpu_debug_patch_registers(&fixture.execution, &patch) !=
                LIB_STATUS_INVALID_ARGUMENT ||
            lib_memory_compare(&before, &fixture.cpu, sizeof(before)) != 0)
            return LIB_FALSE;
        fixture.cpu.data.eax = UINT32_C(1) << bit;
        if (!cpu_instruction_expect_real_fault(&fixture, code, sizeof(code), 13u) ||
            fixture.cpu.data.cr4 != 0u) return LIB_FALSE;
    }
    for (lib_u8 write = 0u; write < 2u; ++write) {
        cpu_instruction_fixture fixture;
        const lib_u8 code[] = {0x0fu, write ? 0x22u : 0x20u, 0xe0u};

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80386);
        if (!cpu_instruction_expect_real_fault(&fixture, code, sizeof(code), 6u))
            return LIB_FALSE;
    }
    for (lib_u32 value = 0u; value < 2u; ++value) {
        cpu_instruction_fixture fixture;
        const core_machine_debug_register_patch patch = {
            .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CR4),
            .values = {[CORE_MACHINE_DEBUG_CR4] = value}
        };
        lib_u32 observed;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cr4 = 1u - value;
        if (core_machine_cpu_debug_patch_registers(&fixture.execution, &patch) != LIB_STATUS_OK ||
            core_machine_cpu_debug_read_register(&fixture.execution, CORE_MACHINE_DEBUG_CR4,
                &observed) != LIB_STATUS_OK || observed != value) return LIB_FALSE;
        core_machine_cpu_state_reset(&fixture.execution);
        if (fixture.cpu.data.cr4 != 0u || fixture.cpu.data.eflags != 2u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_debug_flags(void)
{
    for (lib_u8 sx = 0u; sx < 2u; ++sx)
    for (lib_u8 sample = 0u; sample < 34u; ++sample) {
        cpu_instruction_fixture fixture;
        lib_u32 observed;
        const lib_u32 value = sample < 32u ? UINT32_C(1) << sample :
            (sample == 32u ? 0u : LIB_UINT32_MAX);
        const core_machine_debug_register_patch patch = {
            .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
            .values = {[CORE_MACHINE_DEBUG_EFLAGS] = value}
        };

        cpu_instruction_prepare(&fixture, sx ? CORE_MACHINE_CPU_PROFILE_80486SX :
            CORE_MACHINE_CPU_PROFILE_80386);
        if (core_machine_cpu_debug_patch_registers(&fixture.execution, &patch) != LIB_STATUS_OK ||
            core_machine_cpu_debug_read_register(&fixture.execution,
                CORE_MACHINE_DEBUG_EFLAGS, &observed) != LIB_STATUS_OK ||
            observed != (sx ? (value & 0x003f7fd5u) | 2u : value))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static void vme_prepare(cpu_instruction_fixture *fixture,
    core_machine_cpu_profile profile, lib_u8 iopl, lib_u32 flags, lib_bool enabled)
{
    cpu_instruction_prepare(fixture, profile);
    cpu_instruction_prepare_protected_exception(fixture, 4u);
    fixture->cpu.data.cs.selector = 0u;
    fixture->cpu.data.eflags = flags | 2u | VCPU_EFLAGS_VM | ((lib_u32)iopl << 12u);
    fixture->cpu.data.cr4 = enabled ? VCPU_CR4_VME : 0u;
}

static lib_bool vme_expect_fault(cpu_instruction_fixture *fixture,
    const lib_u8 *code, lib_u8 bytes, lib_u8 vector)
{
    t_cpu before = fixture->cpu;
    t_cpu after;
    lib_u32 frame[10];
    const lib_bool vm86 = (before.data.eflags & VCPU_EFLAGS_VM) != 0u;
    const lib_u8 error_words = vector == 6u ? 0u : 1u;
    const lib_u8 words = (lib_u8)(3u + error_words + (vm86 ? 6u : 0u));
    const lib_u32 stack = vm86 ? 0x1000u : before.data.esp;

    if (vector != 13u)
        lib_memory_copy(fixture->memory + 0x400u + vector * 8u,
            fixture->memory + 0x468u, 8u);
    if (cpu_instruction_run(fixture, code, bytes, &after) != LIB_STATUS_OK ||
        fixture->fault.valid || !fixture->delivered_exception.valid ||
        fixture->delivered_exception.exception_mask != (UINT32_C(1) << vector) ||
        fixture->delivered_exception.exception_code != 0u ||
        fixture->delivered_exception.point.eip != 0u ||
        fixture->delivered_exception.eflags != before.data.eflags ||
        after.data.eip != 0x300u || after.data.esp != stack - words * 4u ||
        after.data.cr4 != before.data.cr4) return LIB_FALSE;
    lib_memory_copy(frame, fixture->memory + after.data.esp, words * 4u);
    for (lib_u8 reg = 0u; reg < 8u; ++reg)
        if (reg != 4u && *vme_gpr(&fixture->cpu, reg) !=
            *vme_gpr(&before, reg)) return LIB_FALSE;
    return (!error_words || frame[0] == 0u) && frame[error_words] == 0u &&
        frame[error_words + 1u] == before.data.cs.selector &&
        frame[error_words + 2u] == (before.data.eflags | VCPU_EFLAGS_RF) &&
        (!vm86 || (frame[error_words + 3u] == before.data.esp &&
            frame[error_words + 4u] == before.data.ss.selector &&
            frame[error_words + 5u] == before.data.es.selector &&
            frame[error_words + 6u] == before.data.ds.selector &&
            frame[error_words + 7u] == before.data.fs.selector &&
            frame[error_words + 8u] == before.data.gs.selector));
}

static lib_bool vme_control_rejections(void)
{
    for (lib_u8 mode = 0u; mode < 5u; ++mode)
    for (lib_u8 locked = 0u; locked < 2u; ++locked)
    for (lib_u8 reg = 0u; reg < 8u; ++reg)
    for (lib_u8 write = 0u; write < 2u; ++write)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        cpu_instruction_fixture fixture;
        lib_u8 code[6], bytes = 0u;

        if (mode == 0u && !locked) continue;
        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        cpu_instruction_prepare_protected_exception(&fixture, mode);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.cr4 = write ? 0u : VCPU_CR4_VME;
        if (reg != 4u) *vme_gpr(&fixture.cpu, reg) = write ? 1u : 0x12345678u;
        if (locked) code[bytes++] = 0xf0u;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0x0fu;
        code[bytes++] = write ? 0x22u : 0x20u;
        code[bytes++] = (lib_u8)(0xe0u | reg);
        if (!vme_expect_fault(&fixture, code, bytes, locked ? 6u : 13u))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_flags(void)
{
    const lib_u8 opcodes[] = {0xfau, 0xfbu, 0x9cu, 0x9du};

    for (core_machine_cpu_profile profile = CORE_MACHINE_CPU_PROFILE_80386;
         profile <= CORE_MACHINE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 iopl = 0u; iopl < 4u; ++iopl)
    for (lib_u8 enabled = 0u; enabled < 2u; ++enabled)
    for (lib_u8 virtual_flags = 0u; virtual_flags < 4u; ++virtual_flags)
    for (lib_u8 interrupt = 0u; interrupt < 2u; ++interrupt)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 operation = 0u; operation < 4u; ++operation)
    for (lib_u8 pop_control = 0u; pop_control < (operation == 3u ? 4u : 1u);
         ++pop_control) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u8 code[3], bytes = 0u;
        const lib_u8 width = code32 != ((prefixes & 1u) != 0u) ? 4u : 2u;
        const lib_bool active = profile == CORE_MACHINE_CPU_PROFILE_80486SX &&
            enabled && iopl < 3u;
        const lib_u32 flags = VCPU_EFLAGS_CF |
            ((virtual_flags & 1u) ? VCPU_EFLAGS_VIF : 0u) |
            ((virtual_flags & 2u) ? VCPU_EFLAGS_VIP : 0u) |
            (interrupt ? VCPU_EFLAGS_IF : 0u) |
            ((pop_control & 2u) ? VCPU_EFLAGS_RF : 0u);
        const lib_u32 popped = 0x403u | (interrupt ? 0u : VCPU_EFLAGS_IF) |
            ((pop_control & 1u) ? VCPU_EFLAGS_TF : 0u);
        lib_bool fault = active && virtual_flags == 3u;
        lib_u32 expected, observed = 0u, expected_sp;

        vme_prepare(&fixture, profile, iopl, flags, enabled != 0u);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = opcodes[operation];
        lib_memory_copy(fixture.memory + fixture.cpu.data.esp, &popped, width);
        if (iopl < 3u) {
            if (!active || (operation >= 2u && width == 4u)) fault = LIB_TRUE;
            if (active && operation == 1u && (virtual_flags & 2u)) fault = LIB_TRUE;
            if (active && operation == 3u && (virtual_flags & 2u) &&
                (popped & VCPU_EFLAGS_IF)) fault = LIB_TRUE;
            if (active && operation == 3u && (popped & VCPU_EFLAGS_TF)) fault = LIB_TRUE;
        }
        if (fault) {
            if (!vme_expect_fault(&fixture, code, bytes, 13u)) {
                lib_c_fprintf(lib_c_stderr,
                    "VME GP: cpu=%u iopl=%u vme=%u virtual=%u if=%u code32=%u prefix=%u op=%u delivered=%x esp=%x\n",
                    (lib_u32)profile, (lib_u32)iopl, (lib_u32)enabled,
                    (lib_u32)virtual_flags, (lib_u32)interrupt, (lib_u32)code32,
                    (lib_u32)prefixes, (lib_u32)operation,
                    fixture.delivered_exception.exception_mask, fixture.cpu.data.esp);
                return LIB_FALSE;
            }
            continue;
        }
        expected = fixture.cpu.data.eflags;
        expected_sp = fixture.cpu.data.esp;
        if (operation == 0u) expected &= ~(active ? VCPU_EFLAGS_VIF : VCPU_EFLAGS_IF);
        if (operation == 1u) expected |= active ? VCPU_EFLAGS_VIF : VCPU_EFLAGS_IF;
        if (operation == 2u) expected_sp -= width;
        if (operation == 3u) {
            expected = (expected & 0xffff0000u) |
                (popped & ~VCPU_EFLAGS_IOPL) | ((lib_u32)iopl << 12u);
            if (active) {
                expected = (expected & ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_VIF)) |
                    (interrupt ? VCPU_EFLAGS_IF : 0u) |
                    ((popped & VCPU_EFLAGS_IF) ? VCPU_EFLAGS_VIF : 0u);
            }
            expected_sp += width;
        }
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            after.data.eip != bytes || after.data.esp != expected_sp ||
            after.data.eflags != expected) {
            lib_c_fprintf(lib_c_stderr,
                "VME flags: cpu=%u iopl=%u vme=%u virtual=%u if=%u code32=%u prefix=%u op=%u flags=%x expected=%x\n",
                (lib_u32)profile, (lib_u32)iopl, (lib_u32)enabled,
                (lib_u32)virtual_flags, (lib_u32)interrupt, (lib_u32)code32,
                (lib_u32)prefixes, (lib_u32)operation, after.data.eflags, expected);
            return LIB_FALSE;
        }
        if (operation == 2u) {
            lib_u32 image = expected & (width == 2u ? 0xffffu :
                ~(VCPU_EFLAGS_VM | VCPU_EFLAGS_RF |
                    (profile == CORE_MACHINE_CPU_PROFILE_80486SX ?
                        VCPU_EFLAGS_RESERVED & ~(VCPU_EFLAGS_AC | VCPU_EFLAGS_ID |
                            VCPU_EFLAGS_VIF | VCPU_EFLAGS_VIP) : VCPU_EFLAGS_RESERVED)));
            if (active) image = (image & ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL)) |
                VCPU_EFLAGS_IOPL | ((virtual_flags & 1u) ? VCPU_EFLAGS_IF : 0u);
            image |= 2u;
            lib_memory_copy(&observed, fixture.memory + after.data.esp, width);
            if (observed != image) {
                lib_c_fprintf(lib_c_stderr,
                    "VME PUSHF: cpu=%u iopl=%u vme=%u virtual=%u width=%u image=%x expected=%x\n",
                    (lib_u32)profile, (lib_u32)iopl, (lib_u32)enabled,
                    (lib_u32)virtual_flags, (lib_u32)width, observed, image);
                return LIB_FALSE;
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool vme_pending_flags(void)
{
    for (lib_u8 iopl = 0u; iopl < 4u; ++iopl)
    for (lib_u8 enabled = 0u; enabled < 2u; ++enabled)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;
        lib_u8 code[3], bytes = 0u;

        vme_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX, iopl,
            VCPU_EFLAGS_IF | VCPU_EFLAGS_VIF | VCPU_EFLAGS_VIP, enabled != 0u);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        before = fixture.cpu;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0x90u;
        if (enabled && iopl < 3u) {
            if (!vme_expect_fault(&fixture, code, bytes, 13u)) return LIB_FALSE;
            continue;
        }
        before.data.eip = bytes;
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid || fixture.writes != 0u ||
            lib_memory_compare(&before, &after, sizeof(before)) != 0)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_stack_priority(void)
{
    for (lib_u8 pop = 0u; pop < 2u; ++pop)
    for (lib_u8 wide = 0u; wide < 2u; ++wide) {
        cpu_instruction_fixture fixture;
        lib_u8 code[2], bytes = 0u;

        vme_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX, 0u,
            VCPU_EFLAGS_IF, LIB_TRUE);
        fixture.cpu.data.ss.limit = 0u;
        if (wide) code[bytes++] = 0x66u;
        code[bytes++] = pop ? 0x9du : 0x9cu;
        if (!vme_expect_fault(&fixture, code, bytes, wide ? 13u : 12u))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_int_redirect(void)
{
    for (lib_u16 vector = 0u; vector < 256u; ++vector)
    for (lib_u8 iopl = 0u; iopl < 4u; ++iopl)
    for (lib_u8 virtual_if = 0u; virtual_if < 2u; ++virtual_if)
    for (lib_u8 physical_if = 0u; physical_if < 2u; ++physical_if)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        cpu_instruction_fixture fixture;
        const lib_u16 iomap_base = 0xa0u;
        const lib_u16 target[] = {0x1234u, 0x1800u};
        lib_u16 frame[3], image;
        lib_u8 code[4], bytes = 0u;
        lib_u32 flags, expected;

        flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_DF | VCPU_EFLAGS_TF |
            (virtual_if ? VCPU_EFLAGS_VIF : 0u) |
            (physical_if ? VCPU_EFLAGS_IF : 0u);
        vme_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX, iopl,
            flags, LIB_TRUE);
        fixture.cpu.data.cs.selector = 0x1000u;
        fixture.cpu.data.cs.base = 0x10000u;
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.tr.limit = 0x9fu;
        lib_memory_copy(fixture.memory + 0x666u, &iomap_base, sizeof(iomap_base));
        lib_memory_copy(fixture.memory + vector * 4u, target, sizeof(target));
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0xcdu;
        code[bytes++] = (lib_u8)vector;
        lib_memory_copy(fixture.memory + 0x10000u, code, bytes);
        image = (lib_u16)fixture.cpu.data.eflags;
        if (iopl < 3u) image = (image & ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL)) |
            VCPU_EFLAGS_IOPL | (virtual_if ? VCPU_EFLAGS_IF : 0u);
        expected = fixture.cpu.data.eflags &
            ~(VCPU_EFLAGS_TF | (iopl == 3u ? VCPU_EFLAGS_IF : VCPU_EFLAGS_VIF));
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.fault.valid ||
            fixture.delivered_exception.valid ||
            fixture.cpu.data.eip != target[0] ||
            fixture.cpu.data.cs.selector != target[1] ||
            fixture.cpu.data.cs.base != 0x18000u ||
            fixture.cpu.data.cs.seg.exec.defsize != LIB_FALSE ||
            fixture.cpu.data.esp != 0x7fau ||
            fixture.cpu.data.eflags != expected || fixture.writes != 3u) {
            lib_c_fprintf(lib_c_stderr,
                "VME INT: vector=%u iopl=%u vif=%u if=%u code32=%u prefix=%u ip=%x flags=%x\n",
                (lib_u32)vector, (lib_u32)iopl, (lib_u32)virtual_if,
                (lib_u32)physical_if, (lib_u32)code32, (lib_u32)prefixes,
                fixture.cpu.data.eip, fixture.cpu.data.eflags);
            return LIB_FALSE;
        }
        lib_memory_copy(frame, fixture.memory + 0x7fau, sizeof(frame));
        if (frame[0] != bytes || frame[1] != 0x1000u || frame[2] != image)
            return LIB_FALSE;
    }
    for (lib_u8 iopl = 0u; iopl < 3u; ++iopl)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        cpu_instruction_fixture fixture;
        const lib_u16 iomap_base = 0xa0u;
        lib_u8 code[4], bytes = 0u;

        vme_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX, iopl,
            VCPU_EFLAGS_IF | VCPU_EFLAGS_VIF, LIB_TRUE);
        fixture.cpu.data.tr.limit = 0x9fu;
        lib_memory_copy(fixture.memory + 0x666u, &iomap_base, sizeof(iomap_base));
        fixture.memory[0x684u] = 2u;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0xcdu;
        code[bytes++] = 0x21u;
        if (!vme_expect_fault(&fixture, code, bytes, 13u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_int_map_edges(void)
{
    const struct { lib_u16 base; lib_u8 vector; lib_u32 offset; } maps[] = {
        {0xa0u,0u,0x80u}, {0xa0u,7u,0x80u}, {0xa0u,8u,0x81u}, {0xa0u,255u,0x9fu},
        {0x100u,0u,0xe0u}, {0x100u,255u,0xffu},
        {0xffffu,0u,0xffdfu}, {0xffffu,255u,0xfffeu},
        {0u,0u,LIB_UINT32_MAX}, {1u,0u,LIB_UINT32_MAX}, {31u,0u,LIB_UINT32_MAX},
        {1u,248u,0u}, {1u,255u,0u}, {31u,8u,0u}, {31u,255u,30u}
    };

    for (lib_size map = 0u; map < sizeof(maps) / sizeof(maps[0]); ++map)
    for (lib_u8 iopl = 0u; iopl < 4u; ++iopl)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_u16 target[] = {0x1234u,0x1800u};
        lib_u8 code[4], bytes = 0u;
        lib_u16 frame[3];

        vme_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX, iopl,
            VCPU_EFLAGS_IF | VCPU_EFLAGS_VIF, LIB_TRUE);
        fixture.cpu.data.tr.limit = maps[map].base < 32u ? 0x9fu :
            (lib_u32)maps[map].base - 1u;
        lib_memory_copy(fixture.memory + 0x666u, &maps[map].base, sizeof(maps[map].base));
        if (maps[map].base >= 32u)
            lib_memory_set(fixture.memory + 0x600u + maps[map].base - 32u, 0xffu, 32u);
        if (maps[map].offset != LIB_UINT32_MAX)
            fixture.memory[0x600u + maps[map].offset] =
                (lib_u8)~(1u << (maps[map].vector & 7u));
        lib_memory_copy(fixture.memory + maps[map].vector * 4u, target, sizeof(target));
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0xcdu;
        code[bytes++] = maps[map].vector;
        if (maps[map].offset == LIB_UINT32_MAX) {
            if (!vme_expect_fault(&fixture, code, bytes, 13u)) return LIB_FALSE;
            continue;
        }
        /* Code must not overwrite vector zero or the edge-vector IVT entry. */
        fixture.cpu.data.cs.selector = 0x1000u;
        fixture.cpu.data.cs.base = 0x10000u;
        lib_memory_copy(fixture.memory + 0x10000u, code, bytes);
        core_machine_cpu_execution_refresh(&fixture.execution);
        after = fixture.cpu;
        lib_memory_copy(frame, fixture.memory + 0x7fau, sizeof(frame));
        if (fixture.execution.stop_requested || fixture.fault.valid || fixture.delivered_exception.valid ||
            after.data.eip != target[0] || after.data.cs.selector != target[1] ||
            after.data.esp != 0x7fau || frame[0] != bytes || frame[1] != 0x1000u)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_int_stack_edges(void)
{
    const lib_u16 stack_offsets[] = {0u,1u,2u,3u,4u,5u,6u,7u,0xfffeu,0xffffu};
    const lib_u16 iomap_base = 0xa0u;
    const lib_u16 target[] = {0x1234u,0x1800u};

    for (lib_size stack = 0u; stack < sizeof(stack_offsets) / sizeof(stack_offsets[0]); ++stack)
    for (lib_u8 iopl = 0u; iopl < 4u; ++iopl)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u8 code[4], bytes = 0u;
        const lib_u16 sp = stack_offsets[stack];
        const lib_bool denied = sp == 1u || sp == 3u || sp == 5u;
        lib_u16 ip, cs, flags;

        vme_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX, iopl,
            VCPU_EFLAGS_IF | VCPU_EFLAGS_VIF, LIB_TRUE);
        fixture.cpu.data.tr.limit = 0x9fu;
        fixture.cpu.data.esp = 0xa5a50000u | sp;
        lib_memory_copy(fixture.memory + 0x666u, &iomap_base, sizeof(iomap_base));
        lib_memory_copy(fixture.memory + 0x84u, target, sizeof(target));
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0xcdu;
        code[bytes++] = 0x21u;
        if (denied) {
            if (!vme_expect_fault(&fixture, code, bytes, 12u)) {
                lib_c_printf("VME stack fault sp=%x iopl=%u prefix=%u\n", sp, iopl, prefixes);
                lib_c_printf("entry esp=%x mask=%x code=%x terminal=%x shutdown=%u\n",
                    fixture.cpu.data.esp, fixture.delivered_exception.exception_mask,
                    fixture.delivered_exception.exception_code, fixture.fault.exception_mask,
                    core_machine_cpu_is_shutdown(&fixture.execution));
                return LIB_FALSE;
            }
            continue;
        }
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            after.data.eip != target[0] || after.data.cs.selector != target[1] ||
            after.data.esp != (0xa5a50000u | (lib_u16)(sp - 6u))) {
            lib_c_printf("VME stack sp=%x iopl=%u prefix=%u ip=%x esp=%x fault=%x delivered=%x\n",
                sp, iopl, prefixes, after.data.eip, after.data.esp,
                fixture.fault.exception_mask, fixture.delivered_exception.exception_mask);
            return LIB_FALSE;
        }
        lib_memory_copy(&ip, fixture.memory + (lib_u16)(sp - 6u), sizeof(ip));
        lib_memory_copy(&cs, fixture.memory + (lib_u16)(sp - 4u), sizeof(cs));
        lib_memory_copy(&flags, fixture.memory + (lib_u16)(sp - 2u), sizeof(flags));
        if (ip != bytes || cs != 0u || flags != 0x3202u) {
            lib_c_printf("VME stack frame sp=%x iopl=%u prefix=%u frame=%x/%x/%x\n",
                sp, iopl, prefixes, ip, cs, flags);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool vme_int_protected_routes(void)
{
    for (lib_u8 kind = 0u; kind < 3u; ++kind)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_u8 vector = kind == 0u ? 0x21u : (kind == 1u ? 3u : 4u);
        const lib_u16 iomap_base = 0xa0u;
        lib_u8 gate[] = {0u, 3u, 8u, 0u, 0u, 0xeeu, 0u, 0u};
        lib_u8 code[4], bytes = 0u;
        lib_u32 frame[9];
        const lib_u32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_VIF |
            (kind == 2u ? VCPU_EFLAGS_OF : 0u);
        lib_u32 oldflags;

        vme_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX, 3u,
            flags, LIB_TRUE);
        fixture.cpu.data.idtr.limit = 0x10fu;
        lib_memory_copy(fixture.memory + 0x400u + vector * 8u, gate, sizeof(gate));
        if (kind == 0u) {
            fixture.cpu.data.tr.limit = 0x9fu;
            lib_memory_copy(fixture.memory + 0x666u, &iomap_base, sizeof(iomap_base));
            fixture.memory[0x684u] = 2u;
        }
        oldflags = fixture.cpu.data.eflags;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = kind == 0u ? 0xcdu : (kind == 1u ? 0xccu : 0xceu);
        if (kind == 0u) code[bytes++] = vector;
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            after.data.eip != 0x300u || after.data.cs.selector != 8u ||
            after.data.esp != 0x1000u - sizeof(frame) ||
            (after.data.eflags & (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF)) != 0u)
            return LIB_FALSE;
        lib_memory_copy(frame, fixture.memory + after.data.esp, sizeof(frame));
        if (frame[0] != bytes || frame[1] != 0u || frame[2] != oldflags ||
            frame[3] != 0x800u || frame[4] != 0x13u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_hardware_pending(void *opaque)
{
    (void)opaque;
    return LIB_TRUE;
}

static lib_status vme_hardware_acknowledge(void *opaque, lib_u8 *vector)
{
    (void)opaque;
    *vector = 0x21u;
    return LIB_STATUS_OK;
}

static lib_bool vme_hardware_routes(void)
{
    for (lib_u8 nmi = 0u; nmi < 2u; ++nmi)
    for (lib_u8 iopl = 0u; iopl < 4u; ++iopl)
    for (lib_u8 physical_if = 0u; physical_if < 2u; ++physical_if)
    for (lib_u8 virtual_if = 0u; virtual_if < 2u; ++virtual_if)
    for (lib_u8 virtual_pending = 0u; virtual_pending < 2u; ++virtual_pending)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32) {
        cpu_instruction_fixture fixture;
        core_machine_cpu_bus_provider bus = cpu_instruction_bus;
        const lib_u8 vector = nmi ? 2u : 0x21u;
        const lib_u8 gate[] = {0u, 3u, 8u, 0u, 0u, 0x8eu, 0u, 0u};
        const lib_u16 iomap_base = 0xa0u;
        const lib_u16 real_target[] = {0x1234u, 0x1800u};
        const lib_bool delivered = nmi || physical_if;
        lib_u32 frame[9];
        t_cpu before;

        /* Inconsistent eligible VIF/VIP faults before instruction completion;
         * vme_pending_flags owns that class, not successful hardware entry. */
        if (iopl < 3u && virtual_if && virtual_pending) continue;
        vme_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX, iopl,
            (physical_if ? VCPU_EFLAGS_IF : 0u) |
                (virtual_if ? VCPU_EFLAGS_VIF : 0u) |
                (virtual_pending ? VCPU_EFLAGS_VIP : 0u), LIB_TRUE);
        fixture.cpu.data.cs.selector = 0x1000u;
        fixture.cpu.data.cs.base = 0x10000u;
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.tr.limit = 0x9fu;
        fixture.cpu.data.idtr.limit = 0x10fu;
        fixture.cpu.data.esp |= 0xa5a50000u;
        lib_memory_copy(fixture.memory + 0x666u, &iomap_base, sizeof(iomap_base));
        /* Every redirect bit is clear: software INT would use the real IVT. */
        lib_memory_copy(fixture.memory + vector * 4u, real_target, sizeof(real_target));
        lib_memory_copy(fixture.memory + 0x400u + vector * 8u, gate, sizeof(gate));
        fixture.memory[0x10000u] = 0x90u;
        bus.interrupt_pending = nmi ? cpu_instruction_interrupt_pending : vme_hardware_pending;
        bus.acknowledge_interrupt = vme_hardware_acknowledge;
        fixture.execution.bus = &bus;
        if (nmi && !core_machine_cpu_request_nmi(&fixture.execution)) return LIB_FALSE;
        before = fixture.cpu;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.fault.valid ||
            fixture.delivered_exception.valid ||
            fixture.cpu.data.cr4 != VCPU_CR4_VME) return LIB_FALSE;
        if (!delivered) {
            if (fixture.cpu.data.eip != 1u || fixture.cpu.data.esp != before.data.esp ||
                fixture.cpu.data.eflags != before.data.eflags || fixture.writes != 0u)
                return LIB_FALSE;
            continue;
        }
        if (fixture.cpu.data.eip != 0x300u || fixture.cpu.data.cs.selector != 8u ||
            fixture.cpu.data.esp != 0x1000u - sizeof(frame) ||
            (fixture.cpu.data.eflags & (VCPU_EFLAGS_VM | VCPU_EFLAGS_IF)) != 0u)
            return LIB_FALSE;
        lib_memory_copy(frame, fixture.memory + fixture.cpu.data.esp, sizeof(frame));
        if (frame[0] != 1u || frame[1] != before.data.cs.selector ||
            frame[2] != before.data.eflags || frame[3] != before.data.esp ||
            frame[4] != before.data.ss.selector || frame[5] != before.data.es.selector ||
            frame[6] != before.data.ds.selector || frame[7] != before.data.fs.selector ||
            frame[8] != before.data.gs.selector) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_int_paging(void)
{
    for (lib_u8 iopl = 0u; iopl < 4u; ++iopl)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 failure = 0u; failure < 6u; ++failure) {
        cpu_instruction_fixture fixture;
        t_cpu before;
        const lib_u32 pde = 0x2007u;
        lib_u32 pages[32] = {0};
        const lib_u16 iomap_base = 0x1a0u;
        const lib_u32 kernel_sp = 0x8800u;
        const lib_u16 kernel_ss = 0x10u;
        const lib_u16 target[] = {0x1234u, 0x1800u};
        const lib_u8 gate[] = {0u, 0xc3u, 8u, 0u, 0u, 0x8eu, 0u, 0u};
        const lib_u8 untouched[6] = {0xa5u, 0xa5u, 0xa5u, 0xa5u, 0xa5u, 0xa5u};
        const lib_u32 fault_address[] = {0u, 0x84u, 0x84u, 0x27feu, 0x7026u, 0x2ffeu};
        const lib_u32 fault_code[] = {0u, 5u, 4u, 7u, 0u, 6u};
        const lib_u32 guest_sp = failure == 5u ? 0x1002u : 0x800u;
        lib_u8 code[4], bytes = 0u;
        lib_u32 flags, frame[10];
        lib_u16 guest_frame[3], image;

        vme_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX, iopl,
            VCPU_EFLAGS_IF | VCPU_EFLAGS_VIF, LIB_TRUE);
        fixture.cpu.data.cs.selector = 0x1000u;
        fixture.cpu.data.cs.base = 0x10000u;
        fixture.cpu.data.ss.selector = 0x200u;
        fixture.cpu.data.ss.base = 0x2000u;
        fixture.cpu.data.esp = guest_sp;
        fixture.cpu.data.tr.base = 0x6fc0u;
        fixture.cpu.data.tr.limit = 0x1ffu;
        fixture.cpu.data.gdtr.base = 0xc000u;
        fixture.cpu.data.idtr.base = 0xc400u;
        fixture.cpu.data.cr0 |= VCPU_CR0_PG;
        fixture.cpu.data.cr3 = 0x1000u;
        /* TSS stack fields are in page 6; its I/O base and bitmap are in page 7. */
        pages[0] = failure == 2u ? 0u : (failure == 1u ? 0x3003u : 0x3007u);
        pages[2] = failure == 5u ? 0u : (failure == 3u ? 0x5005u : 0x5007u);
        pages[3] = failure == 5u ? 0x6007u : 0u;
        pages[6] = 0x9003u;
        pages[7] = failure == 4u ? 0u : 0xb003u;
        pages[8] = 0xa003u;
        pages[12] = 0xd003u;
        pages[16] = 0x4007u;
        lib_memory_copy(fixture.memory + 0x1000u, &pde, sizeof(pde));
        lib_memory_copy(fixture.memory + 0x2000u, pages, sizeof(pages));
        lib_memory_copy(fixture.memory + 0xd008u, fixture.memory + 0x208u, 16u);
        lib_memory_copy(fixture.memory + 0xd470u, gate, sizeof(gate));
        lib_memory_copy(fixture.memory + 0x9fc4u, &kernel_sp, sizeof(kernel_sp));
        lib_memory_copy(fixture.memory + 0x9fc8u, &kernel_ss, sizeof(kernel_ss));
        lib_memory_copy(fixture.memory + 0xb026u, &iomap_base, sizeof(iomap_base));
        lib_memory_copy(fixture.memory + 0x3084u, target, sizeof(target));
        lib_memory_copy(fixture.memory + 0x57fau, untouched, sizeof(untouched));
        lib_memory_copy(fixture.memory + 0x6000u, untouched, sizeof(untouched));
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0xcdu;
        code[bytes++] = 0x21u;
        lib_memory_copy(fixture.memory + 0x4000u, code, bytes);
        flags = fixture.cpu.data.eflags;
        before = fixture.cpu;
        core_machine_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.fault.valid) return LIB_FALSE;
        for (lib_u8 reg = 0u; reg < 8u; ++reg)
            if (reg != 4u && *vme_gpr(&fixture.cpu, reg) != *vme_gpr(&before, reg))
                return LIB_FALSE;
        image = (lib_u16)flags;
        if (iopl < 3u) image |= VCPU_EFLAGS_IOPL;
        if (failure == 0u) {
            lib_memory_copy(guest_frame, fixture.memory + 0x57fau, sizeof(guest_frame));
            if (fixture.delivered_exception.valid || fixture.cpu.data.eip != target[0] ||
                fixture.cpu.data.cs.selector != target[1] || fixture.cpu.data.esp != 0x7fau ||
                fixture.cpu.data.eflags != (flags &
                    ~(iopl == 3u ? VCPU_EFLAGS_IF : VCPU_EFLAGS_VIF)) ||
                guest_frame[0] != bytes || guest_frame[1] != 0x1000u || guest_frame[2] != image)
                return LIB_FALSE;
        } else {
            if (!fixture.delivered_exception.valid ||
                fixture.delivered_exception.exception_mask != VCPUINS_EXCEPT_PF ||
                fixture.delivered_exception.exception_code != fault_code[failure] ||
                fixture.delivered_exception.point.eip != 0u ||
                fixture.delivered_exception.eflags != flags ||
                fixture.cpu.data.cr2 != fault_address[failure] ||
                fixture.cpu.data.eip != 0xc300u || fixture.cpu.data.esp != 0x87d8u) {
                lib_c_fprintf(lib_c_stderr,
                    "VME paged INT: case=%u iopl=%u prefix=%u delivered=%x code=%x cr2=%x ip=%x sp=%x\n",
                    (lib_u32)failure, (lib_u32)iopl, (lib_u32)prefixes,
                    fixture.delivered_exception.exception_mask,
                    fixture.delivered_exception.exception_code, fixture.cpu.data.cr2,
                    fixture.cpu.data.eip, fixture.cpu.data.esp);
                return LIB_FALSE;
            }
            if (failure == 1u || failure == 2u) {
                lib_memory_copy(guest_frame, fixture.memory + 0x57fau, sizeof(guest_frame));
                if (guest_frame[0] != bytes || guest_frame[1] != 0x1000u ||
                    guest_frame[2] != image) return LIB_FALSE;
            } else if (lib_memory_compare(fixture.memory + 0x57fau,
                    untouched, sizeof(untouched)) != 0) return LIB_FALSE;
            if (failure == 5u) {
                lib_u16 accepted_flags;

                lib_memory_copy(&accepted_flags, fixture.memory + 0x6000u,
                    sizeof(accepted_flags));
                if (accepted_flags != image || lib_memory_compare(
                        fixture.memory + 0x6002u, untouched, 4u) != 0) return LIB_FALSE;
            }
            lib_memory_copy(frame, fixture.memory + 0xa7d8u, sizeof(frame));
            if (frame[0] != fault_code[failure] || frame[1] != 0u ||
                frame[2] != 0x1000u || frame[3] != (flags | VCPU_EFLAGS_RF) ||
                frame[4] != guest_sp || frame[5] != 0x200u ||
                frame[6] != before.data.es.selector || frame[7] != before.data.ds.selector ||
                frame[8] != before.data.fs.selector || frame[9] != before.data.gs.selector)
                return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

typedef struct vme_failure_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 fail_address;
    lib_u32 failures;
    lib_u32 stack_writes;
    lib_bool fail_write;
} vme_failure_fixture;

static lib_status vme_failure_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    vme_failure_fixture *fixture = opaque;

    if (!observe_only && !fixture->fail_write && address <= fixture->fail_address &&
        (lib_u64)address + bytes > fixture->fail_address) {
        ++fixture->failures;
        return LIB_STATUS_IO_ERROR;
    }
    return cpu_instruction_read(&fixture->instruction, address, destination, bytes,
        provenance, observe_only, reset_fetch, cache_control);
}

static lib_status vme_failure_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    vme_failure_fixture *fixture = opaque;

    if (fixture->fail_write && address <= fixture->fail_address &&
        (lib_u64)address + bytes > fixture->fail_address) {
        ++fixture->failures;
        return LIB_STATUS_IO_ERROR;
    }
    const lib_status status = cpu_instruction_write(&fixture->instruction, address, source, bytes,
        provenance, cache_control);
    if (status == LIB_STATUS_OK && address >= 0x7fau &&
        (lib_u64)address + bytes <= 0x800u) ++fixture->stack_writes;
    return status;
}

static lib_bool vme_int_provider_failures(void)
{
    const core_machine_cpu_bus_provider bus = {
        .read_memory = vme_failure_read, .write_memory = vme_failure_write,
        .interrupt_pending = cpu_instruction_interrupt_pending,
        .special_cycle = cpu_instruction_special_cycle,
        .cancel_special_cycle = cpu_instruction_cancel_special_cycle
    };
    const struct {
        lib_u32 address;
        lib_bool write;
        lib_u8 accepted;
    } failures[] = {
        {0x666u, LIB_FALSE, 0u}, {0x684u, LIB_FALSE, 0u},
        {0x7feu, LIB_TRUE, 0u}, {0x7fcu, LIB_TRUE, 1u},
        {0x7fau, LIB_TRUE, 2u}, {0x84u, LIB_FALSE, 3u}
    };

    for (lib_size failure = 0u; failure < sizeof(failures) / sizeof(failures[0]); ++failure)
    for (lib_u8 iopl = 0u; iopl < 4u; ++iopl)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        vme_failure_fixture fixture;
        t_cpu before, after;
        const lib_u16 iomap_base = 0xa0u;
        const lib_u16 target[] = {0x1234u, 0x1800u};
        lib_u16 observed[3], expected[3] = {0xa5a5u, 0xa5a5u, 0xa5a5u};
        lib_u8 code[4], bytes = 0u;

        vme_prepare(&fixture.instruction, CORE_MACHINE_CPU_PROFILE_80486SX, iopl,
            VCPU_EFLAGS_IF | VCPU_EFLAGS_VIF, LIB_TRUE);
        fixture.instruction.execution.bus = &bus;
        fixture.instruction.execution.bus_context = &fixture;
        fixture.instruction.cpu.data.tr.limit = 0x9fu;
        fixture.fail_address = failures[failure].address;
        fixture.fail_write = failures[failure].write;
        fixture.failures = 0u;
        fixture.stack_writes = 0u;
        lib_memory_copy(fixture.instruction.memory + 0x666u, &iomap_base, sizeof(iomap_base));
        lib_memory_copy(fixture.instruction.memory + 0x84u, target, sizeof(target));
        lib_memory_copy(fixture.instruction.memory + 0x7fau, expected, sizeof(expected));
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0xcdu;
        code[bytes++] = 0x21u;
        before = fixture.instruction.cpu;
        if (failures[failure].accepted >= 1u)
            expected[2] = (lib_u16)(before.data.flags | (iopl < 3u ? VCPU_EFLAGS_IOPL : 0u));
        if (failures[failure].accepted >= 2u) expected[1] = before.data.cs.selector;
        if (failures[failure].accepted >= 3u) expected[0] = bytes;
        if (cpu_instruction_run(&fixture.instruction, code, bytes, &after) != LIB_STATUS_INTERNAL_ERROR ||
            fixture.failures != 1u || fixture.stack_writes != failures[failure].accepted ||
            !fixture.instruction.fault.valid ||
            fixture.instruction.delivered_exception.valid ||
            fixture.instruction.fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture.instruction.fault.point.eip != 0u ||
            lib_memory_compare(&before, &after, sizeof(before)) != 0) return LIB_FALSE;
        lib_memory_copy(observed, fixture.instruction.memory + 0x7fau, sizeof(observed));
        if (lib_memory_compare(observed, expected, sizeof(expected)) != 0) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_int_map_guards(void)
{
    const struct {
        lib_u16 limit;
        lib_u16 base;
        lib_u8 type;
        lib_bool valid;
    } guards[] = {
        {0x66u,0xa0u,VCPU_DESC_SYS_TYPE_TSS_32_BUSY,LIB_TRUE},
        {0x83u,0xa0u,VCPU_DESC_SYS_TYPE_TSS_32_BUSY,LIB_TRUE},
        {0x9fu,0u,VCPU_DESC_SYS_TYPE_TSS_32_BUSY,LIB_TRUE},
        {0x9fu,1u,VCPU_DESC_SYS_TYPE_TSS_32_BUSY,LIB_TRUE},
        {0x9fu,0xa0u,VCPU_DESC_SYS_TYPE_TSS_32_BUSY,LIB_FALSE},
        {0x9fu,0xa0u,VCPU_DESC_SYS_TYPE_TSS_16_BUSY,LIB_TRUE},
        {0x9fu,0xa0u,VCPU_DESC_SYS_TYPE_TSS_32_AVL,LIB_TRUE}
    };
    const core_machine_cpu_bus_provider bus = {
        .read_memory = vme_failure_read, .write_memory = vme_failure_write,
        .interrupt_pending = cpu_instruction_interrupt_pending,
        .special_cycle = cpu_instruction_special_cycle,
        .cancel_special_cycle = cpu_instruction_cancel_special_cycle
    };
    const lib_u8 untouched[6] = {0xa5u,0xa5u,0xa5u,0xa5u,0xa5u,0xa5u};

    for (lib_size guard = 0u; guard < sizeof(guards) / sizeof(guards[0]); ++guard)
    for (lib_u8 iopl = 0u; iopl < 4u; ++iopl)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 denied_stack = 0u; denied_stack < 2u; ++denied_stack)
    for (lib_u8 access = 0u; access < 2u; ++access) {
        vme_failure_fixture fixture;
        t_cpu before, after;
        lib_u8 code[4], bytes = 0u;

        vme_prepare(&fixture.instruction, CORE_MACHINE_CPU_PROFILE_80486SX, iopl,
            VCPU_EFLAGS_IF | VCPU_EFLAGS_VIF, LIB_TRUE);
        fixture.instruction.execution.bus = &bus;
        fixture.instruction.execution.bus_context = &fixture;
        fixture.instruction.cpu.data.tr.limit = guards[guard].limit;
        fixture.instruction.cpu.data.tr.flagValid = guards[guard].valid;
        fixture.instruction.cpu.data.tr.sys.type = guards[guard].type;
        if (denied_stack) fixture.instruction.cpu.data.ss.limit = 0u;
        fixture.fail_address = access ? 0x84u : 0x7feu;
        fixture.fail_write = access == 0u;
        fixture.failures = 0u;
        fixture.stack_writes = 0u;
        lib_memory_copy(fixture.instruction.memory + 0x666u,
            &guards[guard].base, sizeof(guards[guard].base));
        lib_memory_copy(fixture.instruction.memory + 0x7fau, untouched, sizeof(untouched));
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0xcdu;
        code[bytes++] = 0x21u;
        if (!guards[guard].valid || guards[guard].type != VCPU_DESC_SYS_TYPE_TSS_32_BUSY) {
            before = fixture.instruction.cpu;
            if (cpu_instruction_run(&fixture.instruction, code, bytes, &after) != LIB_STATUS_OK ||
                !core_machine_cpu_is_shutdown(&fixture.instruction.execution) ||
                fixture.instruction.fault.valid || !fixture.instruction.delivered_exception.valid ||
                fixture.instruction.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
                fixture.instruction.delivered_exception.exception_code != 0u ||
                fixture.instruction.delivered_exception.point.eip != before.data.eip ||
                fixture.instruction.delivered_exception.eflags != before.data.eflags ||
                fixture.instruction.writes != 0u || fixture.failures != 0u ||
                fixture.stack_writes != 0u || lib_memory_compare(&before, &after, sizeof(before)) != 0) {
                lib_c_fprintf(lib_c_stderr,
                    "VME TSS guard: row=%u iopl=%u prefix=%u shutdown=%u stop=%u fault=%x delivered=%x writes=%u failures=%u stack=%u diff=%d\n",
                    (lib_u32)guard, (lib_u32)iopl, (lib_u32)prefixes,
                    core_machine_cpu_is_shutdown(&fixture.instruction.execution),
                    fixture.instruction.execution.stop_requested,
                    fixture.instruction.fault.exception_mask,
                    fixture.instruction.delivered_exception.exception_mask,
                    fixture.instruction.writes, fixture.failures, fixture.stack_writes,
                    lib_memory_compare(&before, &after, sizeof(before)));
                return LIB_FALSE;
            }
            continue;
        }
        if (!vme_expect_fault(&fixture.instruction, code, bytes, 13u) ||
            fixture.failures != 0u || fixture.stack_writes != 0u ||
            lib_memory_compare(fixture.instruction.memory + 0x7fau,
                untouched, sizeof(untouched)) != 0) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_iret_monitor_fault(void)
{
    for (lib_u8 iopl = 0u; iopl < 3u; ++iopl)
    for (lib_u8 enabled = 0u; enabled < 2u; ++enabled)
    for (lib_u8 virtual_flags = 0u; virtual_flags < 4u; ++virtual_flags)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 denied_stack = 0u; denied_stack < 2u; ++denied_stack) {
        cpu_instruction_fixture fixture;
        lib_u8 code[3], bytes = 0u;
        const lib_u32 flags = VCPU_EFLAGS_IF |
            ((virtual_flags & 1u) ? VCPU_EFLAGS_VIF : 0u) |
            ((virtual_flags & 2u) ? VCPU_EFLAGS_VIP : 0u);

        vme_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX, iopl,
            flags, enabled != 0u);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        if (denied_stack) fixture.cpu.data.ss.limit = 0u;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0xcfu;
        if (!vme_expect_fault(&fixture, code, bytes, 13u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_iret_protected_flags(void)
{
    for (lib_u8 cpl = 0u; cpl < 4u; ++cpl)
    for (lib_u8 old_bits = 0u; old_bits < 4u; ++old_bits)
    for (lib_u8 new_bits = 0u; new_bits < 4u; ++new_bits)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_bool wide = code32 != ((prefixes & 1u) != 0u);
        const lib_u32 old_virtual = ((old_bits & 1u) ? VCPU_EFLAGS_VIF : 0u) |
            ((old_bits & 2u) ? VCPU_EFLAGS_VIP : 0u);
        const lib_u32 new_virtual = ((new_bits & 1u) ? VCPU_EFLAGS_VIF : 0u) |
            ((new_bits & 2u) ? VCPU_EFLAGS_VIP : 0u);
        const lib_u32 flags = 2u | VCPU_EFLAGS_IOPL | new_virtual;
        const lib_u32 frame32[] = {0x123u,(lib_u32)(8u | cpl),flags};
        const lib_u16 frame16[] = {0x123u,(lib_u16)(8u | cpl),(lib_u16)flags};
        lib_u8 code[3], bytes = 0u;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        cpu_instruction_prepare_protected_exception(&fixture, cpl);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.eflags = 2u | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_IF | old_virtual;
        lib_memory_copy(fixture.memory + 0x800u,
            wide ? (const void *)frame32 : (const void *)frame16,
            wide ? sizeof(frame32) : sizeof(frame16));
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0xcfu;
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            after.data.eip != 0x123u || after.data.cs.selector != (8u | cpl) ||
            after.data.esp != 0x800u + (wide ? 12u : 6u) ||
            after.data.eflags != (2u | VCPU_EFLAGS_IOPL |
                (wide && cpl == 0u ? new_virtual : old_virtual))) {
            lib_c_printf("IRET virtual flags cpl=%u old=%u new=%u code=%u prefix=%u flags=%x ip=%x\n",
                cpl, old_bits, new_bits, code32, prefixes, after.data.eflags, after.data.eip);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool vme_iret_enters_vm86(void)
{
    for (lib_u8 old_bits = 0u; old_bits < 4u; ++old_bits)
    for (lib_u8 new_bits = 0u; new_bits < 4u; ++new_bits)
    for (lib_u8 iopl = 0u; iopl < 4u; ++iopl)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 address32 = 0u; address32 < 2u; ++address32) {
        cpu_instruction_fixture fixture;
        t_cpu before, after;
        const lib_u32 flags = 2u | VCPU_EFLAGS_VM | VCPU_EFLAGS_IF |
            ((lib_u32)iopl << 12u) |
            ((new_bits & 1u) ? VCPU_EFLAGS_VIF : 0u) |
            ((new_bits & 2u) ? VCPU_EFLAGS_VIP : 0u);
        const lib_u32 frame[] = {0x123u,0x1000u,flags,0xa5a50800u,
            0x2000u,0x3000u,0x4000u,0x5000u,0x6000u};
        lib_u8 code[3], bytes = 0u;

        cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
        cpu_instruction_prepare_protected_exception(&fixture, 0u);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.eflags |= ((old_bits & 1u) ? VCPU_EFLAGS_VIF : 0u) |
            ((old_bits & 2u) ? VCPU_EFLAGS_VIP : 0u);
        fixture.cpu.data.cr4 = VCPU_CR4_VME;
        lib_memory_copy(fixture.memory + 0x800u, frame, sizeof(frame));
        if (!code32) code[bytes++] = 0x66u;
        if (address32) code[bytes++] = 0x67u;
        code[bytes++] = 0xcfu;
        before = fixture.cpu;
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid || fixture.writes != 0u ||
            after.data.eip != frame[0] || after.data.cs.selector != frame[1] ||
            after.data.cs.base != 0x10000u || after.data.cs.dpl != 3u ||
            after.data.cs.seg.exec.defsize || after.data.eflags != flags ||
            after.data.esp != frame[3] || after.data.ss.selector != frame[4] ||
            after.data.ss.base != 0x20000u || after.data.ss.dpl != 3u ||
            after.data.ss.seg.data.big || after.data.es.selector != frame[5] ||
            after.data.ds.selector != frame[6] || after.data.fs.selector != frame[7] ||
            after.data.gs.selector != frame[8] || after.data.cr4 != before.data.cr4 ||
            lib_memory_compare(fixture.memory + 0x800u, frame, sizeof(frame)) != 0)
            return LIB_FALSE;
        for (lib_u8 reg = 0u; reg < 8u; ++reg)
            if (reg != 4u && *vme_gpr(&after, reg) != *vme_gpr(&before, reg))
                return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool vme_iret_preserves_virtual_flags(void)
{
    for (lib_u8 vm86 = 0u; vm86 < 2u; ++vm86)
    for (lib_u8 virtual_flags = 0u; virtual_flags < 4u; ++virtual_flags) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        const lib_u8 code[] = {0x66u, 0xcfu};
        const lib_u32 bits = ((virtual_flags & 1u) ? VCPU_EFLAGS_VIF : 0u) |
            ((virtual_flags & 2u) ? VCPU_EFLAGS_VIP : 0u);
        const lib_u32 frame[] = {0x123u, 0u, 0x402u | VCPU_EFLAGS_VM |
            ((VCPU_EFLAGS_VIF | VCPU_EFLAGS_VIP) ^ bits)};

        if (vm86) vme_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX,
            3u, VCPU_EFLAGS_IF | bits, LIB_TRUE);
        else {
            cpu_instruction_prepare(&fixture, CORE_MACHINE_CPU_PROFILE_80486SX);
            fixture.cpu.data.eflags = 2u | VCPU_EFLAGS_IF | bits;
        }
        fixture.cpu.data.esp = 0x800u;
        lib_memory_copy(fixture.memory + 0x800u, frame, sizeof(frame));
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            after.data.eip != 0x123u || after.data.esp != 0x80cu ||
            after.data.eflags != (0x402u | bits |
                (vm86 ? VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL : 0u)))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!vme_control()) { lib_c_printf("VME control failed\n"); return 1; }
    if (!vme_control_rejections()) { lib_c_printf("CR4 rejection failed\n"); return 1; }
    if (!vme_debug_flags()) { lib_c_printf("Debug flags normalization failed\n"); return 1; }
    if (!vme_flags()) { lib_c_printf("VME flags failed\n"); return 1; }
    if (!vme_pending_flags()) { lib_c_printf("VME pending flags failed\n"); return 1; }
    if (!vme_stack_priority()) { lib_c_printf("VME stack priority failed\n"); return 1; }
    if (!vme_int_redirect()) { lib_c_printf("INT redirection failed\n"); return 1; }
    if (!vme_int_map_edges()) { lib_c_printf("INT map edge failed\n"); return 1; }
    if (!vme_int_stack_edges()) { lib_c_printf("INT stack edge failed\n"); return 1; }
    if (!vme_int_protected_routes()) {
        lib_c_printf("INT protected routes failed\n"); return 1;
    }
    if (!vme_int_paging()) { lib_c_printf("INT paging failed\n"); return 1; }
    if (!vme_int_provider_failures()) { lib_c_printf("INT provider failure failed\n"); return 1; }
    if (!vme_int_map_guards()) { lib_c_printf("INT map guard failed\n"); return 1; }
    if (!vme_hardware_routes()) { lib_c_printf("Hardware VME route failed\n"); return 1; }
    if (!vme_iret_monitor_fault()) { lib_c_printf("IRET monitor fault failed\n"); return 1; }
    if (!vme_iret_protected_flags()) { lib_c_printf("IRET protected flags failed\n"); return 1; }
    if (!vme_iret_enters_vm86()) { lib_c_printf("IRET VM86 entry failed\n"); return 1; }
    if (!vme_iret_preserves_virtual_flags()) {
        lib_c_printf("IRET virtual flags preservation failed\n"); return 1;
    }
    lib_c_printf("CPU VME control and flags: OK\n");
    return 0;
}
